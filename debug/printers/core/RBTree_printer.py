# debug/printers/core/RBTree_printer.py

import gdb
import sys
import os
sys.path.insert(0, os.path.dirname(os.path.dirname(__file__)))
from utils import *


def get_successor_node(ptr):
    if ptr == 0:
        return 0
    node = ptr.dereference()
    if node["right_"] != 0:
        ptr = node["right_"]
        node = ptr.dereference()
        while node["left_"] != 0:
            ptr = node["left_"]
            node = ptr.dereference()
        return ptr

    node = ptr.dereference()
    ptr_parent = node["parent_"]
    node = ptr_parent.dereference()

    while ptr_parent != 0 and ptr == node["right_"]:
        ptr = ptr_parent
        node = ptr_parent.dereference()
        ptr_parent = node["parent_"]
    return ptr_parent


class PrinterBase:
    """Pretty printer for derived classes of RBTree in original"""

    def __init__(self, val):
        self.val = val

    def class_name(self):
        return "original::RBTree"

    def to_string(self):
        size = int(call(self.val, "size"))
        return f"{self.class_name()}(size={size}, {addr_str(self.val)})"

    def get_min_node(self):
        p = self.val["root_"]
        if p == 0:
            return p
        node = p.dereference()
        while node["left_"] != 0:
            p = node["left_"]
            node = p.dereference()
        return p

    def get_max_node(self):
        p = self.val["root_"]
        if p == 0:
            return p
        node = p.dereference()
        while node["right_"] != 0:
            p = node["right_"]
            node = p.dereference()
        return p

    def elem(self):
        if self.val["root_"] == 0:
            return
        cur = self.get_min_node()
        end = self.get_max_node()
        while cur != end:
            node = cur.dereference()
            cp = node["data_"]
            yield cp["first_"], cp["second_"]
            cur = get_successor_node(cur)
        if end != 0:
            node = end.dereference()
            cp = node["data_"]
            yield cp["first_"], cp["second_"]
