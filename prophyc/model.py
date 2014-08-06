from collections import namedtuple

class Kind:
    FIXED = 0
    DYNAMIC = 1
    UNLIMITED = 2

""" Model consists of 5 kinds of symbols: Includes, Constants, Enums, Structs, Unions. """

Include = namedtuple("Include", ["name"])

Constant = namedtuple("Constant", ["name", "value"])

Enum = namedtuple("Enum", ["name", "members"])
EnumMember = namedtuple("EnumMember", ["name", "value"])

class Typedef(object):

    def __init__(self, name, type):
        self.name = name
        self.type = type

    def __cmp__(self, other):
        return cmp(other.__dict__, self.__dict__)

    def __repr__(self):
        return '{0} {1}'.format(self.type, self.name)

class Struct(object):

    def __init__(self, name, members):
        self.name = name
        self.members = members

    def __cmp__(self, other):
        return cmp(other.__dict__, self.__dict__)

    def __repr__(self):
        return self.name + ''.join(('\n    {}'.format(x) for x in self.members)) + '\n'

class StructMember(object):

    def __init__(self, name, type,
                 bound = None, size = None,
                 unlimited = False, optional = False):
        assert(sum((bool(bound or size), unlimited, optional)) <= 1)

        self.name = name
        self.type = type
        self.array = bool(bound or size or unlimited)
        self.bound = bound
        self.size = size
        self.optional = optional

    def __cmp__(self, other):
        return cmp(other.__dict__, self.__dict__)

    def __repr__(self):
        fmts = {
            (False, False, False, False): '{0} {1}',
            (True, False, True, False): '{0} {1}[{3}]',
            (True, True, False, False): '{0} {1}<>({2})',
            (True, True, True, False): '{0} {1}<{3}>({2})',
            (True, False, False, False): '{0} {1}<...>',
            (False, False, False, True): '{0}* {1}'
        }
        fmt = fmts[(self.array, bool(self.bound), bool(self.size), self.optional)]
        return fmt.format(self.type, self.name, self.bound, self.size)

    @property
    def dynamic(self):
        return self.bound and not self.size

    @property
    def greedy(self):
        return self.array and not self.bound and not self.size

Union = namedtuple("Union", ["name", "members"])
UnionMember = namedtuple("UnionMember", ["name", "type", "discriminator"])

""" Following functions process model. """

def cross_reference(nodes):
    """Adds definition reference to Typedef and StructMember."""
    types = {node.name: node for node in nodes}
    def do_cross_reference(symbol):
        symbol.definition = types.get(symbol.type)
    for node in nodes:
        if isinstance(node, Typedef):
            do_cross_reference(node)
        elif isinstance(node, Struct):
            map(do_cross_reference, node.members)

def evaluate_kinds(nodes):
    """Adds kind to Struct and StructMember. Requires cross referenced nodes."""
    def lookup_node_kind(node):
        if isinstance(node, Typedef):
            while isinstance(node, Typedef):
                node = node.definition
            return lookup_node_kind(node)
        elif isinstance(node, Struct):
            return node.kind
        else:
            return Kind.FIXED
    def evaluate_member_kind(member):
        if member.definition:
            return lookup_node_kind(member.definition)
        else:
            return Kind.FIXED
    def evaluate_struct_kind(node):
        if node.members:
            if node.members[-1].greedy:
                return Kind.UNLIMITED
            elif any(x.dynamic for x in node.members):
                return  Kind.DYNAMIC
            else:
                return  max(x.kind for x in node.members)
        else:
            return Kind.FIXED
    for node in nodes:
        if isinstance(node, Struct):
            for member in node.members:
                member.kind = evaluate_member_kind(member)
            node.kind = evaluate_struct_kind(node)

def partition(members):
    main = []
    parts = []
    current = main
    for member in members[:-1]:
        current.append(member)
        if member.kind == Kind.DYNAMIC or member.dynamic:
            current = []
            parts.append(current)
    if members:
        current.append(members[-1])
    return main, parts
