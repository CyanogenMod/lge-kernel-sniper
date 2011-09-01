/*
 * Linux port of common routines for finding wl device
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlu_linux_common.c,v 1.1.6.1 2010/09/01 07:50:56 Exp $
 */
#include "wlu_linux_common.h"

int remote_type = NO_REMOTE;
int rwl_os_type = LINUX_OS;

void
syserr(char *s)
{
	fprintf(stderr, "%s: ", wlu_av0);
	perror(s);
	exit(errno);
}

int
wl_ioctl(void *wl, int cmd, void *buf, int len, bool set)
{
	struct ifreq *ifr = (struct ifreq *) wl;
	wl_ioctl_t ioc;
	int ret = 0;
	int s;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	/* do it */
	ioc.cmd = cmd;
	ioc.buf = buf;
	ioc.len = len;
#ifdef DONGLEOVERLAYS
	ioc.action = set ? WL_IOCTL_ACTION_SET : WL_IOCTL_ACTION_GET;
#else
	ioc.set = set;
#endif
	ifr->ifr_data = (caddr_t) &ioc;
	if ((ret = ioctl(s, SIOCDEVPRIVATE, ifr)) < 0) {
		if (cmd != WLC_GET_MAGIC) {
			ret = IOCTL_ERROR;
		}
	}

	/* cleanup */
	close(s);
	return ret;
}


static int
ioctl_queryinformation_fe(void *wl, int cmd, void* input_buf, int *input_len)
{
	int error = NO_ERROR;

	if (remote_type == NO_REMOTE) {
		error = wl_ioctl(wl, cmd, input_buf, *input_len, FALSE);
	} else {
		error = rwl_queryinformation_fe(wl, cmd, input_buf,
		              (unsigned long*)input_len, 0);

	}
	return error;
}

static int
ioctl_setinformation_fe(void *wl, int cmd, void* buf, int *len)
{
	int error = 0;

	if (remote_type == NO_REMOTE) {
		error = wl_ioctl(wl,  cmd, buf, *len, TRUE);
	} else {
		error = rwl_setinformation_fe(wl, cmd, buf, (unsigned long*)len, 0);
	}

	return error;
}

int
wl_get(void *wl, int cmd, void *buf, int len)
{
	int error = 0;
	/* For RWL: When interfacing to a Windows client, need t add in OID_BASE */
	if ((rwl_os_type == WIN32_OS) && (remote_type != NO_REMOTE)) {
		error = (int)ioctl_queryinformation_fe(wl, WL_OID_BASE + cmd, buf, &len);
	} else {
		error = (int)ioctl_queryinformation_fe(wl, cmd, buf, &len);
	}

	if (error == SERIAL_PORT_ERR)
		return SERIAL_PORT_ERR;

	if (error != 0)
		return IOCTL_ERROR;

	return 0;
}

int
wl_set(void *wl, int cmd, void *buf, int len)
{
	int error = 0;

	/* For RWL: When interfacing to a Windows client, need t add in OID_BASE */
	if ((rwl_os_type == WIN32_OS) && (remote_type != NO_REMOTE)) {
		error = (int)ioctl_setinformation_fe(wl, WL_OID_BASE + cmd, buf, &len);
	} else {
		error = (int)ioctl_setinformation_fe(wl, cmd, buf, &len);
	}

	if (error == SERIAL_PORT_ERR)
		return SERIAL_PORT_ERR;

	if (error != 0) {
		return IOCTL_ERROR;
	}
	return 0;
}

static int
wl_get_dev_type(char *name, void *buf, int len)
{
	int s;
	int ret;
	struct ifreq ifr;
	struct ethtool_drvinfo info;

	/* open socket to kernel */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		syserr("socket");

	/* get device type */
	memset(&info, 0, sizeof(info));
	info.cmd = ETHTOOL_GDRVINFO;
	ifr.ifr_data = (caddr_t)&info;
	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	if ((ret = ioctl(s, SIOCETHTOOL, &ifr)) < 0) {

		/* print a good diagnostic if not superuser */
		if (errno == EPERM)
			syserr("wl_get_dev_type");

		*(char *)buf = '\0';
	} else {
		strncpy(buf, info.driver, len);
	}

	close(s);
	return ret;
}

void
wl_find(struct ifreq *ifr)
{
	char proc_net_dev[] = "/proc/net/dev";
	FILE *fp;
	char buf[1000], *c, *name;
	char dev_type[DEV_TYPE_LEN];

	ifr->ifr_name[0] = '\0';

	if (!(fp = fopen(proc_net_dev, "r")))
		return;

	/* eat first two lines */
	if (!fgets(buf, sizeof(buf), fp) ||
	    !fgets(buf, sizeof(buf), fp)) {
		fclose(fp);
		return;
	}

	while (fgets(buf, sizeof(buf), fp)) {
		c = buf;
		while (isspace(*c))
			c++;
		if (!(name = strsep(&c, ":")))
			continue;
		strncpy(ifr->ifr_name, name, IFNAMSIZ);
		if (wl_get_dev_type(name, dev_type, DEV_TYPE_LEN) >= 0 &&
			!strncmp(dev_type, "wl", 2))
			if (wl_check((void *) ifr) == 0)
				break;
		ifr->ifr_name[0] = '\0';
	}

	fclose(fp);
}

int
wl_validatedev(void *dev_handle)
{
	int retval = 1;
	struct ifreq *ifr = (struct ifreq *)dev_handle;
	/* validate the interface */
	if (!ifr->ifr_name || wl_check((void *)ifr)) {
		retval = 0;
	}
	return retval;
}
