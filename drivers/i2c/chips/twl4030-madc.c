/*
 * drivers/i2c/chips/twl4030-madc.c
 *
 * TWL4030 MADC module driver
 *
 * Copyright (C) 2008 Nokia Corporation
 * Mikko Ylinen <mikko.k.ylinen@nokia.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/i2c/twl.h>
#include <linux/i2c/twl4030-madc.h>

#include <linux/uaccess.h>
#ifdef CONFIG_LGE_OMAP3_EXT_PWR
#include <linux/lge_extpwr_type.h>
#endif

#define TWL4030_MADC_PFX	"twl4030-madc: "

struct twl4030_madc_data {
	struct device		*dev;
	struct mutex		lock;
	struct work_struct	ws;
	struct twl4030_madc_request	requests[TWL4030_MADC_NUM_METHODS];
	int imr;
	int isr;
};

static struct twl4030_madc_data *the_madc;

static
const struct twl4030_madc_conversion_method twl4030_conversion_methods[] = {
	[TWL4030_MADC_RT] = {
		.sel	= TWL4030_MADC_RTSELECT_LSB,
		.avg	= TWL4030_MADC_RTAVERAGE_LSB,
		.rbase	= TWL4030_MADC_RTCH0_LSB,
	},
	[TWL4030_MADC_SW1] = {
		.sel	= TWL4030_MADC_SW1SELECT_LSB,
		.avg	= TWL4030_MADC_SW1AVERAGE_LSB,
		.rbase	= TWL4030_MADC_GPCH0_LSB,
		.ctrl	= TWL4030_MADC_CTRL_SW1,
	},
	[TWL4030_MADC_SW2] = {
		.sel	= TWL4030_MADC_SW2SELECT_LSB,
		.avg	= TWL4030_MADC_SW2AVERAGE_LSB,
		.rbase	= TWL4030_MADC_GPCH0_LSB,
		.ctrl	= TWL4030_MADC_CTRL_SW2,
	},
};

static int twl4030_madc_read(struct twl4030_madc_data *madc, u8 reg)
{
	int ret = 0;
	u8 val = 0;

	ret = twl_i2c_read_u8(TWL4030_MODULE_MADC, &val, reg);
	if (ret) {
		dev_dbg(madc->dev, "unable to read register 0x%X\n", reg);
		return ret;
	}

	return val;
}

static void twl4030_madc_write(struct twl4030_madc_data *madc, u8 reg, u8 val)
{
	int ret;

	ret = twl_i2c_write_u8(TWL4030_MODULE_MADC, val, reg);
	if (ret)
		dev_err(madc->dev, "unable to write register 0x%X\n", reg);
}

static int twl4030_madc_channel_raw_read(struct twl4030_madc_data *madc, u8 reg)
{
	u8 msb, lsb;

	/* For each ADC channel, we have MSB and LSB register pair. MSB address
	 * is always LSB address+1. reg parameter is the addr of LSB register */
	msb = twl4030_madc_read(madc, reg + 1);
	lsb = twl4030_madc_read(madc, reg);

	return (int)(((msb << 8) | lsb) >> 6);
}

static int twl4030_madc_read_channels(struct twl4030_madc_data *madc,
		u8 reg_base, u16 channels, int *buf)
{
	int count = 0;
	u8 reg, i;

	if (unlikely(!buf))
		return 0;

	for (i = 0; i < TWL4030_MADC_MAX_CHANNELS; i++) {
		if (channels & (1<<i)) {
			reg = reg_base + 2*i;
			buf[i] = twl4030_madc_channel_raw_read(madc, reg);
			count++;
		}
	}
	return count;
}

static void twl4030_madc_enable_irq(struct twl4030_madc_data *madc, int id)
{
	u8 val;

	val = twl4030_madc_read(madc, madc->imr);
	val &= ~(1 << id);
	twl4030_madc_write(madc, madc->imr, val);
}

static void twl4030_madc_disable_irq(struct twl4030_madc_data *madc, int id)
{
	u8 val;

	val = twl4030_madc_read(madc, madc->imr);
	val |= (1 << id);
	twl4030_madc_write(madc, madc->imr, val);
}

