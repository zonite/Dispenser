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

static void *alloc_mem(void)
{
  //int i;
  //struct page *mem = alloc_pages(GFP_WAIT, 1);
  //void *mem = page_address(mem) //alloc_pages
  //void *mem = vmalloc(PAGE_SIZE);
  unsigned long size = sizeof(pDispenser_mmap) * (1 + cDispenser.col_count + cDispenser.slot_count);
  void *mem = vmalloc(size);

  if (!mem)
    return mem;

  //SetPageReserved(virt_to_page(mem)); //kmalloc
  //SetPageReserved(mem); //alloc_pages
  SetPageReserved(vmalloc_to_page(mem));
  //memset(mem, 0, PAGE_SIZE);
  memset(mem, 0, size);

  return mem;
}

static void free_mem(void *mem)
{
  ClearPageReserved(vmalloc_to_page(mem));
  
  vfree(mem);
  //kfree(mem); //kmalloc
}

static int __init dispenser_init(void)
{
    printk(KERN_INFO "Dispenser starting\n");

    //if (dt_register()) {
    /*
    if (platform_driver_register(&cDispenser.dispenser_driver)) {
        printk("Probe failed. Device tree not found!\n");

        return FAIL;
    }
    */

    if (!pDispenser_mmap)
        pDispenser_mmap = alloc_mem();

    if (pDispenser_mmap < 0) {
        platform_driver_unregister(&cDispenser.dispenser_driver);

        return FAIL;
    }

    dispenser_unit_mmap_set();

    //if (platform_driver_register()) {

    //}

  
    if (init_chardev() < 0) {
        printk(KERN_ALERT "Init_chardev failed\n");
        dispenser_unit_mmap_reset();
        platform_driver_unregister(&cDispenser.dispenser_driver);
        if (pDispenser_mmap)
          free_mem(pDispenser_mmap);

        return FAIL;
    }
  
    //Read params:
    //init_param();
    if (false)
        init_param();
  
    return SUCCESS;
}

static void __exit dispenser_exit(void)
{
  printk(KERN_INFO "Dispenser unload\n");

  cleanup_chardev();
  dispenser_unit_mmap_reset();

  if (pDispenser_mmap)
    free_mem(pDispenser_mmap);
    
  pDispenser_mmap = NULL;

  platform_driver_unregister(&cDispenser.dispenser_driver);
}

module_init(dispenser_init);
module_exit(dispenser_exit);

