import os

from prophyc import model
from prophyc.model_process import StructKind, ProcessedNodes

primitive_types = {
    'u8': 'uint8_t',
    'u16': 'uint16_t',
    'u32': 'uint32_t',
    'u64': 'uint64_t',
    'i8': 'int8_t',
    'i16': 'int16_t',
    'i32': 'int32_t',
    'i64': 'int64_t',
    'r32': 'float',
    'r64': 'double',
    'byte': 'uint8_t',
}

def _indent(string_, spaces):
    indentation = spaces * ' '
    return '\n'.join(indentation + x if x else x for x in string_.split('\n'))

def _generate_def_include(pnodes, include):
    return '#include "{}.pp.hpp"'.format(include.name)

def _generate_def_constant(pnodes, constant):
    return 'enum {{ {} = {} }};'.format(constant.name, constant.value)

def _generate_def_typedef(pnodes, typedef):
    tp = primitive_types.get(typedef.type, typedef.type)
    return 'typedef {} {};'.format(tp, typedef.name)

def _generate_def_enum(pnodes, enum):
    members = ',\n'.join('{} = {}'.format(name, value)
                         for name, value in enum.members)
    return 'enum {}\n{{\n{}\n}};'.format(enum.name, _indent(members, 4))

def _generate_def_struct(pnodes, struct):
    def gen_member(member):
        def build_annotation(member):
            if member.array_size:
                if member.array_bound:
                    annotation = 'limited array, size in {}'.format(member.array_bound)
                else:
                    annotation = ''
            else:
                if member.array_bound:
                    annotation = 'dynamic array, size in {}'.format(member.array_bound)
                else:
                    annotation = 'greedy array'
            return annotation

        typename = primitive_types.get(member.type, member.type)
        if member.array:
            annotation = build_annotation(member)
            size = member.array_size or 1
            if annotation:
                field = '{0} {1}[{2}]; /// {3}\n'.format(typename, member.name, size, annotation)
            else:
                field = '{0} {1}[{2}];\n'.format(typename, member.name, size)
        else:
            field = '{0} {1};\n'.format(typename, member.name)
        if member.optional:
            return 'prophy::bool_t has_{0};\n'.format(member.name) + field
        return field

    def gen_part(i, part):
        generated = 'struct part{0}\n{{\n{1}}} _{0};'.format(
            i + 2,
            _indent(''.join(map(gen_member, part)), 4)
        )
        return _indent(generated, 4)

    main, parts = pnodes.partition(struct.members)
    generated = _indent(''.join(map(gen_member, main)), 4)
    if parts:
        generated += '\n' + '\n\n'.join(map(gen_part, range(len(parts)), parts)) + '\n'

    return 'struct {}\n{{\n{}}};'.format(struct.name, generated)

def _generate_def_union(pnodes, union):
    def gen_member(member):
        typename = primitive_types.get(member.type, member.type)
        return '{0} {1};\n'.format(typename, member.name)

    enum_fields = ',\n'.join('discriminator_{0} = {1}'.format(mem.name,
                                                              mem.discriminator)
                             for mem in union.members)
    union_fields = ''.join(map(gen_member, union.members))
    enum_def = 'enum _discriminator\n{{\n{0}\n}} discriminator;'.format(_indent(enum_fields, 4))
    union_def = 'union\n{{\n{0}}};'.format(_indent(union_fields, 4))
    return 'struct {0}\n{{\n{1}\n\n{2}\n}};'.format(union.name,
                                                    _indent(enum_def, 4),
                                                    _indent(union_def, 4))

_generate_def_visitor = {
    model.Include: _generate_def_include,
    model.Constant: _generate_def_constant,
    model.Typedef: _generate_def_typedef,
    model.Enum: _generate_def_enum,
    model.Struct: _generate_def_struct,
    model.Union: _generate_def_union
}

def _generator_def(nodes):
    pnodes = ProcessedNodes(nodes)
    last_node = None
    for node in nodes:
        prepend_newline = bool(last_node
                               and (isinstance(last_node, (model.Enum, model.Struct, model.Union))
                                    or type(last_node) is not type(node)))
        yield prepend_newline * '\n' + _generate_def_visitor[type(node)](pnodes, node) + '\n'
        last_node = node

def _generate_swap_enum(pnodes, enum):
    return ('template <>\n'
            'inline {0}* swap<{0}>({0}* payload)\n'
            '{{\n'
            '    swap(reinterpret_cast<uint32_t*>(payload));\n'
            '    return payload + 1;\n'
            '}}\n').format(enum.name)

