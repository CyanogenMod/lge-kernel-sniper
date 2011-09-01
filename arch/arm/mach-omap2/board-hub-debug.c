/*
 * linux/arch/arm/mach-omap2/board-hub-debug.c
 *
 * Copyright (C) 2010 LG Electronic Inc.
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

#ifdef CONFIG_ANDROID_RAM_CONSOLE
#define RAM_CONSOLE_START   (0x80000000 + 511 * SZ_1M)
#define RAM_CONSOLE_SIZE    (SZ_1M)
static struct resource ram_console_resource = {
	.start = RAM_CONSOLE_START,
	.end = (RAM_CONSOLE_START + RAM_CONSOLE_SIZE - 1),
	.flags = IORESOURCE_MEM,
};

struct platform_device hub_ram_console_device = {
	.name = "ram_console",
	.id = -1,
	.num_resources = 1,
	.resource = &ram_console_resource,
};
#endif

extern void synaptics_ts_disable_irq();

#ifdef CONFIG_FRAMEBUFFER_CONSOLE
extern void lge_hub_display_message_on_screen(const char *buf);
#endif
extern int log_buf_copy(char *dest, int idx, int len);
extern struct atomic_notifier_head panic_notifier_list;
extern void emergency_restart(void);
extern void reboot_setup(char *str);

char reset_mode = 0;
extern int hidden_reset_enabled;
extern void resume_console(void);

static int lge_hub_panic_event(struct notifier_block *this,
			       unsigned long event, void *ptr)
{
	//resume_console();

	synaptics_ts_disable_irq();

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
	synaptics_ts_disable_irq();
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

