import sys

import yaml
from mako.exceptions import RichTraceback
from mako.template import Template

def _read_yaml_file(path):
    with open(path) as f:
        return yaml.safe_load(f)

class CppTypeData():
    def __init__(self, name, cpptype, include_fname):
        self.name = name
        self.type = cpptype
        self.include = include_fname
    def __str__(self):
        return "{} | {} | {}".format(self.name, self.type, self.include)

def parse_external_types(idl_filename):
    idl = _read_yaml_file(idl_filename)
    print(idl)
    external_types = {}
    for typename, cpptype in idl['external_types'].items():
        incname = None
        if typename in idl['external_types_includes']:
            incname = idl['external_types_includes'][typename]
        external_types[typename] = CppTypeData(typename, cpptype, incname)
    return external_types

def generate_cpp_types(idl_filename, external_types):
    idl = _read_yaml_file(idl_filename)
    # build a map with type and generated filename
    typemap = {}
    # external types
    for tname, tdata in external_types.items():
        typemap[tdata.name] = tdata.include
    # custom types
    def get_filename(typename):
        return "{}.h".format(typename)
    for type_str, fields in idl.items():
        typename = type_str.split()[-1]
        typemap[type_str] = get_filename(typename);
        print(typename, fields)
    print(typemap)


if __name__ == "__main__":
    external_types = parse_external_types('external_types.yml')
    # for a,b in external_types.items():
    #     print(a,b)
    generate_cpp_types(sys.argv[1], external_types)
