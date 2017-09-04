/*
 * The simplest kernel module example from LDD
 */
#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("Dual BSD/GPL");

static char *str = "world";
static int iter = 1;
module_param(iter, int, S_IRUGO);
module_param(str, charp, S_IRUGO);

static int __init hello_init(void)
{
    int i;

    for (i = 0; i < iter; i++)
        printk(KERN_ALERT "%s", str);

    printk(KERN_ALERT "Hello, world\n");
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_ALERT "Goodbyte, cruel wold\n");
}

module_init(hello_init);
module_exit(hello_exit);
