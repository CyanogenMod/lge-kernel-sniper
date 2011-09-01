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
#include <linux/notifier.h>

#include <plat/omap-pm.h>
#include <plat/omap_device.h>
#include <plat/common.h>

#include <linux/dvs_suite.h>

#include "omap3-opp.h"
#include "opp44xx.h"

#if defined(CONFIG_MACH_LGE_OMAP3)
#include "pm.h"

u32 sleep_while_idle;
u32 enable_off_mode;
#endif

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
	if(ds_status.flag_correct_cpu_op_update_path == 0) 
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

	return 0;
}
device_initcall(omap2_common_pm_init);
