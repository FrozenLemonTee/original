# debug/printers/core/autoPtr_printer.py

import gdb
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from utils import *


def is_array_ptr(val):
    type_str = str(val.type)

    if "<" not in type_str or ">" not in type_str:
        return False

    inner = type_str[type_str.find("<")+1:type_str.rfind(">")]
    parts = [p.strip() for p in inner.split(",")]
    if len(parts) < 2:
        return False
    deleter_param = parts[1]
    return deleter_param.endswith("[]>")


class PrinterBase:
    """Pretty printer for derived classes of autoPtr in original"""

    def __init__(self, val):
        self.val = val

    def class_name(self):
        return "original::autoPtr"

    def to_string(self):
        ptr = int(call(self.val, "get"))
        if ptr != 0:
            return f"{self.class_name()}(@{ptr:#x})"
        else:
            return f"{self.class_name()}(nullptr)"

    def children(self):
        ptr = call(self.val, "get")
        ptr_val = int(ptr)

        if ptr_val != 0:
            if is_array_ptr(self.val):
                yield "array", f"@{ptr_val:#x}"
            else:
                yield "pointer", f"@{ptr_val:#x}" if ptr_val != 0 else "nullptr"
                yield "object", ptr.dereference()

        ref_count = self.val['ref_count']
        loaded_ptr = call(ref_count, "operator*")

        try:
            ref_count_base = loaded_ptr.dereference()
            strong_refs_atomic = ref_count_base['strong_refs']
            weak_refs_atomic = ref_count_base['weak_refs']
            strong_refs = call(strong_refs_atomic, "operator*")
            weak_refs = call(weak_refs_atomic, "operator*")
            yield "strong_refs", to_int(strong_refs, U_INTEGER_SIZE)
            yield "weak_refs", to_int(weak_refs, U_INTEGER_SIZE)
        except gdb.error as e:
            yield "ref_count_details", f"[cannot access: {e}]"