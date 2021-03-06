import sys
from lark import Lark, Visitor

type_grammar = r"""
    ?start : types
    types : [(struct|enum) NEWLINE]*

    struct : "struct"? typename ":" NEWLINE struct_members
    struct_members : [member]*
    member : "-"? (typename|typename_vector|typename_array) _name

    enum : "enum" typename ":" NEWLINE enum_values
    enum_values : [enum_value]*
    enum_value : "-"? _name

    typename : _namespace _name
    typename_array : _namespace _name "[" SIGNED_NUMBER "]"
    typename_vector : _namespace _name "[" "]"
    _namespace : (_name ".")*
    _name : CNAME

%import common.CNAME
%import common.SIGNED_NUMBER
%import common.NEWLINE
%import common.WS
%import common.SH_COMMENT
%ignore WS
%ignore SH_COMMENT
"""

class MyVisitor(Visitor):
    def get_fqn(self, args):
        # print(args.data)
        if (args.data.endswith("_array")):
            print("array!!!")
        if (args.data.endswith("_vector")):
            print("vector!!!")
        return ".".join([t.value for t in args.children])

    def struct(self, args):
        struct_name = self.get_fqn(args.children[0])
        print("struct: {}".format(struct_name))
        struct_members = args.children[2]
        for m in struct_members.children:
            typename = self.get_fqn(m.children[0])
            name = m.children[1]
            print("  member: {} : {}".format(typename,name))

    # def member(self, args):
    #     typename = self.get_fqn(args.children[0])
    #     name = args.children[1]
    #     print("  member: {} : {}".format(typename,name))

    def enum(self, args):
        enum_name = self.get_fqn(args.children[0])
        print("enum: {}".format(enum_name))
        # print(args.children[1])
        # print(args.children[2])
        for v in args.children[2].children:
            print("  value: {}".format(self.get_fqn(v)))

    # def enum_value(self, args):
    #     print(args)
    #     typename = self.get_fqn(args)
    #     print("  value: {}".format(typename))
    #     # value = self.get

    # def typename(self, args):
    #     print("typename: {}".format(self.get_fqn(args)))
    # def name(self, args):
    #     print("name: {}".format(self.get_fqn(args)))

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
        visitor.visit_topdown(ast)
