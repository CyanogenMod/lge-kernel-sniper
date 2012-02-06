/*
 *	include/linux/dvs_suite.h
 *
 *	Copyright (C) 2010 Sookyoung Kim <sookyoung.kim@lge.com>
 */

#ifndef _LINUX_DVS_SUITE_H
#define _LINUX_DVS_SUITE_H

/***************************************************************************
 * Headers
 ***************************************************************************/
#include <asm/current.h>	/* For current macro */
#include <asm/div64.h>		/* For division */
#include <asm/cputime.h>

#include <linux/mm.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>
#include <linux/errno.h>	/* For EAGAIN and EWOULDBLOCK */
#include <linux/kernel.h>	/* For printk */
#include <linux/sched.h>	/* For struct task_struct and wait_event* macros */
#include <linux/slab.h>		/* For kmalloc and kfree */
#include <linux/string.h>	/* To use string functions */
#include <linux/times.h>	/* For struct timeval and do_gettimeofday */
#include <linux/timex.h>	/* For struct timeval and do_gettimeofday */
#include <linux/cred.h>		/* To get uid */
#include <linux/workqueue.h>
#include <linux/smp.h>
#include <linux/percpu.h>
#include <linux/cpumask.h>
#include <linux/tick.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/cpu.h>

#include <plat/omap_device.h>
#include <plat/omap-pm.h>

/***************************************************************************
 * Definitions
 ***************************************************************************/

#define DS_CPU_NO	1

/* The 4 operating points (CPU_OP) supported by OMAP3630

	CPU_OP 0: (1000 MHz, 1.35V),	Scaling factor 1	= 0x1000 in fixed point number
	CPU_OP 1: ( 800 MHz, 1.26V),	Scaling factor 0.8	= 0xccc
	CPU_OP 2: ( 600 MHz, 1.10V),	Scaling factor 0.6	= 0x999
	CPU_OP 3: ( 300 MHz, 0.93V),	Scaling factor 0.3	= 0x4cc

	set_opp(&vdd1_opp, VDD1_OPP4)
	set_opp(&vdd1_opp, VDD1_OPP3)
	set_opp(&vdd1_opp, VDD1_OPP2)
	set_opp(&vdd1_opp, VDD1_OPP1)
 */

/* The CPU_OP indices */
#define DS_CPU_OP_INDEX_0		1000000000
#define DS_CPU_OP_INDEX_1		800000000
#define DS_CPU_OP_INDEX_2		600000000
#define DS_CPU_OP_INDEX_3		300000000

#define DS_CPU_OP_INDEX_MAX			DS_CPU_OP_INDEX_0
#define DS_CPU_OP_INDEX_MIN			DS_CPU_OP_INDEX_3

#define DS_CPU_OP_INDEX_INI		DS_CPU_OP_INDEX_MAX

#define DS_CPU_OP_INDEX_TOUCH40	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH39	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH38	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH37	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH36	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH35	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH34	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH33	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH32	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH31	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH30	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH29	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH28	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH27	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH26	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH25	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH24	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH23	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH22	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH21	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH20	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH19	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH18	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH17	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH16	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH15	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH14	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH13	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH12	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH11	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH10	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH9	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH8	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH7	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH6	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH5	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH4	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH3	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH2	DS_CPU_OP_INDEX_MIN
#define DS_CPU_OP_INDEX_TOUCH1	DS_CPU_OP_INDEX_MIN

/* The scaling factors */
/* These values mean the U(20,12) fixed point numbers' 12bit fractions.
 * In this format, 
 *					------ Decimal part ------ -- Fraction --
 * 1				= 0000 0000 0000 0000 0001 0000 0000 0000 = 0x00001000
 * 0.5				= 0000 0000 0000 0000 0000 1000 0000 0000 = 0x00000800
 * 0.25				= 0000 0000 0000 0000 0000 0100 0000 0000 = 0x00000400
 * 0.125			= 0000 0000 0000 0000 0000 0010 0000 0000 = 0x00000200
 * 0.0625			= 0000 0000 0000 0000 0000 0001 0000 0000 = 0x00000100
 * 0.03125			= 0000 0000 0000 0000 0000 0000 1000 0000 = 0x00000080
 * 0.015625			= 0000 0000 0000 0000 0000 0000 0100 0000 = 0x00000040
 * 0.0078125 		= 0000 0000 0000 0000 0000 0000 0010 0000 = 0x00000020	
 * 0.00390625		= 0000 0000 0000 0000 0000 0000 0010 0000 = 0x00000010	
 * 0.0019553125		= 0000 0000 0000 0000 0000 0000 0010 0000 = 0x00000008	
 * 0.0009765625		= 0000 0000 0000 0000 0000 0000 0010 0000 = 0x00000004	
 * 0.00048828125	= 0000 0000 0000 0000 0000 0000 0010 0000 = 0x00000002	
 * 0.000244140625	= 0000 0000 0000 0000 0000 0000 0010 0000 = 0x00000001	
 *
 * Ex)
 * 0.75 = 0.5 + 0.25	= 0000 0000 0000 0000 0000 1100 0000 0000 = 0x00000c00
 */
