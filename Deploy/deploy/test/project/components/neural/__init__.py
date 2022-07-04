import ctypes
import struct
import os

dirs = [e for e in os.listdir('/project/components/neural') if not e.startswith('__')]
for model_name in dirs:
    exec_str = f'from .{model_name} import *'
    exec(exec_str)