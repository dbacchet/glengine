import sys
import tatsu

type_grammar = r"""
    @@grammar::Calc
    start = types $ ;
    types = [(struct|enum) NEWLINE]*
    struct = "struct"? typename ":" NEWLINE struct_members
    struct_members = [member]*
    member = "-"? typename name
    enum = "enum" typename ":" NEWLINE enum_values
    enum_values = [enum_value]*
    enum_value = "-"? name
    typename = _namespace CNAME
    _namespace = (CNAME ".")*
    name = CNAME
    number = SIGNED_NUMBER

%import common.CNAME
%import common.SIGNED_NUMBER
%import common.NEWLINE
%import common.WS
%import common.SH_COMMENT
%ignore WS
%ignore SH_COMMENT
"""

if __name__ == "__main__":
    with open(sys.argv[1]) as f:
        txt = f.read()
        ast = tatsu.parse(type_grammar, txt)
        print(ast)
