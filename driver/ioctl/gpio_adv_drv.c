/***************************************************************************//**
 *  \file       driver.c
 *
 *  \details    Simple Linux device driver (IOCTL)
 *
 *
 *******************************************************************************/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/kmod.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/ioctl.h>
#include <linux/poll.h>
#include <loongson-2k.h>
#include <gpio_adv_drv.h>
#include <event_ioctl.h>


#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)
#define TEST_VALUE _IOW('a','c',int32_t)

int32_t value = 0;
int32_t gpio_value;
gpio_event_t e;

#define GPIO_INT_ENABLE 0x1fe10530
#define INT_POL                 0x1fe11470
#define INT_EDGE                0x1fe11474

#define GPIO_EN 0x1fe10500
#define LS2K_GPIO_EN 0x1fe10500
#define GPIO_O 0x1fe10510
#define GPIO_I  0x1fe10520
#define LS2K_GPIO_O 0x1fe10510


//Waitqueue
DECLARE_WAIT_QUEUE_HEAD(wait_queue_etx_data);

static bool can_write = false;
static bool can_read  = false;


// static struct timer_list etx_timer;
static unsigned int count = 0;


dev_t dev = 0;
static struct class *dev_class;
static struct cdev ls2k_gpio_cdev;

// practice
struct task_struct *task_p = NULL;


#define CBS_MAX         5

#define DBG_GPIO_IN(...)    printk(__VA_ARGS__)
#define DBG_GPIO_OUT(...)   printk(__VA_ARGS__)
#define DBG_GPIO_PWM(...)   printk(__VA_ARGS__)

/*
 * GPIO Definition
 *
 **/
typedef struct
{
	int             index;                  // gpio number

	int             work_mode;              // 工作模式

	/**************************************************************************
	 * IN 工作模式相关参数
	 *
	 */
	int             in_inverse;             // 输入：电平反相

	int             trigger_mode;	    // 触发方式

	int             bounce_ms;              // 按键去抖毫秒数
	unsigned int    bounce_ticks;
	int             bounce_on;		    // 处于去抖状态
	int             bounce_value;	    // 去抖保存的按键值
	int             bounce_edge;	    // 去抖保存边沿值，IO_EDGE_RISING/IO_EDGE_FALLING


	/**************************************************************************
	 * OUT 工作模式相关参数
	 */

	int             out_inverse;            // 输出：电平反相 



} gpio_desc_t;

//-------------------------------------------------------------------------------------------------
// Local variables
//-------------------------------------------------------------------------------------------------
static gpio_desc_t gpios[GPIO_COUNT];               // GPIO 描述符


/*
 ** Function Prototypes
 */
static int      __init ls2k_gpio_driver_init(void);
static void     __exit ls2k_gpio_driver_exit(void);
static int      ls2k_gpio_open(struct inode *inode, struct file *file);
static int      ls2k_gpio_release(struct inode *inode, struct file *file);
static ssize_t  ls2k_gpio_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  ls2k_gpio_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long     ls2k_gpio_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static unsigned int ls2k_gpio_poll(struct file *filp, struct poll_table_struct *wait);


/*
 ** File operation sturcture
 */
static struct file_operations fops =
{
	.owner          = THIS_MODULE,
	.read           = ls2k_gpio_read,
	.write          = ls2k_gpio_write,
	.open           = ls2k_gpio_open,
	.unlocked_ioctl = ls2k_gpio_ioctl,
	.release        = ls2k_gpio_release,
	.poll           = ls2k_gpio_poll

};

/*
 * led up 
 */
static int led_up(int value)
{
	int gpio=value;
	char offset;
	int error;

	offset=26;
	int irq= 66;

	pr_info("output led up gpio :%d\n",gpio);

	pr_info("ls2k_readq(GPIO_EN:%llx):%llx\n",GPIO_EN,ls2k_readq(GPIO_EN));
	ls2k_writeq(ls2k_readq(GPIO_EN) & ~(1 << gpio),  GPIO_EN);    //set edge

	pr_info("ls2k_readq(GPIO_EN:%llx):%llx\n",GPIO_EN,ls2k_readq(GPIO_EN));
	ls2k_writeq(ls2k_readq(GPIO_O) | (1 << gpio),  GPIO_O);    //set edge

	return 0;


}

/*
 ** This function will be called when we open the Device file
 */
static int ls2k_gpio_open(struct inode *inode, struct file *file)
{
	pr_info("Device File Opened...!!!\n");
	return 0;
}

/*
 ** This function will be called when we close the Device file
 */
