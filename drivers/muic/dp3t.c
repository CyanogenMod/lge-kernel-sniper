/*
 *  dp3t switch driver.
 *
 * Copyright (C) 2011 LG Electronics, Inc.
 * Author: Seungho Park <seungho1.park@lge.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
*/

#include <linux/slab.h>
#include <linux/muic/muic.h>
#include <linux/switch_dp3t.h>
#include <linux/muic/muic_client.h>
#include <asm/gpio.h>

//kibum.lee@lge.com, 20120502 MUIC re-work start
//#include <lge/board.h> /* temporary board header file for gpio num */
#include <linux/muic/muic_max14526deewp.h>
//kibum.lee@lge.com, 20120502 MUIC re-work end


TYPE_DP3T_MODE dp3t_mode = DP3T_NC;
//struct dp3t_switch_platform_data *dp3t_switch_data; 

//void dp3t_switch_ctrl(TYPE_DP3T_MODE mode)
int dp3t_switch_ctrl_ops(struct muic_client_device *mcdev)
{
	struct dp3t_switch *dp3t;
	unsigned long mode = mcdev->mode; 
	pr_info("dp3t: mcdev->name , %s\n", mcdev->name);
	pr_info("dp3t: mcdev->mode , %d\n", mcdev->mode);
	dp3t = dev_get_drvdata(&mcdev->dev);
	pr_info("dp3t: ctrl_ifx_vbus_gpio = %d\n", dp3t->ctrl_ifx_vbus_gpio);
	pr_info("dp3t: ctrl_gpio1 = %d\n", dp3t->ctrl_gpio1);
	pr_info("dp3t: ctrl_gpio2 = %d\n", dp3t->ctrl_gpio2);
	
	if (mode == MUIC_AP_UART) {
		gpio_set_value(dp3t->ctrl_ifx_vbus_gpio, 0);
		gpio_set_value(dp3t->ctrl_gpio1, 1);
		gpio_set_value(dp3t->ctrl_gpio2, 0);
		pr_info("dp3t: dp3t_switch_ctrl, AP UART is connected to MUIC UART\n");
	} else if (mode == MUIC_CP_UART) {
		gpio_set_value(dp3t->ctrl_ifx_vbus_gpio, 0);
		gpio_set_value(dp3t->ctrl_gpio1, 0);
		gpio_set_value(dp3t->ctrl_gpio2, 1);
		pr_info("dp3t: dp3t_switch_ctrl, CP UART is connected to MUIC UART\n");
	} else if (mode == MUIC_CP_USB) {
		gpio_set_value(dp3t->ctrl_ifx_vbus_gpio, 1);
		gpio_set_value(dp3t->ctrl_gpio1, 1);
		gpio_set_value(dp3t->ctrl_gpio2, 1);
		pr_info("dp3t: dp3t_switch_ctrl, CP USB is connected to MUIC UART\n");
	} else if (mode == MUIC_NONE) {
		gpio_set_value(dp3t->ctrl_ifx_vbus_gpio, 0);
		gpio_set_value(dp3t->ctrl_gpio1, 0);
		gpio_set_value(dp3t->ctrl_gpio2, 0);
		pr_info("dp3t: dp3t_switch_ctrl, None is connected to MUIC UART\n");
	} else {
		/* Just keep the current path */
	}
	
	dp3t_mode = mode;

	return 0;
}
EXPORT_SYMBOL(dp3t_switch_ctrl_ops);

