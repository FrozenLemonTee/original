# debug/printers/core/tuple_printer.py

import gdb
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from utils import *


class Printer:
    """Pretty printer for original::tuple"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return f"original::tuple(@{address(self.val):#x})"

    def children(self):
        elems = self.val['elems']
        elems_list = []
        current = elems
        index = 0
        while True:
            try:
                elem = current['cur_elem']
                elems_list.append(elem)
                current = current['next']
                index += 1
            except gdb.error:
                break

        yield "size", index
        for i, e in enumerate(elems_list):
            yield f"[{i}]", e

    def display_hint(self):
        return None