static irqreturn_t twl4030_madc_irq_handler(int irq, void *_madc)
{
	struct twl4030_madc_data *madc = _madc;
	u8 isr_val, imr_val;
	int i;

#ifdef CONFIG_LOCKDEP
	/* WORKAROUND for lockdep forcing IRQF_DISABLED on us, which
	 * we don't want and can't tolerate.  Although it might be
	 * friendlier not to borrow this thread context...
	 */
	local_irq_enable();
#endif

	/* Use COR to ack interrupts since we have no shared IRQs in ISRx */
	isr_val = twl4030_madc_read(madc, madc->isr);
	imr_val = twl4030_madc_read(madc, madc->imr);

	isr_val &= ~imr_val;

	for (i = 0; i < TWL4030_MADC_NUM_METHODS; i++) {

		if (!(isr_val & (1<<i)))
			continue;

		twl4030_madc_disable_irq(madc, i);
		madc->requests[i].result_pending = 1;
	}

	schedule_work(&madc->ws);

	return IRQ_HANDLED;
}

static void twl4030_madc_work(struct work_struct *ws)
{
	const struct twl4030_madc_conversion_method *method;
	struct twl4030_madc_data *madc;
	struct twl4030_madc_request *r;
	int len, i;

	madc = container_of(ws, struct twl4030_madc_data, ws);
	mutex_lock(&madc->lock);

	for (i = 0; i < TWL4030_MADC_NUM_METHODS; i++) {

		r = &madc->requests[i];

		/* No pending results for this method, move to next one */
		if (!r->result_pending)
			continue;

		method = &twl4030_conversion_methods[r->method];

		/* Read results */
		len = twl4030_madc_read_channels(madc, method->rbase,
						 r->channels, r->rbuf);

		/* Return results to caller */
		if (r->func_cb != NULL) {
			r->func_cb(len, r->channels, r->rbuf);
			r->func_cb = NULL;
		}

		/* Free request */
		r->result_pending = 0;
		r->active	  = 0;
	}

	mutex_unlock(&madc->lock);
}

static int twl4030_madc_set_irq(struct twl4030_madc_data *madc,
		struct twl4030_madc_request *req)
{
	struct twl4030_madc_request *p;

	p = &madc->requests[req->method];

	memcpy(p, req, sizeof *req);

	twl4030_madc_enable_irq(madc, req->method);

	return 0;
}

static inline void twl4030_madc_start_conversion(struct twl4030_madc_data *madc,
		int conv_method)
{
	const struct twl4030_madc_conversion_method *method;

	method = &twl4030_conversion_methods[conv_method];

	switch (conv_method) {
	case TWL4030_MADC_SW1:
	case TWL4030_MADC_SW2:
		twl4030_madc_write(madc, method->ctrl, TWL4030_MADC_SW_START);
		break;
	case TWL4030_MADC_RT:
	default:
		break;
	}
}

static int twl4030_madc_wait_conversion_ready(
		struct twl4030_madc_data *madc,
		unsigned int timeout_ms, u8 status_reg)
{
	unsigned long timeout;

	timeout = jiffies + msecs_to_jiffies(timeout_ms);
	do {
		u8 reg;

		reg = twl4030_madc_read(madc, status_reg);
		if (!(reg & TWL4030_MADC_BUSY) && (reg & TWL4030_MADC_EOC_SW))
			return 0;
	} while (!time_after(jiffies, timeout));

	return -EAGAIN;
}

int twl4030_madc_conversion(struct twl4030_madc_request *req)
{
	const struct twl4030_madc_conversion_method *method;
	u8 ch_msb, ch_lsb;
	int ret;

	if (unlikely(!req))
		return -EINVAL;

	mutex_lock(&the_madc->lock);

	/* Do we have a conversion request ongoing */
	if (the_madc->requests[req->method].active) {
		ret = -EBUSY;
		goto out;
	}

	ch_msb = (req->channels >> 8) & 0xff;
	ch_lsb = req->channels & 0xff;

	method = &twl4030_conversion_methods[req->method];

	/* Select channels to be converted */
	twl4030_madc_write(the_madc, method->sel + 1, ch_msb);
	twl4030_madc_write(the_madc, method->sel, ch_lsb);

	/* Select averaging for all channels if do_avg is set */
	if (req->do_avg) {
		twl4030_madc_write(the_madc, method->avg + 1, ch_msb);
		twl4030_madc_write(the_madc, method->avg, ch_lsb);
	}

	if ((req->type == TWL4030_MADC_IRQ_ONESHOT) && (req->func_cb != NULL)) {
		twl4030_madc_set_irq(the_madc, req);
		twl4030_madc_start_conversion(the_madc, req->method);
		the_madc->requests[req->method].active = 1;
		ret = 0;
		goto out;
	}

	/* With RT method we should not be here anymore */
	if (req->method == TWL4030_MADC_RT) {
		ret = -EINVAL;
		goto out;
	}

	twl4030_madc_start_conversion(the_madc, req->method);
	the_madc->requests[req->method].active = 1;

	/* Wait until conversion is ready (ctrl register returns EOC) */
	ret = twl4030_madc_wait_conversion_ready(the_madc, 5, method->ctrl);
	if (ret) {
		dev_dbg(the_madc->dev, "conversion timeout!\n");
		the_madc->requests[req->method].active = 0;
		goto out;
	}

	ret = twl4030_madc_read_channels(the_madc, method->rbase, req->channels,
					 req->rbuf);

	the_madc->requests[req->method].active = 0;

out:
	mutex_unlock(&the_madc->lock);

	return ret;
}
EXPORT_SYMBOL(twl4030_madc_conversion);

