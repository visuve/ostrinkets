#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("0BSD");
MODULE_AUTHOR("visuve");
MODULE_DESCRIPTION("Prints out Lorem Ipsum forever.");
MODULE_VERSION("0.1");

int __init lorem_ipsum_init(void)
{
	printk(KERN_INFO "Hello, World!\n");
	return 0;
}

void __exit lorem_ipsum_exit(void)
{
	printk(KERN_INFO "Goodbye, World!\n");
}

module_init(lorem_ipsum_init);
module_exit(lorem_ipsum_exit);