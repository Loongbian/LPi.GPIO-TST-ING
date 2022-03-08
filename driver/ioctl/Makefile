obj-m += gpio_adv_drv.o

EXTRA_CFLAGS += \
	     -I$(src)/include \
	     -DTEST
KDIR = /lib/modules/$(shell uname -r)/build
 
 
all:
	make -C $(KDIR)  M=$(shell pwd) modules
 
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
