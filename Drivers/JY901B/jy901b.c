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

#define JY901B_CNT	1
#define JY901B_NAME	"jy901b"

struct jy901b_dev {
	dev_t devid;			/* 设备号 	 */
	struct cdev cdev;		/* cdev 	*/
	struct class* class;	/* 类 		*/
	struct device* device;	/* 设备 	 */
	struct device_node* nd; /* 设备节点 */
	int major;			/* 主设备号 */
	void* private_data;	/* 私有数据 */
	unsigned short ir, als, ps;		/* 三个光传感器数据 */
};

static struct jy901b_dev jy901bdev;

static int S_WriteData(struct jy901b_dev* dev, u8* data, u8 length) {
	struct i2c_client* client = (struct i2c_client*)dev->private_data;

	return i2c_master_send(client, data, length);
}

static int S_ReadData(struct jy901b_dev* dev, u8* data, u32 length) {
	struct i2c_client* client = (struct i2c_client*)dev->private_data;

	return i2c_master_recv(client, data, length);
}
static int S_Open(struct inode* inode, struct file* filp)
{
	filp->private_data = &jy901bdev;
	return 0;
}

static ssize_t S_Read(struct file* filp, char __user* buf, size_t cnt, loff_t* off)
{
	struct jy901b_dev* dev = (struct jy901b_dev*)filp->private_data;
	u8 data[256] = {};
	int length = 0;
	if (cnt > sizeof(data))cnt = sizeof(data);

	length = S_ReadData(dev, data, cnt);

	copy_to_user(buf, data, cnt);

	return length;
}

static ssize_t S_Write(struct file* filp, const char __user* buf, size_t cnt, loff_t* off) {
	struct jy901b_dev* dev = (struct jy901b_dev*)filp->private_data;
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

static const struct file_operations jy901b_ops = {
	.owner = THIS_MODULE,
	.open = S_Open,
	.read = S_Read,
	.write = S_Write,
	.release = S_Release,
};


static int jy901b_probe(struct i2c_client* client, const struct i2c_device_id* id)
{

	/* 1、构建设备号 */
	if (jy901bdev.major) {
		jy901bdev.devid = MKDEV(jy901bdev.major, 0);
		register_chrdev_region(jy901bdev.devid, JY901B_CNT, JY901B_NAME);
	}
	else {
		alloc_chrdev_region(&jy901bdev.devid, 0, JY901B_CNT, JY901B_NAME);
		jy901bdev.major = MAJOR(jy901bdev.devid);
	}

	/* 2、注册设备 */
	cdev_init(&jy901bdev.cdev, &jy901b_ops);
	cdev_add(&jy901bdev.cdev, jy901bdev.devid, JY901B_CNT);

	/* 3、创建类 */
	jy901bdev.class = class_create(THIS_MODULE, JY901B_NAME);
	if (IS_ERR(jy901bdev.class)) {
		printk("jy901b matching failed!\n");
		return PTR_ERR(jy901bdev.class);
	}

	/* 4、创建设备 */
	jy901bdev.device = device_create(jy901bdev.class, NULL, jy901bdev.devid, NULL, JY901B_NAME);
	if (IS_ERR(jy901bdev.device)) {
		printk("jy901b matching failed!\n");
		return PTR_ERR(jy901bdev.device);
	}

	jy901bdev.private_data = client;
	printk("jy901b matching device!\n");
	return 0;
}

static int jy901b_remove(struct i2c_client* client)
{
	/* 删除设备 */
	cdev_del(&jy901bdev.cdev);
	unregister_chrdev_region(jy901bdev.devid, JY901B_CNT);

	/* 注销掉类和设备 */
	device_destroy(jy901bdev.class, jy901bdev.devid);
	class_destroy(jy901bdev.class);
	return 0;
}

static const struct i2c_device_id jy901b_id[] = {
	{"nagisa,jy901b", 0},
	{}
};

static const struct of_device_id jy901b_of_match[] = {
	{.compatible = "nagisa,jy901b" },
	{}
};

static struct i2c_driver jy901b_driver = {
	.probe = jy901b_probe,
	.remove = jy901b_remove,
	.driver = {
			.owner = THIS_MODULE,
			.name = "jy901b",
			.of_match_table = jy901b_of_match,
		},
	.id_table = jy901b_id,
};

static int __init jy901b_init(void)
{
	int ret = 0;

	ret = i2c_add_driver(&jy901b_driver);
	return ret;
}

static void __exit jy901b_exit(void)
{
	i2c_del_driver(&jy901b_driver);
}

module_init(jy901b_init);
module_exit(jy901b_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("nagisa");