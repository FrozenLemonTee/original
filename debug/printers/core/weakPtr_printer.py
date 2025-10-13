# debug/printers/core/weakPtr_printer.py

import gdb

from autoPtr_printer import PrinterBase as Base
class Printer(Base):
    """Pretty printer for original::weakPtr"""


    def __init__(self, val):
        super().__init__(val)


    def class_name(self):
        return "original::weakPtr"