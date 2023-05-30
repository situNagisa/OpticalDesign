#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ide.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/gpio.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_gpio.h>
#include <linux/semaphore.h>
#include <linux/timer.h>
#include <linux/i2c.h>
#include <asm/mach/map.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#include "Register.h"


#define ESP32_CNT	1
#define ESP32_NAME	"esp32"

struct esp32_dev {
	dev_t devid;			/* 设备号 	 */
	struct cdev cdev;		/* cdev 	*/
	struct class* class;	/* 类 		*/
	struct device* device;	/* 设备 	 */
	struct device_node* nd; /* 设备节点 */
	int major;			/* 主设备号 */
	void* private_data;	/* 私有数据 */
	unsigned short ir, als, ps;		/* 三个光传感器数据 */
};

static struct esp32_dev esp32dev;

static int S_WriteData(struct esp32_dev* dev, u8* data, u8 length) {
	struct i2c_client* client = (struct i2c_client*)dev->private_data;

	return i2c_master_send(client, data, length);
}

static int S_ReadData(struct esp32_dev* dev, u8* data, u32 length) {
	struct i2c_client* client = (struct i2c_client*)dev->private_data;

	return i2c_master_recv(client, data, length);
}
static int S_Open(struct inode* inode, struct file* filp)
{
	filp->private_data = &esp32dev;
	return 0;
}

static ssize_t S_Read(struct file* filp, char __user* buf, size_t cnt, loff_t* off)
{
	struct esp32_dev* dev = (struct esp32_dev*)filp->private_data;
	u8 data[256] = {};
	int length = 0;
	if (cnt > sizeof(data))cnt = sizeof(data);

	length = S_ReadData(dev, data, cnt);

	copy_to_user(buf, data, cnt);

	return length;
}

static ssize_t S_Write(struct file* filp, const char __user* buf, size_t cnt, loff_t* off) {
	struct esp32_dev* dev = (struct esp32_dev*)filp->private_data;
	u8 data[256] = {};
	long err = 0;

	if (cnt > sizeof(data))cnt = sizeof(data);
	err = copy_from_user(data, buf, cnt);

	return S_WriteData(dev, data, cnt);
}

static int S_Release(struct inode* inode, struct file* filp)
{
	return 0;
}

static const struct file_operations esp32_ops = {
	.owner = THIS_MODULE,
	.open = S_Open,
	.read = S_Read,
	.write = S_Write,
	.release = S_Release,
};


static int esp32_probe(struct i2c_client* client, const struct i2c_device_id* id)
{

	/* 1、构建设备号 */
	if (esp32dev.major) {
		esp32dev.devid = MKDEV(esp32dev.major, 0);
		register_chrdev_region(esp32dev.devid, ESP32_CNT, ESP32_NAME);
	}
	else {
		alloc_chrdev_region(&esp32dev.devid, 0, ESP32_CNT, ESP32_NAME);
		esp32dev.major = MAJOR(esp32dev.devid);
	}

	/* 2、注册设备 */
	cdev_init(&esp32dev.cdev, &esp32_ops);
	cdev_add(&esp32dev.cdev, esp32dev.devid, ESP32_CNT);

	/* 3、创建类 */
	esp32dev.class = class_create(THIS_MODULE, ESP32_NAME);
	if (IS_ERR(esp32dev.class)) {
		printk("esp32 matching failed!\n");
		return PTR_ERR(esp32dev.class);
	}

	/* 4、创建设备 */
	esp32dev.device = device_create(esp32dev.class, NULL, esp32dev.devid, NULL, ESP32_NAME);
	if (IS_ERR(esp32dev.device)) {
		printk("esp32 matching failed!\n");
		return PTR_ERR(esp32dev.device);
	}

	esp32dev.private_data = client;
	printk("esp32 matching device!\n");
	return 0;
}

static int esp32_remove(struct i2c_client* client)
{
	/* 删除设备 */
	cdev_del(&esp32dev.cdev);
	unregister_chrdev_region(esp32dev.devid, ESP32_CNT);

	/* 注销掉类和设备 */
	device_destroy(esp32dev.class, esp32dev.devid);
	class_destroy(esp32dev.class);
	return 0;
}

static const struct i2c_device_id esp32_id[] = {
	{"nagisa,esp32", 0},
	{}
};

static const struct of_device_id esp32_of_match[] = {
	{.compatible = "nagisa,esp32" },
	{}
};

static struct i2c_driver esp32_driver = {
	.probe = esp32_probe,
	.remove = esp32_remove,
	.driver = {
			.owner = THIS_MODULE,
			.name = "esp32",
			.of_match_table = esp32_of_match,
		},
	.id_table = esp32_id,
};

static int __init esp32_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&esp32_driver);
	return ret;
}

static void __exit esp32_exit(void)
{
	i2c_del_driver(&esp32_driver);
}

module_init(esp32_init);
module_exit(esp32_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("nagisa");