static int ls2k_gpio_release(struct inode *inode, struct file *file)
{
	pr_info("Device File Closed...!!!\n");
	return 0;
}


/*
 ** This function will be called when we read the Device file
 */
static ssize_t ls2k_gpio_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	int gpio=value;
	printk(KERN_DEBUG "read in ls2k_gpio:%d",gpio);
	pr_info("Read Function\n");

	can_write = true;
	//wake up the waitqueue
	wake_up(&wait_queue_etx_data);

	if( copy_to_user(buf, &value, sizeof(value)))
	{
		pr_err("Data Read : Err!\n");
	}
	pr_info("gpio = %d", value);	

	return 0;
}

/*
 ** This function will be called when we write the Device file
 */
static ssize_t ls2k_gpio_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	int gpio=value;

	printk(KERN_DEBUG "write in ls2k_gpio gpio:%d",gpio);

	pr_info("Write function\n");
	return len;
}

/*
 ** This function will be called when app calls the poll function
 */
static unsigned int ls2k_gpio_poll(struct file *filp, struct poll_table_struct *wait)
{
	__poll_t mask = 0;

	poll_wait(filp, &wait_queue_etx_data, wait);
	pr_info("Poll function\n");

	if( can_read )
	{
		can_read = false;
		mask |= ( POLLIN | POLLRDNORM );
		// mask |= ( EPOLLIN | EPOLLET | EPOLLPRI);
	}

	if( can_write )
	{
		can_write = false;
		mask |= ( POLLOUT | POLLWRNORM );
	}

	return mask;

}

/*
 * 返回GPIO的工作模式
 */

static int gpio_get_workmode(int gpio_num, unsigned int *mode)
{
	CHECK_GPIO_NUM(gpio_num);
	//pr_info("in gpio_get_workmode/n");
	//pr_info("gpio_num is %d/n",gpio_num);
	
	gpio_desc_t *p_gpio = &gpios[gpio_num];
	*mode = p_gpio->work_mode;

	return 0;
}


/*
 * 取消所有GPIO的工作模式
 */
static int gpio_clear_any(int gpio_num)
{
	CHECK_GPIO_NUM(gpio_num);
	gpio_desc_t *p_gpio = &gpios[gpio_num];

	pr_info("in gpio_clear_any/n");
	pr_info("p_gpio->work_mode: %x",p_gpio->work_mode);

	switch (p_gpio->work_mode)
	{
		case GPIO_IN:
			// gpio_in_mode_unset(gpio_num);
			break;

		case GPIO_OUT:
			// gpio_out_mode_unset(gpio_num);
			break;

		case GPIO_PWM:
			// gpio_pwm_mode_unset(gpio_num);
			break;

		case GPIO_NONE:
			break;

		default:
			return -1;

	};

	return 0;

}





/******************************************************************************
 * GPIO IN 基本操作
 */

static int g_input_count = 0;

/*
 * 设置GPIO为IN模式
 */
static int gpio_in_mode_set(int gpio_num, int inverse)
{
	CHECK_GPIO_NUM(gpio_num);
	gpio_desc_t *p_gpio = &gpios[gpio_num];

	//pr_info("gpio_in_mode_set\n");
	p_gpio->in_inverse = inverse;


	// gpio_num=2;	
	pr_info("ls2k_readq(GPIO_EN:%llx):%llx\n",GPIO_EN,ls2k_readq(GPIO_EN));
	ls2k_writeq(ls2k_readq(GPIO_EN) | (1 << gpio_num),  GPIO_EN);    //set edge

	pr_info("ls2k_readq(GPIO_EN:%llx):%llx\n",GPIO_EN,ls2k_readq(GPIO_EN));
	p_gpio->work_mode = GPIO_IN;
	g_input_count += 1;

	DBG_GPIO_IN("set gpio[%i] as IN.\r\n", gpio_num);
	return 0;

}

/*
 * 读取一个GPIO的输入
 */
static int gpio_in_get_value(int gpio_num, unsigned int *in_val)
{
	int value;
	CHECK_GPIO_NUM(gpio_num);
	gpio_desc_t *p_gpio = &gpios[gpio_num];

	value  = ls2k_readq(GPIO_I) >> gpio_num & 1;
	pr_info("value is %d",value);
	*in_val=value;
	pr_info("input: gpio is%d, value is%d\n", gpio_num, *in_val);

	return 0;
}

/*
 * 读取全部64个GPIO的输入
 */
