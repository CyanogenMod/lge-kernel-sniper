/* * drivers/lg_fw/hub_modem_ctrl.c *
 * Copyright (C) 2010 LGE Inc. <james.jang@lge.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License. See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <mach/gpio.h>
#include <plat/mux.h>
#include <mach/hardware.h>

#define DEBUG 0
#define FEATURE_MODEM_RESET_CTRL 1
#define FEATURE_MODEM_POWER_CTRL 1

#ifdef CONFIG_LGE_LAB3_BOARD

#define RESET_MDM 26
#define MDM_PWR_ON 27

#define MODEM_RESET_CTRL_GPIO RESET_MDM
#define MODEM_POWER_CTRL_GPIO MDM_PWR_ON

#else /* DCM */
#define MODEM_RESET_CTRL_GPIO 26
#define MODEM_POWER_CTRL_GPIO 27
#endif

/* sysfs info. */
/*
	/sys/devices/platform/modem_ctrl.0/power_ctrl
	/sys/devices/platform/modem_ctrl.0/resetr_ctrl
	/sys/devices/platform/modem_ctrl.0/uart_path
*/

struct modem_ctrl_struct {
	int power_ctrl;
	int reset_ctrl;
	int uart_path;
};

static struct modem_ctrl_struct modem_ctrl;
static int initialized = 0;

//#ifdef CONFIG_HUB_MUIC	// LGE_DEL 20121106 subum.choi@lge.com [FOTA] 
typedef enum {
	USIF_AP,	// 0
	USIF_DP3T,	// 1
} TYPE_USIF_MODE;
void usif_switch_ctrl(TYPE_USIF_MODE mode);
//#endif

#define ADDR_GPIO_CTRL_1    IO_ADDRESS(0x48310030)
#define ADDR_GPIO_OE_1      IO_ADDRESS(0x48310034)
#define ADDR_GPIO_DATAIN_1  IO_ADDRESS(0x48310038)
#define ADDR_GPIO_DATAOUT_1 IO_ADDRESS(0x4831003C)

void dump_gpio_status(char *s, int d)
{
    // XXX: 20060617 temporarily disabled
#if 0
        u32 reg_value = 0;

        printk("============================================================\n");
        printk("%s: %d\n", s, d);
        printk("------------------------------------------------------------\n");
        reg_value = __raw_readl(ADDR_GPIO_CTRL_1);
        printk(" GPIO_CTRL_1:    0x%x\n", reg_value);
        reg_value = __raw_readl(ADDR_GPIO_OE_1);
        printk(" GPIO_OE_1:      0x%x\n", reg_value);
        reg_value = __raw_readl(ADDR_GPIO_DATAIN_1);
        printk(" GPIO_DATAIN_1:  0x%x\n", reg_value);
        reg_value = __raw_readl(ADDR_GPIO_DATAOUT_1);
        printk(" GPIO_DATAOUT_1: 0x%x\n", reg_value);
        printk("============================================================\n");
#endif
}

/**
 * export function
 * XXX: remove?

int hub_modem_on(void)
{
	if (initialized == 0)
		return -EAGAIN;

	gpio_set_value(MODEM_POWER_CTRL_GPIO, 1);
	modem_ctrl.power_ctrl = 1;

	return 0;
}

EXPORT_SYMBOL(hub_modem_on);

int hub_modem_poweroff(void)
{
	if (initialized == 0)
		return -EAGAIN;

	gpio_set_value(MODEM_POWER_CTRL_GPIO, 0);
	modem_ctrl.power_ctrl = 0;

	return 0;
}
EXPORT_SYMBOL(hub_modem_poweroff);
 */

/** 
 * sysfs
 */

static ssize_t modem_power_ctrl_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s: power switch is %d\n", __FUNCTION__,
		       modem_ctrl.power_ctrl);
}

static ssize_t modem_power_ctrl_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
        int new_power_ctrl;

	if (count == 0)
		return 0;

	new_power_ctrl = simple_strtol(buf, NULL, 10);

	if (modem_ctrl.power_ctrl != new_power_ctrl) {
		modem_ctrl.power_ctrl = new_power_ctrl;

		dump_gpio_status(__FUNCTION__, 1);
		//omap_mux_config("GPIO26_MODEM_POWER_CTRL_GPIO"); // XXX: 20060617 temporarily disabled
		gpio_set_value(MODEM_POWER_CTRL_GPIO, modem_ctrl.power_ctrl);
		gpio_direction_output(MODEM_POWER_CTRL_GPIO, modem_ctrl.power_ctrl);
		dump_gpio_status(__FUNCTION__, 2);
	}
	printk("%s: power switch is %d\n", __FUNCTION__, modem_ctrl.power_ctrl);

	return count;
}

