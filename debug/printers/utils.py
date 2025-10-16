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

def addr_str(item):
    if isinstance(item, int) or isinstance(item, str):
        return f"@{item:#x}"
    return f"@{address(item):#x}"

def type_name(obj):
    return obj.type.name

def call(obj, method_name, *args):
    """
    Safely invoke a member function on an object using GDB.

    :param obj: gdb.Value - the object instance
    :param method_name: str - member function name (e.g., 'size')
    :param args: optional arguments to pass to the function
    :return: gdb.Value | None - result of the call or None if failed
    """
    try:
        if obj.type.code == gdb.TYPE_CODE_REF:
            obj = obj.referenced_value()

        addr = address(obj)
        if addr is None or addr == 0:
            return None

        args_str = ', '.join(str(arg) for arg in args)

        expr = f'(({type_name(obj)}*)({addr}))->{method_name}({args_str})'

        for _ in range(3):
            try:
                return gdb.parse_and_eval(expr)
            except gdb.error as e:
                msg = str(e)
                if "signaled" in msg or "The program being debugged" in msg:
                    return None
                raise

    except Exception as e:
        print(f"[GDB:call] Error calling {method_name} on {obj}: {e}")
        return None
