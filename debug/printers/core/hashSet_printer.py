# debug/printers/core/hashSet_printer.py

import gdb
from hashTable_printer import PrinterBase as Base


class Printer(Base):
    """Pretty printer for original::hashSet"""

    def __init__(self, val):
        super().__init__(val)
        self.display_mode = "array"

    def class_name(self):
        return "original::hashSet"

    def children(self):
        for i, cp in enumerate(self.elem()):
            yield f"[{i}]", cp[0]
        if gdb.parameter('print pretty') > 0:
            yield "buckets", self.val["buckets"]

    def display_hint(self):
        return self.display_mode
