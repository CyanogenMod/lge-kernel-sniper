/*
 * OS independent client specific declarations
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlu_client_shared.h,v 1.1.6.1.230.1 2010/09/01 07:50:56 Exp $
 */
/* 
 *
 * Date        Author         Description
 *
 * 10-March-2008 Suganthi        Version 0.1
 *
 */


/* Newly added wrappers from wlu_client_shared.c */
extern int rwl_shell_cmd_proc(void *wl, char **argv, int cmd);
#ifdef RWL_SOCKET
extern int
rwl_setinformation_fe(void *wl, int cmd, void* buf, unsigned long *len, int debug);
extern int
rwl_queryinformation_fe(void *wl, int cmd, void* input_buf, unsigned long *input_len, int debug);
#else
#define rwl_setinformation_fe(a, b, c, d, e) 0
#define rwl_queryinformation_fe(a, b, c, d, e) 0
#endif
extern void rwl_usage(int remote_type);