//#define DS_CPU_OP_SF_0	0x1000	// It is not an fraction. It should be 1
#define DS_CPU_OP_SF_1			0xccc	// 0.8
#define DS_CPU_OP_SF_2			0x999	// 0.6
#define DS_CPU_OP_SF_3			0x4cc	// 0.3

#define DS_CPU_OP_SF_MIN		DS_CPU_OP_SF_3

/* For ds_cpu_status.cpu_mode */
#define DS_CPU_MODE_IDLE		0
#define DS_CPU_MODE_TASK		1
#define DS_CPU_MODE_SCHEDULE	2
#define DS_CPU_MODE_DVS_SUITE	4

/* For do_dvs_suite() */
#define DS_ENTRY_TIMER_IRQ		0
#define DS_ENTRY_SWITCH_TO				1

/* For ds_initialize_dvs_suite() */
#define DS_INIT_ON_BOOT			0
#define DS_INIT_ON_RESUME		1

/* The macro to convert an unsigned long type to U(20,12) fixed point.
	Result is U(20,12) fixed point. */
#define DS_ULONG2FP12(x)		((x)<<12)
/* The macro to extract the integer part of the given U(20,12) fixed point
	number. Result is unsigned long. */
#define DS_GETFP12INT(x)		(((x)&0xfffff000)>>12)
/* The macro to extract the fraction part of the given U(20,12) fixed point
	number. Result is U(20,12) fixed point. */
#define DS_GETFP12FRA(x)		((x)&0x00000fff)

/* Definitions for compare44bits() */
#define DS_LARGER			1
#define DS_EQUAL 			0
#define DS_SMALLER			-1

/* Process static priority */
#define DS_LINUX_DEFAULT_STATIC_PRIO	120

#define DS_HRT_STATIC_PRIO		105		// 100 nice -15
#define DS_DBSRT_STATIC_PRIO	110		// 110 nice -10
#define DS_RBSRT_STATIC_PRIO	115		// 115 nice	-5
#define DS_NRT_STATIC_PRIO		120		// 120 nice	0
#define DS_IDLE_PRIO			140

#define DS_HRT_NICE				-15		// -15
#define DS_DBSRT_NICE			-10		// -10
#define DS_RBSRT_NICE			-5		// -5
#define DS_NRT_NICE				0		// 0

/* Process rt_priority. 
	p->prio = p->normal_prio = 99 - p->rt_priority for SCHED_RR tasks.
	p->prio = p->normal_prio = p->static_prio for SCHED_NORMAL tasks.
 */
#define DS_HRT_RR_PRIO			29	// p->prio = 70
#define DS_DBSRT_RR_PRIO		19	// p->prio = 80
#define DS_RBSRT_RR_PRIO		9	// p->prio = 90

/* Scheduler type.
 */
#define DS_SCHED_NORMAL			0
#define DS_SCHED_RR				1

/* Process type.
 */
#define DS_HRT_TASK					0x1	// HRT		0001
#define DS_SRT_UI_SERVER_TASK		0x2	// DBSRT	0010
#define DS_SRT_UI_CLIENT_TASK		0x3	// DBSRT	0011
#define DS_SRT_KERNEL_THREAD		0x4	// RBSRT	0100
#define DS_SRT_DAEMON_TASK			0x5	// RBSRT	0101
#define DS_NRT_TASK					0x6	// NRT		0110

#define DS_MIN_RT_SCHED_TYPE	DS_SRT_UI_CLIENT_TASK	

