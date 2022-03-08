#include <linux/module.h>
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0x9a8c5fdb, "module_layout" },
	{ 0xb4d1c225, "kthread_stop" },
	{ 0x55e440e5, "cdev_del" },
	{ 0x608f8683, "device_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x68d9ffad, "wake_up_process" },
	{ 0xc10d6ba7, "kthread_create_on_node" },
	{ 0x24d08582, "class_destroy" },
	{ 0x65feb45e, "device_create" },
	{ 0xee77a63f, "__class_create" },
	{ 0x781385d5, "cdev_add" },
	{ 0x9569678f, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xdecd0b29, "__stack_chk_fail" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xfb578fc5, "memset" },
	{ 0x5397c01e, "__copy_user" },
	{ 0xb3f7646e, "kthread_should_stop" },
	{ 0x3eeb2322, "__wake_up" },
	{ 0xf9a482f9, "msleep" },
	{ 0xc5850110, "printk" },
	{ 0x9514151a, "_mcount" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "4EB118B526207D3FC8A3CE0");
