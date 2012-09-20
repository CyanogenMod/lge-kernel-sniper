/*
 * hdmi_panel.c
 *
 * HDMI library support functions for TI OMAP4 processors.
 *
 * Copyright (C) 2010-2011 Texas Instruments Incorporated - http://www.ti.com/
 * Authors:	Mythri P k <mythripk@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <video/omapdss.h>
#include <linux/switch.h>
#include <linux/gpio.h>
#include <linux/debugfs.h>
#include <linux/delay.h>

#include "dss.h"

#define HDMI_GPIO_HPD	107
#define KERN_WARN KERN_WARNING

static struct {
	struct mutex hdmi_lock;
	struct switch_dev hpd_switch;
} hdmi;

enum {
	HPD_STATE_OFF,
	HPD_STATE_START,
	HPD_STATE_EDID_TRYLAST = HPD_STATE_START + 5,
};

static struct hpd_worker_data {
	struct delayed_work dwork;
	atomic_t state;
} hpd_work;
static struct workqueue_struct *my_workq;

static void hdmi_hotplug_detect_worker(struct work_struct *work)
{
	struct hpd_worker_data *d = container_of(work, typeof(*d), dwork.work);
	struct omap_dss_device *dssdev = NULL;
	int state = atomic_read(&d->state);

	int match(struct omap_dss_device *dssdev, void *arg)
	{
		return sysfs_streq(dssdev->name , "hdmi");
	}
	dssdev = omap_dss_find_device(NULL, match);

	pr_err("in hpd work %d, state=%d\n", state, dssdev->state);
	if (dssdev == NULL)
		return;

	mutex_lock(&hdmi.hdmi_lock);
	if (state == HPD_STATE_OFF)
        switch_set_state(&hdmi.hpd_switch, 0);
	else if (state == HPD_STATE_START)
		switch_set_state(&hdmi.hpd_switch, 1);

	mutex_unlock(&hdmi.hdmi_lock);
}

int hdmi_panel_hpd_handler(int hpd)
{
	__cancel_delayed_work(&hpd_work.dwork);
	atomic_set(&hpd_work.state, hpd ? HPD_STATE_START : HPD_STATE_OFF);
	queue_delayed_work(my_workq, &hpd_work.dwork, 
					msecs_to_jiffies(hpd ? 40 : 100));
	return 0;
}

int hdmi_get_current_hpd()
{
	return gpio_get_value(HDMI_GPIO_HPD);
}

static irqreturn_t hpd_irq_handler(int irq, void *ptr)
{
	u32 gpio_reg;
	int hpd = hdmi_get_current_hpd();
	printk(KERN_WARN "hpd %d\n", hpd);

	hdmi_panel_hpd_handler(hpd);

	return IRQ_HANDLED;
}
static int sil9022_hdmi_disable(void)
{
    // wooho.jeong@lgec.com 2012.07.13
    // ADD : for HDMI Display Size improved
	hdmi_panel_hpd_handler(0);
    //switch_set_state(&hdmi.hpd_switch, 0);
	return 0;
}

static int sil9022_hdmi_enable(void)
{
    // wooho.jeong@lgec.com 2012.07.13
    // ADD : for HDMI Display Size improved
	hdmi_panel_hpd_handler(1);
    //switch_set_state(&hdmi.hpd_switch, 1);
	return 0;
}

DEFINE_SIMPLE_ATTRIBUTE(pm_hdmi_fops, NULL, sil9022_hdmi_enable, "%llu\n");
DEFINE_SIMPLE_ATTRIBUTE(pm_lcd_fops, NULL, sil9022_hdmi_disable, "%llu\n");

int hdmi_panel_init(void)
{
	int r;
	struct dentry *dbg_dir_hdmi;

	mutex_init(&hdmi.hdmi_lock);
    // goochang.jeong@lge.com 2012.06.11
    // MOD : for HDMI Audio Collision
	hdmi.hpd_switch.name = "hdmi";
    //hdmi.hpd_switch.name = "nxp_hdmi";
	switch_dev_register(&hdmi.hpd_switch);
#if 0
	r = request_irq(gpio_to_irq(HDMI_GPIO_HPD), hpd_irq_handler,
		IRQF_DISABLED | IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
							"hpd", NULL);
	if (r < 0) {
			pr_err("hdmi: request_irq %d failed\n",
						gpio_to_irq(HDMI_GPIO_HPD));
		return -EINVAL;
}
#endif
	my_workq = create_singlethread_workqueue("hdmi_hotplug");
	INIT_DELAYED_WORK(&hpd_work.dwork, hdmi_hotplug_detect_worker);

#if 0
	if (hdmi_get_current_hpd())
		hdmi_panel_hpd_handler(1);
#endif
	dbg_dir_hdmi = debugfs_create_dir("sil9022_hdmi", NULL);
	(void) debugfs_create_file("switch_to_hdmi", S_IRUGO | S_IWUSR,
					dbg_dir_hdmi, NULL, &pm_hdmi_fops);
	(void) debugfs_create_file("switch_to_lcd", S_IRUGO | S_IWUSR,
					dbg_dir_hdmi, NULL, &pm_lcd_fops);
	return 0;
}

void hdmi_panel_exit(void)
{
#if 0
	free_irq(gpio_to_irq(HDMI_GPIO_HPD), hpd_irq_handler);
#endif
	destroy_workqueue(my_workq);
	switch_dev_unregister(&hdmi.hpd_switch);
}
