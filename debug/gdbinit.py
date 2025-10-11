import gdb
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "printers"))

from core import couple_printer

pp = gdb.printing.RegexpCollectionPrettyPrinter("original")
pp.add_printer('couple', '^original::couple<.*>$', couple_printer.CouplePrinter)

gdb.printing.register_pretty_printer(gdb.current_objfile(), pp)

print("GDB pretty-printer for original registered.")