static int gpio_in_get_all_values(uint64_t *p_val64)
{
	uint64_t in64;

	// pr_info("gpio_in_get_all_values\n");
	in64 = ls2k_readq(GPIO_I);
	*p_val64 = in64;

	return 0;
}

/*
 * 取消GPIO的IN模式
 */
static int gpio_in_mode_unset(int gpio_num)
{
	CHECK_GPIO_NUM(gpio_num);
	gpio_desc_t *p_gpio = &gpios[gpio_num];

	// pr_info("gpio_in_mode_unset\n");

	p_gpio->work_mode = GPIO_NONE;
	p_gpio->in_inverse = 0;
	g_input_count -= 1;
	DBG_GPIO_IN("unset IN at gpio[%i].\r\n", gpio_num);

	return 0;
}

//-------------------------------------------------------------------------------------------------
// GPIO OUT functions
//-------------------------------------------------------------------------------------------------

/*
 * 设置GPIO为OUT模式
 */
static int gpio_out_mode_set(int gpio_num, int inverse)
{
	CHECK_GPIO_NUM(gpio_num);
	gpio_desc_t *p_gpio = &gpios[gpio_num];


	p_gpio->work_mode = GPIO_OUT;
	p_gpio->out_inverse = inverse;
	ls2k_writeq(ls2k_readq(LS2K_GPIO_EN) & ~(1 << gpio_num),  LS2K_GPIO_EN);    //set edge, 配置GPIO%为输出模式

	return 0;
}

/*
 * GPIO 输出一个值
 */
static int gpio_out_set_value(int gpio_num, unsigned int out_val)
{
	CHECK_GPIO_NUM(gpio_num);
	gpio_desc_t *p_gpio = &gpios[gpio_num];


	if (out_val == 1)
	{

		ls2k_writeq(ls2k_readq(LS2K_GPIO_O) | (1 << gpio_num),  LS2K_GPIO_O);    //set edge
	}
	else
	{

		ls2k_writeq(ls2k_readq(LS2K_GPIO_O) & ~(1 << gpio_num),  LS2K_GPIO_O);    //set edge
	}

	/*
	 * 处理反向信号
	 */

	return 0;
}

/*
 * GPIO 输出全部值
 */
static int gpio_out_set_all_values(uint64_t *p_val64)
{
	return 0;
}

/*
 * 取消GPIO的输出模式
 */
static int gpio_out_mode_unset(int gpio_num)
{
	pr_info("gpio_out_mode_unset\n");
	CHECK_GPIO_NUM(gpio_num);
	gpio_desc_t *p_gpio = &gpios[gpio_num];

	//gpio_disable(gpio_num);
	p_gpio->out_inverse = 0;
	p_gpio->work_mode = GPIO_NONE;
	DBG_GPIO_OUT("clear OUT at gpio[%i].\r\n", gpio_num);

	return 0;
}

static int status = 1, dignity = 3, ego = 5;

/*
 ** This function will be called when we write IOCTL on the Device file
 */
static long ls2k_gpio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

	unsigned int gpio, val, *pv32;
	int32_t v_arg;
	int rt  = 1;
	uint64_t val64;


