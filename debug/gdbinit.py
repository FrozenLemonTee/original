# debug/gdbinit.py

import gdb
import sys
import os
import importlib.util
import inspect


def load_printers_from_directory(pretty_printer_collection, directory):
    """Load all pretty-printer scripts from the specified directory."""
    if not os.path.isdir(directory):
        print(f"[WARN] Skipped non-existent directory: {directory}")
        return

    for file_name in os.listdir(directory):
        if not file_name.endswith("_printer.py"):
            continue

        file_path = os.path.join(directory, file_name)
        module_name = os.path.splitext(file_name)[0]

        try:
            spec = importlib.util.spec_from_file_location(module_name, file_path)
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)

            printer_class = None
            for name, cls in inspect.getmembers(module, inspect.isclass):
                if name == "Printer":
                    printer_class = cls
                    break

            if printer_class is None:
                print(f"[SKIP] {module_name}: no 'Printer' class found")
                continue
            type_name = module_name.replace("_printer", "")
            type_regex = f"^original::{type_name}(<.*>)?$"

            pretty_printer_collection.add_printer(module_name, type_regex, printer_class)
            print(f"Registered pretty-printer: {type_name} -> {printer_class.__name__}")

        except Exception as e:
            print(f"Failed to load {file_name}: {e}")


this_dir = os.path.dirname(__file__)
printers_dir = os.path.join(this_dir, "printers")
core_dir = os.path.join(printers_dir, "core")

sys.path.insert(0, printers_dir)
sys.path.insert(0, core_dir)

pp = gdb.printing.RegexpCollectionPrettyPrinter("original")
load_printers_from_directory(pp, core_dir)
gdb.printing.register_pretty_printer(None, pp)

print("GDB pretty-printers for 'original' registered successfully.")