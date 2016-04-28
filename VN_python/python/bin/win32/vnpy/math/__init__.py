###########################################################
# DLL CHECK
# Ensure that each of the libraries will load correctly
# Raise an exception if this fails

from .._vnpy import *

import os

working_dir = os.getcwd()
math_dll = os.path.dirname(os.path.realpath(__file__)) + '\\_math.pyd'

math_return_string = check_dll_validity(math_dll, working_dir)

if(len(math_return_string) > 0):
    raise ImportError('Dlls Missing in _math:\n' + math_return_string)

# END DLL CHECK
###########################################################

from ._math import *