//      gpio_event_t e;


	switch(cmd) {

		case EVENT_GET_VARIABLES:

			e.gpio_num = status;
			e.edge     = dignity;
			e.bouncetime = ego;

			if (copy_to_user((gpio_event_t *)arg, &e, sizeof(gpio_event_t)))
			{
				return -EACCES;
			}
			break;
		case EVENT_CLR_VARIABLES:
			status = 0;
			dignity = 0;
			ego = 0;
			break;
		case EVENT_SET_VARIABLES:
			if (copy_from_user(&e, (gpio_event_t *)arg, sizeof(gpio_event_t)))
			{
				return -EACCES;
			}

			status = e.gpio_num;
			dignity = e.edge;
			ego = e.bouncetime;

			break;

			/*
			 *GPIO Common
			 */


		case IOCTL_GPIO_GET_MODE:
			// copy from user 
			if( copy_from_user(&v_arg ,(int32_t*) arg, sizeof(v_arg)) )
			{
				pr_err("Data Write : Err!\n");
			}
			pv32 = &v_arg;
			pr_info("arg is %d\n",arg);
			pr_info("pv32 is %d\n",pv32);
			rt = gpio_get_workmode((*pv32), pv32);
			break;


		case IOCTL_GPIO_RESET:
			pr_info("gpio is %d\n", gpio);
			rt = gpio_clear_any(gpio);
			break;
			/*
			 * GPIO 输入模式
			 */
		case IOCTL_IN_ENABLE:
			v_arg=arg;
			UNPACK_16_16(v_arg, gpio, val);
			rt = gpio_in_mode_set(gpio, val);
			break;

		case IOCTL_IN_GET_VAL:
			// copy from user 
			if( copy_from_user(&v_arg ,(int32_t*) arg, sizeof(v_arg)) )
			{
				pr_err("Data Write : Err!\n");
			}
			pv32 = &v_arg;

			// get gpio value
			rt = gpio_in_get_value((*pv32), pv32);

			// copy to user 
			v_arg=*pv32;
			if( copy_to_user((int32_t*) arg, &v_arg, sizeof(v_arg)) )
			{
				pr_err("Data Read : Err!\n");
			}
			break;

		case IOCTL_IN_GET_ALL:
			//IOC_BUFFER_CHECK_RETURN;
			//rt = gpio_in_get_all_values((uint64_t *)ioarg->buffer);
			break;

		case IOCTL_IN_DISABLE:
			gpio = arg;
			rt = gpio_in_mode_unset(gpio);
			break;


			/*
			 * GPIO 输出模式
			 */
		case IOCTL_OUT_ENABLE:
			v_arg=arg;
			UNPACK_16_16(v_arg, gpio, val);
			rt = gpio_out_mode_set(gpio, val);
			break;
		case IOCTL_OUT_SET_VAL:
			v_arg=arg;
			UNPACK_16_16(v_arg, gpio, val);
			rt = gpio_out_set_value(gpio, val);
			break;
		case IOCTL_OUT_SET_ALL:
			//IOC_BUFFER_CHECK_RETURN;
			//rt = gpio_out_set_all_values((uint64_t *)ioarg->buffer);
			break;
		case IOCTL_OUT_DISABLE:
			gpio = arg;
			rt = gpio_out_mode_unset(gpio);
			break;
		case WR_VALUE:
			if( copy_from_user(&value ,(int32_t*) arg, sizeof(value)) )
			{
				pr_err("Data Write : Err!\n");
			}
			pr_info("Value = %d\n", value);
			//led_up(value);
			break;
		case RD_VALUE:
			value = gpio_value;
			if( copy_to_user((int32_t*) arg, &value, sizeof(value)) )
			{
				pr_err("Data Read : Err!\n");
			}
			break;
		case TEST_VALUE:
			pr_info("TEST Value = %d\n",arg );
			break;
		default:
			pr_info("Default\n");
			break;
	}
	return 0;
}





/*
 * GPIO IN 边沿触发处理
 *
 * edge: 当前发生的边沿变化，IN_EDGE_RISING/IN_EDGE_FALLING
 *
 */
static void gpio_in_edge_changed(gpio_desc_t *p_gpio, int edge, int val)
{




	/*
	 * 仅处理上升沿和下降沿
	 */
	if (!((p_gpio->trigger_mode == IN_EDGE_RISING) ||
				(p_gpio->trigger_mode == IN_EDGE_FALLING)))
		return;

	pr_info("....continue\n");
	if (p_gpio->trigger_mode == edge)               /* 等待的边沿方式一致 */
	{

		if (p_gpio->bounce_ms > 0)                  /* 需要去抖 */
		{

			p_gpio->bounce_edge = edge;             /* 记录边沿变化 */
			p_gpio->bounce_value = val;             /* 用于定时到达时比较 */
			p_gpio->bounce_on = 1;

			DBG_GPIO_IN("gpio[%i] IN is going to bounce.\r\n", p_gpio->index);
		}
		else                                        /* 不需要去抖/或者去抖完成，执行回调函数 */
		{
			DBG_GPIO_IN("gpio[%i] IN is occurred, do callback.\r\n", p_gpio->index);

			p_gpio->bounce_on = 0;
		}



	}
	else                                            /* 等待边沿方式相反 */
	{

	}
}

/*
 *
 * 线程，轮询GPIO输入口的数据，检查是否有电平变化
 */
