# debug/printers/core/alternative_printer.py

import gdb
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from utils import *


class Printer:
    """Pretty printer for original::alternative"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        has_value = bool(call(self.val, "operator bool"))
        return f"original::alternative(notnull={has_value}, {addr_str(self.val)})"

    def children(self):
        has_value = bool(call(self.val, "operator bool"))
        if has_value:
            yield "value", self.val["val_"]["type_"]
