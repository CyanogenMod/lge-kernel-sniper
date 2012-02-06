/*
 * SDRC register values for the Hynix H8MBX00U0MER-0EM
 *
 * Copyright (C) 2009 Texas Instruments, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __ARCH_ARM_MACH_OMAP2_SDRAM_HYNIX_H8MBX00U0MER0EM
#define __ARCH_ARM_MACH_OMAP2_SDRAM_HYNIX_H8MBX00U0MER0EM

#include <plat/sdrc.h>

#if 0 /* Froyo original */
/* (100MHz optimized) = 10ns */
#define TDAL_100   5
#define TDPL_100   2
#define TRRD_100   2
#define TRCD_100   3
#define TRP_100    3
#define TRAS_100   5
#define TRC_100    8
#define TRFC_100   9
#define V_ACTIMA_100 ((TRFC_100 << 27) | (TRC_100 << 22) | (TRAS_100 << 18) \
		|(TRP_100 << 15) | (TRCD_100 << 12) |(TRRD_100 << 9) |(TDPL_100 << 6) \
		| (TDAL_100))

#define TWTR_100   1
#define TCKE_100   1
#define TXP_100    2
#define XSR_100   14
#define V_ACTIMB_100 ((TCKE_100 << 12) | (XSR_100 << 0)) | \
				(TXP_100 << 8) | (TWTR_100 << 16)


#define TDAL_200   6
#define TDPL_200   3
#define TRRD_200   2
#define TRCD_200   4
#define TRP_200    3
#define TRAS_200   8
#define TRC_200   11
#define TRFC_200  18
#define V_ACTIMA_200 ((TRFC_200 << 27) | (TRC_200 << 22) | (TRAS_200 << 18) \
		| (TRP_200 << 15) | (TRCD_200 << 12) |(TRRD_200 << 9) | \
		(TDPL_200 << 6) | (TDAL_200))
#define TWTR_200   2
#define TCKE_200   1
#define TXP_200    2
#define XSR_200    28
#define V_ACTIMB_200 ((TCKE_200 << 12) | (XSR_200 << 0)) | \
				(TXP_200 << 8) | (TWTR_200 << 16)
#else
/* Changed by usnoh@ubiquix.com. confirmed by hynix */
/* (100MHz optimized) = 10ns */
#define TDAL_100   4
#define TDPL_100   2
#define TRRD_100   2
#define TRCD_100   3
#define TRP_100    2
#define TRAS_100   5
#define TRC_100    6
#define TRFC_100   8
#define V_ACTIMA_100 ((TRFC_100 << 27) | (TRC_100 << 22) | (TRAS_100 << 18) \
		|(TRP_100 << 15) | (TRCD_100 << 12) |(TRRD_100 << 9) |(TDPL_100 << 6) \
		| (TDAL_100))

#define TWTR_100   1
#define TCKE_100   1
#define TXP_100    2
#define XSR_100   13
#define V_ACTIMB_100 ((TCKE_100 << 12) | (XSR_100 << 0)) | \
				(TXP_100 << 8) | (TWTR_100 << 16)


#define TDAL_200   7
#define TDPL_200   3
#define TRRD_200   3
#define TRCD_200   4
#define TRP_200    4
#define TRAS_200   9
#define TRC_200   12
#define TRFC_200  16
#define V_ACTIMA_200 ((TRFC_200 << 27) | (TRC_200 << 22) | (TRAS_200 << 18) \
		| (TRP_200 << 15) | (TRCD_200 << 12) |(TRRD_200 << 9) | \
		(TDPL_200 << 6) | (TDAL_200))
#define TWTR_200   1
#define TCKE_200   1
#define TXP_200    2
#define XSR_200    25
#define V_ACTIMB_200 ((TCKE_200 << 12) | (XSR_200 << 0)) | \
				(TXP_200 << 8) | (TWTR_200 << 16)


#endif

#define SDP_3430_SDRC_RFR_CTRL_100MHz   0x0002da01 /* 7.8us/10ns = 0x2da */
#define SDP_3430_SDRC_RFR_CTRL_133MHz   0x0003de01 /* 7.8us/7.5ns - 50=0x3de */
#define SDP_3430_SDRC_RFR_CTRL_165MHz   0x0004e201 /* 7.8us/6ns - 50=0x4e2 */

#define HUB_3630_SDRC_RFR_CTRL_83MHz    0x00025601
#define HUB_3630_SDRC_RFR_CTRL_100MHz   0x0002da01
#define HUB_3630_SDRC_RFR_CTRL_133MHz   0x0003dc01 /* 7.8us/7.5ns - 50=0x3dc */
#define HUB_3630_SDRC_RFR_CTRL_165MHz   0x0004dd01 /* 7.8us/6ns - 50=0x4dd */
#define HUB_3630_SDRC_RFR_CTRL_200MHz   0x0005e601 /* 7.8us/5ns - 50=0x5e6 */

#define HUB_SDRC_MR_0_DDR		0x00000032
#define SDP_SDRC_MR_0_DDR		0x00000032


/* Hynix H8MBX00U0MER-0EM */
static struct omap_sdrc_params h8mbx00u0mer0em_sdrc_params[] = {
#if 1
	[0] = {
		.rate        = 200000000,
		.actim_ctrla = V_ACTIMA_200, //0x92E1C4C6,
		.actim_ctrlb = V_ACTIMB_200, //0x0002121C,
		.rfr_ctrl    = HUB_3630_SDRC_RFR_CTRL_200MHz,
		.mr          = HUB_SDRC_MR_0_DDR,
	},
	[1] = {
		.rate        = 100000000,
		.actim_ctrla = V_ACTIMA_100, //0x4A15B485,
		.actim_ctrlb = V_ACTIMB_100, //0x0001120e,
		.rfr_ctrl    = HUB_3630_SDRC_RFR_CTRL_100MHz,
		.mr          = HUB_SDRC_MR_0_DDR,
	},
	[2] = {
		.rate        = 0
	},
#else
	[0] = {
		.rate        = 200000000,
		.actim_ctrla = 0xa2e1b4c6,
		.actim_ctrlb = 0x0002131c,
		.rfr_ctrl    = 0x0005e601,
		.mr          = 0x00000032,
	},
	[1] = {
		.rate        = 166000000,
		.actim_ctrla = 0x629db4c6,
		.actim_ctrlb = 0x00012214,
		.rfr_ctrl    = 0x0004dc01,
		.mr          = 0x00000032,
	},
	[2] = {
		.rate        = 100000000,
		.actim_ctrla = 0x51912284,
		.actim_ctrlb = 0x0002120e,
		.rfr_ctrl    = 0x0002d101,
		.mr          = 0x00000022,
	},
	[3] = {
		.rate        = 83000000,
		.actim_ctrla = 0x31512283,
		.actim_ctrlb = 0x0001220a,
		.rfr_ctrl    = 0x00025501,
		.mr          = 0x00000022,
	},
	[4] = {
		.rate        = 0
	},
#endif
};

#endif
