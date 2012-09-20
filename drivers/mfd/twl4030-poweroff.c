/*
 * driver/mfd/twl4030-poweroff.c
 *
 * Power off device
 *
 * Copyright (C) 2008 Nokia Corporation
 *
 * Written by Peter De Schrijver <peter.de-schrijver@nokia.com>
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
#include <linux/pm.h>
#include <linux/i2c/twl.h>

#define PWR_P1_SW_EVENTS	0x10
#define PWR_DEVOFF	(1<<0)
#define PWR_STOPON_POWERON (1<<6)

#define CFG_P123_TRANSITION	0x03
#define SEQ_OFFSYNC	(1<<0)

// 20100430 sookyoung.kim@lge.com TI patch for additional item 1, on/off by power key [START_LGE]
#define POWERON_BUGFIX
#ifdef POWERON_BUGFIX
#define TWL4030_VDD1_TYPE	0x56
#define TWL4030_VDD2_TYPE	0x64
#define TWL4030_CLKEN_TYPE	0x86
#define TWL4030_HFCLKOUT_TYPE	0x8C
#endif
// 20100430 sookyoung.kim@lge.com TI patch for additional item 1, on/off by power key [END_LGE]

// 20100503 sookyoung.kim@lge.com Reduce PWRANA2 register power for power off state [START_LEG]
#define PHY_TO_OFF_PM_MASTER(p)		(p - 0x36)
#define PHY_TO_OFF_PM_RECEIVER(p)	(p - 0x5B)
#define R_PROTECT_KEY			PHY_TO_OFF_PM_MASTER(0x44)
#define R_CFG_PWRANA2			PHY_TO_OFF_PM_MASTER(0x3F)
// 20100503 sookyoung.kim@lge.com Reduce PWRANA2 register power for power off state [END_LEG]

#define     STARTON_CONDITION   0xEF
extern void twl_rewrite_starton_condition(u8 condition);

void twl4030_poweroff(void)
{
	u8 uninitialized_var(val);
	int err;
// 20100503 sookyoung.kim@lge.com Reduce PWRANA2 register power for power off state [START_LEG]
	u8 pwrana2_val = 0;
// 20100503 sookyoung.kim@lge.com Reduce PWRANA2 register power for power off state [END_LEG]

	twl_rewrite_starton_condition(STARTON_CONDITION);
// 20100504 sookyoung.kim@lge.com Reduce PWRANA2 register power for power off state [START_LEG]
	// Unlock PROTECT_KEY reg's KEY_TST flag.
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0E, R_PROTECT_KEY);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xE0, R_PROTECT_KEY);
	// Get the current PWRANA2 value.
	twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &pwrana2_val, R_CFG_PWRANA2);
	// Make LOJIT1_LOWV (bit 2) and LOJIT0_LOWV (bit 1) 0.
	pwrana2_val &= 0xF9;
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, pwrana2_val, R_CFG_PWRANA2);
	// Lock back PROTECT_KEY reg's KEY_TST flag.
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xFF, R_PROTECT_KEY);
	printk(KERN_WARNING "[kernel: twl4030_poweroff] PWRANA2 was set to high jitter mode\n");
// 20100504 sookyoung.kim@lge.com Reduce PWRANA2 register power for power off state [END_LGE]

// 20100430 sookyoung.kim@lge.com TI patch for additional item 1, on/off by power key [START_LGE]
#ifdef POWERON_BUGFIX
	u8 vdd1_type_value, vdd2_type_value, clken_type_value, hfclkout_type_value;

	// set VDD1_TYPE to TYPE4
	vdd1_type_value = 0x04;
	err = twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, vdd1_type_value, TWL4030_VDD1_TYPE);
	if (err) {
		pr_warning("I2C error %d while writing TWL4030 PM_RECEIVER TWL4030_VDD1_TYPE\n", err);
		return;
	}

	// set VDD2_TYPE to TYPE3
	vdd2_type_value = 0x03;
	err = twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, vdd2_type_value, TWL4030_VDD2_TYPE);
	if (err) {
		pr_warning("I2C error %d while writing TWL4030 PM_RECEIVER TWL4030_VDD2_TYPE\n", err);
		return;
	}

	// set CLKEN_TYPE to TYPE3
	clken_type_value = 0x03;
	err = twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, clken_type_value, TWL4030_CLKEN_TYPE);
	if (err) {
		pr_warning("I2C error %d while writing TWL4030 PM_RECEIVER TWL4030_CLKEN_TYPE\n", err);
		return;
	}

	// set HFCLKOUT_TYPE
	hfclkout_type_value = 0x00;
	err = twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, hfclkout_type_value, TWL4030_HFCLKOUT_TYPE);
	if (err) {
		pr_warning("I2C error %d while writing TWL4030 PM_RECEIVER TWL4030_HFCLKOUT_TYPE\n", err);
		return;
	}
#endif
// 20100430 sookyoung.kim@lge.com TI patch for additional item 1, on/off by power key [END_LGE]

	/* Make sure SEQ_OFFSYNC is set so that all the res goes to wait-on */
	err = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &val,
				   CFG_P123_TRANSITION);
	if (err) {
		pr_warning("I2C error %d while reading TWL4030 PM_MASTER CFG_P123_TRANSITION\n",
			err);
		return;
	}

	val |= SEQ_OFFSYNC;
	err = twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, val,
				    CFG_P123_TRANSITION);
	if (err) {
		pr_warning("I2C error %d while writing TWL4030 PM_MASTER CFG_P123_TRANSITION\n",
			err);
		return;
	}

	err = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &val,
				  PWR_P1_SW_EVENTS);
	if (err) {
		pr_warning("I2C error %d while reading TWL4030 PM_MASTER P1_SW_EVENTS\n",
			err);
		return;
	}

	val |= PWR_STOPON_POWERON | PWR_DEVOFF;

	err = twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, val,
				   PWR_P1_SW_EVENTS);

	if (err) {
		pr_warning("I2C error %d while writing TWL4030 PM_MASTER P1_SW_EVENTS\n",
			err);
		return;
	}

	return;
}

