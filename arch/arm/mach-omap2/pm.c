/*
 * pm.c - Common OMAP2+ power management-related code
 *
 * Copyright (C) 2010 Texas Instruments, Inc.
 * Copyright (C) 2010 Nokia Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/err.h>

// 20110425 prime@sdcmicro.com Patch for INTC autoidle management to make sure it is done in atomic operation with interrupt disabled [START]
#include <linux/notifier.h>
// 20110425 prime@sdcmicro.com Patch for INTC autoidle management to make sure it is done in atomic operation with interrupt disabled [END]

#include <plat/omap-pm.h>
#include <plat/omap_device.h>
#include <plat/common.h>

#ifdef CONFIG_LGE_DVFS
#include <linux/dvs_suite.h>
#endif	// CONFIG_LGE_DVFS

#include "omap3-opp.h"
#include "opp44xx.h"

// LGE_UPDATE_S
#if defined(CONFIG_MACH_LGE_OMAP3)
#include "pm.h"

u32 sleep_while_idle;
u32 enable_off_mode;
#endif
// LGE_UPDATE_E

// 20100520 jugwan.eom@lge.com For power on cause and hidden reset [START_LGE]
// TODO: make more pretty...
enum {
	RESET_NORMAL,
	RESET_CHARGER_DETECT,
	RESET_GLOBAL_SW_RESET,
	RESET_KERNEL_PANIC,
	RESET_HIDDEN_SW_RESET,
};

int reset_status = RESET_NORMAL;
int hidden_reset_enabled = 0;
static int hub_secure_mode = 0;

static ssize_t reset_status_show(struct kobject *, struct kobj_attribute *, char *);
static struct kobj_attribute reset_status_attr =
	__ATTR(reset_status, 0644, reset_status_show, NULL);

static ssize_t hidden_reset_show(struct kobject *, struct kobj_attribute *, char *);
static ssize_t hidden_reset_store(struct kobject *k, struct kobj_attribute *,
			  const char *buf, size_t n);
static struct kobj_attribute hidden_reset_attr =
	__ATTR(hidden_reset, 0644, hidden_reset_show, hidden_reset_store);
static ssize_t secure_mode_show(struct kobject *, struct kobj_attribute *, char *);
static struct kobj_attribute secure_mode_attr =
	__ATTR(secure_mode, 0644, secure_mode_show, NULL);

static void reset_status_setup(char *str)
{
        if (str[0] == 'p')
            reset_status = RESET_KERNEL_PANIC;
        else if (str[0] == 'h')
            reset_status = RESET_HIDDEN_SW_RESET;
        else if (str[0] == 'c')
            reset_status = RESET_CHARGER_DETECT;

        printk("reset_status: %c\n", str[0]);
}
__setup("rs=", reset_status_setup);

static void hub_secure_mode_setup(char *str)
{
	if (str[0] == '1')
		hub_secure_mode = 1;
	else 
		hub_secure_mode = 0;

	printk("hub_secure_mode: %d\n", hub_secure_mode);
}
__setup("secure=", hub_secure_mode_setup);
static ssize_t reset_status_show(struct kobject *kobj, struct kobj_attribute *attr,
			 char *buf)
{
	if (attr == &reset_status_attr)
		return sprintf(buf, "%d\n", reset_status);
	else
		return -EINVAL;
}

static ssize_t secure_mode_show(struct kobject *kobj, struct kobj_attribute *attr,
			 char *buf)
{
	if (attr == &secure_mode_attr)
		return sprintf(buf, "%d\n", hub_secure_mode);
	else
		return -EINVAL;
}

static ssize_t hidden_reset_show(struct kobject *kobj, struct kobj_attribute *attr,
			 char *buf)
{
	if (attr == &hidden_reset_attr)
		return sprintf(buf, "%d\n", hidden_reset_enabled);
	else
		return -EINVAL;
}
static ssize_t hidden_reset_store(struct kobject *kobj, struct kobj_attribute *attr,
			  const char *buf, size_t n)
{
	unsigned short value;

	if (sscanf(buf, "%hu", &value) != 1)
		return -EINVAL;

	if (attr == &hidden_reset_attr) {
                hidden_reset_enabled = value;
	} else {
		return -EINVAL;
	}
	return n;
}

static struct omap_device_pm_latency *pm_lats;

static struct device *mpu_dev;
static struct device *iva_dev;
static struct device *l3_dev;
static struct device *dsp_dev;

#if 1

/* idle notifications late in the idle path (atomic, interrupts disabled) */
static ATOMIC_NOTIFIER_HEAD(idle_notifier);

