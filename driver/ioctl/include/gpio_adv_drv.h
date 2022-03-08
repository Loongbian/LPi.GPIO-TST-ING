#define GPIO_COUNT 64

#define GPIO_NONE       0x00
#define GPIO_IN         0x01
#define GPIO_OUT        0x02
#define GPIO_PWM        0x03

#define CHECK_GPIO_NUM(gpio) do { if ((gpio < 0) || (gpio >= GPIO_COUNT)) return -1; } while (0)

//-------------------------------------------------------------------------------------------------
// GPIO common 操作
//-------------------------------------------------------------------------------------------------
//#define IOCTL_GPIO_GET_MODE 0x0001
#define IOCTL_GPIO_GET_MODE _IOR('a','0x0001',int32_t*)   /* 获取当前GPIO设置的模式  arg: uint32_t*; dual, in:gpio_num, out:mode */

#define IOCTL_GPIO_RESET    0x0002

//-----------------------------------------------------------------------------------------
// GPIO AS IN 模式
//-----------------------------------------------------------------------------------------
#define IOCTL_IN_ENABLE  _IOW('a','0x1001',int32_t)   /* 设置GPIO为输入方式。  a[16]:uint32_t; */
#define IOCTL_IN_GET_VAL _IOR('a','0x1002',int32_t*)   /* 获取GPIO输入值(一个)  arg: uint32_t*; dual, in:gpio_num, out:value */
#define IOCTL_IN_GET_ALL    0x1003   /* 获取GPIO输入值(全部)  arg: uint64_t*; all 64 GPIOs values */
#define IOCTL_IN_DISABLE    _IOW('a','0x1004',int32_t)   /* 撤销GPIO的IN功能      arg: uint32_t;  gpio_num */

// #define IOCTL_GPIO_EDGE_SET  _IOW('a','0x1000',int32_t)   

#define GPIO_IN_ADVANCED    1

/*
 * 输入信号边沿变化
 */
#define IN_EDGE_NONE      0x00
#define IN_EDGE_RISING    0x01      /* 上升沿 */
#define IN_EDGE_FALLING   0x02      /* 下降沿 */

/*
 * 按键消息
 */
typedef struct
{
    int gpio_num;       // GPIO编号
    int edge;           // 发生的边沿
    int value;          // 当前IN值
} edge_message_t;


// #define IOCTL_IN_SET_EDGE    _IOW('a','0x1005',int32_t)   /* 撤销GPIO的IN功能      arg: uint32_t;  gpio_num */
#define IOCTL_IN_SET_BOUNCE  _IOW('a','0x1006',int32_t)  /* 设置去抖事件, 毫秒.  arg: uint32_t;  hi[16]:bounceMS, lo[16]:gpio_num */
#define IOCTL_IN_ADD_CB      _IOW('a','0x1007',int32_t)  /* 设置按键回调函数.    arg: uint64_t*; hi[32]:cb_func, lo[32]:gpio_num */
#define IOCTL_IN_REMOVE_CB   _IOW('a','0x1008',int32_t)  /* 移去按键回调函数.    arg: uint64_t*; hi[32]:cb_func, lo[32]:gpio_num */
//-------------------------------------------------------------------------------------------------
// GPIO AS OUT 模式
//-------------------------------------------------------------------------------------------------

#define IOCTL_OUT_ENABLE    _IOW('a','0x2001',int32_t)    /* 设置GPIO为输出方式.    arg: unit32_t; hi[16]:inverse, lo[16]:gpio_num */
#define IOCTL_OUT_SET_VAL   _IOW('a','0x2002',int32_t)    /* 设置GPIO为输出值(一个) arg: unit32_t; hi[16]:out_value, lo[16]:gpio_num */
#define IOCTL_OUT_SET_ALL   0x2003    /* 设置GPIO为输出值(全部) arg: unit64_t*; all 64 GPIOs values */
#define IOCTL_OUT_DISABLE   _IOW('a','0x2004',int32_t)    /* 撤销GPIO的OUT功能.     arg: unit32_t;  gpio_num */
//-------------------------------------------------------------------------------------------------
//
// 驱动设备相关的操作
//
// GPIO_ADV_DEV_NAME=/dev/ls2k_gpio_device
//
//-------------------------------------------------------------------------------------------------

// fd = open(GPIO_ADV_DEV_NAME, O_RDWR);


#define PACK_16_16(gpio,a)      (((u_int32_t)(a) << 16) | ((gpio) & 0xFFFF))
#define PACK_24_8(gpio,a)       (((u_int32_t)(a) <<  8) | ((gpio) & 0xFF  ))

#define UNPACK_16_16(v,gpio,a)  do { gpio = v & 0xFFFF;  a = v >> 16; } while (0)
#define UNPACK_24_8(v,gpio,a)   do { gpio = v & 0xFF;    a = v >> 8;  } while (0)


/******************************************************************************
 * GPIO IN
 ******************************************************************************/
// #define __gpio_edge_set(fd, gpio, inv)   ioctl(fd, IOCTL_GPIO_EDGE_SET, PACK_16_16(gpio, inv))

#define __gpio_in_enable(fd, gpio, inv)   ioctl(fd, IOCTL_IN_ENABLE, PACK_16_16(gpio, inv))
#define __gpio_in_value(fd, pval32)       ioctl(fd, IOCTL_IN_GET_VAL, pval32)
#define __gpio_in_all(fd, pval64)         ioctl(fd, IOCTL_IN_GET_ALL, pval64)
#define __gpio_in_disable(fd, gpio)       ioctl(fd, IOCTL_IN_DISABLE, gpio)

#define PACK_32_32(gpio,a)      (((u_int64_t)(a) << 32) | (u_int32_t)(gpio))
#define UNPACK_32_32(v,gpio,a)  do { gpio = (u_int32_t)v; a = (u_int32_t)(v >> 32); } while (0)

// #define __gpio_in_set_edge(fd, gpio, e)   ioctl(fd, IOCTL_IN_SET_EDGE, PACK_16_16(gpio, e))
#define __gpio_in_set_bounce(fd, gpio, b) ioctl(fd, IOCTL_IN_SET_BOUNCE, PACK_16_16(gpio, b))
#define __gpio_in_add_cb(fd, gpio, cb)    do { u_int64_t tmp = PACK_32_32(gpio_num, cb); ioctl(fd, IOCTL_IN_ADD_CB, &tmp); } while (0)

/******************************************************************************
 * GPIO OUT
 *****************************************************************************/
#define __gpio_out_enable(fd, gpio, inv)  ioctl(fd, IOCTL_OUT_ENABLE,  PACK_16_16(gpio, inv))
#define __gpio_out_value(fd, gpio, val)   ioctl(fd, IOCTL_OUT_SET_VAL, PACK_16_16(gpio, val))
#define __gpio_out_all(fd, pval64)       ioctl(fd, IOCTL_OUT_SET_ALL, pval64)
#define __gpio_out_disable(fd, gpio)      ioctl(fd, IOCTL_OUT_DISABLE, gpio)


typedef void (*gpio_callback_t)(int gpio_num);