#ifdef CONFIG_LGE_OMAP3_EXT_PWR
static int madc_init_done = 0;
int get_madc_probe_done(void)
{
	return madc_init_done;
}
static void set_madc_probe_done(int done)
{
	extern void set_charging_current(void);

	madc_init_done = !!done;
	set_charging_current();
	return; 
}

#define CONV_VOLTAGE(value) ((value * 2500) / (1023))

static int usb_type = NO_INIT_CABLE;
int get_ext_pwr_type(void)
{
	return usb_type;
}

//20110708 yongman.kwon@lge.com [LS855] Check manual mode for detecting usb cables. [START]
int manual_mode = 0; 
static s32 __init uboot_read_manual_mode(char *str)
{
	sscanf(str, "%d", &manual_mode);
	printk(KERN_INFO "%s: manual mode value %d\n", __func__, manual_mode);
	return 0;
}
__setup("manual_mode=", uboot_read_manual_mode);
//20110708 yongman.kwon@lge.com [LS855] Check manual mode for detecting usb cables. [END]


extern int get_external_power_status(void);

extern int get_usb_dp_dm_status(void);
int lge_usb_acc_detect(void)
{
	int adc_channel = 4;  /* ADCIN4 */
	int is_normal = 0;
	int is_short = 0;
	unsigned int data;
	struct twl4030_madc_request ch_req;

	printk("[charging_msg] %s, start\n", __FUNCTION__);

	if( !get_madc_probe_done() ) {
		return -1;
	}
	
	if( !get_external_power_status() ) {
		usb_type = NO_CABLE;
		return 0;
	}

		ch_req.channels = (1 << adc_channel);
		ch_req.do_avg	= 1; /* par.average; */
		ch_req.method	= TWL4030_MADC_SW1;
		ch_req.func_cb	= NULL;
		twl4030_madc_conversion(&ch_req);
		data = (u16)ch_req.rbuf[adc_channel];

	printk("[charging_msg] %s: USB ID level = %d mV (0x%03X)\n",
		__func__, CONV_VOLTAGE(data), data);

//20110708 yongman.kwon@lge.com [LS855] Check manual mode for detecting usb cables. [START]
//56K_LT detect range extands because of voltage drop.
#if 1
	if (data > 0x030 && data < 0x0CC) {
#else
	if (data > 0x07A && data < 0x0CC) {
#endif
//20110708 yongman.kwon@lge.com [LS855] Check manual mode for detecting usb cables. [END]
		/* 0x0A3 ~ 0x0A6 */
		usb_type = LT_CABLE_56K;
		printk(KERN_DEBUG "[charging_msg] %s: Connected to a LT_56K cable.\n", __func__);
		return 0;
	} else if (data > 0x0F5 && data < 0x146) {
		/* 0x124 */
		usb_type = LT_CABLE_130K;
		printk(KERN_DEBUG "[charging_msg] %s: Connected to a LT_130K cable.\n", __func__);
		return 0;
	} else if (data > 0x213 && data < 0x28E) {
		usb_type = LT_CABLE_910K;
		printk(KERN_DEBUG "[charging_msg] %s: Connected to a LT_910K cable.\n", __func__);
		return 0;
	} else if (data > 0x147 && data < 0x16F) {
		is_normal = 1;
		printk(KERN_DEBUG "[charging_msg] %s: Cable ID 180K is valid for TA/USB.\n", __func__);
	} else if (data > 0x170 && data < 0x198) {
		is_normal = 1;
		printk(KERN_DEBUG "[charging_msg] %s: Cable ID 220K is valid for TA/USB.\n", __func__);
	} else {
//20110708 yongman.kwon@lge.com [LS855] Check manual mode for detecting usb cables. [START]
#if 1	
		if(manual_mode == 1){
			usb_type = LT_CABLE_56K;
			printk(KERN_DEBUG "[charging_msg] %s: Connected to a LT_56K cable(manual mode = %d).\n", __func__, manual_mode);
			return 0;
		}	
		else{
		is_normal = 0;
		printk(KERN_DEBUG "[charging_msg] %s: Cable ID is un-valid.\n", __func__);
		}
#else
		is_normal = 0;
		printk(KERN_DEBUG "[charging_msg] %s: Cable ID is un-valid.\n", __func__);
#endif		
//20110708 yongman.kwon@lge.com [LS855] Check manual mode for detecting usb cables. [END]

	}

	/* 
	 * USB D+, D- short check 
	 */
	is_short = get_usb_dp_dm_status();

	printk(KERN_DEBUG "%s: D+ and D- lines are %s\n", __func__, is_short? "short":"open");

	/*
 	 * Current setting
	 */
	if (is_short) {
		if (is_normal) {
			usb_type = TA_CABLE;
			printk(KERN_DEBUG "%s: Connected to a TA.\n", __func__);
		} else {
			//usb_type = FORGED_TA_CABLE;
			usb_type = TA_CABLE;
			printk(KERN_DEBUG "%s: Connected to a Forged TA.\n", __func__);
		}
	} else {
		/* for USB, charger current set will be done at usbcon_check(),
		   so do nothing here to current set */
		usb_type = USB_CABLE;
		printk(KERN_DEBUG "%s: Connected to an USB.\n", __func__);
	}
	return 0;

}

#endif 

static int twl4030_madc_set_current_generator(struct twl4030_madc_data *madc,
		int chan, int on)
{
	int ret;
	u8 regval = 0;

	/* Current generator is only available for ADCIN0 and ADCIN1. NB:
	 * ADCIN1 current generator only works when AC or VBUS is present */
	if (chan > 1)
		return EINVAL;

	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE,
				  &regval, TWL4030_BCI_BCICTL1);
	if (on)
		regval |= (chan) ? TWL4030_BCI_ITHEN : TWL4030_BCI_TYPEN;
	else
		regval &= (chan) ? ~TWL4030_BCI_ITHEN : ~TWL4030_BCI_TYPEN;
	ret = twl_i2c_write_u8(TWL4030_MODULE_MAIN_CHARGE,
				   regval, TWL4030_BCI_BCICTL1);

	return ret;
}

