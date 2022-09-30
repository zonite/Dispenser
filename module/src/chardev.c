/* Chadev interface */

#include <linux/fs.h>
#include <linux/mm.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
//#include <asm/page.h>

#include "dispenser.h"

//extern int init_chardev(void);
#define FAILURE -1

//static int Major;
static dev_t Major;
static struct cdev c_dev;
static struct class *cl;
static int Device_Open = 0;
static char a_cMsg[BUF_LEN] = "Testi\n";
//static char *p_cMsg;

//Char dev functions
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static int device_mmap(struct file *, struct vm_area_struct *);
static long device_ioctl(struct file *, unsigned int ioctl_num, unsigned long ioctl_param);

static void vma_open(struct vm_area_struct *vma)
{ try_module_get(THIS_MODULE); }
//{ __module_get(THIS_MODULE); }
static void vma_close(struct vm_area_struct *vma)
{ module_put(THIS_MODULE); }

static struct vm_operations_struct remap_vm_ops = {
 .open =  vma_open,
 .close = vma_close
};

static struct file_operations fops = {
  .read = device_read,
  .write = device_write,
  .open = device_open,
  .release = device_release,
  .mmap = device_mmap,
  .unlocked_ioctl = device_ioctl
};

int init_chardev(void)
{
    int result;
    //Major = register_chrdev(0, DEVICE_NAME, &fops);
    printk("Init chardev.\n");
    result = alloc_chrdev_region(&Major, 0, 1, DEVICE_NAME);

    //if (Major < 0 ) {
    if (result < 0 ) {
	printk(KERN_ALERT "Registering char device failed with %d\n", Major);
	return result;
    }
    //Create chardev;
    if ( (cl = class_create(THIS_MODULE, DEVICE_CLASS) ) == NULL) {
	printk(KERN_ALERT "Class creation failed\n");
	unregister_chrdev_region(Major, 1);

	return FAILURE;
    }

    if ((cDispenser.dev = device_create(cl, NULL, Major, NULL, DEVICE_NAME)) == NULL) {
	printk(KERN_ALERT "Device creation failed\n");
	class_destroy(cl);
	unregister_chrdev_region(Major, 1);

	return FAILURE;
    }

    cdev_init(&c_dev, &fops);

    if (cdev_add(&c_dev, Major, 1) == -1) {
	printk(KERN_ALERT "Device addition failed\n");
	device_destroy(cl, Major);
	class_destroy(cl);
	unregister_chrdev_region(Major, 1);

	return FAILURE;
    }

    return SUCCESS;
}

void cleanup_chardev(void)
{
    //unregister_chrdev(Major, DEVICE_NAME);
    cdev_del( &c_dev );
    device_destroy( cl, Major );
    class_destroy( cl );
    unregister_chrdev_region( Major, 1 );
    cDispenser.dev = NULL;
    printk(KERN_ALERT "Device unregistered\n");

    return;
  /*
  if (ret < 0)
    printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);

  return ret;
  */
}

static int device_open(struct inode *inode, struct file *file)
{
    //static int counter = 0;

    ++Device_Open;

    try_module_get(THIS_MODULE);
    printk(KERN_INFO "Dispenser open\n");

    //p_cMsg = a_cMsg;
    inode->i_size = cDispenser.mmap_size;

    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
    --Device_Open;

    module_put(THIS_MODULE);

    return 0;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
    int bytes_read = 0;
    char *p_cMsg = 0;

    if (*offset >= BUF_LEN || *offset < 0)
	return 0;

    p_cMsg = a_cMsg + *offset;

    while (length && *p_cMsg) {
	put_user(*(p_cMsg++), buffer++);

	--length;
	++bytes_read;
    }

    *offset += bytes_read;

    return bytes_read;
}

static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
    printk(KERN_ALERT "Dispenser: Sorry, this operation isn't supported.\n");
    return -EINVAL;
}

static int device_mmap(struct file *pFile, struct vm_area_struct *vma)
{
    unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
    //unsigned long offset = VMA_OFFSET(vma);

    //if (offset != 0 || vma->vm_end - vma->vm_start != PAGE_SIZE)
    if (offset != 0 || vma->vm_end - vma->vm_start > cDispenser.mmap_size || !pDispenser_mmap)
	return -EAGAIN;

    //offset = page_to_pfn(virt_to_page(pDispenser));

  //Not used
  /*
  if (offset >= _&thinsp;_pa(high_memory) || (pFile->f_flags & O_SYNC))
    vma->vm_flags |= VM_IO;
  vma->vm_flags |= VM_RESERVED;
  */

  //if (remap_page_range(vma->vm_start, pDispenser,
  //		       vma->vm_end-vma->vm_start, vma->vm_page_prot))
  //  return -EAGAIN;

    vma->vm_flags |= VM_DONTEXPAND | VM_SHARED;
    vma->vm_flags &= ~(VM_WRITE | VM_EXEC);

    if (remap_pfn_range(vma, vma->vm_start, vmalloc_to_pfn(pDispenser_mmap),
                        PAGE_SIZE, vma->vm_page_prot))
	return -EAGAIN;

    vma->vm_ops = &remap_vm_ops;
    vma_open(vma);

    return 0;
}

int32_t answer = 10;

static long device_ioctl(struct file *filp, unsigned int ioctl_num, unsigned long ioctl_param)
{
	struct dispenser_ioctl ioctl_cmd;
	struct dispenser_slot_list *s = NULL;
	printk(KERN_INFO "Dispenser ioctl %i\n", ioctl_num);
	//printk(KERN_INFO "Dispenser ioctl %ui, param %l\n", ioctl_num, ioctl_param);

	switch(ioctl_num) {
	case WR_VALUE:
		if (copy_from_user(&answer, (int32_t *) ioctl_param, sizeof(answer)))
			printk("ioctl error\n");
		else
			printk("ioctl successful\n");
		break;
	case RD_VALUE:
		if (copy_to_user(&answer, (int32_t *) ioctl_param, sizeof(answer)))
			printk("ioctl error\n");
		else
			printk("ioctl successful\n");
		break;
	case GREETER:
		if (copy_from_user(&ioctl_cmd, (struct dispenser_ioctl *) ioctl_param, sizeof(ioctl_cmd)))
			printk("ioctl error\n");
		else
			printk("ioctl successful: %d\n", ioctl_cmd.cmd);
		break;
	case DISPENSER_CMD:
		if (copy_from_user(&ioctl_cmd, (struct dispenser_ioctl *) ioctl_param, sizeof(ioctl_cmd))) {
			printk("ioctl error\n");
			return 0;
		}

		switch(ioctl_cmd.cmd) {
		case RELEASE_ALL:
			dispenser_unit_release_all(ioctl_cmd.param.release.force);
			break;
		case RELEASE_SLOT:
			dispenser_unit_release(ioctl_cmd.param.slot.col, ioctl_cmd.param.slot.slot);
			break;
		case RELEASE_COLUMN:
			dispenser_unit_release_column(dispenser_unit_get_column(ioctl_cmd.param.release.col), ioctl_cmd.param.release.count, ioctl_cmd.param.release.force);
			break;
		case RELEASE_UNIT:
			dispenser_unit_release_count(ioctl_cmd.param.release.count, ioctl_cmd.param.release.force);
			break;
		case ALL_CLOSED:
			dispenser_unit_filled();
			break;
		case FAILED_SLOT:
			s = dispenser_unit_get(ioctl_cmd.param.slot.col, ioctl_cmd.param.slot.slot);
			dispenser_unit_slot_failed(s);
			break;
		}
	}

	return 0;
}
