import sys

import yaml
from mako.exceptions import RichTraceback
from mako.template import Template

def _read_yaml_file(path):
    with open(path) as f:
        return yaml.safe_load(f)

class EntryData():
    def __init__(self, idl_type, cpp_type, include_fname, name=None):
        self.idl_type = idl_type
        self.cpp_type = cpp_type
        self.cpp_include = include_fname
        self.name = name
    def __str__(self):
        return "{} | {} | {} | {}".format(self.idl_type, self.cpp_type, self.cpp_include, self.name)

# class TypeEntry:
#     def __init__(self, typedata, name):
#         self.typedata = typedata
#         self.name = name

class Type:
    def __init__(self, name):
        self.name = name
        self.fields = []

    def parse_fields(self, field_entries, type)
        for e in field_entries:
            idl_type = e.split()[0]
            field_name = e.split()[1]
            pass

        
def parse_external_types(idl_filename):
    idl = _read_yaml_file(idl_filename)
    # print(idl)
    external_types = {}
    for typename, cpp_type in idl['external_types'].items():
        incname = None
        if typename in idl['external_types_includes']:
            incname = idl['external_types_includes'][typename]
        external_types[typename] = EntryData(typename, cpp_type, incname)
    return external_types

def generate_cpp_types(idl_filename, external_types):
    idl = _read_yaml_file(idl_filename)
    # build a map with type and generated filename
    typemap = {}
    # external types
    for tname, tdata in external_types.items():
        typemap[tdata.idl_type] = tdata.cpp_include
    # custom types
    def get_filename(typename):
        return "{}.h".format(typename)
    for type_str, fields in idl.items():
        typename = type_str.split()[-1]
        typemap[type_str] = get_filename(typename);
        # print(typename, fields)
    print(typemap)
    # at this point all the types we process should be in the map.
    # If they are not, it means we are missing a dependency
    for typename, fields in idl.items():
        if typename not in typemap.keys():
            exit("Error: {} is not a known type".format(typename))
        for f in fields:
            typename = f.split()[0]
            if typename not in typemap.keys():
                exit("Error: {} is not a known type".format(typename))
    # now we should have only known types. Build a container
    for typename, fields in idl.items():
        print("generating {}...".format(typename))





if __name__ == "__main__":
    external_types = parse_external_types('external_types.yml')
    # for a,b in external_types.items():
    #     print(a,b)
    generate_cpp_types(sys.argv[1], external_types)
