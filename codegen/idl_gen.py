#!/usr/bin/env python3

"""
The process:
- read type mappings
- read all idl files
- analyze whether everything can be generated
- generate C++ component code
- generate C++ boilerplate component serialization code
- generate proto files

TODOs:
[x] Generate the ECS code for gathering all the component data for serializing the entire ECS

Nice to haves:
[ ] probably need to create a `tags` structure on types as some types have multiple "traits". E.g.,
    the ECS_ROOT is also really a COMPONENT, and perhaps an EXTERNALLY_DEFINED_TYPE can also be an
    ENUM.

"""

import abc
import argparse
import os
import re
from enum import Enum
from glob import glob
from hashlib import sha256
from pathlib import Path

import colored_traceback
import yaml
from mako.exceptions import RichTraceback
from mako.template import Template

from idl_gen_utils import MetaType, SerializationMode, PRIMITIVE_TYPES

colored_traceback.add_hook()


class Color:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    ENDC = '\033[0m'


class LanguageType(Enum):
    SOURCE_IDL = 1
    CPP = 2
    PROTO = 3


# "Bring into namespace"
SOURCE_IDL = LanguageType.SOURCE_IDL
CPP = LanguageType.CPP
PROTO = LanguageType.PROTO


class Model:
    """Defines the entire code generation model"""

    def __init__(self):
        """Contains definitions for all the types. If it's not here, it doesn't exist."""
        self.types = {}

        """Contains information on how the types map between different domains.

        That is, types might be represented differently (presence / absence of members, namespace,
        snake / camelcase, ...) in different contexts or languages. This dictionary maps the a type
        definition in `types` to these mapped types.
        """
        self.type_mappings = {}

        """Where the generated code goes."""
        self.output_dir = None

        """Where the templates are located."""
        self.templates_dir = None

        """This is really for Bazel, but affects the generated protos.

        This defines the import path for the generated components, that is,
        the other generated types will be imported from this directory.
        """
        self.generated_proto_import_dir = None

        """Output (root) directory of the code generation"""
        self.out_dir = None

        """External proto bazel targets.

        Defines external targets that we depend on when building protocol buffers.
        """
        self.external_proto_bazel_targets = None

        """External C++ targets.

        Defines which C++ targets may be needed when referencing custom types.
        """
        self.external_cc_bazel_targets = None


class Member:
    """Stores information on a member variable."""

    def __init__(self, name, typ=None, is_vec=None, is_serialized=None, value=None, metatype=None):
        self.name = name
        self.type = typ
        self.is_vec = is_vec
        self.is_serialized = is_serialized
        self.value = value
        self.metatype = metatype

    def __repr__(self):
        vec = '[]' if self.is_vec else ''
        ser = '' if self.is_serialized else ' ($)'
        return f'<Member "{self.name}{vec}{ser}">'


def make_fqn(namespace_list, name, sep='.'):
    return sep.join(namespace_list) + sep + name


class IType(abc.ABC):
    def namespace_str(self):
        assert self.namespace_sep != ''
        return self.namespace_sep.join(self.namespace)


class SourceType(IType):
    """Stores information on a (generated) type."""

    def __init__(self, metatype, namespace, name, serialization_mode, members=None,
                 holds_references_instead_of_copies=False):
        if members is None:
            members = []
        self.language = LanguageType.SOURCE_IDL
        self.metatype = metatype
        self.namespace = namespace
        self.name = name
        self.members = members
        self.model = None
        self.derived_types = {
            CPP: None,
            PROTO: None,
        }
        self.serialization_mode = serialization_mode
        # This is a key we'll enforce to be unique. We'll use it for serialization, but there may be
        # other cases where it comes in handy.
        self.hash_value = None
        # For serialization purposes, we create data structures that hold references instead of
        # copies of data structures.
        self.holds_references_instead_of_copies = holds_references_instead_of_copies

        # Below, I'm going to store information that relates to a particular implementation of the
        # type. I'm not yet sure if this is a good idea. Maybe I want a subclass of this class to
        # handle that stuff. I'm going to assume that that's a premature approach right now. We can
        # always change it later on.
        #
        # Alternatively, we could define a "realm" that can be passed as an argument. So in the
        # protobuf realm, we return the protobuf namespace

    def proto_import_path(self):
        """Import path for this type's protobuf definition."""
        return f'{self.model.generated_proto_import_dir}/{self.name}.proto'

    def cpp_import_path(self):
        """Import path for this type's C++ definition."""
        return f'generated_types/{self.name}.hpp'

    def cpp_namespace(self):
        """The namespace for this type in a protobuf definition."""
        return self.namespace

    def proto_namespace(self):
        """The namespace for this type in a protobuf definition."""
        return ['prns'] + self.namespace

    def proto_fqn(self):
        return make_fqn(self.proto_namespace(), self.name, '.')

    def cpp_fqn(self):
        return make_fqn(self.cpp_namespace(), self.name, '::')

    def __repr__(self):
        return f'<ST {self.fully_qualified_name()}>'

    def fully_qualified_name(self):
        ret = '.'.join(self.namespace)
        if ret != '':
            ret += '.'
        ret += self.name
        return ret


