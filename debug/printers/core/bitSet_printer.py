# debug/printers/core/bitSet_printer.py


import gdb
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from utils import *


class Printer:
    """Pretty printer for original::bitset"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return f"original::bitset(@{address(self.val):#x})"

    def children(self):
        size = int(call(self.val, "size"))
        yield "size", size
        for i in range(0, size):
            yield f"[{i}]", call(self.val, "get", f"{i}")

    def display_hint(self):
        return "array"