void omap_idle_notifier_register(struct notifier_block *n)
{
	atomic_notifier_chain_register(&idle_notifier, n);
}
EXPORT_SYMBOL_GPL(omap_idle_notifier_register);

void omap_idle_notifier_unregister(struct notifier_block *n)
{
	atomic_notifier_chain_unregister(&idle_notifier, n);
}
EXPORT_SYMBOL_GPL(omap_idle_notifier_unregister);

void omap_idle_notifier_start(void)
{
	atomic_notifier_call_chain(&idle_notifier, OMAP_IDLE_START, NULL);
}

void omap_idle_notifier_end(void)
{
	atomic_notifier_call_chain(&idle_notifier, OMAP_IDLE_END, NULL);
}

#endif
// 20110425 prime@sdcmicro.com Patch for INTC autoidle management to make sure it is done in atomic operation with interrupt disabled [END]

struct device *omap2_get_mpuss_device(void)
{
	WARN_ON_ONCE(!mpu_dev);
	return mpu_dev;
}
EXPORT_SYMBOL(omap2_get_mpuss_device);

struct device *omap2_get_iva_device(void)
{
	WARN_ON_ONCE(!iva_dev);
	return iva_dev;
}
EXPORT_SYMBOL(omap2_get_iva_device);

struct device *omap2_get_l3_device(void)
{
	WARN_ON_ONCE(!l3_dev);
	return l3_dev;
}
EXPORT_SYMBOL(omap2_get_l3_device);

struct device *omap4_get_dsp_device(void)
{
	WARN_ON_ONCE(!dsp_dev);
	return dsp_dev;
}
EXPORT_SYMBOL(omap4_get_dsp_device);

#ifdef CONFIG_OMAP_PM
static ssize_t vdd_opp_show(struct kobject *, struct kobj_attribute *, char *);
static ssize_t vdd_opp_store(struct kobject *k, struct kobj_attribute *,
			  const char *buf, size_t n);

static struct kobj_attribute vdd1_opp_attr =
	__ATTR(vdd1_opp, 0444, vdd_opp_show, vdd_opp_store);
static struct kobj_attribute vdd2_opp_attr =
	__ATTR(vdd2_opp, 0444, vdd_opp_show, vdd_opp_store);
static struct kobj_attribute vdd1_lock_attr =
	__ATTR(vdd1_lock, 0644, vdd_opp_show, vdd_opp_store);
static struct kobj_attribute vdd2_lock_attr =
	__ATTR(vdd2_lock, 0644, vdd_opp_show, vdd_opp_store);
static struct kobj_attribute dsp_freq_attr =
	__ATTR(dsp_freq, 0644, vdd_opp_show, vdd_opp_store);
static struct kobj_attribute tick_control_attr =
	__ATTR(tick, 0644, vdd_opp_show, vdd_opp_store);

static int vdd1_locked = 0;
static int vdd2_locked = 0;
static struct device sysfs_cpufreq_dev;
extern void tick_nohz_disable(int nohz);

static ssize_t vdd_opp_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	if (attr == &vdd1_opp_attr)
		return sprintf(buf, "%hu\n", opp_find_freq_exact(mpu_dev, opp_get_rate(mpu_dev), true)->opp_id+1);
	else if (attr == &vdd2_opp_attr)
		return sprintf(buf, "%hu\n", opp_find_freq_exact(l3_dev, opp_get_rate(l3_dev), true)->opp_id+1);
	else if (attr == &vdd1_lock_attr)
		return sprintf(buf, "%hu\n", vdd1_locked);
	else if (attr == &vdd2_lock_attr)
		return sprintf(buf, "%hu\n", vdd2_locked);
	else if (attr == &dsp_freq_attr)
		return sprintf(buf, "%lu\n", opp_get_rate(iva_dev)/1000);
	else
		return -EINVAL;
}

