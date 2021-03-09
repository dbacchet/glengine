#!/usr/bin/env python3

import sys
from mako.template import Template
from mako.exceptions import RichTraceback

from test_fbparser import parse_types

# from idl_gen_utils import MetaType, SerializationMode, PRIMITIVE_TYPES

# colored_traceback.add_hook()


def dump_file(path, string):
    with open(path, 'w') as f:
        f.write(string)


def render_mako_template(template, context):
    try:
        mtpl = Template(filename=str(template), strict_undefined=True)
        print(context)
        # print(**context.__dict__)
        # return mtpl.render(**context.__dict__)
        return mtpl.render(**context)
    except Exception:
        print("error rendering mako template")
        traceback = RichTraceback()
        for (filename, lineno, function, line) in traceback.traceback:
            print("File %s, line %s, in %s" % (filename, lineno, function))
            print(f'    {line}')
        print(
            f'{str(traceback.error.__class__.__name__)}: '
                                      f'{traceback.error}')
        exit(1)


if __name__ == "__main__":
    template_name = sys.argv[1]
    types_file_name = sys.argv[2]
    types = parse_types(types_file_name)
    print(types)
    code = render_mako_template(template_name, types)
    print(code)
