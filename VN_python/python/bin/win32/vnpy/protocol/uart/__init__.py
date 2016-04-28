###########################################################
# DLL CHECK
# Ensure that each of the libraries will load correctly
# Raise an exception if this fails

from ..._vnpy import *

import os

working_dir = os.getcwd()
uart_dll = os.path.dirname(os.path.realpath(__file__)) + '\\_uart.pyd'

uart_return_string = check_dll_validity(uart_dll, working_dir)

if(len(uart_return_string) > 0):
    raise ImportError('Dlls Missing in _uart:\n' + uart_return_string)

# END DLL CHECK
###########################################################

from ._uart import *

def get_set_bits(num):
    set_bits = []

    for i in range(0, 15):
        mask = 1 << i
        if mask & num:
            set_bits.append(mask)

    return set_bits

def enum_flag_str(num, enum_type):
    flags = flag_list(num, enum_type)

    s = ''
    for i, f in enumerate(flags):
        s += identity(enum_type(f)).__str_orig__()

        if i < len(flags) - 1:
            s += ' | '

    return s

def flag_list(num, enum_type):
    return [ identity(enum_type(x)) for x in get_set_bits(num) ]

BinaryGroup.flags = lambda self: flag_list(self, BinaryGroup)
BinaryGroup.__str_orig__ = BinaryGroup.__str__
BinaryGroup.__str__ = lambda self: enum_flag_str(self, BinaryGroup)

CommonGroup.flags = lambda self: flag_list(self, CommonGroup)
CommonGroup.__str_orig__ = CommonGroup.__str__
CommonGroup.__str__ = lambda self: enum_flag_str(self, CommonGroup)

TimeGroup.flags = lambda self: flag_list(self, TimeGroup)
TimeGroup.__str_orig__ = TimeGroup.__str__
TimeGroup.__str__ = lambda self: enum_flag_str(self, TimeGroup)

ImuGroup.flags = lambda self: flag_list(self, ImuGroup)
ImuGroup.__str_orig__ = ImuGroup.__str__
ImuGroup.__str__ = lambda self: enum_flag_str(self, ImuGroup)

GpsGroup.flags = lambda self: flag_list(self, GpsGroup)
GpsGroup.__str_orig__ = GpsGroup.__str__
GpsGroup.__str__ = lambda self: enum_flag_str(self, GpsGroup)

AttitudeGroup.flags = lambda self: flag_list(self, AttitudeGroup)
AttitudeGroup.__str_orig__ = AttitudeGroup.__str__
AttitudeGroup.__str__ = lambda self: enum_flag_str(self, AttitudeGroup)

InsGroup.flags = lambda self: flag_list(self, InsGroup)
InsGroup.__str_orig__ = InsGroup.__str__
InsGroup.__str__ = lambda self: enum_flag_str(self, InsGroup)