static int __init twl4030_poweroff_init(void)
{
// 20100503 sookyoung.kim@lge.com Reduce PWRANA2 register power for power off state [START_LEG]
	u8 pwrana2_val = 0;
// 20100503 sookyoung.kim@lge.com Reduce PWRANA2 register power for power off state [END_LEG]

// 20100504 sookyoung.kim@lge.com Reduce PWRANA2 register power for power off state [START_LEG]
	// Unlock PROTECT_KEY reg's KEY_TST flag.
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0x0E, R_PROTECT_KEY);
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xE0, R_PROTECT_KEY);
	// Get the current PWRANA2 value.
	twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &pwrana2_val, R_CFG_PWRANA2);
	// Make LOJIT1_LOWV (bit 2) and LOJIT0_LOWV (bit 1) 1.
	pwrana2_val &= 0xFF;
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, pwrana2_val, R_CFG_PWRANA2);
	// Lock back PROTECT_KEY reg's KEY_TST flag.
	twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, 0xFF, R_PROTECT_KEY);
	printk(KERN_WARNING "[kernel: twl4030_poweroff_init] PWRANA2 was set to low jitter mode\n");
// 20100504 sookyoung.kim@lge.com Reduce PWRANA2 register power for power off state [END_LGE]

	pm_power_off = twl4030_poweroff;

	return 0;
}

static void __exit twl4030_poweroff_exit(void)
{
	pm_power_off = NULL;
}

module_init(twl4030_poweroff_init);
module_exit(twl4030_poweroff_exit);

MODULE_ALIAS("i2c:twl4030-poweroff");
MODULE_DESCRIPTION("Triton2 device power off");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Peter De Schrijver");
