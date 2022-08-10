#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
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
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xc3aaf0a9, "__put_user_1" },
	{ 0x92997ed8, "_printk" },
	{ 0x4e02bd86, "module_put" },
	{ 0x2272c158, "try_module_get" },
	{ 0x3744cf36, "vmalloc_to_pfn" },
	{ 0x5381b5bc, "remap_pfn_range" },
	{ 0xce99b00d, "device_property_present" },
	{ 0xfa06e1a9, "device_property_read_string" },
	{ 0x8c995cb3, "device_property_read_u32_array" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0xde2936c6, "__platform_driver_register" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x55a09676, "vmalloc_to_page" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x516db740, "__class_create" },
	{ 0xfd02f053, "device_create" },
	{ 0xa11276fc, "class_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xfa1312a2, "cdev_init" },
	{ 0xe6e523a5, "cdev_add" },
	{ 0xcadd9d56, "device_destroy" },
	{ 0x24c8e1ee, "cdev_del" },
	{ 0x999e8297, "vfree" },
	{ 0x6ab39cd3, "param_ops_int" },
	{ 0x203adcdf, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Chortensis,dispenser");
MODULE_ALIAS("of:N*T*Chortensis,dispenserC*");
