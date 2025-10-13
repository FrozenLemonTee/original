# debug/printers/core/couple_printer.py

import gdb

class Printer:
    """Pretty printer for original::couple"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        return f"original::couple(@{int(self.val.address):#x})"

    def children(self):
        yield "[0]", self.val['first_']
        yield "[1]", self.val['second_']

    def display_hint(self):
        return "array"