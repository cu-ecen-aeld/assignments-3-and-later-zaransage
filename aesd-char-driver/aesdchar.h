/*
 * aesdchar.h
 *
 *  Created on: Oct 23, 2019
 *      Author: Dan Walkes
 */

#ifndef AESD_CHAR_DRIVER_AESDCHAR_H_
#define AESD_CHAR_DRIVER_AESDCHAR_H_

#ifdef __KERNEL__
#include <linux/cdev.h>
#include <linux/printk.h>
#include <linux/mutex.h>
#include <linux/device.h>
#include <linux/types.h>
#else
#include <stdio.h>
#endif

#include "aesd-circular-buffer.h"

#define AESD_DEBUG 1  // Remove comment on this line to enable debug

#undef PDEBUG
#ifdef AESD_DEBUG
#  ifdef __KERNEL__
#    define PDEBUG(fmt, args...) printk(KERN_DEBUG "aesdchar: " fmt, ## args)
#  else
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) do { } while (0)
#endif

struct aesd_dev
{
    struct cdev cdev;

    struct aesd_circular_buffer buffer;
    struct mutex lock;

    char *pending;
    size_t pending_len;

    dev_t devno;
    struct class *class;
    struct device *device;
};

extern struct file_operations aesd_fops;

int aesd_open(struct inode *inode, struct file *filp);
int aesd_release(struct inode *inode, struct file *filp);
ssize_t aesd_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos);
ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos);

#endif /* AESD_CHAR_DRIVER_AESDCHAR_H_ */