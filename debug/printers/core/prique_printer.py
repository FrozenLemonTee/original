# debug/printers/core/prique_printer.py

import gdb
from containerAdaptor_printer import PrinterBase as Base


class Printer(Base):
    """Pretty printer for original::prique"""

    def __init__(self, val):
        super().__init__(val)

    def class_name(self):
        return "original::prique"