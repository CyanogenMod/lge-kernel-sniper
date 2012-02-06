/*
 * Header file for Modem Watcher
 */

#ifndef __LGE_MDM_WATCHER_H
#define __LGE_MDM_WATCHER_H

#include <linux/interrupt.h>

#define MDM_WATCHER_NAME "mdm_watcher"

typedef enum {
	MDM_HALT,
	MDM_AUTO_SHUTDOWN,
	MDM_EVENT_MAX,
} mdm_event_type;

struct mdm_watcher_event {
	mdm_event_type type;
	unsigned int gpio_irq;
	unsigned long irqf_flags;
	int msecs_delay;
	unsigned int key_code; /* key code to upper layer */
};

struct mdm_watcher_platform_data {
	struct mdm_watcher_event *event;
	unsigned len;
};

#endif /* __LGE_MDM_WATCHER_H */