def _generate_swap_struct(pnodes, struct):
    def gen_member(member, delimiters = []):
        if member.array:
            is_dynamic = pnodes.get_kind(member) == StructKind.DYNAMIC
            swap_mode = 'dynamic' if is_dynamic else 'fixed'
            if member.array_bound:
                bound = member.array_bound
                if member.array_bound not in delimiters:
                    bound = 'payload->' + bound
                return 'swap_n_{0}(payload->{1}, {2})'.format(
                    swap_mode, member.name, bound)
            elif not member.array_bound and member.array_size:
                return 'swap_n_{0}(payload->{1}, {2})'.format(
                    swap_mode, member.name, member.array_size)
        else:
            if member.optional:
                preamble = 'swap(&payload->has_{0});\nif (payload->has_{0}) '.format(member.name)
            else:
                preamble = ''
            return preamble + 'swap(&payload->{0})'.format(member.name)

    def gen_last_member(name, last_mem, delimiters = []):
        if pnodes.is_unlimited(last_mem):
            return 'return cast<{0}*>({1}payload->{2});\n'.format(
                name,
                '' if last_mem.array else '&',
                last_mem.name
            )
        elif pnodes.is_dynamic(last_mem):
            return 'return cast<{0}*>({1});\n'.format(
                name,
                gen_member(last_mem, delimiters)
            )
        else:
            return gen_member(last_mem) + ';\n' + 'return payload + 1;\n'

    def gen_main(main, parts):
        all_names = {mem.name: 'payload' for mem in main}
        for i, part in enumerate(parts):
            all_names.update({mem.name: 'part{0}'.format(i + 2) for mem in part})

        def get_missing(part_number):
            part = parts[part_number]
            names = [mem.name for mem in part]
            return [(all_names[mem.array_bound], mem.array_bound)
                    for mem in part
                    if mem.array_bound and mem.array_bound not in names]

        def gen_missing(part_number):
            return ''.join(', {0}->{1}'.format(x[0], x[1]) for x in get_missing(part_number))

        members = ''.join(gen_member(mem) + ';\n' for mem in main[:-1])
        if parts:
            for i, part in enumerate(parts):
                members += '{0}::part{1}* part{1} = cast<{0}::part{1}*>({2});\n'.format(
                    struct.name,
                    i + 2,
                    'swap(part{0}{1})'.format(i + 1, gen_missing(i - 1)) if i else gen_member(main[-1])
                )
            members += 'return cast<{0}*>(swap(part{1}{2}));\n'.format(struct.name, i + 2, gen_missing(i))
        elif main:
            members += gen_last_member(struct.name, main[-1])
        return ('template <>\n'
                'inline {0}* swap<{0}>({0}* payload)\n'
                '{{\n'
                '{1}'
                '}}\n').format(struct.name, _indent(members, 4))

    def gen_part(part_number, part):
        names = [mem.name for mem in part]
        delimiters = [mem.array_bound for mem in part if mem.array_bound and mem.array_bound not in names]
        members = ''.join(gen_member(mem, delimiters) + ';\n' for mem in part[:-1])
        members += gen_last_member(struct.name + '::part{0}'.format(part_number), part[-1], delimiters)
        return ('inline {0}::part{1}* swap({0}::part{1}* payload{3})\n'
                '{{\n'
                '{2}}}\n').format(struct.name,
                                  part_number,
                                  _indent(members, 4),
                                  ''.join(', size_t {0}'.format(x) for x in delimiters))

    main, parts = pnodes.partition(struct.members)
    return '\n'.join([gen_part(i + 2, part) for i, part in enumerate(parts)] +
                     [gen_main(main, parts)])

def _generate_swap_union(pnodes, union):
    return 'not implemented'

_generate_swap_visitor = {
    model.Enum: _generate_swap_enum,
    model.Struct: _generate_swap_struct,
    model.Union: _generate_swap_union
}

def _generator_swap(nodes):
    pnodes = ProcessedNodes(nodes)
    for node in nodes:
        fun = _generate_swap_visitor.get(type(node))
        if fun:
            yield fun(pnodes, node)

header = """\
#ifndef _PROPHY_GENERATED_{0}_HPP
#define _PROPHY_GENERATED_{0}_HPP

#include <prophy/prophy.hpp>
"""

footer = """\
#endif  /* _PROPHY_GENERATED_{0}_HPP */
"""

class CppGenerator(object):

    def __init__(self, output_dir = "."):
        self.output_dir = output_dir

    def generate_definitions(self, nodes):
        return ''.join(_generator_def(nodes))

    def generate_swap(self, nodes):
        return '\n'.join(_generator_swap(nodes))

    def serialize_string(self, nodes, basename):
        return '\n'.join((
            header.format(basename),
            self.generate_definitions(nodes),
            footer.format(basename)
        ))

    def serialize(self, nodes, basename):
        path = os.path.join(self.output_dir, basename + ".pp.hpp")
        out = self.serialize_string(nodes, basename)
        open(path, "w").write(out)
