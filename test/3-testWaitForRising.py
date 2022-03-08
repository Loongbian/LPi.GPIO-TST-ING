#!/usr/bin/env python

import LPi.GPIO as GPIO
import time
from threading import Timer

LOOP_OUT = 22
LOOP_IN = 1

def makehigh():
    GPIO.output(LOOP_OUT, GPIO.HIGH)
    print('22 pin output HIGH')

# setUp(self)
#GPIO.setwarnings(False)
GPIO.setmode(GPIO.LS2K)
GPIO.setup(LOOP_IN, GPIO.IN)
GPIO.setup(LOOP_OUT, GPIO.OUT)


GPIO.output(LOOP_OUT, GPIO.LOW)
#t = Timer(0.1, makehigh)
t = Timer(3.0, makehigh)
t.start()
GPIO.wait_for_edge(LOOP_IN, GPIO.RISING)
print('wait_for_edge finished')