static ssize_t vdd_opp_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t n)
{
	unsigned long value;
#ifdef CONFIG_LGE_DVFS
	unsigned long lc_freq = 0;
#endif	// CONFIG_LGE_DVFS

	if (sscanf(buf, "%lu", &value) != 1)
		return -EINVAL;

	if (attr == &tick_control_attr) {
		if (value == 1)
			tick_nohz_disable(1);
		else if (value == 0)
			tick_nohz_disable(0);
	}
	/* Check locks */
	if (attr == &vdd1_lock_attr) {
		if (vdd1_locked) {
			/* vdd1 currently locked */
			if (value == 0) {
				if (omap_pm_set_min_mpu_freq(&sysfs_cpufreq_dev, -1)) {
					printk(KERN_ERR "%s: Failed to remove vdd1_lock\n", __func__);
				} else {
					vdd1_locked = 0;
#ifdef CONFIG_LGE_DVFS
					per_cpu(ds_sys_status, 0).locked_min_cpu_op_index = 300000000;
#endif	// CONFIG_LGE_DVFS
					return n;
				}
			} else {
				printk(KERN_ERR "%s: vdd1 already locked to %d\n", __func__, vdd1_locked);
				return -EINVAL;
			}
		} else {
			/* vdd1 currently unlocked */
			if (value != 0) {
				u8 i = 0;
				unsigned long freq = 0;
				struct cpufreq_frequency_table *freq_table = *omap_pm_cpu_get_freq_table();
				if (freq_table == NULL) {
					printk(KERN_ERR "%s: Could not get freq_table\n", __func__);
					return -ENODEV;
				}
				for (i = 0; freq_table[i].frequency != CPUFREQ_TABLE_END; i++) {
					if (freq_table[i].index == value - 1) {
						freq = freq_table[i].frequency;
#ifdef CONFIG_LGE_DVFS
						lc_freq = freq * 1000;
#endif	// CONFIG_LGE_DVFS
						break;
					}
				}
				if (freq_table[i].frequency == CPUFREQ_TABLE_END) {
					printk(KERN_ERR "%s: Invalid value [0..%d]\n", __func__, i-1);
					return -EINVAL;
				}
				if (omap_pm_set_min_mpu_freq(&sysfs_cpufreq_dev, freq * 1000)) {
					printk(KERN_ERR "%s: Failed to add vdd1_lock\n", __func__);
				} else {
#ifdef CONFIG_LGE_DVFS
					per_cpu(ds_sys_status, 0).locked_min_cpu_op_index = lc_freq;
#endif	// CONFIG_LGE_DVFS
					vdd1_locked = value;
				}
			} else {
				printk(KERN_ERR "%s: vdd1 already unlocked\n", __func__);
				return -EINVAL;
			}
		}
	} else if (attr == &vdd2_lock_attr) {
		if (vdd2_locked) {
			/* vdd2 currently locked */
			if (value == 0) {
				if (omap_pm_set_min_bus_tput(&sysfs_cpufreq_dev, OCP_INITIATOR_AGENT, 0)) {
					printk(KERN_ERR "%s: Failed to remove vdd2_lock\n", __func__);
				} else {
#ifdef CONFIG_LGE_DVFS
					per_cpu(ds_sys_status, 0).locked_min_l3_freq = 0;
#endif	// CONFIG_LGE_DVFS
					vdd2_locked = 0;
					return n;
				}
			} else {
				printk(KERN_ERR "%s: vdd2 already locked to %d\n", __func__, vdd2_locked);
				return -EINVAL;
			}
		} else {
			/* vdd2 currently unlocked */
			if (value != 0) {
				unsigned long freq = 0;
				if (cpu_is_omap3630()) {
					if(value == 1) {
						freq = 100*1000*4;
#ifdef CONFIG_LGE_DVFS
						lc_freq = 100000000;
#endif	// CONFIG_LGE_DVFS
					} else if (value == 2) {
						freq = 200*1000*4;
#ifdef CONFIG_LGE_DVFS
						lc_freq = 200000000;
#endif	// CONFIG_LGE_DVFS
					} else {
						printk(KERN_ERR "%s: Invalid value [1,2]\n", __func__);
						return -EINVAL;
					}
				}
				else if (cpu_is_omap44xx()) {
					if (omap_rev() <= OMAP4430_REV_ES2_0) {
						if(value == 1) {
							freq = 100*1000*4;
						} else if (value == 2) {
							freq = 200*1000*4;
						} else {
							printk(KERN_ERR "%s: Invalid value [1,2]\n", __func__);
							return -EINVAL;
						}
					} else {
						if(value == 1) {
							freq = 98304*4;
						} else if (value == 2) {
							freq = 100*1000*4;
						} else if (value == 3) {
							freq = 200*1000*4;
						} else {
							printk(KERN_ERR "%s: Invalid value [1,2,3]\n", __func__);
							return -EINVAL;
						}
					}
				} else {
					printk(KERN_ERR "%s: Unsupported HW [OMAP3630, OMAP44XX]\n", __func__);
					return -ENODEV;
				}
				if (omap_pm_set_min_bus_tput(&sysfs_cpufreq_dev, OCP_INITIATOR_AGENT, freq)) {
					printk(KERN_ERR "%s: Failed to add vdd2_lock\n", __func__);
				} else {
#ifdef CONFIG_LGE_DVFS
					per_cpu(ds_sys_status, 0).locked_min_l3_freq = lc_freq;
#endif	// CONFIG_LGE_DVFS
					vdd2_locked = value;
				}
				return n;
			} else {
				printk(KERN_ERR "%s: vdd2 already unlocked\n", __func__);
				return -EINVAL;
			}
		}
	} else if (attr == &dsp_freq_attr) {
		u8 i, opp_id = 0;
		struct omap_opp *opp_table = omap_pm_dsp_get_opp_table();
		if (opp_table == NULL) {
			printk(KERN_ERR "%s: Could not get dsp opp_table\n", __func__);
			return -ENODEV;
		}
		for (i = 1; opp_table[i].rate; i++) {
			if (opp_table[i].rate >= value) {
				opp_id = i;
#ifdef CONFIG_LGE_DVFS
				switch(i){
					case 1:
						per_cpu(ds_sys_status, 0).locked_min_cpu_op_index = 300000000;	// Unlocked.
						per_cpu(ds_sys_status, 0).locked_min_iva_freq = 260000000;
						break;
					case 2:
						per_cpu(ds_sys_status, 0).locked_min_cpu_op_index = 600000000;
						per_cpu(ds_sys_status, 0).locked_min_iva_freq = 520000000;
						break;
					case 3:
						per_cpu(ds_sys_status, 0).locked_min_cpu_op_index = 800000000;
						per_cpu(ds_sys_status, 0).locked_min_iva_freq = 660000000;
						break;
					case 4:
						per_cpu(ds_sys_status, 0).locked_min_cpu_op_index = 1000000000;
						per_cpu(ds_sys_status, 0).locked_min_iva_freq = 800000000;
						break;
					default:
						per_cpu(ds_sys_status, 0).locked_min_cpu_op_index = 1000000000;
						per_cpu(ds_sys_status, 0).locked_min_iva_freq = 800000000;
						break;
				}
#endif	// CONFIG_LGE_DVFS
				break;
			}
		}
		if (opp_id == 0) {
			printk(KERN_ERR "%s: Invalid value\n", __func__);
			return -EINVAL;
		}
		omap_pm_dsp_set_min_opp(opp_id);

	} else if (attr == &vdd1_opp_attr) {
		printk(KERN_ERR "%s: changing vdd1_opp is not supported\n", __func__);
		return -EINVAL;
	} else if (attr == &vdd2_opp_attr) {
		printk(KERN_ERR "%s: changing vdd2_opp is not supported\n", __func__);
		return -EINVAL;
	} else {
		return -EINVAL;
	}
	return n;
}
#endif

