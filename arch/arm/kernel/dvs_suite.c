/*
 *	linux/arch/arm/kernel/dvs_suite.c
 *
 *	Copyright (C) 2010 Sookyoung Kim <sookyoung.kim@lge.com>
 */

/****************************************************************************************
 * Headers
 ****************************************************************************************/

#include <linux/dvs_suite.h>

/****************************************************************************************
 * Variables and data structures
 ****************************************************************************************/

struct timeval ds_timeval;

DS_CONF ds_configuration;
DS_STAT ds_status;
DS_COUNT ds_counter;
DS_PARAM ds_parameter;

/* Variables for AIDVS */
DS_AIDVS_STAT_STRUCT ds_aidvs_status;

/* Variables for GPScheDVS */
DS_GPSCHEDVS_STAT_STRUCT ds_gpschedvs_status;

/****************************************************************************************
 * Function definitions
 ****************************************************************************************/

asmlinkage void ld_update_cpu_op(void){
	ds_update_cpu_op();
	return;
}

int ld_initialize_dvs_suite(int cpu_mode){
	ds_initialize_dvs_suite(cpu_mode);
	return(0);
}

int ld_update_time_counter(void){
	ds_update_time_counter();
	return(0);
}

int ld_update_priority_normal(struct task_struct *p){
	ds_update_priority_normal(p);
	return(0);
}

void ld_do_dvs_suite(void){
	do_dvs_suite();
	return;
}

/*====================================================================
	The functions involved with U(20,12) fixed point format arithmetic.
	====================================================================*/

/* The unsigned multiplication function for U(20,12) format
	 fixed point fractional numbers.
	 Multiplicand and multiplier should be converted to
	 U(20,12) format fixed point number by << 12 before using the function.
	 Keep in mind that the calculation result should not exceed
	 0xffffffff or 1048575.999755859375, the maximum value U(20,12) can represent.
	 Otherwise, this function will cause segmentation error.
 */
void ds_fpmul12(
unsigned long multiplicand,
unsigned long multiplier,
unsigned long *multiplied)
{

/* umull	r4, r3, %1, %2		%1 = multiplicand, %2 = multiplier, 
 * 					r3 = higher 32 bits, r4 = lower 32 bits.
 * mov		r4, r4, lsr #12		Logical right shift r4 by 12 bits
 * mov		r3, r3, lsl #20		Logical left shift r3 by 20 bits
 * orr		%0, r3, r4		Logical OR r3 and r4 and save it to *multimpied.
 */

//	printk(KERN_INFO "0x%lx * 0x%lx = ", multiplicand, multiplier);

	__asm__ __volatile__("umull	r4, r3, %1, %2\n\t"
					 "mov	r4, r4, lsr #12\n\t"
					 "mov	r3, r3, lsl #20\n\t"
					 "orr	%0, r3, r4"
											 : "=r" (*multiplied)
											 : "r" (multiplicand), "r" (multiplier)
					 : "r4", "r3");

//	printk(KERN_INFO "0x%lx\n", *multiplied);

	return;
}