class DerivedType(IType):
    def __init__(self, source_type, *args, **kwargs):
        # super().__init__(args, kwargs)
        self.source_type = source_type
        self._name = None
        self._namespace = None
        self._members = []
        self._language = None

    ### FIXME remove these properties once refactoring is complete.

    @property
    def metatype(self):
        return self.source_type.metatype

    @property
    def namespace(self):
        return self._namespace if self._namespace is not None else self.source_type.namespace

    @property
    def name(self):
        return self._name if self._name is not None else self.source_type.name

    @property
    def members(self):
        # FIXME the members may have a type specified in type_mappings, e.g. uint64 -> uint64_t,
        # which we are not currently accounting for.
        return self._members
        # return self.source_type.members

    @property
    def fullname(self):
        if len(self.namespace) > 0:
            return self.namespace_str() + self.namespace_sep + self.name
        else:
            return self.name

    def __repr__(self):
        return f'<{str(self._language)} {self.fullname}>'


class TypeCpp(DerivedType):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.namespace_sep = '::'
        self._language = LanguageType.CPP
        self.include_path = None

    @property
    def includes(self):
        ret = set()
        for m in self.members:
            if m.metatype != MetaType.ENUM_VALUE and m.type.include_path is not None:
                ret.add(m.type.include_path)
        return sorted(ret)


class TypeProto(DerivedType):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.namespace_sep = '.'
        self.import_path = None
        self._language = LanguageType.PROTO

    @property
    def namespace(self):
        """Prefix the namespace to ensure there are no namespace collisions with the C++ types."""
        return self._namespace \
            if self._namespace is not None else self.source_type.namespace + ['prns']

    @property
    def imports(self):
        ret = set()
        for m in self.members:
            if m.is_serialized and \
                    m.metatype != MetaType.ENUM_VALUE and \
                    m.type.import_path is not None:
                ret.add(m.type.import_path)
        return sorted(ret)


class TypeIdentifierInfo:
    def __init__(self, identifier_str):
        self.is_serialized = None
        self.meta_type = None
        self.name = None
        self.traits_str = None

        self._parse(identifier_str)

    def _parse(self, istr):
        pattern = r'^(\$)?\s*(component|enum)?\s*([^\s\$]+)\s*(\(.*\))?'
        matches = re.match(pattern, istr)
        self.is_serialized = True if not matches[1] else False
        self.meta_type = {
            'enum': MetaType.ENUM,
            'component': MetaType.COMPONENT,
            'root': MetaType.COMPONENT_ROOT,
            'struct': MetaType.STRUCT,
        }.get(matches[2], MetaType.STRUCT)
        self.name = matches[3]
        self.traits_str = matches[4]


