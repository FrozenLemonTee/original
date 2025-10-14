# debug/printers/core/blocksList_printer.py


import gdb
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from utils import *


class Printer:
    """Pretty printer for original::blocksList"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        size = int(call(self.val, "size"))
        capacity = int(call(self.val["map"], "size")) * 16
        return f"original::blocksList(size={size}, cap={capacity}, {addr_str(self.val)})"

    def children(self):
        size = int(call(self.val, "size"))
        yield "size", size
        for i in range(0, size):
            v = call(self.val, "operator[]", f"{i}")
            if v.type.code == gdb.TYPE_CODE_REF:
                v = v.referenced_value()
            yield f"[{i}]", v

        yield "first", self.val["first_"]
        yield "last", self.val["last_"]
        yield "first block", self.val["first_block"]
        yield "last block", self.val["last_block"]

    def display_hint(self):
        return "array"