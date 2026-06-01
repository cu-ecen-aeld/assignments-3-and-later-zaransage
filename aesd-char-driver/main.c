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
#include <linux/slab.h>
#include <linux/device.h>
#include "aesd_ioctl.h"
#include "aesdchar.h"

int aesd_major = 0;
int aesd_minor = 0;

MODULE_AUTHOR("Dana Marble");
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;

static int aesd_setup_cdev(struct aesd_dev *dev);

int aesd_open(struct inode *inode, struct file *filep)
{
    PDEBUG("open");
    filep->private_data = container_of(inode->i_cdev, struct aesd_dev, cdev);
    return 0;
}

int aesd_release(struct inode *inode, struct file *filep)
{
    PDEBUG("release");
    return 0;
}

ssize_t aesd_read(struct file *filep, char __user *buf, size_t count, loff_t *f_pos)
{
    struct aesd_dev *dev;
    struct aesd_buffer_entry *entry;
    size_t entry_off = 0;
    size_t copied = 0;
    size_t to_copy;

    PDEBUG("read %zu bytes with offset %lld", count, *f_pos);

    if (count == 0)
        return 0;

    dev = (struct aesd_dev *)filep->private_data;

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    while (copied < count) {
        entry = aesd_circular_buffer_find_entry_offset_for_fpos(
            &dev->buffer, (size_t)(*f_pos) + copied, &entry_off);

        if (!entry)
            break;

        to_copy = min(count - copied, entry->size - entry_off);

        if (copy_to_user(buf + copied, entry->buffptr + entry_off, to_copy)) {
            mutex_unlock(&dev->lock);
            return -EFAULT;
        }

        copied += to_copy;
    }

    *f_pos += copied;
    mutex_unlock(&dev->lock);
    return copied;
}

ssize_t aesd_write(struct file *filep, const char __user *buf, size_t count, loff_t *f_pos)
{
    struct aesd_dev *dev;
    char *new_pending;
    size_t i;

    PDEBUG("write %zu bytes with offset %lld", count, *f_pos);

    (void)f_pos;

    if (count == 0)
        return 0;

    dev = (struct aesd_dev *)filep->private_data;

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    new_pending = krealloc(dev->pending, dev->pending_len + count, GFP_KERNEL);
    if (!new_pending) {
        mutex_unlock(&dev->lock);
        return -ENOMEM;
    }
    dev->pending = new_pending;

    if (copy_from_user(dev->pending + dev->pending_len, buf, count)) {
        mutex_unlock(&dev->lock);
        return -EFAULT;
    }
    dev->pending_len += count;

    while (1) {
        size_t cmd_len = 0;
        bool found = false;

        for (i = 0; i < dev->pending_len; i++) {
            if (dev->pending[i] == '\n') {
                cmd_len = i + 1;
                found = true;
                break;
            }
        }
        if (!found)
            break;
        {
            char *cmd;
            struct aesd_buffer_entry entry;
            const char *old;

            cmd = kmalloc(cmd_len, GFP_KERNEL);
            if (!cmd) {
                mutex_unlock(&dev->lock);
                return -ENOMEM;
            }
            memcpy(cmd, dev->pending, cmd_len);

            entry.buffptr = (const char *)cmd;
            entry.size = cmd_len;

            if (dev->buffer.full) {
                old = dev->buffer.entry[dev->buffer.in_offs].buffptr;
                if (old)
                    kfree(old);
                dev->buffer.entry[dev->buffer.in_offs].buffptr = NULL;
                dev->buffer.entry[dev->buffer.in_offs].size = 0;
            }

            aesd_circular_buffer_add_entry(&dev->buffer, &entry);

            memmove(dev->pending, dev->pending + cmd_len, dev->pending_len - cmd_len);
            dev->pending_len -= cmd_len;

            if (dev->pending_len == 0) {
                kfree(dev->pending);
                dev->pending = NULL;
            } else {
                char *shrunk = krealloc(dev->pending, dev->pending_len, GFP_KERNEL);
                if (shrunk)
                    dev->pending = shrunk;
            }
        }
    }

    mutex_unlock(&dev->lock);
    return count;
}