DEVICE_ATTR(power_ctrl, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
	    modem_power_ctrl_show, modem_power_ctrl_store);

static ssize_t modem_reset_ctrl_show(struct device *dev,
				     struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%s: reset switch is %d\n", __FUNCTION__,
		       modem_ctrl.reset_ctrl);
}

static ssize_t modem_reset_ctrl_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
        int new_reset_ctrl;

	if (count == 0)
		return 0;

        new_reset_ctrl = simple_strtol(buf, NULL, 10);

	if (modem_ctrl.reset_ctrl != new_reset_ctrl) {
		modem_ctrl.reset_ctrl = new_reset_ctrl;

                dump_gpio_status(__FUNCTION__, 1);
                //omap_mux_config("GPIO26_MODEM_RESET_CTRL_GPIO"); // XXX: 20060617 temporarily disabled
		gpio_set_value(MODEM_RESET_CTRL_GPIO, modem_ctrl.reset_ctrl);
		gpio_direction_output(MODEM_RESET_CTRL_GPIO, modem_ctrl.reset_ctrl);
                dump_gpio_status(__FUNCTION__, 2);
	}

	printk("%s: reset switch is %d\n", __FUNCTION__,
	       modem_ctrl.reset_ctrl);
	return count;
}

DEVICE_ATTR(reset_ctrl, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
	    modem_reset_ctrl_show, modem_reset_ctrl_store);

static ssize_t modem_uart_path_show(struct device *dev,
				    struct device_attribute *attr, char *buf)
{
	if (modem_ctrl.uart_path == 0) {
		return sprintf(buf, "%s: set to the OMAP UART\n", __FUNCTION__);
	} else {
		return sprintf(buf, "%s: set to the external connector\n",
			       __FUNCTION__);
	}
}

static ssize_t modem_uart_path_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	if (count == 0)
		return 0;

	modem_ctrl.uart_path = simple_strtol(buf, NULL, 10);
	modem_ctrl.uart_path = !!modem_ctrl.uart_path;
//#ifdef CONFIG_HUB_MUIC		// LGE_DEL 20121106 subum.choi@lge.com [FOTA] 
	if (modem_ctrl.uart_path == 0) {
                usif_switch_ctrl(USIF_AP);
		printk("switch to the OMAP UART\n");
	} else {
                usif_switch_ctrl(USIF_DP3T);
		printk("switch to the external connector\n");
	}
//#endif
	return count;
}

DEVICE_ATTR(uart_path, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP,
	    modem_uart_path_show, modem_uart_path_store);

/**
 * platform driver 
 */
static int modem_ctrl_probe(struct platform_device *pdev)
{
	int ret = 0;

#if DEBUG
	dump_gpio_status(__FUNCTION__, 1);
#endif /* DEBUG */
	/* request gpio */
#if FEATURE_MODEM_POWER_CTRL
	ret = gpio_request(MODEM_POWER_CTRL_GPIO, "modem power switch");
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to request gpio %d\n",
			MODEM_POWER_CTRL_GPIO);
		ret = -EBUSY;
		goto err_gpio_request_power_ctrl;
	}
	gpio_direction_output(MODEM_POWER_CTRL_GPIO, 1);
	modem_ctrl.power_ctrl = 1;
/* sudhir Start */
#if 0//DEBUG  //FOTA CP Reset for Retry by seunghyun.yi@lge.com 2011_02_09
	printk("###** MODEM_POWER_CTRL_GPIO: Initial Current, %d\n", gpio_get_value(MODEM_POWER_CTRL_GPIO));
        mdelay(1000);

	gpio_direction_output(MODEM_POWER_CTRL_GPIO, 0);
        gpio_set_value(MODEM_POWER_CTRL_GPIO, 0);
	printk("###** MODEM_POWER_CTRL_GPIO: After Set Low, %d\n", gpio_get_value(MODEM_POWER_CTRL_GPIO));
        mdelay(1000);

	gpio_direction_output(MODEM_POWER_CTRL_GPIO, 1);
        gpio_set_value(MODEM_POWER_CTRL_GPIO, 1);
	printk("###** MODEM_POWER_CTRL_GPIO: After Set High, %d\n", gpio_get_value(MODEM_POWER_CTRL_GPIO));
        mdelay(1000);
#endif /* DEBUG */
#endif // FEATURE_MODEM_POWER_CTRL

#if FEATURE_MODEM_RESET_CTRL
	ret = gpio_request(MODEM_RESET_CTRL_GPIO, "modem reset switch");
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to request gpio %d\n",
			MODEM_RESET_CTRL_GPIO);
		ret = -EBUSY;
		goto err_gpio_request_reset_ctrl;
	}
#if DEBUG
        dump_gpio_status(__FUNCTION__, 2);