def _parse_idl_file(path: Path, file_contents: dict, model) -> list:
    """Returns a list of Type"""
    ret = []
    namespace = os.path.split(path)[0].split('/')

    def make_enum(ident, member_str_list):
        ex = r'([a-zA-Z_][a-zA-Z_0-9]*) = (\d+)'
        members = []
        for member_str in member_str_list:
            m = re.match(ex, member_str)
            m_name, m_value = m.groups()
            members.append(Member(name=m_name, value=m_value, metatype=MetaType.ENUM_VALUE))
        return SourceType(ident.meta_type, namespace, ident.name, None, members)

    def make_struct(ident, member_str_list):
        members = []
        for member_str in member_str_list:
            # Members that shouldn't be serialized are prefixed with a '$' sign and a space.
            is_serialized = member_str[0] != '$'
            if not is_serialized:
                assert member_str[1] == ' '
                member_str = member_str[2:]
            member_type_and_name = re.split(r'\s+', member_str)
            m_type, m_name = member_type_and_name[0:2]
            if len(member_type_and_name) == 2:
                m_value = None
            else:
                assert member_type_and_name[2] == '=', \
                    f'Syntax is <type> <name> = <default value>, not {member_type_and_name}'
                m_value = re.split(r'=\s+', member_str)[1]

            is_vec = False
            if m_type[-2:] == '[]':
                m_type = m_type[:-2]
                is_vec = True
            if m_type not in PRIMITIVE_TYPES and '.' not in m_type:
                # Add the namespace of the package. The only types we allow in the IDL without a
                # namespace are global types and types that are declared in the same package (or a
                # parent package), in which case we add the namespace here.
                def find_referenced_type(name, this_namespace, known_types):
                    # Keep popping the namespace until we find the type
                    namespace = this_namespace[:]
                    while True:
                        fqn = make_fqn(namespace, name)
                        if fqn in known_types:
                            return fqn
                        else:
                            if len(namespace) > 0:
                                namespace.pop()
                            else:
                                # We are now in the global namespace, and the type was not found.
                                assert False, (
                                    f'No reference for type {m_type} found in namespace '
                                    f'{".".join(this_namespace)} or enclosing namespaces.')

                m_type = find_referenced_type(m_type, namespace, model.types.keys())
                assert m_type is not None

            members.append(Member(m_name, model.types[m_type], is_vec, is_serialized, m_value))

        # We currently don't use traits for anything else but we'll probably need to do some more
        # proper parsing here at some point in the future.
        if ident.is_serialized:
            if ident.traits_str is not None and 'once' in ident.traits_str:
                ser_mode = SerializationMode.SERIALIZE_ONCE
            else:
                ser_mode = SerializationMode.SERIALIZE
        else:
            ser_mode = SerializationMode.DO_NOT_SERIALIZE

        return SourceType(ident.meta_type, namespace, ident.name, ser_mode, members)

    def make_type(identifier_str, member_str_list):
        ident = TypeIdentifierInfo(identifier_str)

        fun = {
            MetaType.ENUM: make_enum,
            MetaType.COMPONENT: make_struct,
            MetaType.COMPONENT_ROOT: make_struct,
            MetaType.STRUCT: make_struct,
        }[ident.meta_type]

        if ident.meta_type is MetaType.COMPONENT:
            # The only way in which components are special is in that they have an entity id field
            member_str_list.insert(0, 'uint64 _entity_id')
            # And they have an id used for visualization
            member_str_list.insert(0, '$ uint64 _webviz_id')
        elif ident.meta_type is MetaType.COMPONENT_ROOT:
            # The root type is currently not special, but we might want to add additional
            # functionality later on
            pass
        return fun(ident, member_str_list)

    for identifier_str, member_str_list in file_contents.items():
        ret.append(make_type(identifier_str, member_str_list))

    return ret


def _read_type_mappings(path):
    with open(path) as f:
        return yaml.safe_load(f)


def _parse_idls(path, model):
    files = glob(str(path) + '/**/*.yml', recursive=True)
    files = [Path(os.path.relpath(p, path)) for p in files]

    oldcwd = os.getcwd()
    os.chdir(path)

    def get_contents(path):
        with open(path) as f:
            return yaml.safe_load(f)

    files = list(zip(files, [get_contents(file) for file in files]))

    # Pass 1: Add all the type definitions to our model
    for file_path, file_contents in files:
        namespace = os.path.dirname(file_path).split(os.sep)
        # print('processing namespace ', namespace)
        for key in file_contents.keys():
            ident = TypeIdentifierInfo(key)
            fqn = make_fqn(namespace, ident.name)
            assert fqn not in model.types, f'Found {fqn} in model types while adding its ' \
                                           'definition to the model. Is this type defined in our ' \
                                           'external type mappings?'
            model.types[fqn] = SourceType(
                MetaType.IDL_DEFINED_TYPE_PLACEHOLDER, namespace, ident.name, None)

    #
    # At this point, all the types that we may find while parsing the entire IDL (that is, member
    # variables), should be of a type that already exists in the model. If it does not, it is an
    # error.
    #

    for file_path, file_contents in files:
        # One IDL file may define more types
        types_in_file = _parse_idl_file(file_path, file_contents, model)
        for t in types_in_file:
            assert t.fully_qualified_name() in model.types
            #
            # NOTE: member-wise assignment here so we don't actually overwrite the previously
            # defined type. There are other types that references this particular object.
            #
            target_t = model.types[t.fully_qualified_name()]
            target_t.language = t.language
            target_t.metatype = t.metatype
            target_t.namespace = t.namespace
            target_t.name = t.name
            target_t.members = t.members
            target_t.model = t.model
            target_t.derived_types = t.derived_types
            target_t.serialization_mode = t.serialization_mode

    os.chdir(oldcwd)
    return


