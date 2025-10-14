# debug/printers/core/treeMap_printer.py

import gdb
from RBTree_printer import PrinterBase as Base
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from utils import *


class Printer(Base):
    """Pretty printer for original::treeMap"""

    def __init__(self, val):
        super().__init__(val)

    def class_name(self):
        return "original::treeMap"

    def children(self):
        for i, cp in enumerate(self.elem()):
            yield f"[{i}].key", cp[0]
            yield f"[{i}].value", cp[1]
        if gdb.parameter('print pretty') > 0:
            addr_root = addr_str(self.val["root_"])
            addr_cmp = addr_str(self.val["compare_"])
            yield "root", f"{addr_root}"
            yield "compare", f"{addr_cmp}"

    def display_hint(self):
        return "map"