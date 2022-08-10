/* Kernel interface */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/string.h>

#include "dispenser.h"

//extern void *pDispenser = NULL;
static void *pDispenser = NULL;
static struct dispenser_config cConfig = { 0 };

static void *alloc_mem(void)
{
  //int i;
  //struct page *mem = alloc_pages(GFP_WAIT, 1);
  //void *mem = page_address(mem) //alloc_pages
  void *mem = vmalloc(PAGE_SIZE);

  if (!mem)
    return mem;

  //SetPageReserved(virt_to_page(mem)); //kmalloc
  //SetPageReserved(mem); //alloc_pages
  SetPageReserved(vmalloc_to_page(mem));
  memset(mem, 0, PAGE_SIZE);

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
  if (platform_driver_register(&dispenser_driver)) {
      printk("Probe failed. Device tree not found!\n");
      return FAIL;
  }

  if (!pDispenser)
    pDispenser = alloc_mem();

  if (pDispenser < 0)
    return FAIL;
  
  if (init_chardev() < 0) {
    printk(KERN_ALERT "Init_chardev failed\n");
    return FAIL;
  }
  
  init_param();
  
  return SUCCESS;
}

static void __exit dispenser_exit(void)
{
  printk(KERN_INFO "Dispenser unload\n");

  cleanup_chardev();

  if (pDispenser)
    free_mem(pDispenser);
    
  pDispenser = NULL;

  platform_driver_unregister(&dispenser_driver);
}

module_init(dispenser_init);
module_exit(dispenser_exit);

