/*
 * drivers/media/video/ad5820.h
 *
 * Copyright (C) 2008 Texas Instruments.
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * Copyright (C) 2007 Texas Instruments.
 *
 * Register defines for Auto Focus device
 *
 */
#ifndef CAMAF_DW9716_H
#define CAMAF_DW9716_H

#ifdef __KERNEL__

#include <media/v4l2-int-device.h>
//#define DW9716_AF_I2C_ADDR	0x18
#define DW9716_AF_I2C_ADDR	0x0C
#define DW9716_NAME 		"DW9716"
#define DW9716_I2C_RETRY_COUNT	5
#define MAX_FOCUS_POS	0x3FF

#define CAMAF_DW9716_DISABLE		0x1
#define CAMAF_DW9716_ENABLE		0x0
#define CAMAF_DW9716_POWERDN(ARG)	(((ARG) & 0x1) << 15)
#define CAMAF_DW9716_POWERDN_R(ARG)	(((ARG) >> 15) & 0x1)

#define CAMAF_DW9716_DATA(ARG)		(((ARG) & 0x3FF) << 4)
#define CAMAF_DW9716_DATA_R(ARG)	(((ARG) >> 4) & 0x3FF)

#define CAMAF_DW9716_MODE(ARG)		((ARG) & 0xF)
#define CAMAF_DW9716_MODE_R(ARG)	((ARG) & 0xF)

#define CAMAF_FREQUENCY_EQ1(mclk)     	((u16)(mclk/16000))

/* ioctls definition */
#define		AF_IOC_BASE			       'R'
#define		AF_IOC_MAXNR				2

/*Ioctl options which are to be passed while calling the ioctl*/
#define	AF_SET_POSITION		_IOWR(AF_IOC_BASE, 1, int)
#define	AF_GET_POSITION		_IOWR(AF_IOC_BASE, 2, int)

/* State of lens */
#define LENS_DETECTED 1
#define LENS_NOT_DETECTED 0

/* Focus control values */
#define DEF_LENS_POSN	    0x0
#define LENS_POSN_STEP      1

enum dw9716_drive_mode {
	DW9716_DRIVE_MODE_DIRECT_0,     // 0000 no slew rate control.
	DW9716_DRIVE_MODE_DIRECT_1,     // 0001 no slew rate control.
	DW9716_DRIVE_MODE_DIRECT_2,     // 0010 no slew rate control.
	DW9716_DRIVE_MODE_DIRECT_3,     // 0011 no slew rate control.
	DW9716_DRIVE_MODE_DIRECT_4,     // 0100 no slew rate control.
	DW9716_DRIVE_MODE_8_per_50us,  	// 0101 8 steps for 50 us.
	DW9716_DRIVE_MODE_8_per_100us, 	// 0110 8 steps for 100us.
	DW9716_DRIVE_MODE_8_per_200us, 	// 0111 8 steps for 200us.
	DW9716_DRIVE_MODE_DIRECT_8,     // 1000 no slew rate control.
	DW9716_DRIVE_MODE_4_per_50us,	// 1001 4 steps for 50 us.
	DW9716_DRIVE_MODE_4_per_100us,	// 1010 4 steps for 100 us.
	DW9716_DRIVE_MODE_4_per_200us,	// 1011 4 steps for 200 us.
	DW9716_DRIVE_MODE_DIRECT_12,	// 1100 no slew rate control.
	DW9716_DRIVE_MODE_2_per_50us,	// 1101 2 steps for 50 us.
	DW9716_DRIVE_MODE_2_per_100us,	// 1110 2 steps for 100 us.
	DW9716_DRIVE_MODE_2_per_200us,	// 1111 2 steps for 200 us.
};

enum dw9716_drive_times {
	DW9716_TIME_0_00us		= 0,		// 0us per step.
	DW9716_TIME_6_25us		= 6250,		// 6.25us per step.
	DW9716_TIME_12_50us		= 12500,	// 12.50us per step.
	DW9716_TIME_25_00us		= 25000,	// 25.00us per step.
	DW9716_TIME_50_00us		= 50000,	// 50.00us per step.
	DW9716_TIME_100_00us	= 100000,	// 100.00us per step.
};

/**
 * struct dw9716_platform_data - platform data values and access functions
 * @power_set: Power state access function, zero is off, non-zero is on.
 * @priv_data_set: device private data (pointer) access function
 */
struct dw9716_platform_data {
	int (*power_set)(enum v4l2_power power);
	int (*priv_data_set)(void *);
};

/*
 * Sets the specified focus value [0(far) - 100(near)]
 */
int dw9716_af_setfocus(u16 posn);
int dw9716_af_getfocus(u16 *value);

#endif //__KERNEL__

#define DW9716_MAX_STEP_COUNT (128)

struct dw9716_micro_step
{
    __s32 lens_pos;
    __u32 time_sleep_ns;
    struct timespec exec_time;
} __attribute__((packed));

struct dw9716_micro_steps_list
{
    __u32                           count;
    struct dw9716_micro_step        steps[DW9716_MAX_STEP_COUNT];
} __attribute__((packed));





#endif /* End of of CAMAF_DW9716_H */

