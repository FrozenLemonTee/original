# debug/printers/core/treeSet_printer.py

import gdb
from RBTree_printer import PrinterBase as Base
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from utils import *


class Printer(Base):
    """Pretty printer for original::treeSet"""

    def __init__(self, val):
        super().__init__(val)

    def class_name(self):
        return "original::treeSet"

    def children(self):
        for i, cp in enumerate(self.elem()):
            yield f"[{i}]", cp[0]
        if gdb.parameter('print pretty') > 0:
            addr_root = addr_str(self.val["root_"])
            addr_cmp = addr_str(self.val["compare_"])
            yield "root", f"{addr_root}"
            yield "compare", f"{addr_cmp}"

    def display_hint(self):
        return "array"