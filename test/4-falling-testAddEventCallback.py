#!/usr/bin/env python

import LPi.GPIO as GPIO
import time
from threading import Timer
import sys

LOOP_IN = 1

def cb(channel):
    global callback_count
    callback_count += 1
    print('callback_count',callback_count)

GPIO.setmode(GPIO.LS2K)
GPIO.setup(LOOP_IN, GPIO.IN)

# falling test
callback_count = 0
GPIO.add_event_detect(LOOP_IN, GPIO.RISING)
GPIO.add_event_callback(LOOP_IN, cb)
input()
GPIO.remove_event_detect(LOOP_IN)

