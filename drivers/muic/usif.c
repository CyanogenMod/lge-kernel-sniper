/*
 *  usif switch driver.
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

#include <asm/gpio.h>
#include <linux/slab.h>
#include <linux/muic/muic.h>
#include <linux/muic/muic_client.h>
#include <linux/switch_usif.h>
#include <linux/switch_dp3t.h>
//kibum.lee@lge.com, 20120502 MUIC re-work start
//#include <lge/board.h> /* temporary board header file for gpio num */
#include <linux/muic/muic_max14526deewp.h>
//kibum.lee@lge.com, 20120502 MUIC re-work end

TYPE_USIF_MODE usif_mode = USIF_AP;		// kibum.lee@lge.com
TYPE_USIF_CTRL usif_ctrl = USIF_CTRL_OK; //USIF can't switch

//struct usif_switch_platform_data *usif_switch_data; 

//USIF can't switch

//mode USIF_CTRL_NOK,		// 0
//	USIF_CTRL_OK,		// 1
void usif_switch_none(TYPE_USIF_CTRL mode)
{
	pr_info("usif: %s: usif can't switch %d \n", __func__,mode);
 	usif_ctrl = mode;
}
//USIF can't switch



int usif_switch_ctrl_ops(struct muic_client_device *mcdev)
{
	struct usif_switch *usif;
	unsigned long mode = mcdev->mode; 
//USIF can't switch
	
	if(!usif_ctrl){
		pr_info("usif: %s: usif can't switch  \n", __func__);
		return 1;
	}
	pr_info("usif: %s: mcdev->mode , %d\n", __func__, mcdev->mode);
	usif = dev_get_drvdata(&mcdev->dev);
	pr_info("usif: usif->ctrl_gpio = %d\n", usif->ctrl_gpio);

	if (mode == USIF_AP) {
		gpio_set_value(usif->ctrl_gpio, 0);
		pr_info("usif: usif_switch_ctrl, CP UART is connected to AP\n");
	} else if (mode == USIF_DP3T) {
		gpio_set_value(usif->ctrl_gpio, 1);
		pr_info("usif: usif_switch_ctrl, CP UART is connected to DP3T (then, MUIC)\n");
	} else {
		/* Just keep the current path */
	}
	
	usif_mode = mode;

	return 0;
}
EXPORT_SYMBOL(usif_switch_ctrl_ops);

int usif_on_none(struct muic_client_device *mcdev)
{
	struct usif_switch *usif;
	unsigned long mode = mcdev->mode; 
//USIF can't switch
	
	if(!usif_ctrl){
		pr_info("usif: %s: usif can't switch  \n", __func__);
		return 1;
	}
	
	usif = dev_get_drvdata(&mcdev->dev);
#if defined(CONFIG_MACH_LGE_P2_LU5400)
	gpio_set_value(usif->ctrl_gpio, 1);
// kibum.lee@lge.com 20120502 MUIC re-work start
#elif defined(CONFIG_MUIC) || defined(CONFIG_MACH_LGE_HUB)
	gpio_set_value(usif->ctrl_gpio, 1);
// kibum.lee@lge.com 20120502 MUIC re-work end
#else
	gpio_set_value(usif->ctrl_gpio, 0);
#endif


	pr_info("usif: usif_on_none, CP UART is connected to AP\n");

	usif_mode = mode;

	return 0;

}

int usif_on_ap_uart(struct muic_client_device *mcdev)
{
	struct usif_switch *usif;
	unsigned long mode = mcdev->mode; 
//USIF can't switch
	
	if(!usif_ctrl){
		pr_info("usif: %s: usif can't switch  \n", __func__);
		return 1;
	}
	usif = dev_get_drvdata(&mcdev->dev);
#if defined(CONFIG_MACH_LGE_P2_LU5400)
	gpio_set_value(usif->ctrl_gpio, 1);		
// kibum.lee@lge.com 20120502 MUIC re-work start
#elif defined(CONFIG_MUIC) || defined(CONFIG_MACH_LGE_HUB)
	gpio_set_value(usif->ctrl_gpio, 1);
// kibum.lee@lge.com 20120502 MUIC re-work end
#else
	gpio_set_value(usif->ctrl_gpio, 0);
#endif

	pr_info("usif: usif_on_ap_uart, CP UART is connected to AP\n");
	usif_mode = mode;

	return 0;
}

int usif_on_cp_uart(struct muic_client_device *mcdev)
{
	struct usif_switch *usif;
	unsigned long mode = mcdev->mode; 
//USIF can't switch
	
	if(!usif_ctrl){
		pr_info("usif: %s: usif can't switch  \n", __func__);
		return 1;
	}
	usif = dev_get_drvdata(&mcdev->dev);
	
	gpio_set_value(usif->ctrl_gpio, 1);
	pr_info("usif: usif_on_cp_uart, CP UART is connected to DP3T (then, MUIC)\n");

	usif_mode = mode;

	return 0;
}

