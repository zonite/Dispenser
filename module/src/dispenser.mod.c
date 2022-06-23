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
	{ 0x94670e1c, "module_layout" },
	{ 0x91c63c58, "cdev_del" },
	{ 0x928956c5, "cdev_init" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x7a456bbf, "param_ops_int" },
	{ 0x8a3fbba, "device_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x999e8297, "vfree" },
	{ 0x3744cf36, "vmalloc_to_pfn" },
	{ 0xc3aaf0a9, "__put_user_1" },
	{ 0x7604ca00, "device_create" },
	{ 0x2e467e0c, "cdev_add" },
	{ 0xbbf75cd3, "module_put" },
	{ 0x92997ed8, "_printk" },
	{ 0xe3374ec8, "remap_pfn_range" },
	{ 0x363ef002, "class_destroy" },
	{ 0xeac4ba65, "vmalloc_to_page" },
	{ 0x90c79f4b, "__class_create" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0xa78aaa14, "try_module_get" },
};

MODULE_INFO(depends, "");