/* static int _init_omap_device(struct omap_hwmod *oh, void *user) */
static int _init_omap_device(char *name, struct device **new_dev)
{
	struct omap_hwmod *oh;
	struct omap_device *od;

	oh = omap_hwmod_lookup(name);
	if (WARN(!oh, "%s: could not find omap_hwmod for %s\n",
		 __func__, name))
		return -ENODEV;
	od = omap_device_build(oh->name, 0, oh, NULL, 0, pm_lats, 0, false);
	if (WARN(IS_ERR(od), "%s: could not build omap_device for %s\n",
		 __func__, name))
		return -ENODEV;

	*new_dev = &od->pdev.dev;

	return 0;
}

/*
 * Build omap_devices for processors and bus.
 */
static void omap2_init_processor_devices(void)
{
	struct omap_hwmod *oh;

	_init_omap_device("mpu", &mpu_dev);

	if (cpu_is_omap34xx())
		_init_omap_device("iva", &iva_dev);
	oh = omap_hwmod_lookup("iva");
	if (oh && oh->od)
		iva_dev = &oh->od->pdev.dev;

	oh = omap_hwmod_lookup("dsp");
	if (oh && oh->od)
		dsp_dev = &oh->od->pdev.dev;

	if (cpu_is_omap44xx())
		_init_omap_device("l3_main_1", &l3_dev);
	else
		_init_omap_device("l3_main", &l3_dev);
}

