/*
 * Common inclusions for Linux port
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlu_linux_common.h,v 1.1.6.1 2010/09/01 07:50:56 Exp $
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#ifndef TARGETENV_android
#include <error.h>
#endif /* TARGETENV_android */
#include <wlioctl.h>
#include <bcmutils.h>
#include <wlioctl.h>
#include <bcmutils.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <proto/ethernet.h>
#include <proto/bcmip.h>

#ifndef TARGETENV_android
typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;
typedef u_int64_t __u64;
typedef u_int32_t __u32;
typedef u_int16_t __u16;
typedef u_int8_t __u8;
#endif /* TARGETENV_android */

#include <linux/sockios.h>
#include <linux/ethtool.h>
#include <typedefs.h>


#include "wlu.h"
#include <bcmcdc.h>
#include "wlu_remote.h"
#include "wlu_client_shared.h"
#include "wlu_pipe.h"

#define DEV_TYPE_LEN					3 /* length for devtype 'wl'/'et' */

#define NO_ERROR						0

extern int remote_type;
extern int rwl_os_type;

extern int wl_ioctl(void *wl, int cmd, void *buf, int len, bool set);
extern void wl_find(struct ifreq *ifr);
extern void syserr(char *s);
extern void *wl_getdev(void);
extern void wl_freedev(void *ifr);
extern int wl_getdevlen(void *dev_handle);
