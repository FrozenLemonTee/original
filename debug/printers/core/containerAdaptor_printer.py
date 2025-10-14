# debug/printers/core/containerAdaptor_printer.py

import gdb
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from utils import *


class PrinterBase:
    """Pretty printer for derived classes of containerAdaptor in original"""

    def __init__(self, val):
        self.val = val

    def class_name(self):
        return "original::containerAdaptor"

    def to_string(self):
        return f"{self.class_name()}({addr_str(self.val)})"

    def children(self):
        yield "serial", self.val["serial_"]