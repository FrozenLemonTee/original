# debug\printers\core\couple_printer.py
import gdb

class CouplePrinter:
    """Pretty printer for original::couple"""

    def __init__(self, val):
        self.val = val

    def to_string(self):
        first = self.val['first_']
        second = self.val['second_']
        return f"original::couple({first}, {second})"

    def children(self):
        yield "first", self.val['first_']
        yield "second", self.val['second_']