int gpio_in_monitor_task(void *arg)
{

	uint64_t rd_inputs, b_inputs,v_changed, temp_inputs;
	static uint64_t last_inputs;
	int n = 0;

	int count=0;


	pr_info("gpio_in_monitor_task\n");
	gpio_in_get_all_values(&last_inputs);  /*初值*/

	while(1)
	{
		/*
		 * 读取当前GPIO IN的值，和保存的值进行“异或”运算
		 */
		if (gpio_in_get_all_values(&rd_inputs) < 0)
		{
			msleep(10);		
			//ssleep(3);
			continue;
		}
		// pr_info("rd_inputs: %llx\n", rd_inputs);
		// pr_info("last_inputs: %llx\n", last_inputs);
		msleep(10);

		// initial add

		v_changed = rd_inputs ^ last_inputs;
		if (v_changed)
		{
			pr_err("v_changed %llx\n",v_changed);

			
		        pr_info("rd_inputs: %llx\n", rd_inputs);
		        pr_info("last_inputs: %llx\n", last_inputs);

			int i, edge;
			// last_inputs = rd_inputs;  /*保存的数据*/
			temp_inputs = rd_inputs;  /*保存的数据*/
			
			for (i=0; i<GPIO_COUNT; i++)
			{
				gpio_desc_t *p = &gpios[i];


				if (v_changed & 0x1)
				{
					edge = (rd_inputs & 0x1) ?
						IN_EDGE_RISING :     /* 0--->1 上升沿 */
						IN_EDGE_FALLING;     /* 1--->0 下降沿 */

					//if((rd_inputs & 0x1) && (i == e.gpio_num) && (edge == e.edge))
					if((rd_inputs & 0x1) && (i == e.gpio_num))
					{
						
						pr_info("changed gpio %d?= gpio_num:%d, edge:%d ?= e.edge %d\n", i,e.gpio_num, edge, e.edge);
						msleep(20);
						pr_info("changed gpio is %d, msleep(%d)\n", i, e.bouncetime);
                                                //msleep(e.bouncetime);
						 
	                                        gpio_in_get_all_values(&b_inputs);  /*初值*/
						pr_info("last_inputs: %llx\n", last_inputs);
						pr_info("b_inputs: %llx\n", b_inputs);
						//if(b_inputs ^ last_inputs)
						//{
							can_write = true;

							//wake up the waitqueue
							wake_up(&wait_queue_etx_data);
							//continue;
                                                        pr_err("event occurred\n");
						//}
 
					}

					gpio_value=i;
					// gpio_in_edge_changed(p, edge, rd_inputs & 0x1);
				}

				v_changed >>= 1;		       /* 检测下一位 */
				rd_inputs >>= 1;
			}
			
			last_inputs = temp_inputs;  /*保存的数据*/
		}else{

			last_inputs = rd_inputs;
		}
		msleep(10);   // task sleep 10 ms

		if(kthread_should_stop())
		{
			break;
		}

	}

	return 0;
}

static int gpio_in_monitor_create(void)
{
	task_p = kthread_create(gpio_in_monitor_task,NULL,"task gpio_in_monitor"); 
	if(!IS_ERR(task_p)) {
		wake_up_process(task_p);
	}

	return 0;
}

/*
 ** Module Init function
 */
static int __init ls2k_gpio_driver_init(void)
{
	/*Allocating Major number*/
	if((alloc_chrdev_region(&dev, 0, 1, "ls2k_gpio_Dev")) <0){
		pr_err("Cannot allocate major number\n");
		return -1;
	}
	pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));

	/*Creating cdev structure*/
	cdev_init(&ls2k_gpio_cdev,&fops);

	/*Adding character device to the system*/
	if((cdev_add(&ls2k_gpio_cdev,dev,1)) < 0){
		pr_err("Cannot add the device to the system\n");
		goto r_class;
	}

	/*Creating struct class*/
	if((dev_class = class_create(THIS_MODULE,"ls2k_gpio_class")) == NULL){
		pr_err("Cannot create the struct class\n");
		goto r_class;
	}

	/*Creating device*/
	if((device_create(dev_class,NULL,dev,NULL,"ls2k_gpio_device")) == NULL){
		pr_err("Cannot create the Device 1\n");
		goto r_device;
	}

	gpio_in_monitor_create();

	pr_info("Device Driver Insert...Done!!!\n");
	return 0;

r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(dev,1);
	return -1;
}

/*
 ** Module exit function
 */
static void __exit ls2k_gpio_driver_exit(void)
{
	device_destroy(dev_class,dev);
	class_destroy(dev_class);
	cdev_del(&ls2k_gpio_cdev);
	unregister_chrdev_region(dev, 1);

	// practice
	printk("%s:\n",__func__);
	kthread_stop(task_p);

	pr_info("Device Driver Remove...Done!!!\n");
}

module_init(ls2k_gpio_driver_init);
module_exit(ls2k_gpio_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("...");
MODULE_DESCRIPTION("Simple Linux device driver (IOCTL)");
MODULE_VERSION("1.0");
