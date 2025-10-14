# debug/printers/core/ownerPtr_printer.py

import gdb

from autoPtr_printer import PrinterBase as Base
class Printer(Base):
    """Pretty printer for original::ownerPtr"""


    def __init__(self, val):
        super().__init__(val)


    def class_name(self):
        return "original::ownerPtr"