/*
Copyright (c) 2013-2014 Ben Croston

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


#include "Python.h"
#include "common.h"

// LPi.GPIO
// const int pin_to_gpio_rev[61] = {-1, -1/P3V3, -1/P5V, -1/LS2K_IIC1_SDA, -1/P5V, -1/LS2K_IIC1_SCL, -1/GND, 7/LS2K_GPIO07, -1/UART3_TXD, -1/GND, -1/UART3_RXD, 60/LS2K_GPIO60, 1/LS2K_GPIO1, 2/LS2K_GPIO2, -1/GND, 3/LS2K_GPIO3, 4/LS2K_GPIO04, -1/P3V3, 5/LS2K_GPIO05, -1/LS2K_SPI_SDO/MOSI, -1/GND, -1/LS2K_SPI_SDI/MISO, 6/LS2K_GPIO06, -1/LS2K_SPI_SCK, -1/LS2K_SPI_CSN1, -1/GND, -1/LS2K_SPI_CSN2, -1/LS2K_IIC0_SDA, -1/LS2K_IIC0_SCL, 8/LS2K_GPIO08, -1/GND, 9/LS2K_GPIO09, 10/LS2K_GPIO10, 11/LS2K_GPIO11, -1/GND, 12/LS2K_GPIO12, 37/LS2K_GPIO37, 13/LS2K_GPIO13, 38/LS2K_GPIO38, 40/LS2K_GPIO40, 41/LS2K_GPIO41, 56/LS2K_GPIO56, 57/LS2K_GPIO57, 58/LS2K_GPIO58, 59/LS2K_GPIO59, -1/PWM0, -1/PWM1, -1/PWM2, -1/PWM3, -1/CANH0, -1/CANL0, -1/CANH1, -1/CANL1, -1/UART4_TXD, -1/UART4_RXD, -1/UART5_TXD, -1/UART5_RXD, -1/GND, -1/GND, -1/RS232_DEBUG_TXD0, -1/RS232_DEBUG_RXD0};

int gpio_mode = MODE_UNKNOWN;

const int pin_to_gpio_rev[61] = {-1, -1, -1, -1, -1, -1, -1, 7, -1, -1, -1, 60, 1, 2, -1, 3, 4, -1, 5, -1, -1, -1, 6, -1, -1, -1, -1, -1, -1, 8, -1, 9, 10, 11, -1, 12, 37, 13, 38, 40, 41, 56, 57, 58, 59, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

int check_gpio_priv(void)
{
	// check module has been imported cleanly
	if (setup_error)
	{
		PyErr_SetString(PyExc_RuntimeError, "Module not imported correctly!");
		return 1;
	}

	return 0;
}


int get_gpio_number(int channel, unsigned int *gpio)
{

    // check setmode() has been run

    if (gpio_mode != BOARD && gpio_mode != LS2K)
    {
	
    	PyErr_SetString(PyExc_RuntimeError, "Please set pin numbering mode using GPIO.setmode(GPIO.BOARD) or GPIO.setmode(GPIO.LS2K)");
	return 3;
    }

    // check channel number is in range
    if ( (gpio_mode == LS2K && (channel < 0 || channel > 64))
	|| (gpio_mode == BOARD && (channel < 1 || channel > 61)) )
    {
    	PyErr_SetString(PyExc_ValueError, "The channel sent is invalid on a Loongson Edu Pi");
	return 4;
    }

    // convert channel to gpio
    if (gpio_mode == BOARD)
    {
        if (*(*pin_to_gpio+channel) == -1)
        {
            PyErr_SetString(PyExc_ValueError, "The channel sent is invalid on a Loongson Edu Pi");
            return 5;
        } else {
            *gpio = *(*pin_to_gpio+channel);
        }
    }
    else // gpio_mode == LS2K
    {
        *gpio = channel;
    }

    return 0;

}
