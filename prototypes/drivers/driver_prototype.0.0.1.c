#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Dana Marble");
MODULE_DESCRIPTION("First setup of a Linux Driver");


static int driver_module_init (void){
    printk("Hello World");
    return 0;
}

static void driver_module_exit (void){
    printk("Goodbye!");
}

module_init(driver_module_init);
module_exit(driver_module_exit);