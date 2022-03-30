import ctypes
import struct
import traceback
from .base import *
from .tools import *


if t_path_or_file_exists('/project/dave/neural_network'):
    try:
        from dave.neural_network import *
    except:
        print(traceback.print_exc())