def dump_file(path, string):
    with open(path, 'w') as f:
        f.write(string)


def generate_types(path, lang, model):
    if lang is LanguageType.CPP:
        extension = '.hpp'
        mako_tplt = model.templates_dir / 'cpp/component.hpp.mako'
    elif lang is LanguageType.PROTO:
        extension = '.proto'
        mako_tplt = model.templates_dir / 'proto/component.proto.mako'
    else:
        assert False

    ctx = argparse.Namespace()  # just a container, not a namespace in this context
    for _fqn, t in model.types.items():
        if t.metatype == MetaType.EXTERNALLY_DEFINED_TYPE:
            continue  # don't generate code for externally defined types
        ctx.msg = t.derived_types[lang]
        if ctx.msg is None:
            assert lang is PROTO, 'Only protos can be optional'
            continue

        # set() is not ordered, change to list for deterministic code generation
        ctx.namespace = ctx.msg.namespace_str()
        dump_file(path / Path(t.name + extension), render_mako_template(mako_tplt, ctx))


def generate_protos(path, model):
    generate_types(path, LanguageType.PROTO, model)


def generate_cpp_types(path, model):
    generate_types(path, LanguageType.CPP, model)


def generate_cpp_serialization(path, model):
    ctx = argparse.Namespace()
    for _fqn, t in model.types.items():
        if t.metatype == MetaType.ENUM:
            continue  # don't serialize enums
        if t.metatype == MetaType.EXTERNALLY_DEFINED_TYPE:
            continue  # don't serialize externally defined types; they have their own serializers
        ctx.imports = set()
        ctx.imports.add('generated_types/' + t.name + '.hpp')
        ctx.imports.add('component_generation/generated/proto/' + t.name + '.pb.h')
        # Add imports for the serializers of the types we depend on
        for m in t.members:
            # 1) the type is a global type, e.g. double, and doesn't need importing
            # 2) the type is a generated enum, and doesn't need importing (we just cast)
            # 3) the type is in imports and needs to be imported
            if (m.type.fully_qualified_name() not in PRIMITIVE_TYPES and
                    m.type.metatype != MetaType.EXTERNALLY_DEFINED_TYPE and
                    m.type.fully_qualified_name() in model.types and
                    m.type.metatype != MetaType.ENUM):
                type_name = m.type.name
                ctx.imports.add('serializers/ser_' + type_name + '.hpp')

        # set() is not ordered, change to list for deterministic code generation
        ctx.imports = sorted(ctx.imports)
        ctx.namespace = '.'.join(t.namespace)
        ctx.msg = t
        ctx.proto_msg = t.derived_types[PROTO]
        ctx.cc_msg = t.derived_types[CPP]
        s = render_mako_template(model.templates_dir / 'cpp/serializer.hpp.mako', ctx)
        dump_file(path / Path('ser_' + t.name + '.hpp'), s)