static int __init omap2_common_pm_init(void)
{
// LGE_UPDATE_S : come from pm.c
#if defined(CONFIG_MACH_LGE_OMAP3)
	sleep_while_idle = 0;  // temp... should be checked..
	enable_off_mode = 1;
#endif
// LGE_UPDATE_E : come from pm.c

	int error = -EINVAL;

	omap2_init_processor_devices();
	if (cpu_is_omap34xx())
		omap3_pm_init_opp_table();
	else if (cpu_is_omap44xx())
		omap4_pm_init_opp_table();

	omap_pm_if_init();

	error = sysfs_create_file(power_kobj, &reset_status_attr.attr);
	if (error) {
		printk(KERN_ERR "sysfs_create_file failed: %d\n", error);
		return error;
	}
	error = sysfs_create_file(power_kobj, &hidden_reset_attr.attr);
	if (error) {
		printk(KERN_ERR "sysfs_create_file failed: %d\n", error);
		return error;
	}
	if (hub_secure_mode) {
		error = sysfs_create_file(power_kobj, &secure_mode_attr.attr);
		if (error) {
			printk(KERN_ERR "sysfs_create_file failed: %d\n", error);
			return error;
		}
	}
#ifdef CONFIG_OMAP_PM
	{
		int error = -EINVAL;

		error = sysfs_create_file(power_kobj, &dsp_freq_attr.attr);
		if (error) {
			printk(KERN_ERR "%s: sysfs_create_file(dsp_freq) failed %d\n", __func__, error);
			return error;
		}
		error = sysfs_create_file(power_kobj, &vdd1_opp_attr.attr);
		if (error) {
			printk(KERN_ERR "%s: sysfs_create_file(vdd1_opp) failed %d\n", __func__, error);
			return error;
		}
		error = sysfs_create_file(power_kobj, &vdd2_opp_attr.attr);
		if (error) {
			printk(KERN_ERR "%s: sysfs_create_file(vdd2_opp) failed %d\n", __func__, error);
			return error;
		}
		error = sysfs_create_file(power_kobj, &vdd1_lock_attr.attr);
		if (error) {
			printk(KERN_ERR "%s: sysfs_create_file(vdd1_lock) failed %d\n", __func__ ,error);
			return error;
		}
		error = sysfs_create_file(power_kobj, &vdd2_lock_attr.attr);
		if (error) {
			printk(KERN_ERR "%s: sysfs_create_file(vdd2_lock) failed %d\n", __func__, error);
			return error;
		}
        error = sysfs_create_file(power_kobj, &tick_control_attr.attr);
        if (error) {
            printk(KERN_ERR "%s: sysfs_create_file(tick_control) failed: %d\n", __func__, error);
            return error;
        }
	}
#endif

	return 0;
}
device_initcall(omap2_common_pm_init);
