###########################################################
# DLL CHECK
# Ensure that each of the libraries will load correctly
# Raise an exception if this fails

from .._vnpy import *

import os

working_dir = os.getcwd()
sensors_dll = os.path.dirname(os.path.realpath(__file__)) + '\\_sensors.pyd'
uart_dll = os.path.dirname(os.path.realpath(__file__)) + '\\..\\protocol\\uart\\_uart.pyd'
math_dll = os.path.dirname(os.path.realpath(__file__)) + '\\..\\math\\_math.pyd'

sensors_return_string = check_dll_validity(uart_dll, working_dir)
uart_return_string = check_dll_validity(uart_dll, working_dir)
math_return_string = check_dll_validity(math_dll, working_dir)

if(len(sensors_return_string) > 0):
    raise ImportError('Dlls Missing in _sensors:\n' + sensors_return_string)

if(len(uart_return_string) > 0):
    raise ImportError('Dlls Missing in _sensors:\n' + uart_return_string)

if(len(math_return_string) > 0):
    raise ImportError('Dlls Missing in _sensors:\n' + math_return_string)
# END DLL CHECK
###########################################################

import time
from ._sensors import *
import vnpy.protocol.uart
import vnpy.math

def vnsensor_disconnect(self):

    # KLUDGE: The commented lines below are the current workaround for cleanly
    #         closing down on a Windows machine.

    # Stop the listening thread.
    self.halt()

    # Wait for the thread to finish
    while (self.isStopped() == False):
        time.sleep(0.01)

    # Unregister the listeners
    self.unregister()

    # Shutdown the port
    self.shutdown()

VnSensor.__disconnect_orig = VnSensor.disconnect
VnSensor.disconnect = vnsensor_disconnect

def _AsyncPacketReceivedEvent_add_(self, handler):
    self.add(handler)
    #return self

AsyncPacketReceivedEvent.__add__ = _AsyncPacketReceivedEvent_add_

# VnSensor.__iadd__ = lambda(self, handler):
#
# BinaryGroup.flags = lambda self: flag_list(self, BinaryGroup)
# BinaryGroup.__str_orig__ = BinaryGroup.__str__
# BinaryGroup.__str__ = lambda self: enum_flag_str(self, BinaryGroup)
#
# /*def __init__(self) :
# 		self.__handlers = []
#
# 		def __iadd__(self, handler) :
# 		self.__handlers.append(handler)
# 		return self
#
# 		def __isub__(self, handler) :
# 		self.__handlers.remove(handler)
# 		return self
#
# 		def fire(self, *args, **keywargs) :
# 		for handler in self.__handlers :
# 			handler(*args, **keywargs)
#
# 			def clearObjectHandlers(self, inObject) :
# 			for theHandler in self.__handlers :
# 				if theHandler.im_self == inObject :
# 					self -= theHandler*/
