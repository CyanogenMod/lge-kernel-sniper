
#ifndef _HUB_HEADSET_DET_H
#define _HUB_HEADSET_DET_H

typedef enum {
    HUB_NONE,
	HUB_HEADSET,
	HUB_HEADPHONE,
	
}headset_type_enum;

typedef enum {
    HOOK_PRESSED,
	HOOK_RELEASED
}hook_status_enum;

unsigned int get_headset_type(void);

#endif /* _LINUX_SYNAPTICS_I2C_RMI_H */