//Apparently the book is much older than this definition: /usr/src/linux-headers-5.4.0-216/include/linux/fs.h
long aesd_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {


    size_t total_entries;
    size_t i;
    loff_t new_position = 0;

    // I need to handle locking and mutex etc
    struct aesd_dev *dev = filep->private_data;

    struct aesd_seekto seekto;

    if (copy_from_user(&seekto, (struct aesd_seekto __user *) arg, sizeof(seekto))) {
        return -EFAULT;
    }

    if (mutex_lock_interruptible(&dev->lock)) {
        return -ERESTARTSYS;
    }

    switch(cmd){
        case AESDCHAR_IOCSEEKTO:
            // Let me capture the bytes here and calculate.

            if (dev->buffer.full) {
                total_entries = AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
            } else {
                // Give me the difference of the 'in' vs 'out' offset and total size, then give me a modulous of 2 
                // (if even) or of the actual size of the queue for dynamic allocation.
                total_entries = ((dev->buffer.in_offs - dev->buffer.out_offs) + AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
            }

            if (seekto.write_cmd >= total_entries) {
                mutex_unlock(&dev->lock);
                return -EINVAL;
            }

            // Okay, for all write commands, lets use the offset for the position we need.
            for (i = 0; i < seekto.write_cmd; i++) {
                size_t my_index  = (dev->buffer.out_offs + i) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
                new_position += dev->buffer.entry[my_index].size;
            }

            size_t write_index = (dev->buffer.out_offs + seekto.write_cmd) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;

            if (seekto.write_cmd_offset >= dev->buffer.entry[write_index].size) {
                mutex_unlock(&dev->lock);
                return -EINVAL;
            }

            filep->f_pos = new_position;

            break;

        default:
            mutex_unlock(&dev->lock);
            return -ENOTTY;
    }

    mutex_unlock(&dev->lock);
    return 0;

}

loff_t aesd_llseek(struct file *filep, loff_t offset, int whence){

    // I need to handle locking and mutex here.

    struct aesd_dev *dev = filep->private_data;

    loff_t newpos;

    if (mutex_lock_interruptible(&dev->lock)) {
        return -ERESTARTSYS;
    }

    switch(whence){
        case 0: //seek set
          newpos = offset;
        break;

        case 1: //seek cur
          newpos = filep->f_pos + offset;
        break;

        case 2: // seek end
        {
        // Let me reuse some of the core logic from aesd-circular-buffer.h
          loff_t total_size = 0;
          size_t index;
          struct aesd_buffer_entry *entryptr;
        
          AESD_CIRCULAR_BUFFER_FOREACH(entryptr, &dev->buffer, index) {
              if (entryptr->buffptr){
                  total_size += entryptr->size;
              }
          }
            newpos = total_size + offset;
            break;
        }
                          
        default: // error
          mutex_unlock(&dev->lock);
        return -EINVAL;
        }
 
    if (newpos < 0) {
      mutex_unlock(&dev->lock);
      return -EINVAL;
      }

      filep->f_pos = newpos;
    
      mutex_unlock(&dev->lock);
    
    return newpos;

}

struct file_operations aesd_fops = {
    .owner          = THIS_MODULE,
    .read           = aesd_read,
    .write          = aesd_write,
    .open           = aesd_open,
    .release        = aesd_release,
    .unlocked_ioctl = aesd_ioctl,
    .llseek         = aesd_llseek,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err;
    int devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;

    err = cdev_add(&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev\n", err);
    }
    return err;
}

int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;

    result = alloc_chrdev_region(&dev, aesd_minor, 1, "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }

    memset(&aesd_device, 0, sizeof(struct aesd_dev));

    aesd_circular_buffer_init(&aesd_device.buffer);
    mutex_init(&aesd_device.lock);
    aesd_device.pending = NULL;
    aesd_device.pending_len = 0;

    result = aesd_setup_cdev(&aesd_device);
    if (result) {
        unregister_chrdev_region(dev, 1);
        return result;
    }

    aesd_device.devno = MKDEV(aesd_major, aesd_minor);

    aesd_device.class = class_create(THIS_MODULE, "aesdchar");
    if (IS_ERR(aesd_device.class)) {
        result = PTR_ERR(aesd_device.class);
        cdev_del(&aesd_device.cdev);
        unregister_chrdev_region(dev, 1);
        return result;
    }

    aesd_device.device = device_create(aesd_device.class, NULL, aesd_device.devno, NULL, "aesdchar");
    if (IS_ERR(aesd_device.device)) {
        result = PTR_ERR(aesd_device.device);
        class_destroy(aesd_device.class);
        cdev_del(&aesd_device.cdev);
        unregister_chrdev_region(dev, 1);
        return result;
    }

    return 0;
}

void aesd_cleanup_module(void)
{
    size_t index;
    struct aesd_buffer_entry *entry;
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    if (aesd_device.class) {
        device_destroy(aesd_device.class, aesd_device.devno);
        class_destroy(aesd_device.class);
    }

    cdev_del(&aesd_device.cdev);

    if (aesd_device.pending) {
        kfree(aesd_device.pending);
        aesd_device.pending = NULL;
        aesd_device.pending_len = 0;
    }

    AESD_CIRCULAR_BUFFER_FOREACH(entry, &aesd_device.buffer, index) {
        if (entry->buffptr) {
            kfree(entry->buffptr);
            entry->buffptr = NULL;
            entry->size = 0;
        }
    }

    unregister_chrdev_region(devno, 1);
}

module_init(aesd_init_module);
module_exit(aesd_cleanup_module);