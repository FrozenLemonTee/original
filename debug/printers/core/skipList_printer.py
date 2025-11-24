# debug/printers/core/skipList_printer.py

import gdb
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from utils import *


class PrinterBase:
    """Pretty printer for derived classes of skipList in original"""

    def __init__(self, val):
        self.val = val
        self.name = "original::skipList"

    def class_name(self):
        return self.name

    def to_string(self):
        size = int(call(self.val, "size"))
        levels = int(call(self.val, "getCurLevels"))
        return f"{self.class_name()}(size={size}, levels={levels}, {addr_str(self.val)})"

    def elem(self):
        size = int(call(self.val, "size"))
        if size == 0:
            return
        cur = self.val["head_"]
        first = True
        while cur != 0:
            node = cur.dereference()
            cp = node["data_"]
            if first:
                first = False
            else:
                yield cp["first_"], cp["second_"]
            next_lst = node["next_"]
            cur = call(next_lst, "get", "0")
