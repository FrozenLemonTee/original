# debug/printers/utils.py


U_INTEGER_SIZE = 4

def to_int(val, size = 8):
    mask = (1 << (size * 8)) - 1
    v = int(val)
    return v & mask