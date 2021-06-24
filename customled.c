#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/gpio/consumer.h> 

struct _customled_data {
	struct cdev cdev;
	uint8_t data;
	struct gpio_desc *gpio_pin;
};
typedef struct _customled_data cl_data;

static cl_data customled_data;
static struct class *cl;
static dev_t dev;


static int customled_open(struct inode *inode, struct file *file) {
	return 0;
}

static int customled_release(struct inode *inode, struct file *file) {
	return 0;
}

static ssize_t customled_read(struct file *file, char __user *user_buffer,
			      size_t size, loff_t *offset) {
	// Return current value of GPIO pin
	int val = customled_data.data;
	char buffer = val + '0';
	int len = 1 - *offset;

	// Don't return error if len smaller than 0
	if(len < 0)
		len = 0;

	// read data from buffer to user buffer
	if (copy_to_user(user_buffer, &buffer, len))
		return -EFAULT;

	*offset += len;
	return len;
}

static ssize_t customled_write(struct file *file, const char __user *user_buffer,
			       size_t size, loff_t *offset) {
	char *buffer = kmalloc(sizeof(char), GFP_KERNEL);;

	if(copy_from_user(buffer, user_buffer, 1)) {
		return -EFAULT;
	}

	// If correct buffer value, change LED state
	if (buffer[0] == '1' || buffer[0] == '0'){
		int val = *buffer - '0';
		gpiod_set_value(customled_data.gpio_pin, val);
		customled_data.data = val;
		printk(KERN_INFO "LED status: %d", val);
		return 2;
	}

	printk(KERN_ERR "customled: Wrong data, must be 1 or 0");
	return size;
}

const struct file_operations customled_fops = {
    .owner = THIS_MODULE,
    .open = customled_open,
    .release = customled_release,
    .write = customled_write,
    .read = customled_read
};

static int __init customled_init(void) {
	int ret;
	struct device *dev_ret;

	// Create character device region
	ret = alloc_chrdev_region(&dev, 0, 1, "customled");
	if (ret < 0) {
		return ret;
	}

	// Create class for sysfs
	cl = class_create(THIS_MODULE, "chardrv");
	if (IS_ERR(cl)) {
		unregister_chrdev_region(dev, 1);
		return PTR_ERR(cl);
	}

	// Create device for sysfs
	dev_ret = device_create(cl, NULL, dev, NULL, "customled");
	if (IS_ERR(dev_ret)) {
		class_destroy(cl);
		unregister_chrdev_region(dev, 1);
		return PTR_ERR(dev_ret);
	}

	// Get GPIO descriptor
	// customled_data.gpio_pin = gpiod_get_index(dev_ret, "customled", 0, GPIOD_OUT_LOW);
	customled_data.gpio_pin = gpio_to_desc(21);
	if (IS_ERR(customled_data.gpio_pin)) {
		device_destroy(cl, dev);
		class_destroy(cl);
		unregister_chrdev_region(dev, 1);
		printk(KERN_ERR "GPIO assignment error");
		return PTR_ERR(customled_data.gpio_pin);
	}
	customled_data.data = 0;

	// Create character device
	cdev_init(&customled_data.cdev, &customled_fops);
	ret = cdev_add(&customled_data.cdev, dev, 1);
	if (ret < 0) {
		gpiod_put(customled_data.gpio_pin);
		device_destroy(cl, dev);
		class_destroy(cl);
		unregister_chrdev_region(dev, 1);
		return ret;
	}

	printk(KERN_INFO "Custom LED driver initialized");
	return 0;
}

static void __exit customled_exit(void) {
	gpiod_put(customled_data.gpio_pin);
	device_destroy(cl, dev);
	class_destroy(cl);
	cdev_del(&customled_data.cdev);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "Custom LED driver removed");
}

module_init(customled_init);
module_exit(customled_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tristan Šneider <tristan.sneider@gmail.com>");
MODULE_DESCRIPTION("Custom LED driver");
