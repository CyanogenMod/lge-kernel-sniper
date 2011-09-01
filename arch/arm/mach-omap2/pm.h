/*
 * OMAP2/3 Power Management Routines
 *
 * Copyright (C) 2008 Nokia Corporation
 * Jouni Hogander
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __ARCH_ARM_MACH_OMAP2_PM_H
#define __ARCH_ARM_MACH_OMAP2_PM_H

#include <plat/powerdomain.h>
#include <linux/init.h>

extern u32 enable_off_mode;
extern u32 sleep_while_idle;
extern u32 omap4_device_off_counter;

extern void *omap3_secure_ram_storage;
extern void omap3_pm_off_mode_enable(int);
extern void omap4_pm_off_mode_enable(int);
extern void omap_sram_idle(void);
extern int omap3_can_sleep(void);
extern int set_pwrdm_state(struct powerdomain *pwrdm, u32 state);
#ifdef CONFIG_PM
extern int omap4_set_pwrdm_state(struct powerdomain *pwrdm, u32 state);
extern u32 omap4_is_device_off_wakeup(void);
extern void omap4_device_off_set_state(u8 enable);
extern u32 omap4_device_off_read_prev_state(void);
extern u32 omap4_device_off_read_next_state(void);
#else
static inline int omap4_set_pwrdm_state(struct powerdomain *pwrdm, u32 state)
{
	return 0;
}
static inline void omap4_device_off_set_state(u8 enable)
{
}
static inline u32 omap4_device_off_read_prev_state(void)
{
	return 0;
}
static inline u32 omap4_device_off_read_next_state(void)
{
	return 0;
}
#endif
extern int omap3_idle_init(void);
#if defined(CONFIG_PM) && defined(CONFIG_ARCH_OMAP3)
extern void pm_alloc_secure_ram(void);
#else
static inline void pm_alloc_secure_ram(void) { }
#endif
extern int omap4_idle_init(void);
extern int omap4_can_sleep(void);
extern void omap4_enter_sleep(unsigned int cpu, unsigned int power_state);
extern void omap4_trigger_ioctrl(void);

struct prm_setup_vc {
	u16 clksetup;
	u16 voltsetup_time1;
	u16 voltsetup_time2;
	u16 voltoffset;
	u16 voltsetup2;

/* PRM_VC_CMD_VAL_0 specific bits */
	u16 vdd0_on;
	u16 vdd0_onlp;
	u16 vdd0_ret;
	u16 vdd0_off;
/* PRM_VC_CMD_VAL_1 specific bits */
	u16 vdd1_on;
	u16 vdd1_onlp;
	u16 vdd1_ret;
	u16 vdd1_off;

/* Values for VDD registers */
	u32 i2c_slave_ra;
	u32 vdd_vol_ra;
	u32 vdd_cmd_ra;
	u32 vdd_ch_conf;
	u32 vdd_i2c_cfg;
};

struct cpuidle_params {
	u8  valid;
	u32 sleep_latency;
	u32 wake_latency;
	u32 threshold;
};

#if defined(CONFIG_PM) && defined(CONFIG_CPU_IDLE)
extern void omap3_pm_init_cpuidle(struct cpuidle_params *cpuidle_board_params);
#else
static
inline void omap3_pm_init_cpuidle(struct cpuidle_params *cpuidle_board_params)
{
}
#endif

extern int omap3_pm_get_suspend_state(struct powerdomain *pwrdm);
extern int omap3_pm_set_suspend_state(struct powerdomain *pwrdm, int state);

extern u32 wakeup_timer_seconds;
extern u32 wakeup_timer_milliseconds;
extern struct omap_dm_timer *gptimer_wakeup;

#ifdef CONFIG_PM_DEBUG
extern void omap2_pm_dump(int mode, int resume, unsigned int us);
extern void omap2_pm_wakeup_on_timer(u32 seconds, u32 milliseconds);
extern int omap2_pm_debug;
#else
#define omap2_pm_dump(mode, resume, us)		do {} while (0);
#define omap2_pm_wakeup_on_timer(seconds, milliseconds)	do {} while (0);
#define omap2_pm_debug				0
#endif

#if defined(CONFIG_CPU_IDLE)
extern void omap3_cpuidle_update_states(void);
#endif

#if defined(CONFIG_PM_DEBUG) && defined(CONFIG_DEBUG_FS)
extern void pm_dbg_update_time(struct powerdomain *pwrdm, int prev);
extern int pm_dbg_regset_save(int reg_set);
extern int pm_dbg_regset_init(int reg_set);
#else
#define pm_dbg_update_time(pwrdm, prev) do {} while (0);
#define pm_dbg_regset_save(reg_set) do {} while (0);
#define pm_dbg_regset_init(reg_set) do {} while (0);
#endif /* CONFIG_PM_DEBUG */

extern void omap24xx_idle_loop_suspend(void);

extern void omap24xx_cpu_suspend(u32 dll_ctrl, void __iomem *sdrc_dlla_ctrl,
					void __iomem *sdrc_power);
extern void omap34xx_cpu_suspend(u32 *addr, int save_state);
extern void save_secure_ram_context(u32 *addr);
extern void omap3_save_scratchpad_contents(void);

extern unsigned int omap24xx_idle_loop_suspend_sz;
extern unsigned int omap34xx_suspend_sz;
extern unsigned int save_secure_ram_context_sz;
extern unsigned int omap24xx_cpu_suspend_sz;
extern unsigned int omap34xx_cpu_suspend_sz;

/**
 * struct omap3_secure_copy_data - describe behavior for the secure ram copy
 * @size:	size of copy to be saved - this is based on the PPA used
 *		secure ram size could be configured to various sizes, this is
 *		the size used + 64 byte header required.
 *
 * Different platforms use different security PPAs based on their unique needs.
 * This structure describes the delta behavior expected for these custom
 * platforms. The defaults are configured for official TI OMAP3 PPA behavior.
 */
struct omap3_secure_copy_data {
	u32 size;
};

#if defined(CONFIG_PM)
extern int __init omap3_secure_copy_data_set(struct omap3_secure_copy_data *d);
#else
static inline int omap3_secure_copy_data_set(struct omap3_secure_copy_data *d)
{
	return -EINVAL;
}
#endif

#endif
