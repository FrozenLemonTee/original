# debug/printers/utils.py

import gdb


U_INTEGER_SIZE = 4

def to_int(val, size = 8):
    mask = (1 << (size * 8)) - 1
    v = int(val)
    return v & mask

def address(obj):
    try:
        return int(obj.address) if obj.address else None
    except gdb.error:
        return None

def type_name(obj):
    return obj.type.name

def call(obj, method_name, *args):
    try:
        args_str = ', '.join(str(arg) for arg in args)
        addr = address(obj)
        expr = f'(({type_name(obj)}*)({addr}))->{method_name}({args_str})'
        return gdb.parse_and_eval(expr)
    except Exception as e:
        print(f"Error calling {method_name} on {obj}: {e}")
        return None