def generate_cpp_python_bindings(path, model):
    ctx = argparse.Namespace()
    for _fqn, t in model.types.items():
        if t.metatype == MetaType.ENUM:
            pass
        elif t.metatype == MetaType.EXTERNALLY_DEFINED_TYPE:
            continue
        else:
            ctx.imports = set()
            ctx.imports.add('generated_types/' + t.name + '.hpp')
            # Add imports for the serializers of the types we depend on
            for m in t.members:
                # 1) the type is a global type, e.g. double, and doesn't need importing
                # 2) the type is in imports and needs to be imported
                if m.type.metatype != MetaType.EXTERNALLY_DEFINED_TYPE:
                    type_name = m.type.name
                    ctx.imports.add('python_bindings/bind_' + type_name + '.hpp')

        # set() is not ordered, change to list for deterministic code generation
        ctx.imports = sorted(ctx.imports)
        ctx.namespace = '.'.join(t.namespace)
        ctx.primitive_types = PRIMITIVE_TYPES
        ctx.msg = t
        ctx.proto_msg = t.derived_types[PROTO]
        ctx.cc_msg = t.derived_types[CPP]
        s = render_mako_template(model.templates_dir / 'cpp/python_binding.hpp.mako', ctx)
        dump_file(path / Path('bind_' + t.name + '.hpp'), s)


def render_mako_template(template_dir, context):
    try:
        mtpl = Template(filename=str(template_dir), strict_undefined=True)
        print(context)
        # print(**context.__dict__)
        return mtpl.render(**context.__dict__)
    except Exception:
        print(Color.FAIL + 'Error occurred rendering mako template:' + Color.ENDC)
        print('Here is the context:')
        print(context)
        print(Color.FAIL + 'Trace below:' + Color.ENDC)
        print(Color.FAIL + 'vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv' + Color.ENDC)
        traceback = RichTraceback()
        for (filename, lineno, function, line) in traceback.traceback:
            print("File %s, line %s, in %s" % (filename, lineno, function))
            print(f'    {line}')
        print(
            Color.FAIL + Color.BOLD + f'{str(traceback.error.__class__.__name__)}: '
                                      f'{traceback.error}' + Color.ENDC)
        print(Color.FAIL + '^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^' + Color.ENDC)
        # raise
        exit(1)


def generate_proto_build_file(out_dir, model):
    template_path = model.templates_dir / 'proto/bazel_build_proto.mako'
    out_path = out_dir / 'BUILD'
    ctx = argparse.Namespace()
    ctx.external_proto_bazel_targets = model.external_proto_bazel_targets
    ctx.msgs = []
    for _fqn, msg in model.types.items():  # these are all the messages we are building protos for
        if msg.metatype == MetaType.EXTERNALLY_DEFINED_TYPE:
            continue
        ns = argparse.Namespace()
        ns.name = msg.name
        ns.src = msg.name + '.proto'  # BUILD file is in same folder, so this is the relative path
        # Created a sorted set. Sorted for determinism, set because a type may have multiple members
        # of the same type.
        if msg.metatype == MetaType.ENUM:
            ns.dependent_types = set()
        else:
            ns.dependent_types = sorted(set(
                [member.type.name for member in msg.members
                 if member.type.metatype != MetaType.EXTERNALLY_DEFINED_TYPE and
                 member.type.fully_qualified_name() not in PRIMITIVE_TYPES]))
        ctx.msgs.append(ns)

    dump_file(out_path, render_mako_template(template_path, ctx))


def generate_cpp_build_file(out_dir, model):
    template_path = model.templates_dir / 'cpp/bazel_build_types.mako'
    out_path = out_dir / 'BUILD'
    ctx = argparse.Namespace()
    ctx.type_deps = model.external_cc_bazel_targets

    dump_file(out_path, render_mako_template(template_path, ctx))


def generate_serialization_build_file(out_dir, model):
    template_path = model.templates_dir / 'cpp/bazel_build_serializers.mako'
    out_path = out_dir / 'BUILD'
    ctx = argparse.Namespace()

    dump_file(out_path, render_mako_template(template_path, ctx))


def generate_python_bindings_build_file(out_dir, model):
    template_path = model.templates_dir / 'cpp/bazel_build_python_bindings.mako'
    out_path = out_dir / 'BUILD'
    ctx = argparse.Namespace()

    dump_file(out_path, render_mako_template(template_path, ctx))


