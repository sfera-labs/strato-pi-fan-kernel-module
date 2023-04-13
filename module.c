/*
 * Strato Pi Fan kernel module
 *
 *     Copyright (C) 2021-2023 Sfera Labs S.r.l.
 *
 *     For information, visit https://www.sferalabs.cc
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * LICENSE.txt file for more details.
 *
 */

#include <linux/delay.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/time.h>
#include <linux/i2c.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sfera Labs - http://sferalabs.cc");
MODULE_DESCRIPTION("Strato Pi Fan driver module");
MODULE_VERSION("1.1");

struct DeviceAttrBean {
	struct device_attribute devAttr;
	uint8_t reg;
	uint8_t lsbMask;
};

struct DeviceBean {
	char *name;
	struct device *pDevice;
	struct DeviceAttrBean *devAttrBeans;
};

static struct class *pDeviceClass;

static ssize_t devAttrLm75a_show(struct device* dev,
		struct device_attribute* attr, char *buf);

static ssize_t devAttrLm75a_store(struct device* dev,
		struct device_attribute* attr, const char *buf, size_t count);

struct i2c_client *lm75a_i2c_client = NULL;
struct mutex stratopifan_i2c_mutex;

static struct DeviceAttrBean devAttrBeansSysTemp[] = {
	{
		.devAttr = {
			.attr = {
				.name = "temp",
				.mode = 0440,
			},
			.show = devAttrLm75a_show,
			.store = NULL,
		},
		.reg = 0,
		.lsbMask = 0xe0,
	},

	{ }
};

static struct DeviceAttrBean devAttrBeansFan[] = {
	{
		.devAttr = {
			.attr = {
				.name = "temp_on",
				.mode = 0660,
			},
			.show = devAttrLm75a_show,
			.store = devAttrLm75a_store,
		},
		.reg = 3,
		.lsbMask = 0x80,
	},

	{
		.devAttr = {
			.attr = {
				.name = "temp_off",
				.mode = 0660,
			},
			.show = devAttrLm75a_show,
			.store = devAttrLm75a_store,
		},
		.reg = 2,
		.lsbMask = 0x80,
	},

	{ }
};

static struct DeviceBean devices[] = {
	{
		.name = "sys_temp",
		.devAttrBeans = devAttrBeansSysTemp,
	},

	{
		.name = "fan",
		.devAttrBeans = devAttrBeansFan,
	},

	{ }
};

static struct DeviceAttrBean* devAttrGetBean(struct device* dev,
		struct device_attribute* attr) {
	int di, ai;
	di = 0;
	while (devices[di].name != NULL) {
		if (dev == devices[di].pDevice) {
			ai = 0;
			while (devices[di].devAttrBeans[ai].devAttr.attr.name != NULL) {
				if (attr == &devices[di].devAttrBeans[ai].devAttr) {
					return &devices[di].devAttrBeans[ai];
				}
				ai++;
			}
			break;
		}
		di++;
	}
	return NULL;
}

static bool stratopifan_i2c_lock(void) {
	uint8_t i;
	for (i = 0; i < 20; i++) {
		if (mutex_trylock(&stratopifan_i2c_mutex)) {
			return true;
		}
		msleep(1);
	}
	return false;
}

static void stratopifan_i2c_unlock(void) {
	mutex_unlock(&stratopifan_i2c_mutex);
}

static ssize_t devAttrLm75a_show(struct device* dev,
		struct device_attribute* attr, char *buf) {
	int32_t res;
	struct DeviceAttrBean* dab;
	dab = devAttrGetBean(dev, attr);
	if (dab == NULL) {
		return -EFAULT;
	}

	if (lm75a_i2c_client == NULL) {
		return -ENODEV;
	}

	if (!stratopifan_i2c_lock()) {
		return -EBUSY;
	}

	res = i2c_smbus_read_word_data(lm75a_i2c_client, dab->reg);

	stratopifan_i2c_unlock();

	if (res < 0) {
		return res;
	}

	res = ((res & 0xff) << 8) + ((res >> 8) & dab->lsbMask);
	res = ((int16_t) res) * 100 / 256;

	return sprintf(buf, "%d\n", res);
}

