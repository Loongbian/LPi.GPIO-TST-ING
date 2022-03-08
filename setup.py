from distutils.core import setup, Extension
setup(name = 'LPi.GPIO', version = '1.0',  \
   ext_modules = [Extension('LPi.GPIO', ['source/py_gpio.c', 'source/constants.c', 'source/c_gpio.c', 'source/common.c', 'source/cpuinfo.c', 'source/py_pwm.c', 'source/event_gpio.c'])])
