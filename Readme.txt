# install python LPi.GPIO module

sudo python3 setup.py install --record files.txt 

# remove LPi.GPIO
sudo rm -rf build/ && cat files.txt | sudo xargs rm -rf 

# test LPi.GPIO
sudo python3 test/
  
