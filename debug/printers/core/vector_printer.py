# debug/printers/core/vector_printer.py


import gdb
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from utils import *


class Printer:
    """Pretty printer for original::vector"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        size = int(call(self.val, "size"))
        capacity = self.val["max_size"]
        return f"original::vector(size={size}, cap={capacity}, {addr_str(self.val)})"

    def children(self):
        size = int(call(self.val, "size"))
        for i in range(0, size):
            v = call(self.val, "operator[]", f"{i}")
            if v.type.code == gdb.TYPE_CODE_REF:
                v = v.referenced_value()
            yield f"[{i}]", v
        if gdb.parameter('print pretty') > 0:
            yield "max size", self.val["max_size"]
            yield "inner begin", self.val["inner_begin"]

    def display_hint(self):
        return "array"