int usif_on_cp_usb(struct muic_client_device *mcdev)
{
	struct usif_switch *usif;
	unsigned long mode = mcdev->mode; 
//USIF can't switch
	
	if(!usif_ctrl){
		pr_info("usif: %s: usif can't switch  \n", __func__);
		return 1;
	}
	
	pr_info("usif: %s: mcdev->mode , %d\n", __func__, mcdev->mode);
	usif = dev_get_drvdata(&mcdev->dev);
#if defined(CONFIG_MACH_LGE_P2_LU5400)
	gpio_set_value(usif->ctrl_gpio, 1);
// kibum.lee@lge.com 20120502 MUIC re-work start
#elif defined(CONFIG_MUIC) || defined(CONFIG_MACH_LGE_HUB)
	gpio_set_value(usif->ctrl_gpio, 1);
// kibum.lee@lge.com 20120502 MUIC re-work end
#endif
	pr_info("usif: usif_on_cp_usb, CP USB is connected to DP3T (then, MUIC)\n");
	
	usif_mode = mode;

	return 0;
}

// 20120502, created seungho1.park@lge.com, porting kibum.lee
//This function was created to support legacy systems.
// It is recommended not to use the future.
void usif_switch_ctrl(TYPE_USIF_MODE mode)
{
	pr_info("usif: %s()\n", __func__);
//USIF can't switch
	
	gpio_direction_output(GPIO_USIF_IN_1, 0);		// LGE_MOD 20121106 subum.choi@lge.com [FOTA] UART4 Path

	if(!usif_ctrl){
		pr_info("usif: %s: usif can't switch  \n", __func__);
		return;
	}
	if (mode == USIF_AP) {
		gpio_set_value(GPIO_USIF_IN_1, 0);
		pr_info("usif: usif_switch_ctrl, CP UART is connected to AP\n");
	} else if (mode == USIF_DP3T) {
		gpio_set_value(GPIO_USIF_IN_1, 1);
		pr_info("usif: usif_switch_ctrl, CP UART is connected to DP3T (then, MUIC)\n");
	} else {
		/* Just keep the current path */
	}

	usif_mode = mode;

	return ;
}
EXPORT_SYMBOL(usif_switch_ctrl);

static struct muic_client_ops usif_ops = {
	.on_none = usif_on_none,
	.on_unknown = usif_on_none,
	.on_ap_uart = usif_on_ap_uart,
	.on_ap_usb = usif_on_ap_uart,
	.on_cp_uart = usif_on_cp_uart,
	.on_cp_usb = usif_on_cp_usb,
};

static int usif_switch_probe(struct platform_device *pdev)
{
	struct usif_switch_platform_data *pdata = pdev->dev.platform_data;
	struct usif_switch *usif;
	int ret = 0;

	pr_info("usif: usif_switch_probe\n");	
	
	usif = kzalloc(sizeof(struct usif_switch), GFP_KERNEL); 
	if (!usif) {
		pr_err("usif: %s: no memory\n", __func__);
		return -ENOMEM;
	}

	usif->pdev = pdev;

	muic_client_dev_register(pdev->name, usif, &usif_ops);

	usif->ctrl_gpio = pdata->ctrl_gpio;

	ret = gpio_request(usif->ctrl_gpio, "USIF switch control GPIO");
	if (ret < 0) {
		pr_err("usif: GPIO 165 USIF1_SW is already occupied by other driver!\n");
		/*
		 * We know board_cosmo.c:ifx_n721_configure_gpio() performs a gpio_request on this pin first.
		 * Because the prior gpio_request is also for the analog switch control, this is not a confliction.
		 */
		return -ENOSYS;
	}
	
	usif_ctrl = USIF_CTRL_OK ; //USIF can't switch
	
#if defined(CONFIG_MACH_LGE_P2_LU5400)
	ret = gpio_direction_output(usif->ctrl_gpio, 1);
// kibum.lee@lge.com 20120502 MUIC re-work start
#elif defined(CONFIG_MUIC) || defined(CONFIG_MACH_LGE_HUB)
	gpio_set_value(usif->ctrl_gpio, 1);
// kibum.lee@lge.com 20120502 MUIC re-work end
#else
	ret = gpio_direction_output(usif->ctrl_gpio, 0);

#endif
	
	if (ret < 0) {
		pr_err("usif: gpio_16 USIF_IN_1_GPIO direction initialization failed!\n");
		return -ENOSYS;
	}
    /* gpio output mode setting */
    ret = gpio_direction_output(GPIO_USIF_IN_1, 0); //hyuntae.bok@lge.com for FOTA
    if (ret < 0) {
		pr_err("usif: gpio_16 GPIO_USIF_IN_1 direction initialization failed!\n");
		return -ENOSYS;
	}
    
	platform_set_drvdata(pdev, usif);

	pr_info("usif: usif_switch_probe done!\n");	
	return 0;
}

static int __devexit usif_switch_remove(struct platform_device *pdev)
{
	struct usif_switch *usif= platform_get_drvdata(pdev);
	
	gpio_free(usif->ctrl_gpio);

	kfree(usif);

	return 0;
}
	
static struct platform_driver usif_switch_driver = {
	.probe		= usif_switch_probe,
	.remove		= __devexit_p(usif_switch_remove),
	.driver		= {
		.name	= "switch-usif",
		.owner	= THIS_MODULE,
	},
};

static int __init usif_switch_init(void)
{
	return platform_driver_register(&usif_switch_driver);
}

static void __exit usif_switch_exit(void)
{
	platform_driver_unregister(&usif_switch_driver);
}

module_init(usif_switch_init);
module_exit(usif_switch_exit);

MODULE_AUTHOR("Seungho Park <seungho1.park@lge.com>");
MODULE_DESCRIPTION("USIF Switch driver");
MODULE_LICENSE("GPL");
