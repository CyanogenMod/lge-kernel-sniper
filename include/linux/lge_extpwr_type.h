/*
 * Definition File For LGE External Power Type
 *
 * Copyright (C) 2011 LGE Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#ifndef __LGE_EXTPWR_TYPE_H__
#define __LGE_EXTPWR_TYPE_H__
/*
 * usb_type
 *  0 : No Init. Cable
 *  1 : Unknown Cable
 *  2 : TA Cable
 *  3 : LT Cable
 *  4 : USB Cable
 *  5 : Forged TA Cable
 *  6 : Abnormal USB 100mA Cable
 *  7 : Abnormal USB 400mA Cable
 *  8 : No Cable
 */
#define NO_INIT_CABLE       0
#define UNKNOWN_CABLE       1
#define TA_CABLE            2
//#define LT_CABLE            3
#define USB_CABLE           4
#define FORGED_TA_CABLE     5
#define ABNORMAL_USB_100MA  6
#define ABNORMAL_USB_400MA  7
#define NO_CABLE            8
#define LT_CABLE_56K 		9  /* General LT Cable */
#define LT_CABLE_130K 		10 /* Cal LT Cable */
#define LT_CABLE_910K 		11 /* Download LT Cable */


extern int get_ext_pwr_type(void);
#endif /*__LGE_EXTPWR_TYPE_H__*/
