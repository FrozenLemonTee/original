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
        self.display_mode = "array"

    def to_string(self):
        size = int(call(self.val, "size"))
        cnt = int(call(self.val, "count"))
        return f"original::bitset(count={cnt}, cap={size}, {addr_str(self.val)})"

    def children(self):
        size = int(call(self.val, "size"))
        for i in range(0, size):
            yield f"[{i}]", call(self.val, "get", f"{i}")
        if gdb.parameter('print pretty') > 0:
            yield "map", self.val["map"]

    def display_hint(self):
        return self.display_mode