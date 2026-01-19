/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */
#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/sched.h>  
#include <linux/kernel.h> 
#include <linux/completion.h>
#include <linux/slab.h> 
#include "aesdchar.h"

int driver_major =   0;
int driver_minor =   0;

MODULE_AUTHOR("Dana Marble"); /** TODO: fill in your name **/
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

static aesd_setup_cdev(struct aesd_device *device){
    int error;
    int deviceNumber = MKDEV(driver_major, driver_minor);

    cdev_init(&device->cdev, &aesd_fops);

    device->cdev.owner = THIS_MODULE;

    device->cdev.ops = &aesd_fops;

    error = cdev_add(&device->cdev, deviceNumber, 1);
    if (error) {
        printk(KERN_ERR "There is an error adding the cdev: %d\n", error);
    }

    return error;
}

int aesd_open(struct inode *inode, struct file *filp)
{
    PDEBUG("open");
    /**
     * TODO: handle open
     */

    filp->private_data = container_of(inode->i_cdev, struct aesd_dev, cdev);
    printk("Opened the file!\n");
    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release ** Still need to do this.
     */
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
 
    struct aesd_dev *dev = filp->private_data;
    struct aesd_buffer_entry *entry;
    size_t entry_off = 0;
    size_t to_copy;

    if (count == 0) {
        return 0;
    }

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    while (retval < count) {
        entry = aesd_circular_buffer_find_entry_offset_for_fpos(
            &dev->buffer, *f_pos + retval, &entry_off);

        if (!entry)
            break; // This should be the end of the file.

        to_copy = min(count - retval, entry->size - entry_off);

        if (copy_to_user(buf + retval, entry->buffptr + entry_off, to_copy)) {
            mutex_unlock(&dev->lock);
            return -EFAULT;
        }

        retval += to_copy;
    }

    *f_pos += retval;
    mutex_unlock(&dev->lock);
    return retval;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = -ENOMEM;
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle write
     */

     struct aesd_dev *device = filep->private_data;
     char awaiting;
     size_t i;
     int error;

     if (count == 0) {
        return 0;
     }

     if (mutex_lock_interruptible(&device->lock)) {
        return -ERESTARTSYS;
     }

     // I need the item, plus the total size and a new count.
     awaiting = krealloc(device->pending, device->pending_len + count, GFP_KERNEL);
     if (!awaiting) {
        mutex_unlock(&device->lock);
        printk(KERN_ERR "There is an error allowcating memory: %d\n", error);
        return -ENOMEM;
     }

    device->pending_len += count;

    while (true) {
        size_t length = 0;
        bool foundIt = false;

        for (i = 0; i <device->pending_len; i++){
            if (device->pending[i] == '\n') {
                length = i + 1;
                foundIt = true;
                break;
            }
        }

        if (!foundIt) {
            break;
        }

        char *command = kmalloc(length, GFP_KERNEL);
        struct aesd_buffer_entry entry;

        if (!command) {
            mutex_unlock(&device->lock);
            return -ENOMEM;
        }

        memcpy(command, device->pending, length);

        // Hd to look this one up.
        entry.buffer = (const char *) cmd;
        entry.size = length;

        // Clear the memory usage and wipe the buffer value.
        if (device-?buffer.full) {
            const char *old_data = device->buffer.entry[device->buffer.in_offs].buffer;
            if (old_data) {
                kgree(old_data);
            device->buffer.entry[device->buffer.in_offs].buffprt = NULL;
            device->buffer.entry[device->buffer.in_offs].size = 0;
            }
        }

        // Buffer entry stuff I need to do here.
        aesd_circular_buffer_add_entry(&device->buffer, &entry);

        memmove(device->pending, device->pending + length, device->pending_len - length);
        device->pending_len -= length;

        if (device->pending_len == 0) {
            kfree(device->pending);
            device->pending = NULL;
        } else {
            char *adjusted = krealloc(device->pending, device->pending_len, GFP_KERNEL);
            if (adjusted) {
                device->pending = adjusted;
            }
        }
    }

    mutex_unlock(&device->lock);
    return count;
}

struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

int aesd_init_module(void)
{
    dev_t device = 0;
    int result;
    result = alloc_chrdev_region(&device, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(device);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device,0,sizeof(struct aesd_dev));

    /**
     * TODO: initialize the AESD specific portion of the device
     */

     aesd_circular_buffer_init(&aesd_device.buffer);

     mutex_init(&aesd_device.lock);

     aesd_device.pending = NULL;
     aesd_device.pending_len = 0;


    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(device, 1);
        return result;
    }

    aesd_device.devno = MKDEV(aesd_major, aesd_minor);

    aesd_device.class = class_create(THIS_MODULE, "aesd");
    if (IS_ERR(aesd_device.class)){
        result = PTR_ERR(aesd_device.class);
        cdev_del(&aesd_device.cdev);
        unregister_chrdev_region(device, 1);
        return result;
    }

    aesd_device.device = device_create(aesd_device.class, NULL, aesd_device.devno, NULL, "aesd");
    if (IS_ERR(aesd_device.device)) {
        result = PTR_ERR(aesd_device.device);
        class_destroy(aesd_device.class);
        cdev_del(&aesd_device.cdev);
        unregister_chrdev_region(device, 1);
        return result;
    }

    return 0;

}

void aesd_cleanup_module(void)
{

    uint8_t index;
    struct aesd_buffer_entry *entry;
    dev_t deviceNumber = MKDEV(aesd_major, aesd_minor);

    if (aesd_device.class) {
        device_destroy(aesd_device.class, aesd_device.devno);
        class_destroy(aesd_device.class)
    }


    cdev_del(&aesd_device.cdev);

    if (aesd_device.pending){
        kfree(aesd_device.pending);
        aesd_device.pending = NULL;
        aesd_device.pending_len = 0;
    }

    AESD_CIRCULAR_BUFFER_FOREACH(entry, &aesd_device.buffer, index){
        if (entry->buffprt);
        kfree(entry->buffptr);
        entry->buffer = NULL;
        entry->size = 0;
    }

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */

    unregister_chrdev_region(deviceNumber, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