#define DS_TYPE_M				0xF		// 0000 0000 0000 1111
#define DS_TYPE_N				0xFFF0	// 1111 1111 1111 0000
#define DS_TYPE_FIXED_M			0x10	// 0000 0000 0001 0000
#define DS_TYPE_FIXED_N			0xFFEF	// 1111 1111 1110 1111
#define DS_TYPE_2B_CHANGED_M	0x20	// 0000 0000 0010 0000
#define DS_TYPE_2B_CHANGED_N	0xFFDF	// 1111 1111 1101 1111

/* The maximum allowable number of PID. 0 ~ 32767. */
#define DS_PID_LIMIT			32768

/* To cope with touch and key inputs */
#define DS_TOUCH_CPU_OP_UP_CNT_MAX			40		// Fixed.
#define DS_TOUCH_CPU_OP_UP_INTERVAL			100000	// 100 msec. Don't touch this. LG standard.
#define DS_CPU_OP_INDEX_CONT_TOUCH			DS_CPU_OP_INDEX_1
#define DS_CONT_TOUCH_THRESHOLD_USEC		150000	// 15 msec by 10.
#define DS_CONT_TOUCH_CARE_WAIT_SEC			3		// 3 seconds

/* Definitions for AIDVS */
#define DS_AIDVS_PE_MOVING_AVG_WEIGHT			3		/* 3. Weight for post early suspend */
#define DS_AIDVS_NM_MOVING_AVG_WEIGHT			3		/* 3 */

/* Definitions for GPScheDVS */
#define DS_AIDVS_PE_INTERVAL_LENGTH				500000	/* 500 msec */
#define DS_AIDVS_NM_INTERVAL_LENGTH				30000	/* 30 msec <-- TUNE IT! */

#define DS_MIN_CPU_OP_UPDATE_INTERVAL_U			3000	/* 3 msec */
#define DS_MIN_CPU_OP_UPDATE_INTERVAL_D			3000	/* 3 msec */

#define DS_INIT_DELAY_SEC						30		/* 60 seconds */
#define DS_POST_EARLY_SUSPEND_DELAY_SEC			3		/* 3 seconds */

/* Consideration for the frequency locking through cpufreq sysfs */
#define DS_CPU_OP_INDEX_LOCKED_MIN				DS_CPU_OP_INDEX_2
#define DS_CPU_OP_LOCK_SUSTAIN_SEC				30

/***************************************************************************
 * Variables and data structures
 ***************************************************************************/

extern struct workqueue_struct *dvs_suite_wq;
extern struct work_struct dvs_suite_work;

/*
 * Unique data structures
	 */

typedef struct dvs_suite_control DS_CTRL;
struct dvs_suite_control {
	int on_dvs;
	int flag_run_dvs;

	int aidvs_moving_avg_weight;
	unsigned long aidvs_interval_length;
};
extern DS_CTRL ds_control;

typedef struct dvs_suite_parameter DS_PARAM;
struct dvs_suite_parameter {
	int entry_type;
	struct task_struct *prev_p;
	struct task_struct *next_p;
};
extern DS_PARAM ds_parameter;

struct dvs_suite_system_status {
	/*
		DS_TYPE_M				0xF		= 0000 0000 0000 1111
		DS_TYPE_FIXED_M			0x10	= 0000 0000 0001 0000
		DS_TYPE_2B_CHANGED_M	0x20	= 0000 0000 0010 0000
 */
	int type[DS_PID_LIMIT];
	char tg_owner_comm[DS_PID_LIMIT][16];

	unsigned int locked_min_cpu_op_index;
	unsigned long locked_min_cpu_op_release_sec;
	int flag_locked_min_cpu_op;
	unsigned int locked_max_cpu_op_index;
	unsigned int locked_min_iva_freq;	// OMAP3630 specific. See mach-omap2/pm.c
	unsigned int locked_min_l3_freq;	// OMAP3630 specific. See mach-omap2/pm.c

	int flag_touch_timeout_count;
	unsigned long touch_timeout_sec;
	unsigned long touch_timeout_usec;

	int flag_consecutive_touches;
	unsigned long new_touch_sec;
	unsigned long new_touch_usec;
	unsigned long first_consecutive_touch_sec;
	int flag_long_consecutive_touches;

	int flag_post_early_suspend;
	int flag_do_post_early_suspend;
	unsigned long do_post_early_suspend_sec;
};
DECLARE_PER_CPU(struct dvs_suite_system_status, ds_sys_status);

/*
 * Per-core data structures
 */

struct dvs_suite_cpu_status {
	int cpu_mode;
	int dvs_suite_mutex;

	int flag_update_cpu_op;
	int cpu_op_mutex;