int dp3t_on_ap_uart(struct muic_client_device *mcdev)
{
	struct dp3t_switch *dp3t;
	unsigned long mode = mcdev->mode; 
	
	dp3t = dev_get_drvdata(&mcdev->dev);
#if defined (CONFIG_PRODUCT_LGE_LU6800)
	gpio_set_value(dp3t->ctrl_ifx_vbus_gpio, 0);
	gpio_set_value(dp3t->ctrl_gpio1, 0);
	gpio_set_value(dp3t->ctrl_gpio2, 0);
	pr_info("dp3t: dp3t_switch_ctrl, AP UART is connected to MUIC UART\n");
#elif defined (CONFIG_PRODUCT_LGE_KU5900)
	gpio_set_value(dp3t->ctrl_ifx_vbus_gpio, 0);
	gpio_set_value(dp3t->ctrl_gpio1, 1);
	gpio_set_value(dp3t->ctrl_gpio2, 0);
	pr_info("dp3t: dp3t_switch_ctrl, AP UART is connected to MUIC UART\n");
#else 
	gpio_set_value(dp3t->ctrl_ifx_vbus_gpio, 0);
	gpio_set_value(dp3t->ctrl_gpio1, 1);
	gpio_set_value(dp3t->ctrl_gpio2, 0);
	pr_info("dp3t: dp3t_switch_ctrl, AP UART is connected to MUIC UART\n");
#endif
	dp3t_mode = mode;
	return 0;
}

int dp3t_on_cp_uart(struct muic_client_device *mcdev)
{
	struct dp3t_switch *dp3t;
	unsigned long mode = mcdev->mode; 
	
	dp3t = dev_get_drvdata(&mcdev->dev);
	
	gpio_set_value(dp3t->ctrl_ifx_vbus_gpio, 0);
	gpio_set_value(dp3t->ctrl_gpio1, 0);
	gpio_set_value(dp3t->ctrl_gpio2, 1);
	pr_info("dp3t: dp3t_switch_ctrl, CP UART is connected to MUIC UART\n");

	dp3t_mode = mode;
	return 0;
}

int dp3t_on_cp_usb(struct muic_client_device *mcdev)
{
	struct dp3t_switch *dp3t;
	unsigned long mode = mcdev->mode; 
	
	dp3t = dev_get_drvdata(&mcdev->dev);
	
	gpio_set_value(dp3t->ctrl_ifx_vbus_gpio, 1);
	gpio_set_value(dp3t->ctrl_gpio1, 1);
	gpio_set_value(dp3t->ctrl_gpio2, 1);
	pr_info("dp3t: dp3t_switch_ctrl, CP USB is connected to MUIC UART\n");

	dp3t_mode = mode;
	return 0;
}