static int twl4030_madc_set_power(struct twl4030_madc_data *madc, int on)
{
	u8 regval;

	regval = twl4030_madc_read(madc, TWL4030_MADC_CTRL1);
	if (on)
		regval |= TWL4030_MADC_MADCON;
	else
		regval &= ~TWL4030_MADC_MADCON;
	twl4030_madc_write(madc, TWL4030_MADC_CTRL1, regval);

	return 0;
}

static long twl4030_madc_ioctl(struct file *filp, unsigned int cmd,
			       unsigned long arg)
{
	struct twl4030_madc_user_parms par;
	int val, ret;

	ret = copy_from_user(&par, (void __user *) arg, sizeof(par));
	if (ret) {
		dev_dbg(the_madc->dev, "copy_from_user: %d\n", ret);
		return -EACCES;
	}

	switch (cmd) {
	case TWL4030_MADC_IOCX_ADC_RAW_READ: {
		struct twl4030_madc_request req;
		if (par.channel >= TWL4030_MADC_MAX_CHANNELS)
			return -EINVAL;

		req.channels = (1 << par.channel);
		req.do_avg	= par.average;
		req.method	= TWL4030_MADC_SW1;
		req.func_cb	= NULL;

		val = twl4030_madc_conversion(&req);
		if (val <= 0) {
			par.status = -1;
		} else {
			par.status = 0;
			par.result = (u16)req.rbuf[par.channel];
		}
		break;
					     }
	default:
		return -EINVAL;
	}

	ret = copy_to_user((void __user *) arg, &par, sizeof(par));
	if (ret) {
		dev_dbg(the_madc->dev, "copy_to_user: %d\n", ret);
		return -EACCES;
	}

	return 0;
}

