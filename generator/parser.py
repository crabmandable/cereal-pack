import toml
from . import schema
import re
from .errors import CrunchyBytesException
from .property_types import length_length
from .validation import *
from .globals import Globals

def load_toml(file_path):
    try:
        return toml.load(file_path)
    except toml.decoder.TomlDecodeError as e:
        raise CrunchyBytesException(file_path, 'TOML decode error: {}'.format(str(e)))

def parse_globals(file_path):
    raw = load_toml(file_path)

    if 'lengths' in raw:
        if not isinstance(raw['lengths'], dict):
            raise CrunchyBytesException(file_path, 'expected "lengths" to be a dictionary')
        for name, val in raw['lengths'].items():
            validate_uint32(file_path, val, 'length ' + name)

    if 'enums' in raw:
        validate_enums(file_path, raw['enums'])

    if 'max_crunchy_bytes_serial_length' in raw:
        validate_uint32(file_path, raw['max_crunchy_bytes_serial_length'], '"max_crunchy_bytes_serial_length"')

    return Globals(raw)

def parse_schema(file_path, globals=None):
    raw = load_toml(file_path)

    if 'name' not in raw:
        raise CrunchyBytesException(file_path, 'top level field "name" not found')

    if not isinstance(raw['name'], str):
        raise CrunchyBytesException(file_path, 'top level field "name" is not a string')

    if 'props' not in raw:
        raise CrunchyBytesException(file_path, 'top level field "props" not found')

    if not isinstance(raw['props'], dict):
        raise CrunchyBytesException(file_path, 'top level field "props" is not a dictionary')

    if 'namespace' in raw and not isinstance(raw['namespace'], str):
        raise CrunchyBytesException(file_path, 'top level field "namespace" is not a string')

    if 'order' in raw and not isinstance(raw['order'], list):
        raise CrunchyBytesException(file_path, 'top level field "order" is not an array')

    name = raw['name']
    props = raw['props']

    namespace = None
    if 'namespace' in raw:
        namespace = raw['namespace']

    order = []
    if 'order' in raw:
        order = raw['order']
        for o in order:
            if not isinstance(o, str):
                raise CrunchyBytesException(file_path, 'top level property "order" should be an array of property names')


    # validate enums
    enums = {}
    if 'enums' in raw:
        enums = raw['enums']
        validate_enums(file_path, enums)

    return schema.Schema(file_path, raw['name'], enums, raw['props'], namespace, order, globals)

def load_schemas(files, globals_file=None):
    schemas = {}

    globals = parse_globals(globals_file)
    for file in files:
        s = parse_schema(file, globals)
        if s.name_with_namespace in schemas:
            raise CrunchyBytesException(s.file_path, 'Schema has same name as "{}"'.format(schemas[s.name_with_namespace].file_path))
        schemas[s.name_with_namespace] = s

    # resolve references
    for schema in schemas.values():
        for ref_name in schema.references:
            if ref_name not in schemas:
                raise CrunchyBytesException(schema.file_path, 'unable to resolve reference to "{}"'.format(ref_name))

            reference = schemas[ref_name]

            for name, prop in schema.props.items():
                if prop.reference == ref_name:
                    prop.reference = reference


    # recursive function used to resolve max_length of references
    def resolve_max_length(schema):
        for _, prop in schema.props.items():
            if prop.reference is not None:
                if prop.reference.max_length() is None:
                    len = resolve_max_length(prop.reference)
                else:
                    len = prop.reference.max_length()

                if prop.type == 'set':
                    prop.max_length = length_length + (prop.max_items * len)
                else:
                    prop.max_length = len

        return schema.max_length()

    # update max_length of references
    for name, schema in schemas.items():
        try:
            resolve_max_length(schema)
        except RecursionError:
            raise CrunchyBytesException(schema.file_path, 'unable to resolve references due to circular reference')

        if globals.max_crunchy_bytes_serial_length:
            if schema.max_length() > globals.max_crunchy_bytes_serial_length:
                raise CrunchyBytesException(
                    schema.file_path,
                    'schema exceeds max length defined in "{}": {} > {}'.format(globals_file, schema.max_length(), globals.max_crunchy_bytes_serial_length)
                )

    return schemas, globals

if __name__ == '__main__':
    import sys
    schemas = load_schemas(sys.argv[1:])

    for _, s in schemas.items():
        print(s)
