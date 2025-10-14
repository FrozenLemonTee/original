# debug/printers/core/chain_printer.py


import gdb
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from utils import *


class Printer:
    """Pretty printer for original::chain"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        size = int(call(self.val, "size"))
        return f"original::chain(size={size}, {addr_str(self.val)})"

    def children(self):
        size = int(call(self.val, "size"))
        yield "size", size
        p = self.val["begin_"]
        for i in range(0, size):
            node = p.dereference()
            yield f"[{i}]", node["data_"]
            p = node["next"]
        yield "begin", self.val["begin_"]
        yield "end", self.val["end_"]