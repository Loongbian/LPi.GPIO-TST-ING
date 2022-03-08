/*
Copyright (c) 2012-2019 Ben Croston

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include "c_gpio.h"
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include<sys/ioctl.h>
#include <sys/stat.h>
#include "gpio_adv_drv.h"


#define PAGE_SIZE  (4*1024)
#define BLOCK_SIZE (4*1024)

#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)

// int fd_gpio;
static volatile uint32_t *gpio_map;

int setup(void)
{
    int mem_fd;


    fd_gpio = open("/dev/ls2k_gpio_device", O_RDWR);
    if(fd_gpio < 0) {
            printf("Cannot open device file...\n");
            return 0;
    }

    // mmap the GPIO memory registers
    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0)
        return SETUP_DEVMEM_FAIL;

    return SETUP_OK;
}

void set_pullupdn(int gpio, int pud)
{
    printf("set_pullupdn()\n");
}

void setup_gpio(int gpio, int direction, int pud)
{

    // set_pullupdn(gpio, pud);

    if (direction == INPUT)
    {
	__gpio_in_enable(fd_gpio, gpio, 1);
    } else {   // direction == OUTPUT
	__gpio_out_enable(fd_gpio, gpio, 0);
    
    }

}

void output_gpio(int gpio, int value)
{

   __gpio_out_value(fd_gpio, gpio, value);

}

int input_gpio(int gpio)
{
   int value=gpio;

   __gpio_in_value(fd_gpio, &value);

   return value;
	
}

void cleanup(void)
{
	
        close(fd_gpio);

}
