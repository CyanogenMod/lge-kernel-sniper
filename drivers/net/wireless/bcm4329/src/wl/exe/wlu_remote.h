/*
 * OS independent remote wl declarations
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: wlu_remote.h,v 1.2.66.2.2.2.4.3.2.1 2010/03/09 22:03:56 Exp $
 */

#ifndef _TYPEDEFS_H_
#include <typedefs.h>
#endif
#include "proto/bcmip.h"

/* Remote wl declararions */
#define NO_REMOTE       0
#define REMOTE_SERIAL 	1
#define REMOTE_SOCKET 	2
#define REMOTE_WIFI     3
#define REMOTE_DONGLE   4
#define SHELL_CMD       -1 /* Invalid cmd id for shell */
#define ASD_CMD       	-2 /* Cmd id for ASD command */

/* For cross OS support */
#define LINUX_OS  	1
#define WIN32_OS  	2
#define MAC_OSX		3
#define BACKLOG 	4

/* Used in cdc_ioctl_t.flags field */
#define REMOTE_SET_IOCTL			1
#define REMOTE_GET_IOCTL			2
#define REMOTE_REPLY				4
#define REMOTE_SHELL_CMD			8
#define REMOTE_FINDSERVER_IOCTL		16 /* Find remote server */
#define REMOTE_ASD_CMD				32 /* ASD integration */

#define RWL_WIFI_DEFAULT_TYPE 		0x00
#define RWL_WIFI_DEFAULT_SUBTYPE 	0x00
#define RWL_ACTION_FRAME_DATA_SIZE 	1024	/* fixed size for the wifi frame data */
#define RWL_WIFI_CDC_HEADER_OFFSET 	0
#define RWL_WIFI_FRAG_DATA_SIZE 	960 /* max size of the frag data */
#define RWL_DEFAULT_WIFI_FRAG_COUNT 127 /* maximum fragment count */
#define RWL_WIFI_RETRY				5	/* CMD retry count for wifi */
#define RWL_WIFI_RX_RETRY			20  /* WIFI response rerty count */
#define RWL_WIFI_SEND				5   /* WIFI frame sent count */
#define RWL_WIFI_RETRY_DELAY   		1000 /* wifi specific retry delay  */
#define RWL_WIFI_SEND_DELAY			100  /* delay between two frames */
#define RWL_WIFI_RX_DELAY			250  /*  wait between send and receive */
#define RWL_WIFI_RX_SHELL_DELAY		1000 /* delay added for shell cmd response read */
#define RWL_WIFI_SERV_COUNT			8
#define RWL_WIFI_BUF_LEN			64
#define RWL_WIFI_SHELL_CMD			1
#define RWL_WIFI_WL_CMD            	0
#define RWL_WIFI_FIND_SER_CMD 		"findserver"
#define RWL_WIFI_GET_ACTION_CMD		"rwlwifivsaction"
#define RWL_WIFI_ACTION_CMD			"rwlwifivsaction"
#define DATA_FRAME_LEN				960
/* wl & shell cmd work fine for 960 (512+256+128+64) */

#define SUCCESS 					1
#define FAIL   						-1
#define NO_PACKET        			-2
#define SERIAL_PORT_ERR  			-3

#define DEFAULT_SERVER_PORT   		8000

#define WL_MAX_BUF_LEN 				(127 * 1024)

/* Used to send ioctls over the transport pipe */
typedef struct remote_ioctl {
	cdc_ioctl_t 	msg;
	uint		data_len;
} rem_ioctl_t;
#define REMOTE_SIZE	sizeof(rem_ioctl_t)

/* Used to set the cmds for wifi specific init */
typedef struct remote_wifi_cmds {
	uint32	cmd;		/* common ioctl definition */
	uint32	len;		/* length of data immediately following this header */
	char	*data;		/* max size of the data length */
	int		value;
} remote_wifi_cmds_t;

/* Added for debug utility support */
#define ERR		stderr
#define OUTPUT	stdout

#define DEBUG_DEFAULT	0x0001
#define DEBUG_ERR		0x0001
#define DEBUG_INFO		0x0002
#define DEBUG_DBG		0x0004

#define DPRINT_ERR	if (defined_debug & DEBUG_ERR) \
			    fprintf
#define DPRINT_INFO	if (defined_debug & DEBUG_INFO) \
			    fprintf
#define DPRINT_DBG if (defined_debug & DEBUG_DBG) \
			    fprintf
extern unsigned short defined_debug;

#ifdef WIN32
/* Function defined in wlu.c for client and wlu_server_ce.c for server */
extern int wl_atoip(const char *a, struct ipv4_addr *n);
#endif

/* Macros to access remote type */
extern int remote_type;
#define rwl_get_remote_type()		(remote_type)
#define rwl_set_remote_type(type)	(remote_type = type)

/* Macros to access server IP and port globals */
extern char *g_rwl_servIP;
#define rwl_get_server_ip()			(g_rwl_servIP)
#define rwl_set_server_ip(ip) 		(g_rwl_servIP = ip)
extern unsigned short g_rwl_servport;
#define rwl_get_server_port()		(g_rwl_servport)
#define rwl_set_server_port(port) 	(g_rwl_servport = port)

/* socket support */
struct sockaddr;
struct sockaddr_in;

extern int rwl_connectsocket(int SocketDes, struct sockaddr* SerAddr, int SizeOfAddr);
extern int rwl_send_to_streamsocket(int SocketDes, const char* SendBuff, int data_size, int Flag);
extern int rwl_receive_from_streamsocket(int SocketDes, char* RecvBuff, int data_size, int Flag);
extern int rwl_read_serial_port(void* hndle, char* read_buf, uint data_size, uint *numread);
extern int rwl_write_serial_port(void* hndle, char* write_buf, unsigned long size,
unsigned long *numwritten);
extern void rwl_sleep(int delay);
extern void* rwl_open_transport(int remote_type, char *port, int ReadTotalTimeout, int debug);
extern int rwl_close_transport(int remote_type, void * handle);
extern int rwl_poll_serial_buffer(void *hndle);
extern int rwl_serial_handle_timeout(void);
extern void rwl_sync_delay(uint noframes);
extern int rwl_init_server_socket_setup(int argc, char** argv, uint remote_type);
extern  int rwl_acceptconnection(int SocketDes, struct sockaddr *ClientAddr, int *SizeOfAddr);
/* Linux specific function in wlu_pipe_linux.c */
extern int rwl_get_if_addr(char *ifname, struct sockaddr_in *sa);

/* Win32 specific function wlu_pipe_win32.c */
extern int rwl_init_ws2_32dll(void);
extern int rwl_terminate_ws2_32dll(void);

/* Function definitions for wlu_client_shared.c and wlu_server_shared.c */
extern int rwl_var_getbuf(void *wl, const char *iovar, void *param, int param_len, void **bufptr);
extern int rwl_var_setbuf(void *wl, const char *iovar, void *param, int param_len);

/* Function definitions for wlu_ndis.c/wlu_linux.c  and wlu_server_ce.c/wlu_server_linux.c  */
extern int wl_get(void *wl, int cmd, void *buf, int len);
extern int wl_set(void *wl, int cmd, void *buf, int len);
#ifdef RWLASD
typedef unsigned char BYTE;
extern void wfa_dut_init(BYTE **tBuf, BYTE **rBuf, BYTE **paBuf, BYTE **cBuf, struct timeval **);
extern int remote_asd_exec(unsigned char* command, int* cmd_len);
void wfa_dut_deinit(void);
#endif