	unsigned int current_cpu_op_index;
	unsigned int target_cpu_op_index;
	unsigned long cpu_op_last_update_sec;
	unsigned long cpu_op_last_update_usec;
};
DECLARE_PER_CPU(struct dvs_suite_cpu_status, ds_cpu_status);

struct dvs_suite_counter {
	int flag_counter_initialized;
	unsigned int counter_mutex;

	cputime64_t wall_usec_base;
	cputime64_t idle_usec_base;
	cputime64_t iowait_usec_base;

	unsigned long elapsed_sec;
	unsigned long elapsed_usec;

	unsigned long busy_sec;
	unsigned long busy_usec;
};
DECLARE_PER_CPU(struct dvs_suite_counter, ds_counter);

typedef struct ds_aidvs_status_structure DS_AIDVS_STAT_STRUCT;
struct ds_aidvs_status_structure {
	int moving_avg_weight;
	unsigned long interval_length;

	unsigned long time_usec_interval;
	unsigned long time_sec_interval_inc_base;
	unsigned long time_usec_interval_inc_base;

	unsigned long time_usec_work;
	unsigned long time_sec_work_inc_base;
	unsigned long time_usec_work_inc_base;

	unsigned long utilization_int_ulong;
	unsigned long utilization_fra_fp12;

	unsigned long moving_avg_int_ulong;
	unsigned long moving_avg_fra_fp12;

	unsigned int cpu_op_index;
};
DECLARE_PER_CPU(DS_AIDVS_STAT_STRUCT, ds_aidvs_status);

/***************************************************************************
 * Function definitions
 ***************************************************************************/

extern void ds_fpmul12(unsigned long, unsigned long, unsigned long *);
extern void ds_fpdiv12(unsigned long, unsigned long,
											 unsigned long *, unsigned long *);
extern void ds_div32(unsigned long, unsigned long,
										 unsigned long *, unsigned long *);
extern int ds_find_first1_in_integer_part(unsigned long);
extern int ds_find_first1_in_fraction_part(unsigned long);
extern int ds_compare45bits(int, unsigned long, unsigned long,
														int, unsigned long, unsigned long);
extern int ds_shiftleft44bits(int, unsigned long, unsigned long,
															int, int *, unsigned long *, unsigned long *);
extern int ds_subtract44bits(int, unsigned long, unsigned long,
														 int, unsigned long, unsigned long,
														 int *, unsigned long *, unsigned long *);
extern int ds_fpmul(unsigned long, unsigned long, unsigned long, unsigned long,
										unsigned long *, unsigned long *);
extern int ds_fpdiv(unsigned long, unsigned long, unsigned long, unsigned long,
										unsigned long *, unsigned long *);

extern unsigned int ds_get_next_high_cpu_op_index(unsigned long, unsigned long);

/*
 * The functions for each DVS scheme.
 */

/* AIDVS */
extern int ds_do_dvs_aidvs(int, unsigned int *, DS_AIDVS_STAT_STRUCT *);

/* GPScheDVS */
extern int ds_do_dvs_gpschedvs(int, unsigned int *);

/*
 * Wrappers to be used in the existing kernel codes 
 * to call the main dvs suite function.
 */
extern int ld_initialize_ds_control(void);
extern int ld_initialize_ds_sys_status(void);
extern int ld_initialize_ds_cpu_status(int);
extern int ld_initialize_ds_counter(void);
extern int ld_initialize_aidvs(void);
extern int ld_update_priority_normal(int, struct task_struct *);
extern void ld_do_dvs_suite(int);

/*
 * The main dvs suite function.
 */
extern int ds_initialize_ds_control(void);
extern int ds_initialize_ds_sys_status(void);
extern int ds_initialize_ds_cpu_status(int, int);
extern int ds_initialize_ds_counter(int, int);
extern int ds_initialize_aidvs(int, int);
extern int ds_update_time_counter(int);
extern int ds_update_priority_normal(int, struct task_struct *);
extern asmlinkage void ds_update_cpu_op(int);
extern int ds_detect_task_type(int);
extern void ds_up_aux_cpu(int, int);
extern void ds_down_aux_cpu(int, int);
extern void do_dvs_suite(int);
extern void do_dvs_suite_timer(struct work_struct *);
extern inline void dvs_suite_timer_init(void);
extern inline void dvs_suite_timer_exit(void);

#endif /* !(_LINUX_DVS_SUITE_H) */
