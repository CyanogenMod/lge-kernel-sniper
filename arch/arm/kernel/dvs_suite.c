/*
 *	linux/arch/arm/kernel/dvs_suite.c
 *
 *	Copyright (C) 2010 Sookyoung Kim <sookyoung.kim@lge.com>
 */

/****************************************************************************************
 * Headers
 ****************************************************************************************/

#include <linux/dvs_suite.h>
#include <linux/module.h>	// For EXPORT_PER_CPU_SYMBOL()

/****************************************************************************************
 * Variables and data structures
 ****************************************************************************************/

struct workqueue_struct *dvs_suite_wq;
struct work_struct dvs_suite_work;

/*
 * Unique data structures
 */

DS_CTRL ds_control;
DS_PARAM ds_parameter;

DEFINE_PER_CPU(struct dvs_suite_system_status, ds_sys_status);
EXPORT_PER_CPU_SYMBOL(ds_sys_status);

/*
 * Per-core data structures
 */

DEFINE_PER_CPU(struct dvs_suite_cpu_status, ds_cpu_status);
EXPORT_PER_CPU_SYMBOL(ds_cpu_status);

DEFINE_PER_CPU(struct dvs_suite_counter, ds_counter);
EXPORT_PER_CPU_SYMBOL(ds_counter);

DEFINE_PER_CPU(DS_AIDVS_STAT_STRUCT, ds_aidvs_status);
EXPORT_PER_CPU_SYMBOL(ds_aidvs_status);

/****************************************************************************************
 * Function definitions
 ****************************************************************************************/

void do_dvs_suite_timer(struct work_struct *work);

int ld_initialize_ds_control(void){
	ds_initialize_ds_control();
	return(0);
}

int ld_initialize_ds_sys_status(void){
	ds_initialize_ds_sys_status();
	return(0);
}

int ld_initialize_ds_cpu_status(int cpu_mode){
	ds_initialize_ds_cpu_status(0, cpu_mode);
	//ds_initialize_ds_cpu_status(1, cpu_mode);	// Single core
	return(0);
}

int ld_initialize_ds_counter(void){
	ds_initialize_ds_counter(0, 0);
	//ds_initialize_ds_counter(1, occasion);	// Single core
	return(0);
}

int ld_initialize_aidvs(void){
	ds_initialize_aidvs(0, 0);
	//ds_initialize_aidvs(1);	// Single core
	return(0);
}

int ld_update_priority_normal(int ds_cpu, struct task_struct *p){
	ds_update_priority_normal(ds_cpu, p);
	return(0);
}

void ld_do_dvs_suite(int ds_cpu){
	do_dvs_suite(ds_cpu);
	return;
}
