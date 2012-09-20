/*
 * linux/arch/arm/mach-omap2/board-hub-debug.c
 *
 * Copyright (C) 2010 LG Electronic Inc.
 * Sung Kyun Yu <sunggyun.yu@lge.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/reboot.h>


/*20110310 seven.kim@lge.com to prevent IRQ during soft reset [START] */
#ifdef CONFIG_TOUCHSCREEN_JUSTIN_KR_SYNAPTICS
extern void synaptics_ts_disable_irq();
#endif
/*20110310 seven.kim@lge.com to prevent IRQ during soft reset [END] */

/* LGE_CHANGE_S [skykrkrk@lge.com] 2009-12-07, Error handling */
#ifdef CONFIG_FRAMEBUFFER_CONSOLE
extern void lge_hub_display_message_on_screen(const char *buf);
extern int log_buf_copy(char *dest, int idx, int len);
#endif

char reset_mode = 0;
#ifdef CONFIG_HIDDEN_RESET
extern int hidden_reset_enabled;
#else
static int hidden_reset_enabled = 0;
#endif // CONFIG_HIDDEN_RESET
extern void resume_console();

static int lge_hub_panic_event(struct notifier_block *this,
			       unsigned long event, void *ptr)
{
	printk("[SHYUN] [%s] - [%d] [IN]\n", __func__, __LINE__);

	resume_console();

/*20110310 seven.kim@lge.com to prevent IRQ during soft reset [START] */
#ifdef CONFIG_TOUCHSCREEN_JUSTIN_KR_SYNAPTICS
	synaptics_ts_disable_irq();
#endif
/*20110310 seven.kim@lge.com to prevent IRQ during soft reset [END] */

	if (hidden_reset_enabled) {
		reset_mode = 'h';  /* SW Hidden Reset */
        } else {
#ifdef CONFIG_FRAMEBUFFER_CONSOLE
#if 0
		char prefix_buf[120];	/* 60 * 2 */
		char print_buf[2880];	/* 60 * (50 - 2) */
		char crash_info_buf[3000];
#else
		char print_buf[3000];	/* 60 * 50 */
#endif
		int idx = 0;
		char *start_offset = 0;
		int start_len = 0;
		char *backtrace_offset = 0;
		char *backtrace_end = 0;

#if 0
		sprintf(prefix_buf, "OMAP CRASH! The system has rebooted.\nPress [Volume Down] to continue booting...[EndOfPanic]\n");
#endif

		printk(KERN_EMERG "OMAP CRASH! The system has rebooted.\nPress [Volume Down] to continue booting...[EndOfPanic]\n");
		while (log_buf_copy(print_buf, idx, ARRAY_SIZE(print_buf) - 1) > 0) {
			print_buf[ARRAY_SIZE(print_buf)-1] = 0;
			start_offset = strstr(print_buf, "Internal error:");
			if (start_offset)
			{
				start_offset = idx + start_offset - print_buf;
				break;
			}
			idx += (ARRAY_SIZE(print_buf) - 16);
		}

		while (log_buf_copy(print_buf, idx, ARRAY_SIZE(print_buf) - 1) > 0) {
//#ifdef CONFIG_ARM_UNWIND
//			backtrace_offset = strstr(print_buf, "[<");
//#else
			backtrace_offset = strstr(print_buf, "Backtrace:");
//#endif
			if (backtrace_offset)
			{
				backtrace_offset = idx + backtrace_offset - print_buf;
				break;
			}
			idx += (ARRAY_SIZE(print_buf) - 11);
		}

		memset(print_buf, 0, ARRAY_SIZE(print_buf));
		log_buf_copy(print_buf, (int)start_offset, ARRAY_SIZE(print_buf) - 1);
		start_len = (int)strstr(print_buf, "Stack: ");
		if (start_len)
			start_len -= (int)print_buf;

		if (backtrace_offset) {
			log_buf_copy((char *)(print_buf + start_len), (int)backtrace_offset, ARRAY_SIZE(print_buf) - start_len - 1);
			backtrace_end = (int)strstr(print_buf + start_len, "[EndOfPanic]");
			if(backtrace_end)
				*backtrace_end = 0;
		}

#if 0
		memcpy(crash_info_buf, prefix_buf, ARRAY_SIZE(prefix_buf));
		memcpy(crash_info_buf + ARRAY_SIZE(prefix_buf), print_buf, ARRAY_SIZE(print_buf));
#endif

		lge_hub_display_message_on_screen(print_buf);

#endif /* CONFIG_FRAMEBUFFER_CONSOLE */

		reset_mode = 'p';
	}

	emergency_restart();

	return NOTIFY_DONE;
}

static int lge_hub_reboot_event(struct notifier_block *this,
				unsigned long event, void *cmd)
{
	printk("[SHYUN] [%s] - [%d] [IN]\n", __func__, __LINE__);

	if (event == SYS_HALT) {
		/* TODO */
	} else if (event == SYS_RESTART) {
		if (cmd) {
			if (!strcmp((char*)cmd, "recovery"))
				reset_mode = 'f';
			else if (!strcmp((char*)cmd, "download"))
				reset_mode = 'd';
			else if (!strcmp((char*)cmd, "ftm"))
				reset_mode = 't';
			else if (!strcmp((char*)cmd, "frdone"))
				reset_mode = 'F';
			else if (!strcmp((char*)cmd, "hidden"))
				reset_mode = 'h';
			else if (!strcmp((char*)cmd, "android"))
				if (hidden_reset_enabled)
					reset_mode = 'h';
		}
	}
/* 20110310 seven.kim@lge.com to prevent IRQ during soft reset [START] */
#ifdef CONFIG_TOUCHSCREEN_JUSTIN_KR_SYNAPTICS
	synaptics_ts_disable_irq();
#endif
/* 20110310 seven.kim@lge.com to prevent IRQ during soft reset [END] */
	return NOTIFY_DONE;
}

struct notifier_block lge_hub_panic_nb = {
	.notifier_call = lge_hub_panic_event,
};

struct notifier_block lge_hub_reboot_nb = {
	.notifier_call = lge_hub_reboot_event,
};

extern int register_reboot_notifier(struct notifier_block *nb);

static int hub_debug_init(void)
{
	printk("%s\n", __func__);
	atomic_notifier_chain_register(&panic_notifier_list, &lge_hub_panic_nb);
	register_reboot_notifier(&lge_hub_reboot_nb);
	return 0;
}

module_init(hub_debug_init);

MODULE_DESCRIPTION("Sniper Debugging System Driver");
MODULE_AUTHOR("Sunggyun Yu <sunggyun.yu@lge.com>");
MODULE_LICENSE("GPL");

/* LGE_CHANGE_E [skykrkrk@lge.com] 2009-12-07, Error handling */
