/**
 * twl4030-pwrbutton.c - TWL4030 Power Button Input Driver
 *
 * Copyright (C) 2008-2009 Nokia Corporation
 *
 * Written by Peter De Schrijver <peter.de-schrijver@nokia.com>
 * Several fixes by Felipe Balbi <felipe.balbi@nokia.com>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License. See the file "COPYING" in the main directory of this
 * archive for more details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c/twl.h>

#if defined(CONFIG_MACH_LGE_OMAP3)
#include <linux/workqueue.h>	// 100920 sookyoung.kim@lge.com For INIT_WORK()
#include <linux/delay.h>		// 100920 sookyoung.kim@lge.com
#endif

#define PWR_PWRON_IRQ (1 << 0)

#define STS_HW_CONDITIONS 0xf

#if defined(CONFIG_MACH_LGE_OMAP3)
static struct workqueue_struct *pwrbutton_wq;	// 100920 sookyoung.kim@lge.com
static struct work_struct pwrbutton_wk;		// 100920 sookyoung.kim@lge.com
static struct input_dev *pwr;			// 100920 sookyoung.kim@lge.com
#endif

#if defined(CONFIG_MACH_LGE_OMAP3)
// 20100920 sookyoung.kim@lge.com Refine IRQ handlder with a work queue [START_LGE]
static void pwrbutton_wq_func(struct work_struct *work)
{

	int err;
	u8 value;

	err = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &value, STS_HW_CONDITIONS);
	if (!err)  {

		if((value & PWR_PWRON_IRQ) == 0)
			printk("[PWR_BUTTON Release]\n");
		else if((value & PWR_PWRON_IRQ) == 1)
			printk("[PWR_BUTTON Press]\n");
		else
			printk("[PWR_BUTTON]\n");

		input_report_key(pwr, KEY_POWER, value & PWR_PWRON_IRQ);
		input_sync(pwr);
	}
	else {
		dev_err(pwr->dev.parent, "twl4030: i2c error %d while reading"
			" TWL4030 PM_MASTER STS_HW_CONDITIONS register\n", err);
	}
}

static irqreturn_t powerbutton_irq(int irq, void *_pwr)
{
	pwr = _pwr;

	queue_work(pwrbutton_wq, &pwrbutton_wk);
	return IRQ_HANDLED;
}
// 20100920 sookyoung.kim@lge.com Refine IRQ handlder with a work queue [END_LGE]
#else // defined(CONFIG_MACH_LGE_OMAP3)
static irqreturn_t powerbutton_irq(int irq, void *_pwr)
{
	struct input_dev *pwr = _pwr;
	int err;
	u8 value;

	err = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &value,
				STS_HW_CONDITIONS);
	if (!err) {
		input_report_key(pwr, KEY_POWER, value & PWR_PWRON_IRQ);
		input_sync(pwr);
	} 
	else {
		dev_err(pwr->dev.parent, "twl4030: i2c error %d while reading"
			" TWL4030 PM_MASTER STS_HW_CONDITIONS register\n", err);
	}

	return IRQ_HANDLED;
}
#endif // defined(CONFIG_MACH_LGE_OMAP3)

static int __devinit twl4030_pwrbutton_probe(struct platform_device *pdev)
{
#if !defined(CONFIG_MACH_LGE_OMAP3)
	struct input_dev *pwr;
#endif
	int irq = platform_get_irq(pdev, 0);
	int err;

#if defined(CONFIG_MACH_LGE_OMAP3)
	pwrbutton_wq = create_workqueue("pwrbutton_workqueue");	// 100920 sookyoung.kim@lge.com
#endif

	pwr = input_allocate_device();
	if (!pwr) {
		dev_dbg(&pdev->dev, "Can't allocate power button\n");
		return -ENOMEM;
	}

	pwr->evbit[0] = BIT_MASK(EV_KEY);
	pwr->keybit[BIT_WORD(KEY_POWER)] = BIT_MASK(KEY_POWER);
	pwr->name = "twl4030_pwrbutton";
	pwr->phys = "twl4030_pwrbutton/input0";
	pwr->dev.parent = &pdev->dev;

#if defined(CONFIG_MACH_LGE_OMAP3)
	INIT_WORK(&pwrbutton_wk, pwrbutton_wq_func);
	err = request_irq(irq, powerbutton_irq,
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			"twl4030_pwrbutton", pwr);
#else
	err = request_threaded_irq(irq, NULL, powerbutton_irq,
			IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
			"twl4030_pwrbutton", pwr);
#endif
	if (err < 0) {
		dev_dbg(&pdev->dev, "Can't get IRQ for pwrbutton: %d\n", err);
		goto free_input_dev;
	}

	err = input_register_device(pwr);
	if (err) {
		dev_dbg(&pdev->dev, "Can't register power button: %d\n", err);
		goto free_irq;
	}

	platform_set_drvdata(pdev, pwr);

	return 0;

free_irq:
	free_irq(irq, NULL);
free_input_dev:
	input_free_device(pwr);
	return err;
}

static int __devexit twl4030_pwrbutton_remove(struct platform_device *pdev)
{
	struct input_dev *pwr = platform_get_drvdata(pdev);
	int irq = platform_get_irq(pdev, 0);

	free_irq(irq, pwr);
	input_unregister_device(pwr);

#if defined(CONFIG_MACH_LGE_OMAP3)
	flush_workqueue(pwrbutton_wq);	// 100920 sookyoung.kim@lge.com
	destroy_workqueue(pwrbutton_wq);	// 100920 sookyoung.kim@lge.com
#endif

	return 0;
}

struct platform_driver twl4030_pwrbutton_driver = {
	.probe		= twl4030_pwrbutton_probe,
	.remove		= __devexit_p(twl4030_pwrbutton_remove),
	.driver		= {
		.name	= "twl4030_pwrbutton",
		.owner	= THIS_MODULE,
	},
};

static int __init twl4030_pwrbutton_init(void)
{
	return platform_driver_register(&twl4030_pwrbutton_driver);
}
module_init(twl4030_pwrbutton_init);

static void __exit twl4030_pwrbutton_exit(void)
{
	platform_driver_unregister(&twl4030_pwrbutton_driver);
}
module_exit(twl4030_pwrbutton_exit);

MODULE_ALIAS("platform:twl4030_pwrbutton");
MODULE_DESCRIPTION("Triton2 Power Button");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Peter De Schrijver <peter.de-schrijver@nokia.com>");
MODULE_AUTHOR("Felipe Balbi <felipe.balbi@nokia.com>");

