# debug/printers/core/strongPtr_printer.py

import gdb

from autoPtr_printer import PrinterBase as Base
class Printer(Base):
    """Pretty printer for original::strongPtr"""


    def __init__(self, val):
        super().__init__(val)


    def class_name(self):
        return "original::strongPtr"