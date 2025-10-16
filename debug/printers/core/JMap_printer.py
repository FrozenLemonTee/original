# debug/printers/core/JMap_printer.py

import gdb
from skipList_printer import PrinterBase as Base
from utils import addr_str


class Printer(Base):
    """Pretty printer for original::JMap"""

    def __init__(self, val):
        super().__init__(val)
        self.display_mode = "map"

    def class_name(self):
        return "original::JMap"

    def children(self):
        for i, cp in enumerate(self.elem()):
            yield f"[{i}].key", cp[0]
            yield f"[{i}].value", cp[1]
        if gdb.parameter('print pretty') > 0:
            yield "head", addr_str(self.val["head_"])
            yield "compare", self.val["compare_"]

    def display_hint(self):
        return self.display_mode