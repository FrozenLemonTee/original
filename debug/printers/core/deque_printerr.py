# debug/printers/core/deque_printer.py

import gdb
from containerAdaptor_printer import PrinterBase as Base


class Printer(Base):
    """Pretty printer for original::deque"""

    def __init__(self, val):
        super().__init__(val)

    def class_name(self):
        return "original::deque"