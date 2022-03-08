import LPi.GPIO as GPIO

LED_PIN = 11
LOOP_IN = 22
LOOP_OUT = 15
SWITCH_PIN =13

GPIO.setmode(GPIO.LS2K)


"""Test that an output() can be input()"""

GPIO.setup(LED_PIN, GPIO.OUT)

GPIO.output(LED_PIN, GPIO.HIGH)
if GPIO.HIGH == GPIO.input(LED_PIN):
    print('GPIO.HIGH == GPIO.input(LED_PIN):')

GPIO.output(LED_PIN, GPIO.LOW)
if GPIO.LOW == GPIO.input(LED_PIN):
    print('GPIO.LOW == GPIO.input(LED_PIN):')


"""Test output loops back to another input"""
GPIO.setup(LOOP_IN, GPIO.IN, pull_up_down=GPIO.PUD_OFF)
GPIO.setup(LOOP_OUT, GPIO.OUT, initial=GPIO.LOW)
if GPIO.LOW == GPIO.input(LOOP_IN):
    print('GPIO.LOW == GPIO.input(LOOP_IN):')
GPIO.output(LOOP_OUT, GPIO.HIGH)
if GPIO.HIGH == GPIO.input(LOOP_IN):
    print('GPIO.HIGH == GPIO.input(LOOP_IN):')


"""Test output() using lists"""
GPIO.setup(LOOP_OUT, GPIO.OUT)
GPIO.setup(LED_PIN, GPIO.OUT)

GPIO.output( [LOOP_OUT, LED_PIN], GPIO.HIGH)
#
GPIO.output( (LOOP_OUT, LED_PIN), GPIO.LOW)
GPIO.output( [LOOP_OUT, LED_PIN], [GPIO.HIGH, GPIO.LOW] )
GPIO.output( (LOOP_OUT, LED_PIN), (GPIO.LOW, GPIO.HIGH) )

input("input continue:")


GPIO.cleanup()