static const struct file_operations twl4030_madc_fileops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = twl4030_madc_ioctl
};

static struct miscdevice twl4030_madc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "twl4030-madc",
	.fops = &twl4030_madc_fileops
};

#ifdef CONFIG_LGE_OMAP3_EXT_PWR
/* boot argument from boot loader */
static s32 __init uboot_ext_pwr_state(char *str)
{
	sscanf(str, "%d", &usb_type);
	printk(KERN_INFO "%s: uboot ext power type %d\n", __func__, usb_type);
	return 0;
}
__setup("extpwrtype=", uboot_ext_pwr_state);
#endif

static int __devinit twl4030_madc_probe(struct platform_device *pdev)
{
	struct twl4030_madc_data *madc;
	struct twl4030_madc_platform_data *pdata = pdev->dev.platform_data;
	int ret = 0;
	u8 regval = 0;

	madc = kzalloc(sizeof *madc, GFP_KERNEL);
	if (!madc)
		return -ENOMEM;

	madc->dev = &pdev->dev;

	if (!pdata) {
		dev_dbg(&pdev->dev, "platform_data not available\n");
		ret = -EINVAL;
		goto err_pdata;
	}

	madc->imr = (pdata->irq_line == 1) ? TWL4030_MADC_IMR1
						: TWL4030_MADC_IMR2;
	madc->isr = (pdata->irq_line == 1) ? TWL4030_MADC_ISR1
						: TWL4030_MADC_ISR2;



	ret = misc_register(&twl4030_madc_device);
	if (ret) {
		dev_dbg(&pdev->dev, "could not register misc_device\n");
		goto err_misc;
	}
	twl4030_madc_set_power(madc, 1);
	twl4030_madc_set_current_generator(madc, 0, 1);

	ret = twl_i2c_read_u8(TWL4030_MODULE_MAIN_CHARGE,
				  &regval, TWL4030_BCI_BCICTL1);

	regval |= TWL4030_BCI_MESBAT;

	ret = twl_i2c_write_u8(TWL4030_MODULE_MAIN_CHARGE,
				   regval, TWL4030_BCI_BCICTL1);

	ret = request_irq(platform_get_irq(pdev, 0), twl4030_madc_irq_handler,
			  0, "twl4030_madc", madc);
	if (ret) {
		dev_dbg(&pdev->dev, "could not request irq\n");
		goto err_irq;
	}

	platform_set_drvdata(pdev, madc);
	mutex_init(&madc->lock);
	INIT_WORK(&madc->ws, twl4030_madc_work);

	the_madc = madc;

#ifdef CONFIG_LGE_OMAP3_EXT_PWR
	set_madc_probe_done(1);
#endif 

	return 0;

err_irq:
	misc_deregister(&twl4030_madc_device);

err_misc:
err_pdata:
	kfree(madc);

	return ret;
}

static int __devexit twl4030_madc_remove(struct platform_device *pdev)
{
	struct twl4030_madc_data *madc = platform_get_drvdata(pdev);

	twl4030_madc_set_power(madc, 0);
	twl4030_madc_set_current_generator(madc, 0, 0);
	free_irq(platform_get_irq(pdev, 0), madc);
	cancel_work_sync(&madc->ws);
	misc_deregister(&twl4030_madc_device);

	return 0;
}

static struct platform_driver twl4030_madc_driver = {
	.probe		= twl4030_madc_probe,
	.remove		= __devexit_p(twl4030_madc_remove),
	.driver		= {
		.name	= "twl4030_madc",
		.owner	= THIS_MODULE,
	},
};

static int __init twl4030_madc_init(void)
{
	return platform_driver_register(&twl4030_madc_driver);
}
module_init(twl4030_madc_init);

static void __exit twl4030_madc_exit(void)
{
	platform_driver_unregister(&twl4030_madc_driver);
}
module_exit(twl4030_madc_exit);

MODULE_ALIAS("platform:twl4030-madc");
MODULE_AUTHOR("Nokia Corporation");
MODULE_DESCRIPTION("twl4030 ADC driver");
MODULE_LICENSE("GPL");
