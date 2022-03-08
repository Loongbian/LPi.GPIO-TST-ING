/*
Copyright (c) 2013-2018 Ben Croston

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
#include "py_pwm.h"
#include <fcntl.h>


typedef struct
{
    PyObject_HEAD
    unsigned int gpio;
    float freq;
    float dutycycle;
} PWMObject;

//channel 0 1 2 3
int pwm_channel=-1;

/************* /sys/class/pwm/ functions ************/
#define x_write(fd, buf, len) do {                                  \
        size_t x_write_len = (len);                                     \
        \
        if ((size_t)write((fd), (buf), x_write_len) != x_write_len) {   \
                close(fd);                                                  \
                return (-1);                                                \
        }                                                               \
} while (/* CONSTCOND */ 0)


int pwm_export(unsigned int channel)
{
    int fd, len;
    char str_gpio[3];
    char filename[43];
    int num=0;

    snprintf(filename, sizeof(filename), "/sys/class/pwm/pwmchip%d/pwm0/enable", channel);

    /* return if pwm already exported */
    
    if (access(filename, F_OK) != -1) {
        return 0;
    }
    

    snprintf(filename, sizeof(filename), "/sys/class/pwm/pwmchip%d/export", channel);
    if ((fd = open(filename, O_WRONLY)) < 0) {
       return -1;
    }

    len = snprintf(str_gpio, sizeof(str_gpio), "%d", num);
    x_write(fd, str_gpio, len);

    close(fd);
    pwm_channel = channel;
    return 0;

}

int pwm_unexport(unsigned int channel)
{
    int fd, len;
    char str_gpio[3];
    char filename[43];
    int num=0;

    snprintf(filename, sizeof(filename), "/sys/class/pwm/pwmchip%d/unexport", channel);
    
    if ((fd = open(filename, O_WRONLY)) < 0)
        return -1;

    len = snprintf(str_gpio, sizeof(str_gpio), "%d", num);
    x_write(fd, str_gpio, len);
    close(fd);


    return 0;
}

int pwm_set_dutycycle(unsigned int channel, unsigned int dutycycle)
{
    int fd, len;
    char filename[43];
    char str_dutycycle[15];
	
    snprintf(filename, sizeof(filename), "/sys/class/pwm/pwmchip%d/pwm0/duty_cycle", channel);

    if ((fd = open(filename, O_WRONLY)) < 0)
        return -1;

    len = snprintf(str_dutycycle, sizeof(str_dutycycle), "%d", dutycycle);
    x_write(fd, str_dutycycle, len);
    close(fd);

    
    return 0;
}


int pwm_set_period(unsigned int channel, unsigned int period)
{

    int fd, len;
    char filename[43];
    char str_period[15];
	
    snprintf(filename, sizeof(filename), "/sys/class/pwm/pwmchip%d/pwm0/period", channel);
    if ((fd = open(filename, O_WRONLY)) < 0)
        return -1;

    len = snprintf(str_period, sizeof(str_period), "%d", period);
    x_write(fd, str_period, len);
    close(fd);


    return 0;
}


// python method PWM.__init__(self, channel, frequency)
static int PWM_init(PWMObject *self, PyObject *args)
{
    int channel;
    float frequency;

    if (!PyArg_ParseTuple(args, "i", &channel))
        return NULL;


    pwm_export(channel);

    
    if (channel < 0 || channel > 3)
    {
        PyErr_SetString(PyExc_ValueError, "channel must be from 0 to 3");
        return -1;
    }


    return 0;
}

// python method PWM.set(self, period, dutycycle)
static PyObject *PWM_set(PWMObject *self, PyObject *args)
{
    int dutycycle;
    int period;

    if (!PyArg_ParseTuple(args, "ii", &period, &dutycycle))
        return NULL;
	
    if (pwm_channel == -1) 
    {
	PyErr_SetString(PyExc_RuntimeError, "You must init pwm channel");
        return -1;
    
    }
    pwm_set_period(pwm_channel, period);
    pwm_set_dutycycle(pwm_channel, dutycycle);

    Py_RETURN_NONE;
}

// python method PWM.start(self, dutycycle)
static PyObject *PWM_start(PWMObject *self, PyObject *args)
{

    int fd, len;
    char str_gpio[3];
    char filename[43];
    int value=1;

    snprintf(filename, sizeof(filename), "/sys/class/pwm/pwmchip%d/pwm0/enable", pwm_channel);

    if ((fd = open(filename, O_WRONLY)) < 0)
        return -1;


    len = snprintf(str_gpio, sizeof(str_gpio), "%d", value);
    x_write(fd, str_gpio, len);
    close(fd);

    Py_RETURN_NONE;
}

