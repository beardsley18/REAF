###########################################################
# DLL CHECK
# Ensure that each of the libraries will load correctly
# Raise an exception if this fails

# No point in checking here since if anything is wrong with
# _vnpy the program would fail before we could check.

# END DLL CHECK
###########################################################

import os, platform

if platform.architecture()[0] != '64bit' or os.name != 'posix':
    raise Exception('detected {0}|{1} environment whereas this library supports only 64bit|posix'.format(platform.architecture()[0], os.name))

from ._vnpy import *
