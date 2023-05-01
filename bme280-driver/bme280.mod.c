#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
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
__used __section("__versions") = {
	{ 0x32e21920, "module_layout" },
	{ 0xc7466e35, "cdev_del" },
	{ 0x320c3a3f, "cdev_init" },
	{ 0x6627bdce, "i2c_smbus_read_byte_data" },
	{ 0xae927f14, "i2c_del_driver" },
	{ 0x948ce243, "i2c_smbus_read_i2c_block_data" },
	{ 0xe76d7921, "i2c_smbus_write_byte_data" },
	{ 0xc4847533, "i2c_put_adapter" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xefa56918, "i2c_new_client_device" },
	{ 0xacb11033, "i2c_unregister_device" },
	{ 0x37d70e96, "i2c_register_driver" },
	{ 0x6a0a71c7, "cdev_add" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0x92997ed8, "_printk" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x656e4a6e, "snprintf" },
	{ 0x3ae4522f, "i2c_get_adapter" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "CCBFF1600053C132547B5C8");