#endif

	gpio_direction_output(MODEM_RESET_CTRL_GPIO, 1);
	modem_ctrl.reset_ctrl = 1;

#if 0//DEBUG  //FOTA CP Reset for Retry by seunghyun.yi@lge.com 2011_02_09
/* sudhir end */
        dump_gpio_status(__FUNCTION__, 3);

	printk("###** MODEM_RESET_CTRL_GPIO: Initial Current, %d\n", gpio_get_value(MODEM_RESET_CTRL_GPIO));

        mdelay(1000);
	gpio_direction_output(MODEM_RESET_CTRL_GPIO, 0);
        gpio_set_value(MODEM_RESET_CTRL_GPIO, 0);
        dump_gpio_status(__FUNCTION__, 4);
	printk("###** MODEM_RESET_CTRL_GPIO: After Set Low, %d\n", gpio_get_value(MODEM_RESET_CTRL_GPIO));

        mdelay(1000);
	gpio_direction_output(MODEM_RESET_CTRL_GPIO, 1);
        gpio_set_value(MODEM_RESET_CTRL_GPIO, 1);
        dump_gpio_status(__FUNCTION__, 5);
	printk("###** MODEM_RESET_CTRL_GPIO: After Set High, %d\n", gpio_get_value(MODEM_RESET_CTRL_GPIO));

        mdelay(1000);
        gpio_direction_output(MODEM_RESET_CTRL_GPIO, 0);
        gpio_set_value(MODEM_RESET_CTRL_GPIO, 0);
        dump_gpio_status(__FUNCTION__, 6);
	printk("###** MODEM_RESET_CTRL_GPIO: After Set Low, %d\n", gpio_get_value(MODEM_RESET_CTRL_GPIO));

        mdelay(1000);
        gpio_direction_output(MODEM_RESET_CTRL_GPIO, 1);
        gpio_set_value(MODEM_RESET_CTRL_GPIO, 1);
        dump_gpio_status(__FUNCTION__, 7);
	printk("###** MODEM_RESET_CTRL_GPIO: After Set High, %d\n", gpio_get_value(MODEM_RESET_CTRL_GPIO));
#endif /* DEBUG */
#endif // FEATURE_MODEM_RESET_CTRL

        modem_ctrl.uart_path = 0;

	/* sysfs */
#if FEATURE_MODEM_POWER_CTRL
	ret = device_create_file(&pdev->dev, &dev_attr_power_ctrl);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to create sysfs power\n");
		ret = -ENOMEM;
		goto err_sysfs_power_ctrl;
	}
#endif

#if FEATURE_MODEM_RESET_CTRL
	ret = device_create_file(&pdev->dev, &dev_attr_reset_ctrl);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to create sysfs power\n");
		ret = -ENOMEM;
		goto err_sysfs_reset_ctrl;
	}
#endif

	ret = device_create_file(&pdev->dev, &dev_attr_uart_path);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to create sysfs power_switch\n");
		ret = -ENOMEM;
		goto err_sysfs_uart_ctrl;
	}

	initialized = 1;
	return ret;

      err_sysfs_uart_ctrl:
	device_remove_file(&pdev->dev, &dev_attr_reset_ctrl);

      err_sysfs_reset_ctrl:
	device_remove_file(&pdev->dev, &dev_attr_power_ctrl);

      err_sysfs_power_ctrl:
	gpio_free(MODEM_RESET_CTRL_GPIO);

      err_gpio_request_reset_ctrl:
	gpio_free(MODEM_POWER_CTRL_GPIO);

      err_gpio_request_power_ctrl:
	return ret;
}

static int modem_ctrl_remove(struct platform_device *pdev)
{

	device_remove_file(&pdev->dev, &dev_attr_power_ctrl);
	device_remove_file(&pdev->dev, &dev_attr_reset_ctrl);
	device_remove_file(&pdev->dev, &dev_attr_uart_path);

	gpio_free(MODEM_POWER_CTRL_GPIO);
	gpio_free(MODEM_RESET_CTRL_GPIO);

	initialized = 0;
	return 0;
}

static struct platform_driver modem_ctrl_driver = {
	.probe = modem_ctrl_probe,
	.remove = modem_ctrl_remove,
	.driver = {
		   .name = "modem_ctrl",
		   },
};

/**
 * module init/exit
 */
static int __init modem_ctrl_init(void)
{
	return platform_driver_register(&modem_ctrl_driver);
        return 0;
}

static void __exit modem_ctrl_exit(void)
{
	platform_driver_unregister(&modem_ctrl_driver);
}

late_initcall(modem_ctrl_init);
module_exit(modem_ctrl_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("Modem Control Driver");
MODULE_LICENSE("GPL v2");
