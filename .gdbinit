python
import sys
import os
sys.path.insert(0, os.path.join(os.getcwd(), 'debug'))
import gdbinit
print("GDB pretty-printer for original initialized.")
end