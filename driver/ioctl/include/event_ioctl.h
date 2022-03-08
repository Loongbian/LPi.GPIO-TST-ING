#ifndef EVENT_IOCTL_H
#define EVENT_IOCTL_H
#include <linux/ioctl.h>

/*
 * GPIO event Definition
 */

typedef struct 
{
        int gpio_num;
        int edge;
        int bouncetime;
} gpio_event_t;

#define EVENT_GET_VARIABLES _IOR('q', 1, gpio_event_t *)
#define EVENT_CLR_VARIABLES _IO('q', 2)
#define EVENT_SET_VARIABLES _IOW('q', 3, gpio_event_t *)
#endif
