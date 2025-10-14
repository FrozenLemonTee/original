# debug/printers/core/hashMap_printer.py

import gdb
from hashTable_printer import PrinterBase as Base


class Printer(Base):
    """Pretty printer for original::hashMap"""

    def __init__(self, val):
        super().__init__(val)

    def class_name(self):
        return "original::hashMap"

    def children(self):
        for i, cp in enumerate(self.elem()):
            yield f"[{i}].key", cp[0]
            yield f"[{i}].value", cp[1]
        if gdb.parameter('print pretty') > 0:
            yield "buckets", self.val["buckets"]

    def display_hint(self):
        return "map"