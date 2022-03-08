import LPi.GPIO as GPIO

LED_PIN_LS2K = 13
        
# Test trying to change mode after it has been set
GPIO.setmode(GPIO.LS2K)
GPIO.setup(LED_PIN_LS2K, GPIO.IN)
GPIO.cleanup()


