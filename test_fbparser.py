import sys
from lark import Lark, Visitor

fb_type_grammar = r"""
?start : include* ( namespace_decl | type_decl | enum_decl )+

include : "include" STRING_CONSTANT ";"

namespace_decl : "namespace" IDENT ( "." IDENT )* ";"

type_decl : "struct" type_name "{" type_fields "}" 
enum_decl : "enum" type_name "{" enum_fields "}"

type_fields : field_decl+
field_decl : type_name ":" type type_init ";"
enum_fields : enumval_decl+
enumval_decl : type_name enum_init ","

type_name : IDENT
type : BASE_TYPE_NAME | (IDENT ".")* IDENT
type_init : ("=" TYPE_INIT_VALUE)?
enum_init : ("=" SIGNED_NUMBER)?

BASE_TYPE_NAME : "bool" | "i8" | "u8" | "i16" | "u16" | "i32" | "u32" | "i64" | "u64" | "f32" | "f64" | "string"
TYPE_INIT_VALUE : /[^;]+/

%import common.CNAME -> IDENT
%import common.ESCAPED_STRING -> STRING_CONSTANT
%import common.SIGNED_NUMBER
// ignored terminals
%import common.NEWLINE
%import common.WS
%import common.CPP_COMMENT
%import common.SH_COMMENT
%ignore NEWLINE
%ignore WS
%ignore CPP_COMMENT
%ignore SH_COMMENT

"""

class EnumTypeValue:
    def __init__(self, name_, init_=""):
        self.name = name_
        self.init = init_

class EnumType:
    def __init__(self, name):
        self.name = name
        self.values = []

    def __str__(self):
        mm = ""
        for m in self.members:
            mm = mm + "({} - {} - {})".format(m.name, m.type, m.init)
        return "struct {}: [{}]".format(self.name, mm)

class StructTypeMember:
    def __init__(self, name_, type_, init_=""):
        self.name = name_
        self.type = type_
        self.init = init_

class StructType:
    def __init__(self, name):
        self.name = name
        self.members = []

    def __str__(self):
        mm = ""
        for m in self.members:
            mm = mm + "({} - {} - {})".format(m.name, m.type, m.init)
        return "struct {}: [{}]".format(self.name, mm)


class TypeVisitor(Visitor):
    def __init__(self):
        self.types = {}
        self.types['namespace'] = []
        self.types['enums'] = []
        self.types['structs'] = []
        pass

    def get_fqn(self, val):
        return ".".join(val.children)

    def namespace_decl(self, args):
        for cc in args.children:
            self.types['namespace'].append(cc)

    def type_decl(self, args):
        # a type decl has 2 children: type_name, type_fields
        type_name = args.children[0]
        type_fields = args.children[1]
        # print("\nstruct {}".format(self.get_fqn(type_name)))
        s = StructType(self.get_fqn(type_name))
        for c in type_fields.children:
            # self.print_field_decl(c)
            s.members.append(self.get_struct_member(c))
        self.types['structs'].append(s)

    def enum_decl(self, args):
        # an enum decl has 2 children: type_name, enum_fields
        enum_name = args.children[0]
        enum_fields = args.children[1]
        # print("\nenum {}".format(self.get_fqn(enum_name)))
        e = EnumType(self.get_fqn(enum_name))
        for c in enum_fields.children:
            # self.print_enumval_decl(c)
            e.values.append(self.get_enum_value(c))
        self.types['enums'].append(e)

    def get_enum_value(self, args):
        # a field has 3 children: type_name, name and type_init
        enum_value_name = args.children[0]
        enum_value_init = args.children[1]
        return EnumTypeValue( self.get_fqn(enum_value_name), self.get_fqn(enum_value_init) )

    def print_enumval_decl(self, args):
        # an enumval has 2 children: type_name and enum_init
        type_name = args.children[0]
        enum_init = args.children[1]
        print("  value : {}".format(self.get_fqn(type_name)))
        print("    init : {}".format(self.get_fqn(enum_init)))

    def get_struct_member(self, args):
        # a field has 3 children: type_name, name and type_init
        name = args.children[0]
        type_name = args.children[1]
        type_init = args.children[2]
        return StructTypeMember( self.get_fqn(name), self.get_fqn(type_name), self.get_fqn(type_init) )

    def print_field_decl(self, args):
        # a field has 3 children: type_name, name and type_init
        name = args.children[0]
        type_name = args.children[1]
        type_init = args.children[2]
        print("  member : {}".format(self.get_fqn(name)))
        print("    type : {}".format(self.get_fqn(type_name)))
        print("    init : {}".format(self.get_fqn(type_init)))

def parse_types(filename):
    with open(filename) as f:
        txt = f.read()
        type_parser = Lark(fb_type_grammar, parser="lalr")#, transformer=MyTransformer())
        # type_parser = Lark(fb_type_grammar)
        ast = type_parser.parse(txt)
        print(ast.pretty())
        visitor = TypeVisitor()
        visitor.visit_topdown(ast)
        print(visitor.types)
        for t in visitor.types['structs']:
            print(t)
        return visitor.types
    return None

if __name__ == "__main__":
    parse_types(sys.argv[1])
