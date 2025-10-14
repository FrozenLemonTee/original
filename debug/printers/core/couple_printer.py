# debug/printers/core/couple_printer.py

import gdb
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from utils import *


class Printer:
    """Pretty printer for original::couple"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return f"original::couple({addr_str(self.val)})"

    def children(self):
        yield "[0]", self.val['first_']
        yield "[1]", self.val['second_']

    def display_hint(self):
        return "array"