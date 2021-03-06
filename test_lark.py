import sys
from lark import Lark, Visitor, Transformer

type_grammar = r"""
    ?start : types
    types : [(struct|enum) NEWLINE]*
    struct : "struct"? typename ":" NEWLINE struct_members
    struct_members : [member]*
    member : "-"? typename name
    enum : "enum" typename ":" NEWLINE enum_values
    enum_values : [enum_value]*
    enum_value : "-"? name
    typename : _namespace CNAME
    _namespace : (CNAME ".")*
    name : CNAME
    number : SIGNED_NUMBER

%import common.CNAME
%import common.SIGNED_NUMBER
%import common.NEWLINE
%import common.WS
%import common.SH_COMMENT
%ignore WS
%ignore SH_COMMENT
"""

basic_tokenizer = Lark(r"""
    tokens : [token NEWLINE]*
    token : identifier ":" NEWLINE [ "-" identifier*]*
    namespace : CNAME "."
    identifier : (namespace)* CNAME

%import common.CNAME
%import common.NEWLINE
%import common.WS
%import common.SH_COMMENT
%ignore WS
%ignore SH_COMMENT
""", start = "tokens")

json_parser = Lark(r"""
    ?start: value
    ?value: object
          | array
          | string
          | SIGNED_NUMBER      -> number
          | "true"             -> true
          | "false"            -> false
          | "null"             -> null
    array  : "[" [value ("," value)*] "]"
    object : "{" [pair ("," pair)*] "}"
    pair   : string ":" value
    string : ESCAPED_STRING
    %import common.ESCAPED_STRING
    %import common.SIGNED_NUMBER
    %import common.WS
    %ignore WS
""")

class MyVisitor(Visitor):
    def get_fqn(self, args):
        return ".".join([t.value for t in args.children])

    def member(self, args):
        print(args)
        data = args.find_data("typename")
    def typename(self, args):
        print("typename: {}".format(self.get_fqn(args)))
    def name(self, args):
        print("name: {}".format(self.get_fqn(args)))
    # def name(self, args):
    #     print(args.data)
    #     print(args.children)
    #     t = args.children[0]
    #     print("{} {}".format(t.type, t.value))

class MyTransformer(Transformer):
    def get_fqn(self, args):
        return ".".join([t.value for t in args.children])

    def member(self, args):
        print(args)
        data = args.find_data("typename")
    def typename(self, args):
        return self.get_fqn(args)
    def name(self, args):
        return self.get_fqn(args)
    # def name(self, args):
    #     print(args.data)
    #     print(args.children)
    #     t = args.children[0]
    #     print("{} {}".format(t.type, t.value))

if __name__ == "__main__":
    with open(sys.argv[1]) as f:
        txt = f.read()
        # print(txt)
        # ast = basic_tokenizer.parse("A B CCC")
        # ast = basic_tokenizer.parse(txt)
        # ast = json_parser.parse('[{"a": "b"}, [1,-2.6,3,"bbb",true] ]')
        type_parser = Lark(type_grammar)#, parser='lalr', transformer=MyTransformer())
        ast = type_parser.parse(txt)
        print(ast.pretty())
        # print(ast)
        # for c in ast.children:
        #     print(c)
        visitor = MyVisitor()
        visitor.visit(ast)
