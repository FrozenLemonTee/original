# debug/printers/core/hashTable_printer.py

import gdb
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from utils import *


class PrinterBase:
    """Pretty printer for derived classes of hashTable in original"""

    def __init__(self, val):
        self.val = val

    def class_name(self):
        return "original::hashTable"

    def to_string(self):
        size = int(call(self.val, "size"))
        return f"{self.class_name()}(size={size}, {addr_str(self.val)})"

    def elem(self):
        size = int(call(self.val, "size"))
        if size == 0:
            return
        cnt = 0
        buckets = self.val['buckets']
        length = int(call(buckets, "size"))
        for i in range(length):
            p = call(buckets, "operator[]", f"{i}")
            while p != 0:
                node = p.dereference()
                cp = node["data_"]
                yield cp["first_"], cp["second_"]
                cnt += 1
                if cnt == size:
                    return
                p = node["next_"]