// 20120502, created seungho1.park@lge.com, porting kibum.lee
//This function was created to support legacy systems.
// It is recommended not to use the future.
void dp3t_switch_ctrl(TYPE_DP3T_MODE mode)
{
	pr_info("dp3t: dp3t_switch_ctrl()\n");
#if 0 
	printk("dp3t->ctrl_ifx_vbus_gpio = %d\n", dp3t->ctrl_ifx_vbus_gpio);
	printk("dp3t->ctrl_gpio1 = %d\n", dp3t->ctrl_gpio1);
	printk("dp3t->ctrl_gpio2 = %d\n", dp3t->ctrl_gpio2);
	if (mode == DP3T_AP_UART) {
		gpio_set_value(dp3t->ctrl_ifx_vbus_gpio, 0);
		gpio_set_value(dp3t->ctrl_gpio1, 1);
		gpio_set_value(dp3t->ctrl_gpio2, 0);
		printk(KERN_INFO "[MUIC] dp3t_switch_ctrl, AP UART is connected to MUIC UART\n");
	} else if (mode == DP3T_CP_UART) {
		gpio_set_value(dp3t->ctrl_ifx_vbus_gpio, 0);
		gpio_set_value(dp3t->ctrl_gpio1, 0);
		gpio_set_value(dp3t->ctrl_gpio2, 1);
		printk(KERN_INFO "[MUIC] dp3t_switch_ctrl, CP UART is connected to MUIC UART\n");
	} else if (mode == DP3T_CP_USB) {
		gpio_set_value(dp3t->ctrl_ifx_vbus_gpio, 1);
		gpio_set_value(dp3t->ctrl_gpio1, 1);
		gpio_set_value(dp3t->ctrl_gpio2, 1);
		printk(KERN_INFO "[MUIC] dp3t_switch_ctrl, CP USB is connected to MUIC UART\n");
	} else if (mode == DP3T_NC) {
		gpio_set_value(dp3t->ctrl_ifx_vbus_gpio, 0);
		gpio_set_value(dp3t->ctrl_gpio1, 0);
		gpio_set_value(dp3t->ctrl_gpio2, 0);
		printk(KERN_INFO "[MUIC] dp3t_switch_ctrl, None is connected to MUIC UART\n");
	} else {
		/* Just keep the current path */
	}
	
	dp3t_mode = mode;
	//printk(KERN_WARNING "[MUIC] dp3t_switch_ctrl(): dp3t_mode = %d\n", dp3t_mode);
#else
	if (mode == DP3T_AP_UART) {
		gpio_set_value(GPIO_IFX_USB_VBUS_EN, 0);
		gpio_set_value(GPIO_DP3T_IN_1, 1);
		gpio_set_value(GPIO_DP3T_IN_2, 0);
		pr_info("dp3t: dp3t_switch_ctrl, AP UART is connected to MUIC UART\n");
	} else if (mode == DP3T_CP_UART) {
		gpio_set_value(GPIO_IFX_USB_VBUS_EN, 0);
		gpio_set_value(GPIO_DP3T_IN_1, 0);
		gpio_set_value(GPIO_DP3T_IN_2, 1);
		pr_info("dp3t: dp3t_switch_ctrl, CP UART is connected to MUIC UART\n");
	} else if (mode == DP3T_CP_USB) {
		gpio_set_value(GPIO_IFX_USB_VBUS_EN, 1);
		gpio_set_value(GPIO_DP3T_IN_1, 1);
		gpio_set_value(GPIO_DP3T_IN_2, 1);
		pr_info("dp3t: dp3t_switch_ctrl, CP USB is connected to MUIC UART\n");
	} else if (mode == DP3T_NC) {
		gpio_set_value(GPIO_IFX_USB_VBUS_EN, 0);
		gpio_set_value(GPIO_DP3T_IN_1, 0);
		gpio_set_value(GPIO_DP3T_IN_2, 0);
		pr_info("dp3t: dp3t_switch_ctrl, None is connected to MUIC UART\n");
	} else {
		/* Just keep the current path */
	}
	
	dp3t_mode = mode;
	//printk(KERN_WARNING "[MUIC] dp3t_switch_ctrl(): dp3t_mode = %d\n", dp3t_mode);

#endif
	return;
}
EXPORT_SYMBOL(dp3t_switch_ctrl);

static struct muic_client_ops dp3t_ops = {
	.on_ap_uart = dp3t_on_ap_uart,
	.on_ap_usb = dp3t_on_ap_uart,
	.on_cp_uart = dp3t_on_cp_uart,
	.on_cp_usb = dp3t_on_cp_usb,
	.on_lg_ta = dp3t_on_ap_uart,
	.on_unknown = dp3t_on_ap_uart,
	.on_na_ta = dp3t_on_ap_uart,
	.on_1a_ta = dp3t_on_ap_uart,
	.on_invalid_chg = dp3t_on_ap_uart,
};


