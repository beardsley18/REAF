###########################################################
# DLL CHECK
# Ensure that each of the libraries will load correctly
# Raise an exception if this fails

from .._vnpy import *

import os

working_dir = os.getcwd()
xplat_dll = os.path.dirname(os.path.realpath(__file__)) + '\\_xplat.pyd'

xplat_return_string = check_dll_validity(xplat_dll, working_dir)

if(len(xplat_return_string) > 0):
    raise ImportError('Dlls Missing in _xplat:\n' + xplat_return_string)

# END DLL CHECK
###########################################################

from ._xplat import *
