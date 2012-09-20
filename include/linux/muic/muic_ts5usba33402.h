/*
 *  drivers/muic/muic_class.c
 *
 * Copyright (C) 2011 LG Electronics, Inc.
 * Author: Seungho Park <seungho1.park@lge.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
*/

#ifndef _MUIC_TS5USBA33402_H_
#define _MUIC_TS5USBA33402_H_

#define TD_INT_STAT	70000	/* INT_STAT bits settle down time since MUIC INT falls on TI chip*/
#define TD_STATUS	250000	/* STATUS bits settle down time since MUIC INT falls on TI chip */
#define TD_DP_DM	1000	/* DP, DM path settle down time since SW_CONTROL writing on TI chip */

/* I2C addresses of MUIC internal registers */
#define	DEVICE_ID	(u8)0x00
#define	CONTROL_1	(u8)0x01
#define	CONTROL_2	(u8)0x02
#define	SW_CONTROL	(u8)0x03
#define	INT_STAT	(u8)0x04
//!![S] 2011-06-23 by pilsu.kim@lge.com : [MUIC] update MUIC detect fail issue in Rev B (TSU5611 is revisioned register change)
#if defined (CONFIG_MUIC_TS5USBA33402_REV_A)
#define	STATUS		(u8)0x05
#else
#define	INT_STAT2	(u8)0x05
#define	STATUS		(u8)0x06
#endif
//!![E] 2011-06-23 by pilsu.kim@lge.com : 

/* Masks for the each bit of CONTROL_1 register */
#define	ID_2P2		(u8)0x40
#define	ID_620		(u8)0x20
#define	ID_200		(u8)0x10
#define	VLDO		(u8)0x08
#define	SEMREN		(u8)0x04
#define	ADC_EN		(u8)0x02
#define	CP_EN		(u8)0x01

/* Masks for the each bit of CONTROL_2 register */
#define	INTPOL		(u8)0x80
#define	INT_EN		(u8)0x40
#define	MIC_LP		(u8)0x20
#define	CP_AUD		(u8)0x10
#define	CHG_TYPE	(u8)0x02
#define	USB_DET_DIS	(u8)0x01

/* Masks for the each bit of SW_CONTROL register */
#define	MIC_ON		(u8)0x40
#define DP		(u8)0x38
#define DM		(u8)0x07

/* DP, DM settings */
#define DP_USB		(u8)0x00
#define	DP_UART		(u8)0x08
#define	DP_AUDIO	(u8)0x10
#define	DP_OPEN		(u8)0x38
#define DM_USB		(u8)0x00
#define	DM_UART		(u8)0x01
#define	DM_AUDIO	(u8)0x02
#define	DM_OPEN		(u8)0x07


/* Combined masks of SW_CONTROLl register */
#define USB	DP_USB   | DM_USB 	/* 0x00 */
#define UART 	DP_UART  | DM_UART 	/* 0x09 */
#define AUDIO	DP_AUDIO | DM_AUDIO	/* 0x12 */
#define OPEN	DP_OPEN  | DM_OPEN 	/* 0x3f */


/* Masks for the each bit of INT_STATUS register */
#define	CHGDET		(u8)0x80
#define	MR_COMP		(u8)0x40
#define	SENDEND		(u8)0x20
#define	VBUS		(u8)0x10
#define	IDNO		(u8)0x0f

/* Masks for the each bit of STATUS register */
#define	DCPORT		(u8)0x80
#define	CHPORT		(u8)0x40
#define C1COMP      	(u8)0x01	/* only on MAXIM */


//changseok.kim@lge.com
/* IDNO */
#define IDNO_0000     0x00
#define IDNO_0001     0x01
#define IDNO_0010     0x02
#define IDNO_0011     0x03
#define IDNO_0100     0x04
#define IDNO_0101     0x05
#define IDNO_0110     0x06
#define IDNO_0111     0x07
#define IDNO_1000     0x08
#define IDNO_1001     0x09
#define IDNO_1010     0x0A
#define IDNO_1011     0x0B

/* DP2 */
#define COMP2_TO_DP2  	0x00
#define COMP2_TO_U2   	0x08
#define COMP2_TO_AUD2 	0x10
#define COMP2_TO_HZ   	0x20

/* DM */
#define COMN1_TO_DN1  	0x00
#define COMN1_TO_U1   	0x01
#define COMN1_TO_AUD1 	0x02
#define COMN1_TO_C1COMP 0x03
#define COMN1_TO_HZ   	0x04

/* MUIC chip vendor */
#define TS5USBA33402 	0x10
#define MAX14526 	0x20
#define ANY_VENDOR 	0xff


#endif
