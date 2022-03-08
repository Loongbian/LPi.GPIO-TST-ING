import LPi.GPIO as GPIO

p = GPIO.PWM(0)
p.set(1000000000,500000000)
input("continue:")  #随意按下一个键继续执行
p.start()
p.ChangePeriod(1000000000)
p.ChangeDutyCycle(300000000)
input("exit:") #随意按下就退出
p.stop()
