#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_AUTHOR("visuve");
MODULE_DESCRIPTION("Lorem Ipsum generating character device.");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");

int lorem_ipsum_init(void);
int lorem_ipsum_open(struct inode*, struct file*);
int lorem_ipsum_read(struct file*, char __user*, size_t, loff_t*);
int lorem_ipsum_release(struct inode*, struct file*);
void lorem_ipsum_exit(void);

module_init(lorem_ipsum_init);
module_exit(lorem_ipsum_exit);

struct file_operations lorem_ipsum_operations =
{
	.owner = THIS_MODULE,
	.open = lorem_ipsum_open,
	.read = lorem_ipsum_read,
	.release = lorem_ipsum_release
};

const dev_t lorem_ipsum_device_number = MKDEV(0, 1);
const unsigned int lorem_ipsum_device_count = 1;

struct cdev* lorem_ipsum_device_data = NULL;

const char lorem_ipsum_class_name[] = "lorem_ipsum_device";
struct class* lorem_ipsum_class = NULL;

const char lorem_ipsum_device_name[] = "lorem_ipsum";
struct device* lorem_ipsum_device = NULL;

const char lorem_impsum_text[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
const size_t lorem_impsum_text_size = sizeof(lorem_impsum_text);

int lorem_ipsum_init(void)
{
	int error_code = register_chrdev_region(
		lorem_ipsum_device_number,
		lorem_ipsum_device_count,
		lorem_ipsum_device_name);

	if (error_code != 0)
	{
		return error_code;
	}

	cdev_init(lorem_ipsum_device_data, &lorem_ipsum_operations);
	cdev_add(lorem_ipsum_device_data, lorem_ipsum_device_number, 1);

	lorem_ipsum_class = class_create(THIS_MODULE, lorem_ipsum_class_name);

	lorem_ipsum_device = device_create(
		lorem_ipsum_class,
		NULL,
		lorem_ipsum_device_number,
		NULL,
		lorem_ipsum_device_name);

	return 0;
}

int lorem_ipsum_open(struct inode* inodep, struct file* filep)
{
	return 0;
}

int lorem_ipsum_read(struct file* file, char __user* user_buffer, size_t size, loff_t* offset)
{
	ssize_t length = min(lorem_impsum_text_size - *offset, size);

	if (length <= 0)
		return 0;

	if (copy_to_user(user_buffer, lorem_impsum_text + *offset, length))
	{
		return -EFAULT;
	}

	*offset += length;
	return length;
}

int lorem_ipsum_release(struct inode* inodep, struct file* filep)
{
	return 0;
}

void lorem_ipsum_exit(void)
{
	device_destroy(lorem_ipsum_class, lorem_ipsum_device_number);

	class_unregister(lorem_ipsum_class);
	class_destroy(lorem_ipsum_class);

	cdev_del(lorem_ipsum_device_data);

	unregister_chrdev_region(lorem_ipsum_device_number, lorem_ipsum_device_count);
}