def generate_root_serializer(out_dir, model):
    template_path = model.templates_dir / 'cpp/root_serializer.mako'
    out_path = out_dir / 'root_serializer.hpp'
    ctx = argparse.Namespace()
    ctx.model = model
    root_types = [t for t in model.types.values() if t.metatype is MetaType.COMPONENT_ROOT]
    assert len(root_types) == 1, f'Multiple root types found: {[t.name for t in root_types]}'
    ctx.msg = root_types[0]
    ctx.cc_msg = ctx.msg.derived_types[LanguageType.CPP]
    ctx.proto_msg = ctx.msg.derived_types[LanguageType.PROTO]

    dump_file(out_path, render_mako_template(template_path, ctx))


def generate_root_serializer_build_file(out_dir, model):
    template_path = model.templates_dir / 'cpp/bazel_build_root_serializer.mako'
    out_path = out_dir / 'BUILD'
    ctx = argparse.Namespace()
    ctx.model = model

    dump_file(out_path, render_mako_template(template_path, ctx))


def get_namespace_and_name(fqn_str, sep='.'):
    tmp = fqn_str.split(sep)
    return tmp[:-1], tmp[-1]


def build_derived_type(model, t, built_types, lang):
    include_path = None
    import_path = None
    if lang == LanguageType.CPP:
        sep = '::'
        T = TypeCpp
        include_path = f'generated_types/{t.name}.hpp'
    elif lang == LanguageType.PROTO:
        sep = '.'
        T = TypeProto
        import_path = f'component_generation/generated/proto/{t.name}.proto'
    else:
        assert False

    assert isinstance(t, SourceType)
    if built_types[t.fully_qualified_name()]:
        pass
    elif t.metatype == MetaType.EXTERNALLY_DEFINED_TYPE:
        pass
    else:
        assert t.derived_types[lang] is None, 'already defined'

        t.derived_types[lang] = T(source_type=t)
        # I don't like doing this here...
        if lang == LanguageType.CPP:
            t.derived_types[lang].include_path = include_path
        elif lang == LanguageType.PROTO:
            t.derived_types[lang].import_path = import_path
        else:
            assert False

        # Create member variables for derived types
        def make_derived_members(lang):
            ret = []
            for m_t in t.members:
                if m_t.metatype == MetaType.ENUM_VALUE:
                    mapped_type = m_t.type
                else:
                    # The dependent type wasn't processed yet so take care of that one first
                    # Recursive because I'm too lazy to build a toposort
                    mapped_type = \
                        build_derived_type(model, m_t.type, built_types, lang).derived_types[lang]
                    assert mapped_type is not None or (not m_t.is_serialized and lang == PROTO)
                ret.append(Member(m_t.name, typ=mapped_type, is_vec=m_t.is_vec,
                                  is_serialized=m_t.is_serialized, value=m_t.value,
                                  metatype=m_t.metatype))
            return ret

        t.derived_types[lang]._members = make_derived_members(lang)

    built_types[t.fully_qualified_name()] = True
    return t


def build_derived_types(model, lang):
    """Create derived types, e.g. for C++ and Protobuf generation.

    Assign them to the type definitions.
    """
    # Keep track of which types we've processed (we can't simply walk the list b/c dependencies)
    built_types = dict(zip(model.types.keys(), [False] * len(model.types.keys())))

    for t in model.types.values():
        # Currently, we always create a type definition for C++ and protos, but this may change in
        # the future. If so, we probably need to change that here.
        build_derived_type(model, t, built_types, lang)

    for k, v in built_types.items():
        assert v is True, f'Unhandled type {k} (shouldn\'t be possible)'


def make_component_hash(typ):
    """Make a hash value that can be used to uniquely identify components.

    This is used for assigning stable ids to protobuf members (in the root serializer), so we can
    only possibly introduce breaking changes in individual components, but not in the main ECS data
    structure. 2047 is the maximum number that fits inside two bytes in the protobuf format. See
    https://developers.google.com/protocol-buffers/docs/proto3.
    """
    # Note: hash() is not stable between invocations of the interpreter!
    return int(sha256(typ.fully_qualified_name().encode('utf-8')).hexdigest(), 16) % 2047