// python method PWM.ChangeDutyCycle(self, dutycycle)
static PyObject *PWM_ChangeDutyCycle(PWMObject *self, PyObject *args)
{
    int dutycycle = 0;
    

    if (!PyArg_ParseTuple(args, "i", &dutycycle))
        return NULL;

    if (dutycycle <= 0)
    {
        PyErr_SetString(PyExc_ValueError, "dutycycle must be greater than 0");
        return NULL;
    }
    

    pwm_set_dutycycle(pwm_channel, dutycycle);


    Py_RETURN_NONE;
}

// python method PWM.ChangePeriod(self, period)
static PyObject *PWM_ChangePeriod(PWMObject *self, PyObject *args)
{

    int period = 0;


    if (!PyArg_ParseTuple(args, "i", &period))
        return NULL;

    if (period <= 0)
    {
        PyErr_SetString(PyExc_ValueError, "period must be greater than 0.0");
        return NULL;
    }


    pwm_set_period(pwm_channel, period);


    Py_RETURN_NONE;
}


// python method PWM. ChangeFrequency(self, frequency)
static PyObject *PWM_ChangeFrequency(PWMObject *self, PyObject *args)
{
    float frequency = 1.0;



    if (!PyArg_ParseTuple(args, "f", &frequency))
        return NULL;

    if (frequency <= 0)
    {
        PyErr_SetString(PyExc_ValueError, "frequency must be greater than 0.0");
        return NULL;
    }

    self->freq = frequency;

    
    Py_RETURN_NONE;
}

// python function PWM.stop(self)
static PyObject *PWM_stop(PWMObject *self, PyObject *args)
{

    int fd, len;
    char str_gpio[3];
    char filename[43];
    int value=0;


    snprintf(filename, sizeof(filename), "/sys/class/pwm/pwmchip%d/pwm0/enable", pwm_channel);

    if ((fd = open(filename, O_WRONLY)) < 0)
        return -1;

    len = snprintf(str_gpio, sizeof(str_gpio), "%d", value);
    x_write(fd, str_gpio, len);
    close(fd);

    pwm_unexport(pwm_channel);

    Py_RETURN_NONE;
}


// deallocation method
static void PWM_dealloc(PWMObject *self)
{
    Py_TYPE(self)->tp_free((PyObject*)self);
}


static PyMethodDef
PWM_methods[] = {

   { "set", (PyCFunction)PWM_set, METH_VARARGS, "set dutycycle and period\n" },
   { "start", (PyCFunction)PWM_start, METH_VARARGS, "Start software PWM\ndutycycle - the duty cycle (0.0 to 100.0)" },
   //{ "ChangeDutyCycle", (PyCFunction)PWM_ChangeDutyCycle, METH_VARARGS, "Change the duty cycle\ndutycycle - between 0.0 and 100.0" },
   { "ChangeDutyCycle", (PyCFunction)PWM_ChangeDutyCycle, METH_VARARGS, "Change the duty cycle\n" },
   { "ChangePeriod", (PyCFunction)PWM_ChangePeriod, METH_VARARGS, "Change the Period\n" },
   /*
   { "ChangeFrequency", (PyCFunction)PWM_ChangeFrequency, METH_VARARGS, "Change the frequency\nfrequency - frequency in Hz (freq > 1.0)" },
   */
   { "stop", (PyCFunction)PWM_stop, METH_VARARGS, "Stop software PWM" },
   
   { NULL }
};


PyTypeObject PWMType = {
   PyVarObject_HEAD_INIT(NULL,0)
   "RPi.GPIO.PWM",            // tp_name
   sizeof(PWMObject),         // tp_basicsize
   0,                         // tp_itemsize
   (destructor)PWM_dealloc,   // tp_dealloc
   0,                         // tp_print
   0,                         // tp_getattr
   0,                         // tp_setattr
   0,                         // tp_compare
   0,                         // tp_repr
   0,                         // tp_as_number
   0,                         // tp_as_sequence
   0,                         // tp_as_mapping
   0,                         // tp_hash
   0,                         // tp_call
   0,                         // tp_str
   0,                         // tp_getattro
   0,                         // tp_setattro
   0,                         // tp_as_buffer
   Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flag
   "Pulse Width Modulation class",    // tp_doc
   0,                         // tp_traverse
   0,                         // tp_clear
   0,                         // tp_richcompare
   0,                         // tp_weaklistoffset
   0,                         // tp_iter
   0,                         // tp_iternext
   PWM_methods,               // tp_methods
   0,                         // tp_members
   0,                         // tp_getset
   0,                         // tp_base
   0,                         // tp_dict
   0,                         // tp_descr_get
   0,                         // tp_descr_set
   0,                         // tp_dictoffset
   (initproc)PWM_init,        // tp_init
   0,                         // tp_alloc
   0,                         // tp_new
};




PyTypeObject *PWM_init_PWMType(void)
{
   // Fill in some slots in the type, and make it ready
   PWMType.tp_new = PyType_GenericNew;
   if (PyType_Ready(&PWMType) < 0)
      return NULL;
   //printf("PWM_init_PWMType\n");
   return &PWMType;

}


