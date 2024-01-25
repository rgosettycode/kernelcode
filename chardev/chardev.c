#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define MAX_DEV 3

static int chardev_open(struct inode *inode, struct file *file)
{
	printk("chardev: device open\n");
	return 0;
}

static int chardev_release(struct inode *inode, struct file *file)
{
	printk("chardev: device close\n");
	return 0;

}

static ssize_t chardev_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	char *data = "Hello from kernel world\n";
	size_t datalen = strlen(data) + 1;

	printk("Reading device: %d\n", MINOR(file->f_path.dentry->d_inode->i_rdev));
	if (size > datalen) {
		size = datalen;
	}
	if (copy_to_user(buf, data, size)) {
		return -EFAULT;
	}

	return size;
}

static ssize_t chardev_write(struct file *file, const char __user *buf, size_t size, loff_t *ppos)
{
	size_t maxdatalen = 30, ncopied;
	//char databuf[maxdatalen];
	char databuf[30];

	printk("writing device: %d\n", MINOR(file->f_path.dentry->d_inode->i_rdev));

	if (size < maxdatalen) {
		maxdatalen = size;
	}
	ncopied = copy_from_user(databuf, buf, maxdatalen);
	if(ncopied == 0) {
		printk("copied %zd bytes from the user\n", maxdatalen);
	} else {
		printk("couldn't copy %zd bytes from the user\n", ncopied);
	}

	databuf[maxdatalen] = '\0';

	printk("data from the user: %s\n", databuf);

	return size;
}

static long chardev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	printk("chardev: device ioctl\n");
	return 0;

}

static const struct file_operations chardev_fops = {
	.owner		= THIS_MODULE,
	.open		= chardev_open,
	.release	= chardev_release,
	.read		= chardev_read,
	.write		= chardev_write,
	.unlocked_ioctl = chardev_ioctl
};

struct mychardevice_data {
	struct cdev cdev;
};

static int dev_major = 0;
static struct class *mychardev_class = NULL;
static struct mychardevice_data mychardev_data[MAX_DEV];

static int chardev_uevent(struct device *dev, struct kobj_uevent_env *env)
{
	add_uevent_var(env, "DEVMODE=%#o", 0666);
	return 0;
}

static int __init drv_init(void)
{
	int err, i;
	dev_t dev;

	err = alloc_chrdev_region(&dev, 0, MAX_DEV, "mychardev");
	dev_major = MAJOR(dev);

	mychardev_class = class_create(THIS_MODULE, "mychardev");
	mychardev_class->dev_uevent = chardev_uevent;

	for (i = 0; i < MAX_DEV; i++) {
		cdev_init(&mychardev_data[i].cdev, &chardev_fops);
		mychardev_data[i].cdev.owner = THIS_MODULE;
		cdev_add(&mychardev_data[i].cdev, MKDEV(dev_major, i), 1);
		device_create(mychardev_class, NULL, MKDEV(dev_major, i), NULL, "mychardev-%d", i);
	}
	printk("chardev init\n");
	return 0;
}

static void __exit drv_exit(void)
{
	int i;

	for (i = 0; i < MAX_DEV; i++) {
		device_destroy(mychardev_class, MKDEV(dev_major, i));
	}
	
	class_unregister(mychardev_class);
	class_destroy(mychardev_class);

	unregister_chrdev_region(MKDEV(dev_major, 0), MINORMASK);

	printk("chardev exit\n");
}

module_init(drv_init);
module_exit(drv_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Char Device file operations");
MODULE_AUTHOR("Gosetty Rama Prasad <rgosetty@gmail.com>");