def generate(args):
    """Generate code for types in C++ as well as protobuf.

    Features:
    The main goal is to take in YAML files that define types for the simulator. The only reason YAML
    was used is so we don't have to come up with an IDL and corresponding parser, while still being
    very easy to write. These types look e.g. like this:

    ```
    Transform:
    - voy.sim.Entity parent
    - voy.sim.Entity[] children
    - voy.sim.Timestamp ts
    - Pose pose

    Pose:
    - Vec3 vec
    - Quat quat
    ```
    The rules are simple:
    - Type definitions in here create C++ and protobuf types (and corresponding C++
      implementations).
    - Member variables are either single types or vector types. Vectors have a `[]` indicator.
    - Any type used in here that does not have a namespace (e.g. voy.sim in this example) needs to
      be defined as a type as well. So "Transform" requires "Pose" to be defined, just like "Pose"
      requires "Vec3" and "Quat" to be defined.
    - The folder structure the YAML files are located in define their namespace
    - Types may optionally be referenced with their fully qualified name. So "Pose" in this example
      may, inside "Transform", still be referenced as "voy.sim.component.Pose" if the YAML excerpt
      above were located in voy/sim/component.
    - Any type used in here that is defined with a namespace we'll try to match against 1) other
      generated types and 2) types defined in type_mappings.yml, in that order. This file contains
      further information on e.g., where to find the file, what its corresponding proto type should
      be, etcetera.
    """
    type_mappings_yml = _read_type_mappings(Path(args.aux_dir / 'type_mappings.yml'))

    model = Model()
    model.output_dir = args.out_dir
    model.templates_dir = args.template_dir
    model.type_mappings = type_mappings_yml['type_mappings']
    model.generated_proto_import_dir = type_mappings_yml['generated_proto_import_dir']
    model.external_cc_bazel_targets = type_mappings_yml['external_cc_bazel_targets']
    model.external_proto_bazel_targets = type_mappings_yml['external_proto_bazel_targets']

    #
    # Some types are not defined in the IDL. These should exist in our type mappings file.
    # Grab all these externally defined types and add a definition for them in our model
    #
    for mt, (t_cpp, t_proto) in model.type_mappings.items():
        ns, name = get_namespace_and_name(mt)
        st = SourceType(MetaType.EXTERNALLY_DEFINED_TYPE, ns, name, [])
        # Add derived types because we have "complete" (as complete as we'll get) knowledge of them
        st.derived_types[CPP] = TypeCpp(st)
        st.derived_types[CPP]._namespace, st.derived_types[CPP]._name = get_namespace_and_name(
            t_cpp, '::')
        st.derived_types[CPP].include_path = type_mappings_yml['cc_type_includes'].get(t_cpp, None)
        # mapped proto types are optional
        if t_proto is not None:
            # Type is not mapped (implication: the type cannot be serialized)
            st.derived_types[PROTO] = TypeProto(st)
            st.derived_types[PROTO]._namespace, st.derived_types[
                PROTO]._name = get_namespace_and_name(t_proto, '.')
            st.derived_types[PROTO].import_path = type_mappings_yml['proto_type_imports'].get(
                t_proto, None)

        model.types[mt] = st

    #
    # Parse the IDL files to obtain all type information
    #
    _parse_idls(args.idl_dir, model)

    #
    # Enforce some assumptions on the type system here
    #
    for t in model.types.values():
        assert t.metatype != MetaType.IDL_DEFINED_TYPE_PLACEHOLDER
        for m in t.members:
            if m.type is None:
                assert m.metatype is MetaType.ENUM_VALUE
            else:
                assert m.type.metatype != MetaType.IDL_DEFINED_TYPE_PLACEHOLDER

    #
    # For serialization purposes, our types (at least our components, or things will break
    # every.single.time, need a field ID that remains stable across versions. In order to automate
    # this, we'll take the name and hash it.
    # Note: If we want to also have stable ids for members inside components, we should also do
    #       this here.
    # Note: If we ever want to really stabilize things, we may have to add fixed IDs in the IDL.
    #       This will be a compatibility-breaking change.
    # Note: Changing the hash function will also be a compatibility-breaking change.
    #
    component_filter = lambda t: t.metatype == MetaType.COMPONENT
    for t in filter(component_filter, model.types.values()):
        t.hash_value = make_component_hash(t)

    all_hash_values = [t.hash_value for t in filter(component_filter, model.types.values())]
    assert len(all_hash_values) == len(set(all_hash_values)), \
        f'Colliding hash values ({len(all_hash_values) - len(set(all_hash_values))}) detected. ' \
        f'Unfortunately, we will need to do something more clever.'

    #
    # One last type we create is a special "component": it is the root component structure.
    #
    # Its purpose is to serve as a type such that, when we serialize it, we have all the component
    # state in one place.
    #
    # Note that compilation might fail when you try to serialize a component with custom members,
    # because the C++ type might not be copy-constructible.
    #
    components = filter(
        lambda t: t.metatype == MetaType.COMPONENT and \
                  t.serialization_mode != SerializationMode.DO_NOT_SERIALIZE,
        model.types.values())
    members = [Member(t.name, t, is_vec=True, is_serialized=True) for t in components]
    root_type = SourceType(MetaType.COMPONENT_ROOT, ['sim'], 'EcsRoot',
                           SerializationMode.SERIALIZE, members=members,
                           holds_references_instead_of_copies=True)
    model.types['sim.EcsRoot'] = root_type


    # ----------------------------------------------------------------------------------------------
    # At this point, every type that can possibly be referenced should exist.
    # ----------------------------------------------------------------------------------------------

    # Build the derived types. Any rules for custom imports and all that good stuff will happen
    # here.
    build_derived_types(model, CPP)
    build_derived_types(model, PROTO)

    for t in model.types.values():
        for m in t.members:
            if t.metatype == MetaType.ENUM:
                assert m.metatype == MetaType.ENUM_VALUE

    # At this point, every type should be defined in type_mappings, and if any type needs to be
    # imported (only exceptions I can think of are primitive types, e.g. double), it should have
    # been included now in proto_imports and cc_includes.

    #
    # Step 1: generate the proto and hpp files
    #
    Path(args.out_dir / 'proto').mkdir(parents=True, exist_ok=True)
    Path(args.out_dir / 'cpp').mkdir(parents=True, exist_ok=True)
    generate_protos(args.out_dir / 'proto', model=model)
    generate_cpp_types(args.out_dir / 'cpp', model=model)

    #
    # Step 2: generate serialization code
    #
    Path(args.out_dir / 'cpp/serializers').mkdir(parents=True, exist_ok=True)
    generate_cpp_serialization(args.out_dir / 'cpp/serializers', model=model)

    #
    # Step 3: generate python bindings
    #
    Path(args.out_dir / 'cpp/python_bindings').mkdir(parents=True, exist_ok=True)
    generate_cpp_python_bindings(args.out_dir / 'cpp/python_bindings', model=model)

    #
    # Generate BUILD file for .proto files. The output of this will be a C++ and Python library
    #
    generate_proto_build_file(
        out_dir=args.out_dir / 'proto',
        model=model,
    )

    #
    # Generate BUILD file for the C++ types
    #
    generate_cpp_build_file(
        out_dir=args.out_dir / 'cpp',
        model=model,
    )

    #
    # Generate BUILD file for serialization of C++ types into protobufs
    #
    generate_serialization_build_file(
        out_dir=args.out_dir / 'cpp/serializers',
        model=model,
    )

    #
    # Generate BUILD file for python bindings of C++ types
    #
    generate_python_bindings_build_file(
        out_dir=args.out_dir / 'cpp/python_bindings',
        model=model,
    )

    #
    # Generate the serializer that we'll use for logging (serializes all the components)
    #
    Path(args.out_dir / 'cpp/root_serializer').mkdir(parents=True, exist_ok=True)
    generate_root_serializer(
        out_dir=args.out_dir / 'cpp/root_serializer',
        model=model,
    )
    # Generate the corresponding build file
    Path(args.out_dir / 'cpp/root_serializer').mkdir(parents=True, exist_ok=True)
    generate_root_serializer_build_file(
        out_dir=args.out_dir / 'cpp/root_serializer',
        model=model,
    )


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--idl-dir', default='idl')
    p.add_argument('--aux-dir', default='aux')
    p.add_argument('--out-dir', default='generated')
    p.add_argument('--template-dir', default='templates')
    args = p.parse_args()

    # Turn path components into objects so they're easier to manipulate
    def make_path(x):
        setattr(args, x, Path(getattr(args, x)))

    [make_path(key) for key in ['idl_dir', 'aux_dir', 'out_dir', 'template_dir']]

    # Generate the code
    generate(args)


if __name__ == "__main__":
    main()