static ssize_t devAttrLm75a_store(struct device* dev,
		struct device_attribute* attr, const char *buf, size_t count) {
	int32_t res;
	long temp;
	struct DeviceAttrBean* dab;
	dab = devAttrGetBean(dev, attr);
	if (dab == NULL) {
		return -EFAULT;
	}

	if (lm75a_i2c_client == NULL) {
		return -ENODEV;
	}

	res = kstrtol(buf, 10, &temp);
	if (res < 0) {
		return res;
	}
	if (temp > 12750) {
		return -EINVAL;
	}
	if (temp < -12800) {
		return -EINVAL;
	}

	temp = temp * 256 / 100;
	temp = ((temp & dab->lsbMask) << 8) + ((temp >> 8) & 0xff);

	if (!stratopifan_i2c_lock()) {
		return -EBUSY;
	}

	res = i2c_smbus_write_word_data(lm75a_i2c_client, dab->reg, temp);

	stratopifan_i2c_unlock();

	if (res < 0) {
		return res;
	}

	return count;
}

static int stratopifan_i2c_probe(struct i2c_client *client,
		const struct i2c_device_id *id) {
	lm75a_i2c_client = client;
	printk(KERN_INFO "stratopifan: - | i2c probe addr 0x%02hx\n", client->addr);
	return 0;
}

const struct of_device_id stratopifan_of_match[] = {
	{ .compatible = "sferalabs,stratopifan", },
	{ },
};
MODULE_DEVICE_TABLE(of, stratopifan_of_match);

static const struct i2c_device_id stratopifan_i2c_id[] = {
	{ "stratopifan", 0 },
	{ },
};
MODULE_DEVICE_TABLE(i2c, stratopifan_i2c_id);

static struct i2c_driver stratopifan_i2c_driver = {
	.driver = {
		.name = "stratopifan",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(stratopifan_of_match),
	},
	.probe = stratopifan_i2c_probe,
	.id_table = stratopifan_i2c_id,
};

static void cleanup(void) {
	int di, ai;

	i2c_del_driver(&stratopifan_i2c_driver);
	mutex_destroy(&stratopifan_i2c_mutex);

	di = 0;
	while (devices[di].name != NULL) {
		if (devices[di].pDevice && !IS_ERR(devices[di].pDevice)) {
			ai = 0;
			while (devices[di].devAttrBeans[ai].devAttr.attr.name != NULL) {
				device_remove_file(devices[di].pDevice,
						&devices[di].devAttrBeans[ai].devAttr);
				ai++;
			}
		}
		device_destroy(pDeviceClass, 0);
		di++;
	}

	if (!IS_ERR(pDeviceClass)) {
		class_destroy(pDeviceClass);
	}
}

static int __init stratopifan_init(void) {
	int result = 0;
	int di, ai;

	printk(KERN_INFO "stratopifan: - | init\n");

	i2c_add_driver(&stratopifan_i2c_driver);

	pDeviceClass = class_create(THIS_MODULE, "stratopifan");
	if (IS_ERR(pDeviceClass)) {
		printk(KERN_ALERT "stratopifan: * | failed to create device class\n");
		goto fail;
	}

	di = 0;
	while (devices[di].name != NULL) {
		devices[di].pDevice = device_create(pDeviceClass, NULL, 0, NULL,
				devices[di].name);
		if (IS_ERR(devices[di].pDevice)) {
			printk(KERN_ALERT "stratopifan: * | failed to create device '%s'\n",
					devices[di].name);
			goto fail;
		}

		ai = 0;
		while (devices[di].devAttrBeans[ai].devAttr.attr.name != NULL) {
			result = device_create_file(devices[di].pDevice,
					&devices[di].devAttrBeans[ai].devAttr);
			if (result) {
				printk(
						KERN_ALERT "stratopifan: * | failed to create device file '%s/%s'\n",
						devices[di].name,
						devices[di].devAttrBeans[ai].devAttr.attr.name);
				goto fail;
			}
			ai++;
		}
		di++;
	}

	printk(KERN_INFO "stratopifan: - | ready\n");
	return 0;

	fail:
	printk(KERN_ALERT "stratopifan: * | init failed\n");
	cleanup();
	return -1;
}

static void __exit stratopifan_exit(void) {
	cleanup();
	printk(KERN_INFO "stratopifan: - | exit\n");
}

module_init(stratopifan_init);
module_exit(stratopifan_exit);
