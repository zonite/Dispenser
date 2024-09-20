/* Kernel interface */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/string.h>

#include "dispenser.h"

//extern void *pDispenser = NULL;
//static void *pDispenser = NULL;
//static struct dispenser_config cConfig = { 0 };

static int dispenser_alloc_mmap(void)
{
	//int i;
	//struct page *mem = alloc_pages(GFP_WAIT, 1);
	//void *mem = page_address(mem) //alloc_pages
	//void *mem = vmalloc(PAGE_SIZE);
	//unsigned long size = sizeof(union dispenser_mmap) * (1 + cDispenser.col_count + cDispenser.slot_count);
	unsigned long size = (((sizeof(union dispenser_mmap) * (1 + cDispenser.col_count + cDispenser.slot_count)) / PAGE_SIZE) + 1) * PAGE_SIZE;
	void *mem = NULL;

	if (pDispenser_mmap) {
		if (size == cDispenser.mmap_size)
			return 0;
		dispenser_free_mmap();
	}

	mem = vmalloc(size);

	pDispenser_mmap = mem;
	printk("Allocated size(%li) %px for dispenser mmap.", size, mem);
	printk("Unit should be %px", pDispenser_mmap);
	printk("Cols should be %px", pDispenser_mmap + 1);
	printk("Slots should be %px", pDispenser_mmap + 1 + cDispenser.col_count);

	if (!pDispenser_mmap) {
		return -ENOMEM;
	}

	//SetPageReserved(virt_to_page(mem)); //kmalloc
	//SetPageReserved(mem); //alloc_pages
	SetPageReserved(vmalloc_to_page(mem));
	//memset(mem, 0, PAGE_SIZE);
	memset(mem, 0, size);

	cDispenser.mmap_size = size;

	return 0;
}

static void dispenser_free_mmap(void)
{
	cDispenser.mmap_size = 0;

	if (pDispenser_mmap) {
		ClearPageReserved(vmalloc_to_page(pDispenser_mmap));

		vfree(pDispenser_mmap);

		pDispenser_mmap = NULL;
	}
	//kfree(mem); //kmalloc
}

static int __init dispenser_init(void)
{
	printk(KERN_INFO "Dispenser starting\n");

	//if (dt_register()) {
	if (platform_driver_register(&cDispenser.dispenser_driver)) {
		printk("Probe failed. Device tree not found!\n");

		return FAIL;
	}

	/*
	if (!pDispenser_mmap)
		pDispenser_mmap = alloc_mem();

	if (pDispenser_mmap < 0) {
		platform_driver_unregister(&cDispenser.dispenser_driver);

		return FAIL;
	}
	*/

	//dispenser_unit_mmap_set();
	//dispenser_unit_mmap_reset();

	//if (platform_driver_register()) {

	//}


	if (init_chardev() < 0) {
		printk(KERN_ALERT "Init_chardev failed\n");
		dispenser_unit_mmap_reset();
		platform_driver_unregister(&cDispenser.dispenser_driver);
		dispenser_free_mmap();

		return FAIL;
	}

	dispenser_genl_init();

	//Read params:
	//init_param();
	if (false)
		init_param();
	if (false)
		sensor_init();

	return SUCCESS;
}

static void __exit dispenser_exit(void)
{
	printk(KERN_INFO "Dispenser unload\n");

	if (false)
		sensor_close();

	dispenser_genl_exit();
	cleanup_chardev();
	dispenser_unit_mmap_reset();

	dispenser_free_mmap();

	platform_driver_unregister(&cDispenser.dispenser_driver);
}

module_init(dispenser_init);
module_exit(dispenser_exit);

