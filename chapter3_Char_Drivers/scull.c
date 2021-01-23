#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/stat.h>

#define DEV_SCULL_NAME  "scull"
#define DEV_SCULL_NB    4

#define SCULL_QUANTIUM  4000
#define SCULL_QSET      1000

static int major = 0;
static int minor = 0;

// S_IRUGO: Allow user/group/other to read the parameter in the sysfs
// Example:
// With S_IRUGO|S_IWUSR:
//      # ls -al /sys/module/scull/parameters/major 
//      -rw-r--r-- 1 root root 4096 janv. 23 12:42 /sys/module/scull/parameters/major
// If set to 0, not entry created in :
//      # ls /sys/module/scull/parameters/major
//      ls: cannot access '/sys/module/scull/parameters/major': No such file or directory
module_param(major, int, S_IRUGO);
module_param(minor, int, S_IRUGO);

struct scull_qset {
    void **data;
    struct scull_qset *next;
};

struct scull_dev {
    struct scull_qset *data;    /* pointer to the first quantun set */
    int quantum;                /* the current quantum size */
    int qset;                   /* the current qset size */
    unsigned long size;         /* amount of data stored here */
    struct cdev cdev;           /* char device structure */
};

struct scull_dev scull_devices[DEV_SCULL_NB];

static ssize_t scull_read(struct file *filp, char __user *buff, size_t count, loff_t *offp)
{
    printk("Scull read\n");
    
    return 0;
}

static ssize_t scull_write(struct file *filp, const char __user *buff, size_t count, loff_t *offp)
{
    printk("Scull write\n");
    
    return count;
}

static int scull_open(struct inode *inode, struct file *filp)
{
    // A other method can to be to use the minor
    // Get the major and minor:
    //      imajor(inode)
    //      iminor(inode)
    
    struct scull_dev *dev;
    
    printk("Scull open\n");
    
    dev = container_of(inode->i_cdev, struct scull_dev, cdev);
    filp->private_data = dev;
    
    return 0;
}

static int scull_release(struct inode *inode, struct file *filp)
{
    printk("Scull release\n");
    
    return 0;
}

struct file_operations scull_fops = {
    .owner          = THIS_MODULE,
    .read           = scull_read,
    .write          = scull_write,
    .open           = scull_open,
    .release        = scull_release,
};

static int __init scull_init(void)
{
    int rc = 0;
    dev_t dev;
    struct cdev scull_cdev;
    int i;
    
    printk("Hello World!\n");
    
    // To get list of devices
    //      # cat /proc/devices
    
    // Major device static allocation
    if (major > 0)
    {
        dev = MKDEV(major, minor);
        
        rc = register_chrdev_region(dev, DEV_SCULL_NB, DEV_SCULL_NAME);
        if (rc != 0)
        {
            printk("register_chrdev_region failed: name=%s, major=%d, nb=%d, err=%d\n",
                DEV_SCULL_NAME, major, DEV_SCULL_NB, rc);
            goto fail_chrdev_region;
        }
    }
    // Major device dynamic allocation
    else
    {
        rc = alloc_chrdev_region(&dev, minor, DEV_SCULL_NB, DEV_SCULL_NAME);
        if (rc != 0)
        {
            printk("alloc_chrdev_region failed: name=%s, nb=%d, err=%d\n",
                DEV_SCULL_NAME, DEV_SCULL_NB, rc);
            goto fail_chrdev_region;
        }
        
        major = MAJOR(dev);
    }
    
    // Create an unique cdev for each minor scull device
    for (i=0; i<DEV_SCULL_NB; i++)
    {
        memset(&scull_devices[i], 0, sizeof(scull_devices[i]));
        
        scull_devices[i].quantum = SCULL_QUANTIUM;
        scull_devices[i].qset = SCULL_QSET;
        
        // We can also use cdev_alloc() to get a pointer struct cdev*
        cdev_init(&scull_devices[i].cdev, &scull_fops);    
        scull_cdev.owner = scull_fops.owner;
    
        rc = cdev_add(&scull_devices[i].cdev, MKDEV(major, minor+i), 1);
        if (rc != 0)
        {
            printk("cdev_add failed, err=%d\n", rc);
            goto fail_cdev_alloc;
        }
    }
    
    return 0;

fail_cdev_alloc:
    unregister_chrdev_region(dev, DEV_SCULL_NB);
    
fail_chrdev_region:

    return rc;
}

static void __exit scull_exit(void)
{
    // TODO cdev_del()
    
    unregister_chrdev_region(MKDEV(major, minor), DEV_SCULL_NB);
    
    printk("Bye!\n");
}

module_init(scull_init);
module_exit(scull_exit);

MODULE_LICENSE("GPL");