static int dp3t_switch_probe(struct platform_device *pdev)
{
	struct dp3t_switch_platform_data *pdata = pdev->dev.platform_data;
	struct dp3t_switch *dp3t;
	int ret = 0;

	pr_info("dp3t: dp3t_switch_probe\n");

	dp3t = kzalloc(sizeof(struct dp3t_switch), GFP_KERNEL); 

	dp3t->pdev = pdev;

	muic_client_dev_register(pdev->name, dp3t, &dp3t_ops);

	dp3t->ctrl_gpio1 = pdata->ctrl_gpio1;
	dp3t->ctrl_gpio2 = pdata->ctrl_gpio2;
	dp3t->ctrl_ifx_vbus_gpio = pdata->ctrl_ifx_vbus_gpio;


	/*
	 * Initializes gpio_11 (OMAP_UART_SW) and gpio_12 (IFX_UART_SW).
	 * Checks if other driver already occupied them.
	 */
	ret = gpio_request(dp3t->ctrl_gpio1, "DP3T switch control 1 GPIO");
	if (ret < 0) {
		pr_err("dp3t: GPIO %d is already used!\n", dp3t->ctrl_gpio1);
		return -ENOSYS;
	}

	ret = gpio_direction_output(dp3t->ctrl_gpio1, 0);
	if (ret < 0) {
		pr_err("dp3t: GPIO %d direction initialization failed!\n",
				dp3t->ctrl_gpio1);
		return -ENOSYS;
	}
	
	ret = gpio_request(dp3t->ctrl_gpio2, "DP3T switch control 2 GPIO");
	if (ret < 0) {
		pr_err("dp3t: GPIO %d is already used\n", dp3t->ctrl_gpio2);
		return -ENOSYS;
	}

	ret = gpio_direction_output(dp3t->ctrl_gpio2, 0);
	if (ret < 0) {
		pr_err("dp3t: GPIO %d direction initialization failed!\n",
				dp3t->ctrl_gpio2);
		return -ENOSYS;
	}

	ret = gpio_request(dp3t->ctrl_ifx_vbus_gpio, "DP3T switch control 2 GPIO");
	if (ret < 0) {
		pr_err("dp3t: GPIO %d is already used\n",
				dp3t->ctrl_ifx_vbus_gpio);
		return -ENOSYS;
	}

	ret = gpio_direction_output(dp3t->ctrl_ifx_vbus_gpio, 0);
	if (ret < 0) {
		pr_err("dp3t: GPIO %d direction initialization failed!\n",
				dp3t->ctrl_ifx_vbus_gpio);
		return -ENOSYS;
	}
	
	platform_set_drvdata(pdev, dp3t);
//hyuntae.bok@lge.com for FOTA START
    /* gpio output mode setting */
    
    ret = gpio_direction_output(GPIO_IFX_USB_VBUS_EN, 0);
    if (ret < 0) {
		pr_err("dp3t: GPIO %d direction initialization failed!\n",
				GPIO_IFX_USB_VBUS_EN);
		return -ENOSYS;
	}
	
    ret = gpio_direction_output(GPIO_DP3T_IN_1, 0);
    if (ret < 0) {
		pr_err("dp3t: GPIO %d direction initialization failed!\n",
				GPIO_DP3T_IN_1);
		return -ENOSYS;
	}
    ret = gpio_direction_output(GPIO_DP3T_IN_2, 0);
    if (ret < 0) {
		pr_err("dp3t: GPIO %d direction initialization failed!\n",
				GPIO_DP3T_IN_2);
		return -ENOSYS;
	}
//hyuntae.bok@lge.com for FOTA END
	pr_info("dp3t: dp3t_switch_probe done!\n");
	
	return 0;
}

static int __devexit dp3t_switch_remove(struct platform_device *pdev)
{
	struct dp3t_switch_platform_data *pdata = pdev->dev.platform_data;
	struct dp3t_switch *dp3t= platform_get_drvdata(pdev);
	
	gpio_free(pdata->ctrl_ifx_vbus_gpio);
	gpio_free(pdata->ctrl_gpio1);
	gpio_free(pdata->ctrl_gpio2);

	kfree(dp3t);

	return 0;
}
	
static struct platform_driver dp3t_switch_driver = {
	.probe		= dp3t_switch_probe,
	.remove		= __devexit_p(dp3t_switch_remove),
	.driver		= {
		.name	= "switch-dp3t",
		.owner	= THIS_MODULE,
	},
};

static int __init dp3t_switch_init(void)
{
	return platform_driver_register(&dp3t_switch_driver);
}

static void __exit dp3t_switch_exit(void)
{
	platform_driver_unregister(&dp3t_switch_driver);
}

module_init(dp3t_switch_init);
module_exit(dp3t_switch_exit);

MODULE_AUTHOR("Seungho Park <seungho1.park@lge.com>");
MODULE_DESCRIPTION("DP3T Switch driver");
MODULE_LICENSE("GPL");
