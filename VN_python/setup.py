import platform, os
from distutils.core import setup, Extension

_package_data = {}
_package_dir = None
if os.name is 'nt':
    _package_data.update({
        'vnpy': ['_vnpy.pyd', 'boost_python-vc100-mt-1_58.dll', 'cpp-core.dll'],
        #'vnpy': ['boost_python-vc100-mt-1_58.dll', 'cpp-core.dll'],
        'vnpy.math': ['_math.pyd'],
        'vnpy.sensors': ['_sensors.pyd'],
        'vnpy.protocol.uart': ['_uart.pyd'],
        'vnpy.xplat': ['_xplat.pyd']
    })
    _package_dir = {'': 'python/bin/win32'}
    # TODO: Useful for development purposes below. -Paul
    #_package_dir = {'': 'python'}
elif os.name is 'posix':
    _package_data.update({
        'vnpy': ['_vnpy.so', 'libboost_python3.so', 'libboost_python3.so.1.58.0', 'libcpp-core.so'],
        'vnpy.math': ['_math.so'],
        'vnpy.sensors': ['_sensors.so'],
        'vnpy.protocol.uart': ['_uart.so'],
        'vnpy.xplat': ['_xplat.so']
    })

    bit_arch = platform.architecture()[0]
    if bit_arch == '32bit':
        _package_dir = {'': 'python/bin/linux_i386'}
    elif bit_arch == '64bit':
        _package_dir = {'': 'python/bin/linux_amd64'}

#_package_data['vnpy._vnpy']

#extensions = {
#    'vnpy.cpp-core2': ['cpp/src/vn/m']
#    'vnpy._vnpy': ['cpp/src/vn/python.cpp'],
#    #'vnpy.math._math': ['cpp/src/vn/math/python.cpp']
#}

#ext_list = []
#for k, v in extensions.items():
#    ext_list.append(Extension(
#        k,
#        v,
#        include_dirs=['cpp/include', 'libs/boost'],
#        library_dirs=['libs/boost/stage/lib'],
#
#        # HACK: Added to be able to compile vn/math/python.cpp
#        extra_compile_args=['/bigobj']
#    ))

setup(
    name='vnpy',
    version='1.1.0.115',
    description='VectorNav Programming Library',
    author='VectorNav Technologies, LLC',
    author_email='support@vectornav.com',
    package_dir = _package_dir,
    packages=[
        'vnpy',
        'vnpy.math',
        'vnpy.sensors',
        'vnpy.protocol',
        'vnpy.protocol.uart',
        'vnpy.xplat'],
    package_data=_package_data,
    #ext_modules=ext_list
    #ext_modules=[
    #    Extension('vnpy._vnpy', ['cpp/src/vn/python.cpp'], include_dirs=['cpp/include', 'libs/boost'], library_dirs=['libs/boost/stage/lib']),
    #    #Extension('vnpy._vnpy', ['cpp/src/vn/python.cpp'], include_dirs=['-IG:/Workspace/pl/cpp/include']),
    #]
    #data_files=[
    #    ('', ['python/vnpy/_vnpy.pyd'])
    #]
)
