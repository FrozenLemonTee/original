# debug/printers/core/autoPtr_printer.py

import gdb
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from utils import *

class PrinterBase:
    """Pretty printer for derived classes of autoPtr in original"""

    def __init__(self, val):
        self.val = val

    def class_name(self):
        pass

    def to_string(self):
        ptr = int(gdb.parse_and_eval(f'(({self.val.type.name}*)({int(self.val.address)}))->get()'))
        if ptr != 0:
            return f"{self.class_name()}(@{ptr:#x})"
        else:
            return f"{self.class_name()}(nullptr)"


    def children(self):
        ptr = gdb.parse_and_eval(f'(({self.val.type.name}*)({int(self.val.address)}))->get()')
        ptr_val = int(ptr)
        yield "pointer", f"{ptr_val:#x}" if ptr_val != 0 else "nullptr"

        if ptr_val != 0:
            yield "object", ptr.dereference()

        ref_count = self.val['ref_count']
        loaded_ptr = gdb.parse_and_eval(f'(({ref_count.type.name}*)({int(ref_count.address)}))->operator*()')

        try:
            ref_count_base = loaded_ptr.dereference()
            strong_refs_atomic = ref_count_base['strong_refs']
            weak_refs_atomic = ref_count_base['weak_refs']
            strong_refs = gdb.parse_and_eval(f"(({strong_refs_atomic.type.name}*)({int(strong_refs_atomic.address)}))->operator*()")
            weak_refs = gdb.parse_and_eval(f"(({weak_refs_atomic.type.name}*)({int(weak_refs_atomic.address)}))->operator*()")
            yield "strong_refs", to_int(strong_refs, U_INTEGER_SIZE)
            yield "weak_refs", to_int(weak_refs, U_INTEGER_SIZE)
        except gdb.error as e:
            yield "ref_count_details", f"[cannot access: {e}]"