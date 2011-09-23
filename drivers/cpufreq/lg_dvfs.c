/*
 *	linux/drivers/cpufreq/lg_dvfs.c
 *
 *	Copyright (C) 2010 Sookyoung Kim <sookyoung.kim@lge.com>
 */

/* LKM START ***********************/
#define __KERNEL__
#define MODULE

//#include <linux/modversions.h>
#include <linux/module.h>
#include <linux/init.h>
//#include <linux/kernel.h>
/* LKM END *************************/

#include "lg_dvfs.h"

/* LKM START ***********************/
#define DRIVER_AUTHOR	"Sookyoung Kim <sookyoung.kim@lge.com>"
#define DRIVER_DESC		"LG-DVFS"
/* LKM END *************************/

static const int ds_prio_to_weight[40] = {
 /* -20 */	 88761,	 71755,	 56483,	 46273,	 36291,
 /* -15 */	 29154,	 23254,	 18705,	 14949,	 11916,
 /* -10 */	  9548,	  7620,	  6100,	  4904,	  3906,
 /*  -5 */	  3121,	  2501,	  1991,	  1586,	  1277,
 /*   0 */	  1024,	   820,	   655,	   526,	   423,
 /*   5 */	   335,	   272,	   215,	   172,	   137,
 /*  10 */	   110,		87,		70,		56,		45,
 /*  15 */		36,		29,		23,		18,		15,
};

static const u32 ds_prio_to_wmult[40] = {
 /* -20 */	 48388,	 59856,	 76040,	 92818,	118348,
 /* -15 */	147320,	184698,	229616,	287308,	360437,
 /* -10 */	449829,	563644,	704093,	875809,   1099582,
 /*  -5 */   1376151,   1717300,   2157191,   2708050,   3363326,
 /*   0 */   4194304,   5237765,   6557202,   8165337,  10153587,
 /*   5 */  12820798,  15790321,  19976592,  24970740,  31350126,
 /*  10 */  39045157,  49367440,  61356676,  76695844,  95443717,
 /*  15 */ 119304647, 148102320, 186737708, 238609294, 286331153,
};

#if 0	// {
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
#endif	// }

/****************************************************************************************
 * Function definitions
 ****************************************************************************************/

/*====================================================================
	The functions involved with U(20,12) fixed point format arithmetic.
	====================================================================*/

#if 0	// {

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

#endif	// }

/* The unsigned division function for U(20,12) format fixed point fractional numbers.
	 Dividend and divisor should be converted to U(20,12) format fixed point number
	 by << 12 before using the function.
	 Keep in mind that the calculation result should not exceed 0xffffffff or
	 1048575.999755859375, the maximum value U(20,12) can represent.
	 Otherwise, this function will cause segmentation error.
 */
void ds_fpdiv12(
unsigned long dividend,
unsigned long divisor,
unsigned long *quotient,
unsigned long *remainder)
{

	unsigned long long lc_dividend;

	lc_dividend = (unsigned long long)dividend;
	lc_dividend = lc_dividend << 12;

	//printk(KERN_INFO "0x%lx / 0x%lx = ", dividend, divisor);

	do_div(lc_dividend, divisor);

	*quotient = (unsigned long)lc_dividend;
	*remainder = 0;

	//printk(KERN_INFO "0x%lx + 0x%lx\n", *quotient, *remainder);

 return;
}

/* Normal 32-bit division.
	 With this function, we can obtain both of the quotient and remainder at a time.
 */
void ds_div12(
unsigned long dividend,
unsigned long divisor,
unsigned long *quotient,
unsigned long *remainder)
{

	unsigned long long lc_dividend;

	lc_dividend = (unsigned long long)dividend;

//	printk(KERN_INFO "0x%lx / 0x%lx = ", dividend, divisor);

	do_div(lc_dividend, divisor);

	*quotient = (unsigned long)lc_dividend;
	*remainder = 0;

//	printk(KERN_INFO "0x%lx + 0x%lx\n", *quotient, *remainder);

 return;
}

int ds_find_first1_in_integer_part(unsigned long target_value){

	if(target_value == 0){
		return(0);
	}
	else{
		switch(target_value & 0xf0000000){
			case 0xf0000000: return(32);
			case 0xe0000000: return(32);
			case 0xd0000000: return(32);
			case 0xc0000000: return(32);
			case 0xb0000000: return(32);
			case 0xa0000000: return(32);
			case 0x90000000: return(32);
			case 0x80000000: return(32);
			case 0x70000000: return(31);
			case 0x60000000: return(31);
			case 0x50000000: return(31);
			case 0x40000000: return(31);
			case 0x30000000: return(30);
			case 0x20000000: return(30);
			case 0x10000000: return(29);
		}
		switch(target_value & 0xf000000){
			case 0xf000000: return(28);
			case 0xe000000: return(28);
			case 0xd000000: return(28);
			case 0xc000000: return(28);
			case 0xb000000: return(28);
			case 0xa000000: return(28);
			case 0x9000000: return(28);
			case 0x8000000: return(28);
			case 0x7000000: return(27);
			case 0x6000000: return(27);
			case 0x5000000: return(27);
			case 0x4000000: return(27);
			case 0x3000000: return(26);
			case 0x2000000: return(26);
			case 0x1000000: return(25);
		}
		switch(target_value & 0xf00000){
			case 0xf00000: return(24);
			case 0xe00000: return(24);
			case 0xd00000: return(24);
			case 0xc00000: return(24);
			case 0xb00000: return(24);
			case 0xa00000: return(24);
			case 0x900000: return(24);
			case 0x800000: return(24);
			case 0x700000: return(23);
			case 0x600000: return(23);
			case 0x500000: return(23);
			case 0x400000: return(23);
			case 0x300000: return(22);
			case 0x200000: return(22);
			case 0x100000: return(21);
		}
		switch(target_value & 0xf0000){
			case 0xf0000: return(20);
			case 0xe0000: return(20);
			case 0xd0000: return(20);
			case 0xc0000: return(20);
			case 0xb0000: return(20);
			case 0xa0000: return(20);
			case 0x90000: return(20);
			case 0x80000: return(20);
			case 0x70000: return(19);
			case 0x60000: return(19);
			case 0x50000: return(19);
			case 0x40000: return(19);
			case 0x30000: return(18);
			case 0x20000: return(18);
			case 0x10000: return(17);
		}
		switch(target_value & 0xf000){
			case 0xf000: return(16);
			case 0xe000: return(16);
			case 0xd000: return(16);
			case 0xc000: return(16);
			case 0xb000: return(16);
			case 0xa000: return(16);
			case 0x9000: return(16);
			case 0x8000: return(16);
			case 0x7000: return(15);
			case 0x6000: return(15);
			case 0x5000: return(15);
			case 0x4000: return(15);
			case 0x3000: return(14);
			case 0x2000: return(14);
			case 0x1000: return(13);
		}
		switch(target_value & 0xf00){
			case 0xf00: return(12);
			case 0xe00: return(12);
			case 0xd00: return(12);
			case 0xc00: return(12);
			case 0xb00: return(12);
			case 0xa00: return(12);
			case 0x900: return(12);
			case 0x800: return(12);
			case 0x700: return(11);
			case 0x600: return(11);
			case 0x500: return(11);
			case 0x400: return(11);
			case 0x300: return(10);
			case 0x200: return(10);
			case 0x100: return(9);
		}
		switch(target_value & 0xf0){
			case 0xf0: return(8);
			case 0xe0: return(8);
			case 0xd0: return(8);
			case 0xc0: return(8);
			case 0xb0: return(8);
			case 0xa0: return(8);
			case 0x90: return(8);
			case 0x80: return(8);
			case 0x70: return(7);
			case 0x60: return(7);
			case 0x50: return(7);
			case 0x40: return(7);
			case 0x30: return(6);
			case 0x20: return(6);
			case 0x10: return(5);
		}
		switch(target_value & 0xf){
			case 0xf: return(4);
			case 0xe: return(4);
			case 0xd: return(4);
			case 0xc: return(4);
			case 0xb: return(4);
			case 0xa: return(4);
			case 0x9: return(4);
			case 0x8: return(4);
			case 0x7: return(3);
			case 0x6: return(3);
			case 0x5: return(3);
			case 0x4: return(3);
			case 0x3: return(2);
			case 0x2: return(2);
			case 0x1: return(1);
		}
	}

	return(0);
}

int ds_find_first1_in_fraction_part(unsigned long target_value){

	if(target_value == 0){
		return(0);
	}
	else{
		switch(target_value & 0xf00){
			case 0xf00: return(1);
			case 0xe00: return(1);
			case 0xd00: return(1);
			case 0xc00: return(1);
			case 0xb00: return(1);
			case 0xa00: return(1);
			case 0x900: return(1);
			case 0x800: return(1);
			case 0x700: return(2);
			case 0x600: return(2);
			case 0x500: return(2);
			case 0x400: return(2);
			case 0x300: return(3);
			case 0x200: return(3);
			case 0x100: return(4);
		}
		switch(target_value & 0xf0){
			case 0xf0: return(5);
			case 0xe0: return(5);
			case 0xd0: return(5);
			case 0xc0: return(5);
			case 0xb0: return(5);
			case 0xa0: return(5);
			case 0x90: return(5);
			case 0x80: return(5);
			case 0x70: return(6);
			case 0x60: return(6);
			case 0x50: return(6);
			case 0x40: return(6);
			case 0x30: return(7);
			case 0x20: return(7);
			case 0x10: return(8);
		}
		switch(target_value & 0xf){
			case 0xf: return(9);
			case 0xe: return(9);
			case 0xd: return(9);
			case 0xc: return(9);
			case 0xb: return(9);
			case 0xa: return(9);
			case 0x9: return(9);
			case 0x8: return(9);
			case 0x7: return(10);
			case 0x6: return(10);
			case 0x5: return(10);
			case 0x4: return(10);
			case 0x3: return(11);
			case 0x2: return(11);
			case 0x1: return(12);
		}
	}

	return(0);
}

/* A sub function for ds_fpdiv().
	 This function compares (1 bit carry + 32 bits integer part + 12 bits fraction part)
	 of operand and operator.
 */
int ds_compare45bits(
int operand_carry, unsigned long operand_int_ulong, unsigned long operand_fra_fp12,
int operator_carry, unsigned long operator_int_ulong, unsigned long operator_fra_fp12)
{
	int lc_result = 0;

	lc_result = (operand_carry > operator_carry ? DS_LARGER :
							(operand_carry < operator_carry ? DS_SMALLER : 
							(operand_int_ulong > operator_int_ulong ? DS_LARGER :
							(operand_int_ulong < operator_int_ulong ? DS_SMALLER :
							(operand_fra_fp12 > operator_fra_fp12 ? DS_LARGER :
							(operand_fra_fp12 < operator_fra_fp12 ? DS_SMALLER : DS_EQUAL
							))))));

	return(lc_result);
}

/* A sub function for ds_fpdiv().
	 The carry for operand should be 0.
 */
int ds_shiftleft44bits(
int operand_carry, unsigned long operand_int_ulong, unsigned long operand_fra_fp12,
int shift,
int *operated_carry, unsigned long *operated_int_ulong, unsigned long *operated_fra_fp12)
{
	unsigned long targ_int_ulong = 0;
	unsigned long targ_fra_fp12 = 0;
	int operand_int_first1 = 0;

	*operated_carry = 0;
	*operated_int_ulong = 0;
	*operated_fra_fp12 = 0;

	if(operand_carry != 0){
		printk(KERN_INFO "[ds_shiftleft44bits] Error1: Shift left beyond 33 bits.\n");
		return(-1);
	}

	operand_int_first1 = ds_find_first1_in_integer_part(operand_int_ulong);

	if(operand_int_first1+shift > 33){
		printk(KERN_INFO "[ds_shiftleft44bits] Error2: Shift left beyond 33 bits.\n");
		return(-1);
	}
	else{
		if(operand_int_first1+shift == 33) *operated_carry = 1;
		targ_int_ulong = operand_int_ulong << shift;
		if(shift >= 12){
			targ_int_ulong |= operand_fra_fp12 << (shift-12);
			targ_fra_fp12 = 0x0;
		}
		else{
			switch(shift){
				case 11:
					targ_int_ulong |= operand_fra_fp12 >> 1;
					targ_fra_fp12 = (operand_fra_fp12 & 0x1) << 11;
					break;
				case 10:
					targ_int_ulong |= operand_fra_fp12 >> 2;
					targ_fra_fp12 = (operand_fra_fp12 & 0x3) << 10;
					break;
				case 9:
					targ_int_ulong |= operand_fra_fp12 >> 3;
					targ_fra_fp12 = (operand_fra_fp12 & 0x7) << 9;
					break;
				case 8:
					targ_int_ulong |= operand_fra_fp12 >> 4;
					targ_fra_fp12 = (operand_fra_fp12 & 0xf) << 8;
					break;
				case 7:
					targ_int_ulong |= operand_fra_fp12 >> 5;
					targ_fra_fp12 = (operand_fra_fp12 & 0x1f) << 7;
					break;
				case 6:
					targ_int_ulong |= operand_fra_fp12 >> 6;
					targ_fra_fp12 = (operand_fra_fp12 & 0x3f) << 6;
					break;
				case 5:
					targ_int_ulong |= operand_fra_fp12 >> 7;
					targ_fra_fp12 = (operand_fra_fp12 & 0x7f) << 5;
					break;
				case 4:
					targ_int_ulong |= operand_fra_fp12 >> 8;
					targ_fra_fp12 = (operand_fra_fp12 & 0xff) << 4;
					break;
				case 3:
					targ_int_ulong |= operand_fra_fp12 >> 9;
					targ_fra_fp12 = (operand_fra_fp12 & 0x1ff) << 3;
					break;
				case 2:
					targ_int_ulong |= operand_fra_fp12 >> 10;
					targ_fra_fp12 = (operand_fra_fp12 & 0x3ff) << 2;
					break;
				case 1:
					targ_int_ulong |= operand_fra_fp12 >> 11;
					targ_fra_fp12 = (operand_fra_fp12 & 0x7ff) << 1;
					break;
				case 0:
					targ_fra_fp12 |= operand_fra_fp12 & 0xfff;
			}
		}
		*operated_int_ulong = targ_int_ulong;
		*operated_fra_fp12 = targ_fra_fp12;
		return(0);
	}
}

/* A sub function for ds_fpdiv().
	 Operand should be larger than operator.
 */
int ds_subtract44bits(
int operand_carry, unsigned long operand_int_ulong, unsigned long operand_fra_fp12,
int operator_carry, unsigned long operator_int_ulong, unsigned long operator_fra_fp12,
int *operated_carry, unsigned long *operated_int_ulong, unsigned long *operated_fra_fp12)
{
	if(operand_carry == operator_carry){
		*operated_carry = 0;
		if(operand_fra_fp12 >= operator_fra_fp12){
			*operated_fra_fp12 = operand_fra_fp12 - operator_fra_fp12;
			*operated_int_ulong = operand_int_ulong - operator_int_ulong;
		}
		else{
			*operated_fra_fp12 = operand_fra_fp12 + 0x1000 - operator_fra_fp12;
			*operated_int_ulong = operand_int_ulong - 0x1 - operator_int_ulong;
		}
	}
	/* If operand_carry != operator_carry,
		 natually operand_carry == 1 and operator_carry == 0 */
	else{
		if(operand_int_ulong >= operator_int_ulong){
			*operated_carry = 1;
			if(operand_fra_fp12 >= operator_fra_fp12){
				*operated_fra_fp12 = operand_fra_fp12 - operator_fra_fp12;
				*operated_int_ulong = operand_int_ulong - operator_int_ulong;
			}
			else{
				*operated_fra_fp12 = operand_fra_fp12 + 0x1000 - operator_fra_fp12;
				*operated_int_ulong = operand_int_ulong - 0x1 - operator_int_ulong;
			}
		}
		else{
			*operated_carry = 0;
			if(operand_fra_fp12 >= operator_fra_fp12){
				*operated_fra_fp12 = operand_fra_fp12 - operator_fra_fp12;
				*operated_int_ulong = (0xffffffff - operator_int_ulong) + 1 + operand_int_ulong;
			}
			else{
				*operated_fra_fp12 = operand_fra_fp12 + 0x1000 - operator_fra_fp12;
				*operated_int_ulong = (0xffffffff - operator_int_ulong) + operand_int_ulong;
			}
		}
	}

	return(0);
}

/* ds_fpmul12 can treat only the case where the entire value
	 (integer part + fractional part) of the multiplicand, multiplier,
	 and multiplied all can be expressed in U(20,12) fixed point fractional number format,
	 i.e., max. 20 bits of integer part (for fractional parts, we don't need to worry about
	 because, whatever the fractional value is, it will be expressed in fixed 12 bits).

	 This function covers the rest cases that ds_fpmul12 can not treat by repeated additions
	 instead of the direct multiplication using ds_fpmul12.
	 For the case that ds_fpmul12 can treat, this function uses ds_fpmul12.

	 Note that multiplicand_fra_fp12 and multiplier_fra_fp12 should not include integer part.

	 multiplicand_int_ulong is the integer part of multiplicand.
	 multiplicand_fra_fp12 is the fraction part of multiplicand.
	 multiplier_int_ulong is the integer part of multiplier.
	 multiplier_fra_fp12 is the fraction part of multiplier.
	 multiplied_int_ulong is the integer part of multiplied.
	 multiplied_fra_fp12 is the fraction part of multiplied.
 */
int ds_fpmul(unsigned long multiplicand_int_ulong, unsigned long multiplicand_fra_fp12,
						 unsigned long multiplier_int_ulong, unsigned long multiplier_fra_fp12,
						 unsigned long *multiplied_int_ulong, unsigned long *multiplied_fra_fp12)
{
	int multiplicand_int_digits = 0;
	int multiplier_int_digits = 0;
	int i = 0;

	unsigned long operand_fp12 = 0;
	unsigned long operand_int_ulong = 0;
	unsigned long operand_fra_fp12 = 0;
	unsigned long operand_int_ulong_rest = 0;
	unsigned long operand_fp12_rest = 0;

	unsigned long operator_fp12 = 0;
	unsigned long operator_int_ulong = 0;
	unsigned long operator_fra_fp12 = 0;

	unsigned long operated_fp12 = 0;
	unsigned long operated_int_ulong = 0;
	unsigned long operated_fra_fp12 = 0;

	int operand_int_digits = 0;
	int operator_int_digits = 0;

	*multiplied_int_ulong = 0;
	*multiplied_fra_fp12 = 0;

	/* (1) Check if the integer part of the multiplied value will not be able to expressed
				 by a 32-bit unsigned long variable.
	 */
	multiplicand_int_digits = ds_find_first1_in_integer_part(multiplicand_int_ulong);
	multiplier_int_digits = ds_find_first1_in_integer_part(multiplier_int_ulong);
	if(multiplicand_int_digits + multiplier_int_digits > 32){
		printk(KERN_INFO "[ds_fpmul] multiplicand_int_ulong digits %d + \
										 multiplier_int_ulong digits %d exceeds 32\n",
					 multiplicand_int_digits, multiplier_int_digits);
		return(-1);
	}
	else{
		/* (2) If the integer + fraction of the multiplicand, multiplier, and multiplied
					 all can be represented in U(20,12), directly use ds_fpmul12.
		 */
		if(multiplicand_int_digits <= 20 &&
			 multiplier_int_digits <= 20 &&
			 multiplicand_int_digits + multiplier_int_digits <= 20)
		{
			operand_fp12 = DS_ULONG2FP12(multiplicand_int_ulong) | multiplicand_fra_fp12;
			operator_fp12 = DS_ULONG2FP12(multiplier_int_ulong) | multiplier_fra_fp12;
			ds_fpmul12(operand_fp12, operator_fp12, &operated_fp12);
			*multiplied_int_ulong = DS_GETFP12INT(operated_fp12);
			*multiplied_fra_fp12 = DS_GETFP12FRA(operated_fp12);
		}
		/* (3) If the integer + fraction of the multiplicand, multiplier, and multiplied
					 can not all be represented in U(20,12), use subsequent additions.

					 Now possible cases are

					 1. The integer + fraction of both the multiplicand and multiplier
							can be represented in U(20,12), but not for the multiplied.

					 2. The integer + fraction of either the multiplicand or multiplier
							can not be represented in U(20,12), and natually not for the multiplied.

					 Note that it is impossible that the integer + fraction of both the
					 multiplicand and multiplier can not be represented in U(20,12)
					 because such cases should have been filtered out by (1).
		 */
		else{

			/* Take one between the multiplicand and multiplier that can be respresented in
				 U(20,12) as operator.
			 */
			if(multiplicand_int_digits >= multiplier_int_digits){
				operand_int_digits = multiplicand_int_digits;
				operand_int_ulong = multiplicand_int_ulong;
				operand_fra_fp12 = multiplicand_fra_fp12;

				operator_int_digits = multiplier_int_digits;
				operator_int_ulong = multiplier_int_ulong;
				operator_fra_fp12 = multiplier_fra_fp12;
				operator_fp12 = DS_ULONG2FP12(multiplier_int_ulong) | multiplier_fra_fp12;
			}
			else{
				operand_int_digits = multiplier_int_digits;
				operand_int_ulong = multiplier_int_ulong;
				operand_fra_fp12 = multiplier_fra_fp12;

				operator_int_digits = multiplicand_int_digits;
				operator_int_ulong = multiplicand_int_ulong;
				operator_fra_fp12 = multiplicand_fra_fp12;
				operator_fp12 = DS_ULONG2FP12(multiplicand_int_ulong) | multiplicand_fra_fp12;
			}

			/* A multiplication is the sum of the operator shifted by each 1 in operand.
				 Thus, we repeatly add the operator (entire 32 bits) shifted by each 1 in operand
				 until the remaineder of the integer part of the operand becomes sufficiently small
				 such that the integer part of the operated is smaller than 20 bits.
				 Once the remainder of the integer part of the operand becomes sufficiently small,
				 then we use ds_fpmul12 for the remainder.
			 */
			for(i=operand_int_digits-1;i>(19-operator_int_digits);i--){

				/* Repeatedly add the shifted operator upon every 1 in operand */
				if((operand_int_ulong & (1<<i)) != 0){
					operated_int_ulong += operator_int_ulong << i;
					/* If the shift is not smaller than 12,
						 the entire operator_fra_fp12 becomes integer value.
						 Thus, just add the shifted operator_fra_fp12
						 to the integer part of the result. */
					if(i >= 12){
						operated_int_ulong += operator_fra_fp12 << (i-12);
					}
					/* If the shift is smaller than 12,
						 a fraction of operator_fra_fp12 will remains as fractional value.
						 Thus, just add the shifted operator_fra_fp12
						 to the integer part of the result. */
					else{
						switch(i){
							case 11:
								operated_int_ulong += operator_fra_fp12 >> 1;
								operated_fra_fp12 += (operator_fra_fp12 & 0x1) << 11;
								break;
							case 10:
								operated_int_ulong += operator_fra_fp12 >> 2;
								operated_fra_fp12 += (operator_fra_fp12 & 0x3) << 10;
								break;
							case 9:
								operated_int_ulong += operator_fra_fp12 >> 3;
								operated_fra_fp12 += (operator_fra_fp12 & 0x7) << 9;
								break;
							case 8:
								operated_int_ulong += operator_fra_fp12 >> 4;
								operated_fra_fp12 += (operator_fra_fp12 & 0xf) << 8;
								break;
							case 7:
								operated_int_ulong += operator_fra_fp12 >> 5;
								operated_fra_fp12 += (operator_fra_fp12 & 0x1f) << 7;
								break;
							case 6:
								operated_int_ulong += operator_fra_fp12 >> 6;
								operated_fra_fp12 += (operator_fra_fp12 & 0x3f) << 6;
								break;
							case 5:
								operated_int_ulong += operator_fra_fp12 >> 7;
								operated_fra_fp12 += (operator_fra_fp12 & 0x7f) << 5;
								break;
							case 4:
								operated_int_ulong += operator_fra_fp12 >> 8;
								operated_fra_fp12 += (operator_fra_fp12 & 0xff) << 4;
								break;
							case 3:
								operated_int_ulong += operator_fra_fp12 >> 9;
								operated_fra_fp12 += (operator_fra_fp12 & 0x1ff) << 3;
								break;
							case 2:
								operated_int_ulong += operator_fra_fp12 >> 10;
								operated_fra_fp12 += (operator_fra_fp12 & 0x3ff) << 2;
								break;
							case 1:
								operated_int_ulong += operator_fra_fp12 >> 11;
								operated_fra_fp12 += (operator_fra_fp12 & 0x7ff) << 1;
								break;
							case 0:
								operated_fra_fp12 += operator_fra_fp12 & 0xfff;
						}
						operated_int_ulong += DS_GETFP12INT(operated_fra_fp12);
						operated_fra_fp12 = DS_GETFP12FRA(operated_fra_fp12);
					}
				}
			}

			/* Now we can use ds_fpmul12.
				 Extract the remained operand,
				 i.e., the lower (20-operator_int_digits) digits only */
			switch(operator_int_digits){
				case 20: operand_int_ulong_rest = 0; break;
				case 19: operand_int_ulong_rest = operand_int_ulong & 0x1; break;
				case 18: operand_int_ulong_rest = operand_int_ulong & 0x3; break;
				case 17: operand_int_ulong_rest = operand_int_ulong & 0x7; break;
				case 16: operand_int_ulong_rest = operand_int_ulong & 0xf; break;
				case 15: operand_int_ulong_rest = operand_int_ulong & 0x1f; break;
				case 14: operand_int_ulong_rest = operand_int_ulong & 0x3f; break;
				case 13: operand_int_ulong_rest = operand_int_ulong & 0x7f; break;
				case 12: operand_int_ulong_rest = operand_int_ulong & 0xff; break;
				case 11: operand_int_ulong_rest = operand_int_ulong & 0x1ff; break;
				case 10: operand_int_ulong_rest = operand_int_ulong & 0x3ff; break;
				case 9: operand_int_ulong_rest = operand_int_ulong & 0x7ff; break;
				case 8: operand_int_ulong_rest = operand_int_ulong & 0xfff; break;
				case 7: operand_int_ulong_rest = operand_int_ulong & 0x1fff; break;
				case 6: operand_int_ulong_rest = operand_int_ulong & 0x3fff; break;
				case 5: operand_int_ulong_rest = operand_int_ulong & 0x7fff; break;
				case 4: operand_int_ulong_rest = operand_int_ulong & 0xffff; break;
				case 3: operand_int_ulong_rest = operand_int_ulong & 0x1ffff; break;
				case 2: operand_int_ulong_rest = operand_int_ulong & 0x3ffff; break;
				case 1: operand_int_ulong_rest = operand_int_ulong & 0x7ffff; break;
				case 0: operand_int_ulong_rest = operand_int_ulong & 0xfffff;
			}

			/* Call ds_fpmul12 */
			operand_fp12_rest = DS_ULONG2FP12(operand_int_ulong_rest) | operand_fra_fp12;
			operator_fp12 = DS_ULONG2FP12(operator_int_ulong) | operator_fra_fp12;
			ds_fpmul12(operand_fp12_rest, operator_fp12, &operated_fp12);
			operated_int_ulong += DS_GETFP12INT(operated_fp12);
			operated_fra_fp12 += DS_GETFP12FRA(operated_fp12);
			operated_int_ulong += DS_GETFP12INT(operated_fra_fp12);
			operated_fra_fp12 = DS_GETFP12FRA(operated_fra_fp12);
			*multiplied_int_ulong = operated_int_ulong;
			*multiplied_fra_fp12 = operated_fra_fp12;
		}
		return(0);
	}
}

/* ds_fpdiv12 can treat only in the case where the entire value
	 (integer part + fractional part) of the dividend, divisor,
	 and divided all can be expressed in U(20,12) fixed point fractional number format,
	 i.e., max. 20 bits of integer part (for fractional parts, we don't need to worry about
	 because, whatever the fractional value is, it will be expressed in fixed 12 bits.).

	 This function covers the rest cases that ds_fpdiv12 can not treat by repeated
	 subtraction instead of the direct division using ds_fpidv12.
	 For the case that ds_fpidv12 can treat, this function uses ds_fpidv12.

	 Note that dividend_fra_fp12 and divisor_fra_fp12 should not include integer part.

	 dividend_int_ulong is the integer part of dividend.
	 dividend_fra_fp12 is the fraction part of dividend.
	 divisor_int_ulong is the integer part of divisor.
	 divisor_fra_fp12 is the fraction part of divisor.
	 divided_int_ulong is the integer part of divided.
	 divided_fra_fp12 is the fraction part of divided.
 */
int ds_fpdiv(unsigned long dividend_int_ulong, unsigned long dividend_fra_fp12,
						 unsigned long divisor_int_ulong, unsigned long divisor_fra_fp12,
						 unsigned long *divided_int_ulong, unsigned long *divided_fra_fp12)
{
	int dividend_int_digits = 0;
	int divisor_int_digits = 0;
	int divisor_fra_digits = 0;
	int i = 0;

	unsigned long operand_fp12 = 0;
	unsigned long operand_int_ulong = 0;
	unsigned long operand_fra_fp12 = 0;

	unsigned long operand_int_ulong_shifted = 0;
	unsigned long operand_fra_fp12_shifted = 0;

	int operand_carry = 0;
	int operand_carry_shifted = 0;

	unsigned long operator_fp12 = 0;
	unsigned long operator_int_ulong = 0;
	unsigned long operator_fra_fp12 = 0;
	unsigned long operator_int_ulong_shifted = 0;
	unsigned long operator_fra_fp12_shifted = 0;

	int operator_carry = 0;
	int operator_carry_shifted = 0;

	unsigned long operated_fp12 = 0;
	unsigned long operated_int_ulong = 0;
	unsigned long operated_fra_fp12 = 0;

	unsigned long tmp_ulong = 0;

	int accumulated_operand_shifted_digits = 0;

	*divided_int_ulong = 0;
	*divided_fra_fp12 = 0;

	if(divisor_int_ulong == 0 && divisor_fra_fp12 == 0){
		printk(KERN_INFO "[ds_fpdiv] Error1: Divided by 0 \
											(divisor_int_ulong == 0 divisor_fra_fp12 == 0).\n");
		return(-1);
	}

	if(dividend_int_ulong == 0 && dividend_fra_fp12 == 0){
		*divided_int_ulong = 0;
		*divided_fra_fp12 = 0;
		return(0);
	}

	if(dividend_int_ulong == divisor_int_ulong && dividend_fra_fp12 == divisor_fra_fp12){
		*divided_int_ulong = 1;
		*divided_fra_fp12 = 0;
		return(0);
	}

	/* (1) Check if the integer part of the multiplied value will not be able to expressed
				 by a 32-bit unsigned long variable.
	 */
	dividend_int_digits = ds_find_first1_in_integer_part(dividend_int_ulong);
	divisor_int_digits = ds_find_first1_in_integer_part(divisor_int_ulong);
	divisor_fra_digits = ds_find_first1_in_fraction_part(divisor_fra_fp12);
	if(divisor_int_ulong == 0 && dividend_int_digits + divisor_fra_digits > 32){
		printk(KERN_INFO "[ds_fpdiv] Error2: divisor_int_ulong == 0 && \
										 dividend_int_digits %d + divisor_fra_digits %d > 32.\n",
					 dividend_int_digits, divisor_fra_digits);
		return(-1);
	}
	else{

		/* (2) If the integer + fraction of the dividend, divisor, and divided
					 all can be represented in U(20,12), directly use ds_fpdiv12.
		 */
		if(dividend_int_digits <= 20 &&
			 divisor_int_digits <= 20 &&
			 dividend_int_digits + divisor_fra_digits <= 20)
		{
			operand_fp12 = DS_ULONG2FP12(dividend_int_ulong) | dividend_fra_fp12;
			operator_fp12 = DS_ULONG2FP12(divisor_int_ulong) | divisor_fra_fp12;
			ds_fpdiv12(operand_fp12, operator_fp12, &operated_fp12, &tmp_ulong);
			*divided_int_ulong = DS_GETFP12INT(operated_fp12);
			*divided_fra_fp12 = DS_GETFP12FRA(operated_fp12);
		}
		/* (3) If the integer + fraction of the dividend, divisor, and divided
					 can not all be represented in U(20,12), use subsequent subtractions.

					 Now possible cases are

					 1. The integer + fraction of both the dividend and divisor
							can be represented in U(20,12),
							but the integer + fraction of divided can not be represented in U(20,12).

					 2. The integer + fraction of the dividend can not be represented in U(20,12)
							but that of the divisor can be.
							In this case, the integer + fraction of the divided may or may not be
							represented in U(20,12).

					 3. The integer + fraction of the dividend can be represented in U(20,12)
							but that of the divisor can not be.
							In this case, the integer + fraction of the divided always can be
							represented in U(20,12).

					 4. The integer + fraction of neigther the dividend nor divisor can be
							represented in U(20,12).
							In this case, the integer + fraction of the divided always can be
							represented in U(20,12).
		 */
		else{
			operand_carry = 0;
			operand_int_ulong = dividend_int_ulong;
			operand_fra_fp12 = dividend_fra_fp12;

			operator_carry = 0;
			operator_int_ulong = divisor_int_ulong;
			operator_fra_fp12 = divisor_fra_fp12;

			/* Calculate the integer part of the operated
				 as long as the operand is larger than the operator.
			 */
			while(ds_compare45bits(operand_carry,
														 operand_int_ulong,
														 operand_fra_fp12,
														 operator_carry,
														 operator_int_ulong,
														 operator_fra_fp12) == DS_LARGER)
			{
				/* Find the maximum number of digits
					 the operator can be shifted without exceeding the operand */
				i = 0;
				do{
					i ++;
					if(ds_shiftleft44bits(operator_carry,
																operator_int_ulong,
																operator_fra_fp12,
																i,
																&operator_carry_shifted,
																&operator_int_ulong_shifted,
																&operator_fra_fp12_shifted) < 0)
						return(-1);
				}while(ds_compare45bits(operator_carry_shifted,
																operator_int_ulong_shifted,
																operator_fra_fp12_shifted,
																operand_carry,
																operand_int_ulong,
																operand_fra_fp12) != DS_LARGER);
				/* The found number of digit to shift is i-1 */

				/* The found digit number is a fraction of the quotient */
				operated_int_ulong |= 1<<(i-1);

				/* Shift the operator left by the found digit number */
				if(ds_shiftleft44bits(operator_carry,
															operator_int_ulong,
															operator_fra_fp12, i-1,
															&operator_carry_shifted,
															&operator_int_ulong_shifted,
															&operator_fra_fp12_shifted) < 0) return(-1);

				/* Then, subtract the shifted operator
					 from the operand to get the remained value of the operand */
				ds_subtract44bits(operand_carry,
													operand_int_ulong,
													operand_fra_fp12,
													operator_carry_shifted,
													operator_int_ulong_shifted,
													operator_fra_fp12_shifted,
													&operand_carry,
													&operand_int_ulong,
													&operand_fra_fp12);

				/* If no remained value exists in the operand, it's the end of division */
				if(operand_carry == 0 && operand_int_ulong == 0 && operand_fra_fp12 == 0){
					*divided_int_ulong = operated_int_ulong;
					*divided_fra_fp12 = 0;
					return(0);
				}
			}

			/* Calculate the fractional part of the operated after the operand becomes
				 smaller than the operator.
				 This calculation needs to be repeated just until the LSB of the 12-bits
				 fraction part of result is determined.
				 And, obviously, the LSB is determined when the accumulated shifted digit
				 number of the operand reaches 12.
			 */
			accumulated_operand_shifted_digits = 0;
			while(accumulated_operand_shifted_digits < 12){
				/* Find the minimum number of digits the operand should be shifted to be
					 same or larger than the operator */
				i = 0;
				do{
					i ++;
					if(ds_shiftleft44bits(operand_carry,
																operand_int_ulong,
																operand_fra_fp12, i,
																&operand_carry_shifted,
																&operand_int_ulong_shifted,
																&operand_fra_fp12_shifted) < 0) return(-1);
				}while(ds_compare45bits(operand_carry_shifted,
																operand_int_ulong_shifted,
																operand_fra_fp12_shifted,
																operator_carry,
																operator_int_ulong,
																operator_fra_fp12) == DS_SMALLER);
				/* The found number of digit to shift is i */

				/* Update accumulated_operand_shifted_digits */
				accumulated_operand_shifted_digits += i;

				/* After the finding, if accumulated shifted digit number of the operand
					 exceeds 12, stop calculation */
				if(accumulated_operand_shifted_digits > 12){
					*divided_int_ulong = operated_int_ulong;
					*divided_fra_fp12 = operated_fra_fp12;
					return(0);
				}

				/* The found digit number is a fraction of the fractional part of the result */
				operated_fra_fp12 |= 1<<(12-accumulated_operand_shifted_digits);

				/* Subtract the operator from the shifted operand to get the remained value
					 of the shifted operand */
				ds_subtract44bits(operand_carry_shifted,
													operand_int_ulong_shifted,
													operand_fra_fp12_shifted,
													operator_carry,
													operator_int_ulong,
													operator_fra_fp12,
													&operand_carry,
													&operand_int_ulong,
													&operand_fra_fp12);

				/* If no remained value exists in the operand, it's the end of division */
				if(operand_carry == 0 && operand_int_ulong == 0 && operand_fra_fp12 == 0){
					*divided_int_ulong = operated_int_ulong;
					*divided_fra_fp12 = operated_fra_fp12;
					return(0);
				}
			}

			*divided_int_ulong = operated_int_ulong;
			*divided_fra_fp12 = operated_fra_fp12;
		}
		return(0);
	}
}

/*====================================================================
	The function which finds and returns the next high CPU_OP index.
	====================================================================*/

unsigned int ds_get_next_high_cpu_op_index(
unsigned long perf_requirement_int_ulong,
unsigned long perf_requirement_fra_fp12)
{

	/* If perf_requirement_int_ulong > 0,
		 which means that the required performance is greater than 1,
		 apply the maximum CPU_OP. */
	if(perf_requirement_int_ulong > 0){
		return(DS_CPU_OP_INDEX_0);
	}
	else if((perf_requirement_fra_fp12 & 0xff) != 0){
		switch(perf_requirement_fra_fp12 & 0xf00){
			case 0xf00: return(DS_CPU_OP_INDEX_0);	/* > 0.9375 and < 1 */
			case 0xe00: return(DS_CPU_OP_INDEX_0);	/* > 0.875 and < 0.9375 */
			case 0xd00: return(DS_CPU_OP_INDEX_0);	/* > 0.8125 and < 0.875 */
			case 0xc00: return(DS_CPU_OP_INDEX_1);	/* > 0.75 and < 0.8125 */
			case 0xb00: return(DS_CPU_OP_INDEX_1);	/* > 0.6875 and < 0.75 */
			case 0xa00: return(DS_CPU_OP_INDEX_1);	/* > 0.625 and < 0.6875 */
			case 0x900: return(DS_CPU_OP_INDEX_2);	/* > 0.5625 and < 0.625 */
			case 0x800: return(DS_CPU_OP_INDEX_2);	/* > 0.5 and < 0.5625 */
			case 0x700: return(DS_CPU_OP_INDEX_2);	/* > 0.4375 and < 0.5 */
			case 0x600: return(DS_CPU_OP_INDEX_2);	/* > 0.375 and < 0.4375 */
			case 0x500: return(DS_CPU_OP_INDEX_2);	/* > 0.3125 and < 0.375 */
			case 0x400: return(DS_CPU_OP_INDEX_3);	/* > 0.25 and < 0.3125 */
			case 0x300: return(DS_CPU_OP_INDEX_3);	/* > 0.1875 and < 0.25 */
			case 0x200: return(DS_CPU_OP_INDEX_3);	/* > 0.125 and < 0.1875 */
			case 0x100: return(DS_CPU_OP_INDEX_3);	/* > 0.0625 and < 0.125 */
			default: return(DS_CPU_OP_INDEX_3);	/* < 0.0625 */
		}
	}
	else{
		switch(perf_requirement_fra_fp12){
			case 0xf00: return(DS_CPU_OP_INDEX_0);	/* == 0.9375 */
			case 0xe00: return(DS_CPU_OP_INDEX_0);	/* == 0.875 */
			case 0xd00: return(DS_CPU_OP_INDEX_0);	/* == 0.8125 */
			case 0xc00: return(DS_CPU_OP_INDEX_1);	/* == 0.75 */
			case 0xb00: return(DS_CPU_OP_INDEX_1);	/* == 0.6875 */
			case 0xa00: return(DS_CPU_OP_INDEX_1);	/* == 0.625 */
			case 0x900: return(DS_CPU_OP_INDEX_2);	/* == 0.5625 */
			case 0x800: return(DS_CPU_OP_INDEX_2);	/* == 0.5 */
			case 0x700: return(DS_CPU_OP_INDEX_2);	/* == 0.4375 */
			case 0x600: return(DS_CPU_OP_INDEX_2);	/* == 0.375 */
			case 0x500: return(DS_CPU_OP_INDEX_2);	/* == 0.3125 */
			case 0x400: return(DS_CPU_OP_INDEX_3);	/* == 0.25 */
			case 0x300: return(DS_CPU_OP_INDEX_3);	/* == 0.1875 */
			case 0x200: return(DS_CPU_OP_INDEX_3);	/* == 0.125 */
			case 0x100: return(DS_CPU_OP_INDEX_3);	/* == 0.0625 */
			default: return(DS_CPU_OP_INDEX_3);	/* <= 0.0625 */
		}
	}
}

/*====================================================================
	The function which finds and returns the next low CPU_OP index.
	====================================================================*/

unsigned int ds_get_next_low_cpu_op_index(
unsigned long perf_requirement_int_ulong,
unsigned long perf_requirement_fra_fp12)
{

	/* If perf_requirement_int_ulong > 0,
		 which means that the required performance is greater than 1,
		 apply the maximum CPU_OP. */
	if(perf_requirement_int_ulong > 0){
		return(DS_CPU_OP_INDEX_0);
	}
	else if((perf_requirement_fra_fp12 & 0xff) != 0){
		switch(perf_requirement_fra_fp12 & 0xf00){
			case 0xf00: return(DS_CPU_OP_INDEX_1);	/* > 0.9375 and < 1 */
			case 0xe00: return(DS_CPU_OP_INDEX_1);	/* > 0.875 and < 0.9375 */
			case 0xd00: return(DS_CPU_OP_INDEX_1);	/* > 0.8125 and < 0.875 */
			case 0xc00: return(DS_CPU_OP_INDEX_2);	/* > 0.75 and < 0.8125 */
			case 0xb00: return(DS_CPU_OP_INDEX_2);	/* > 0.6875 and < 0.75 */
			case 0xa00: return(DS_CPU_OP_INDEX_2);	/* > 0.625 and < 0.6875 */
			case 0x900: return(DS_CPU_OP_INDEX_3);	/* > 0.5625 and < 0.625 */
			case 0x800: return(DS_CPU_OP_INDEX_3);	/* > 0.5 and < 0.5625 */
			case 0x700: return(DS_CPU_OP_INDEX_3);	/* > 0.4375 and < 0.5 */
			case 0x600: return(DS_CPU_OP_INDEX_3);	/* > 0.375 and < 0.4375 */
			case 0x500: return(DS_CPU_OP_INDEX_3);	/* > 0.3125 and < 0.375 */
			case 0x400: return(DS_CPU_OP_INDEX_3);	/* > 0.25 and < 0.3125 */
			case 0x300: return(DS_CPU_OP_INDEX_3);	/* > 0.1875 and < 0.25 */
			case 0x200: return(DS_CPU_OP_INDEX_3);	/* > 0.125 and < 0.1875 */
			case 0x100: return(DS_CPU_OP_INDEX_3);	/* > 0.0625 and < 0.125 */
			default: return(DS_CPU_OP_INDEX_3);	/* < 0.0625 */
		}
	}
	else{
		switch(perf_requirement_fra_fp12){
			case 0xf00: return(DS_CPU_OP_INDEX_1);	/* == 0.9375 */
			case 0xe00: return(DS_CPU_OP_INDEX_1);	/* == 0.875 */
			case 0xd00: return(DS_CPU_OP_INDEX_1);	/* == 0.8125 */
			case 0xc00: return(DS_CPU_OP_INDEX_2);	/* == 0.75 */
			case 0xb00: return(DS_CPU_OP_INDEX_2);	/* == 0.6875 */
			case 0xa00: return(DS_CPU_OP_INDEX_2);	/* == 0.625 */
			case 0x900: return(DS_CPU_OP_INDEX_3);	/* == 0.5625 */
			case 0x800: return(DS_CPU_OP_INDEX_3);	/* == 0.5 */
			case 0x700: return(DS_CPU_OP_INDEX_3);	/* == 0.4375 */
			case 0x600: return(DS_CPU_OP_INDEX_3);	/* == 0.375 */
			case 0x500: return(DS_CPU_OP_INDEX_3);	/* == 0.3125 */
			case 0x400: return(DS_CPU_OP_INDEX_3);	/* == 0.25 */
			case 0x300: return(DS_CPU_OP_INDEX_3);	/* == 0.1875 */
			case 0x200: return(DS_CPU_OP_INDEX_3);	/* == 0.125 */
			case 0x100: return(DS_CPU_OP_INDEX_3);	/* == 0.0625 */
			default: return(DS_CPU_OP_INDEX_3);	/* <= 0.0625 */
		}
	}
}

/*====================================================================
	The function which updates the fractions of
	busy (= task + schedule + dvs suite) and idle time at each CPU_OP.
	====================================================================*/

int ds_update_time_counter(void){

	unsigned long lc_sec_interval = 0;
	unsigned long lc_usec_interval = 0;
	unsigned long lc_usec_interval_fse = 0;
	unsigned long lc_usec_interval_fse_fra_fp12 = 0;

	/* (1) No rdtsc support in ARM.
			So, we instead use ds_status.cpu_op_index for the last time interval
			to calculate the full speed equivalent elapsed time.
			I.e., the fse elapsed time = 
			elapsed time (measured by do_gettimeofday) * scaling factor
	 */

	do_gettimeofday(&ds_timeval);
	ds_status.tv_sec_curr = ds_timeval.tv_sec;
	ds_status.tv_usec_curr = ds_timeval.tv_usec;

	if(ds_status.flag_time_base_initialized == 0){
		lc_usec_interval = 0;
		ds_status.flag_time_base_initialized = 1;
	}
	else{
		if(ds_status.tv_sec_curr == ds_status.tv_sec_base){
			if(ds_status.tv_usec_curr >= ds_status.tv_usec_base){
				lc_usec_interval = ds_status.tv_usec_curr - ds_status.tv_usec_base;
			}
			else{
				/* Time inversion. 
					This happens due to the inaccuracy in do_gettimeofday() function
					when it is called consecutively in a very short time inverval.
					In this case, we apply 1msec, which is the minimum context switch interval, 
					as default.
				 */
				lc_usec_interval = 1000;
			}
		}
		else{
			/* ds_status.tv_usec_curr should be ds_status.tv_usec_base + 1.
				This is because of that, 
				no matter how long the interval between two consecutive 
				ds_update_time_counter() calls is, it never ever able to 
				exceed 1 sec in Linux kernel since the function is called
				4 times at every context swtiching.
			 */
			lc_usec_interval = ds_status.tv_usec_curr + 1000000 - ds_status.tv_usec_base;
		}

	}
	ds_status.tv_sec_base = ds_status.tv_sec_curr;
	ds_status.tv_usec_base = ds_status.tv_usec_curr;

	switch(ds_status.cpu_op_index){
		case DS_CPU_OP_INDEX_0:
		lc_usec_interval_fse = lc_usec_interval;
		lc_usec_interval_fse_fra_fp12 = 0x0;
		break;
	case DS_CPU_OP_INDEX_1:
		ds_fpmul(lc_usec_interval, 0, 0, 0xccc, 
			 &lc_usec_interval_fse, &lc_usec_interval_fse_fra_fp12);
		break;
	case DS_CPU_OP_INDEX_2:
		ds_fpmul(lc_usec_interval, 0, 0, 0x999, 
			 &lc_usec_interval_fse, &lc_usec_interval_fse_fra_fp12);
		break;
	case DS_CPU_OP_INDEX_3:
		ds_fpmul(lc_usec_interval, 0, 0, 0x4cc, 
			 &lc_usec_interval_fse, &lc_usec_interval_fse_fra_fp12);
	default:
		lc_usec_interval_fse = lc_usec_interval;
		lc_usec_interval_fse_fra_fp12 = 0x0;
		break;
	}

	/* (2) Update ds_counter by using lc_usec_interval, lc_usec_interval_fse,
				 lc_usec_interval_fse_fra_fp12, ds_status.cpu_op_index,
				 ds_status.cpu_op_sf, ds_status.cpu_op_index_nr,
				 ds_status.cpu_op_mhz, and ds_status.cpu_mode.
	 */

	/* Elapsed */
	ds_counter.elapsed_usec += lc_usec_interval;
	if(ds_counter.elapsed_usec >= 1000000){
		ds_counter.elapsed_sec += 1;
		ds_counter.elapsed_usec -= 1000000;
	}

	/* Idle */
	if(ds_status.cpu_mode == DS_CPU_MODE_IDLE){
#if 0	// Not needed unless we want statistics
		ds_counter.idle_usec[ds_status.cpu_op_index_nr] += lc_usec_interval;
		if(ds_counter.idle_usec[ds_status.cpu_op_index_nr] >= 1000000){
			ds_counter.idle_sec[ds_status.cpu_op_index_nr] += 1;
			ds_counter.idle_usec[ds_status.cpu_op_index_nr] -= 1000000;
		}

		ds_counter.idle_total_usec += lc_usec_interval;
		if(ds_counter.idle_total_usec >= 1000000){
			ds_counter.idle_total_sec += 1;
			ds_counter.idle_total_usec -= 1000000;
		}
#endif
	}
	/* Busy */
	else{
#if 0	// Not needed unless we want statistics
		ds_counter.busy_usec[ds_status.cpu_op_index_nr] += lc_usec_interval;
		if(ds_counter.busy_usec[ds_status.cpu_op_index_nr] >= 1000000){
			ds_counter.busy_sec[ds_status.cpu_op_index_nr] += 1;
			ds_counter.busy_usec[ds_status.cpu_op_index_nr] -= 1000000;
		}
#endif

		ds_counter.busy_total_usec += lc_usec_interval;
		if(ds_counter.busy_total_usec >= 1000000){
			ds_counter.busy_total_sec += 1;
			ds_counter.busy_total_usec -= 1000000;
		}

		ds_counter.busy_fse_usec += lc_usec_interval_fse;
		if(ds_counter.busy_fse_usec >= 1000000){
			ds_counter.busy_fse_sec += 1;
			ds_counter.busy_fse_usec -= 1000000;
		}
		ds_counter.busy_fse_usec_fra_fp12 += lc_usec_interval_fse_fra_fp12;
		if(ds_counter.busy_fse_usec_fra_fp12 >= 0x1000){
			ds_counter.busy_fse_usec += 1;
			ds_counter.busy_fse_usec_fra_fp12 -= 0x1000;
		}

#if 0	// Not needed unless we want statistics
		switch(ds_status.cpu_mode){

			case DS_CPU_MODE_TASK:

				ds_counter.busy_task_usec[ds_status.cpu_op_index_nr] += lc_usec_interval;
				if(ds_counter.busy_task_usec[ds_status.cpu_op_index_nr] >= 1000000){
					ds_counter.busy_task_sec[ds_status.cpu_op_index_nr] += 1;
					ds_counter.busy_task_usec[ds_status.cpu_op_index_nr] -= 1000000;
				}

				ds_counter.busy_task_total_usec += lc_usec_interval;
				if(ds_counter.busy_task_total_usec >= 1000000){
					ds_counter.busy_task_total_sec += 1;
					ds_counter.busy_task_total_usec -= 1000000;
				}

				ds_counter.busy_task_fse_usec += lc_usec_interval_fse;
				if(ds_counter.busy_task_fse_usec >= 1000000){
					ds_counter.busy_task_fse_sec += 1;
					ds_counter.busy_task_fse_usec -= 1000000;
				}
				ds_counter.busy_task_fse_usec_fra_fp12 += lc_usec_interval_fse_fra_fp12;
				if(ds_counter.busy_task_fse_usec_fra_fp12 >= 0x1000){
					ds_counter.busy_task_fse_usec += 1;
					ds_counter.busy_task_fse_usec_fra_fp12 -= 0x1000;
				}
				break;

			case DS_CPU_MODE_SCHEDULE:

				ds_counter.busy_schedule_usec[ds_status.cpu_op_index_nr] += lc_usec_interval;
				if(ds_counter.busy_schedule_usec[ds_status.cpu_op_index_nr] >= 1000000){
					ds_counter.busy_schedule_sec[ds_status.cpu_op_index_nr] += 1;
					ds_counter.busy_schedule_usec[ds_status.cpu_op_index_nr] -= 1000000;
				}

				ds_counter.busy_schedule_total_usec += lc_usec_interval;
				if(ds_counter.busy_schedule_total_usec >= 1000000){
					ds_counter.busy_schedule_total_sec += 1;
					ds_counter.busy_schedule_total_usec -= 1000000;
				}

				ds_counter.busy_schedule_fse_usec += lc_usec_interval_fse;
				if(ds_counter.busy_schedule_fse_usec >= 1000000){
					ds_counter.busy_schedule_fse_sec += 1;
					ds_counter.busy_schedule_fse_usec -= 1000000;
				}
				ds_counter.busy_schedule_fse_usec_fra_fp12 += lc_usec_interval_fse_fra_fp12;
				if(ds_counter.busy_schedule_fse_usec_fra_fp12 >= 0x1000){
					ds_counter.busy_schedule_fse_usec += 1;
					ds_counter.busy_schedule_fse_usec_fra_fp12 -= 0x1000;
				}
				break;

			case DS_CPU_MODE_DVS_SUITE:

				ds_counter.busy_dvs_suite_usec[ds_status.cpu_op_index_nr] += lc_usec_interval;
				if(ds_counter.busy_dvs_suite_usec[ds_status.cpu_op_index_nr] >= 1000000){
					ds_counter.busy_dvs_suite_sec[ds_status.cpu_op_index_nr] += 1;
					ds_counter.busy_dvs_suite_usec[ds_status.cpu_op_index_nr] -= 1000000;
				}

				ds_counter.busy_dvs_suite_total_usec += lc_usec_interval;
				if(ds_counter.busy_dvs_suite_total_usec >= 1000000){
					ds_counter.busy_dvs_suite_total_sec += 1;
					ds_counter.busy_dvs_suite_total_usec -= 1000000;
				}

				ds_counter.busy_dvs_suite_fse_usec += lc_usec_interval_fse;
				if(ds_counter.busy_dvs_suite_fse_usec >= 1000000){
					ds_counter.busy_dvs_suite_fse_sec += 1;
					ds_counter.busy_dvs_suite_fse_usec -= 1000000;
				}
				ds_counter.busy_dvs_suite_fse_usec_fra_fp12 += lc_usec_interval_fse_fra_fp12;
				if(ds_counter.busy_dvs_suite_fse_usec_fra_fp12 >= 0x1000){
					ds_counter.busy_dvs_suite_fse_usec += 1;
					ds_counter.busy_dvs_suite_fse_usec_fra_fp12 -= 0x1000;
				}
				break;
		}
#endif
	}

	return(0);
}

/*====================================================================
	The functions to perform DVS scheme:
	AIDVS.
	====================================================================*/

#if 0
int ds_do_dvs_aidvs(unsigned int *target_cpu_op_index, 
					DS_AIDVS_STAT_STRUCT *stat,
					int target_static_prio, 
					int interval_window_size,
					unsigned long interval_window_length,
					unsigned long speedup_threshold, 
					unsigned long speedup_interval)
{
	unsigned long lc_time_usec_interval_inc = 0;
	unsigned long lc_time_usec_work_inc = 0;
	unsigned long lc_time_usec_work_in_window = 0;
	unsigned long lc_time_usec_interval_in_window = 0;
	unsigned long lc_time_usec_since_last_util_calc = 0;

	unsigned long lc_utilization_int_ulong_old_by_weight = 0;
	unsigned long lc_utilization_fra_fp12_old_by_weight = 0;
	unsigned long lc_numerator_int_ulong = 0;
	unsigned long lc_numerator_fra_fp12 = 0;
	unsigned long lc_moving_avg_int_ulong = 0;
	unsigned long lc_moving_avg_fra_fp12 = 0;

	/* (1) Update stat->time_usec_interval, 
			stat->time_usec_work_fse,
			stat->time_usec_work_fse_lasting, OR
			stat->time_usec_work,
			stat->time_usec_work_lasting.
	 */
	if(stat->base_initialized == 0 ||
		(stat->base_initialized != 0 &&
		ds_counter.elapsed_sec > stat->time_sec_interval_inc_base + 1))
	{
		stat->time_usec_util_calc_base = ds_counter.elapsed_usec;
		stat->time_usec_interval_inc_base = ds_counter.elapsed_usec;
		stat->time_sec_interval_inc_base = ds_counter.elapsed_sec;
		stat->time_usec_work_inc_base = ds_counter.busy_total_usec;
		stat->cpu_op_index = DS_CPU_OP_INDEX_MAX;
		stat->base_initialized = 1;
	}

	/* This calaculation assumes that the interval
	   between any consecutive context switches is shorter than 1 sec.
	 */
	if(ds_counter.elapsed_usec >= stat->time_usec_interval_inc_base){
		lc_time_usec_interval_inc =
			ds_counter.elapsed_usec - stat->time_usec_interval_inc_base;
	}
	else{
		lc_time_usec_interval_inc =
			ds_counter.elapsed_usec + (1000000 - stat->time_usec_interval_inc_base);
	}
	stat->time_usec_interval += lc_time_usec_interval_inc;
	stat->time_usec_interval_inc_base = ds_counter.elapsed_usec;
	stat->time_sec_interval_inc_base = ds_counter.elapsed_sec;

	if(stat->flag_in_busy_half == 1){
		if(ds_counter.busy_total_usec >= stat->time_usec_work_inc_base){
			lc_time_usec_work_inc =
				ds_counter.busy_total_usec - stat->time_usec_work_inc_base;
		}
		else{
			lc_time_usec_work_inc =
				ds_counter.busy_total_usec + (1000000 - stat->time_usec_work_inc_base);
		}
		stat->time_usec_work += lc_time_usec_work_inc;
		stat->time_usec_work_lasting += lc_time_usec_work_inc;
		stat->time_usec_work_inc_base = ds_counter.busy_total_usec;
	}

	/* (2) If a series of workload lasts over speedup_threshold,
			update CPU_OP to cope with this workload burst.
	 */
	if((stat->consecutive_speedup_count == 0 &&
		stat->time_usec_work_lasting >= speedup_threshold) ||
		(stat->consecutive_speedup_count > 0 &&
		stat->time_usec_work_lasting >= speedup_interval))
	{
		/* Rampup stat->cpu_op_index by one available level */
		switch(stat->cpu_op_index){
			case DS_CPU_OP_INDEX_0: break;
			case DS_CPU_OP_INDEX_1: stat->cpu_op_index = DS_CPU_OP_INDEX_0; break;
			case DS_CPU_OP_INDEX_2: stat->cpu_op_index = DS_CPU_OP_INDEX_1; break;
			case DS_CPU_OP_INDEX_3: stat->cpu_op_index = DS_CPU_OP_INDEX_2; break;
			default: stat->cpu_op_index = DS_CPU_OP_INDEX_0; break;
		}

		/* After the speedup, initialize followings */
		stat->time_usec_work_lasting = 0;
		stat->consecutive_speedup_count ++;
	}

	/* (3) Basic AIDVS operation at every context switch.
	 */

	/* If ds_parameter.entry_type == DS_ENTRY_SWITCH_TO */
	if(ds_parameter.entry_type == DS_ENTRY_SWITCH_TO){

		/* Idle -> Busy, i.e. the boundary of two consecutive intervals. */
		if((ds_parameter.prev_p->static_prio > target_static_prio ||
			ds_parameter.prev_p->pid == 0) &&
			ds_parameter.next_p->static_prio <= target_static_prio)
		{

			/* Until all the stat->interval_window_array[] is filled, 
				this calculation is based on the less number of intervals 
				than interval_window_size.
				The following code automatically discards the oldest interval's
				time_usec_interval and time_usec_work_fse OR time_usec_work 
				to keep the number of intervals that are used for 
				the calculation interval_window_size.
			 */
#if 0	// MOVING WINDOW
			if(stat->time_usec_interval_in_window < 
				stat->interval_window_array[stat->interval_window_index].time_usec_interval)
				stat->time_usec_interval_in_window = 0;
			else
				stat->time_usec_interval_in_window -=
					stat->interval_window_array[stat->interval_window_index].time_usec_interval;

			if(stat->time_usec_work_fse_in_window < 
				stat->interval_window_array[stat->interval_window_index].time_usec_work_fse)
				stat->time_usec_work_fse_in_window = 0;
			else
				stat->time_usec_work_fse_in_window -=
					stat->interval_window_array[stat->interval_window_index].time_usec_work_fse;

			stat->interval_window_array[stat->interval_window_index].time_usec_interval = 
				stat->time_usec_interval;
			stat->interval_window_array[stat->interval_window_index].time_usec_work_fse = 
				stat->time_usec_work_fse;

			stat->time_usec_interval_in_window +=
				stat->interval_window_array[stat->interval_window_index].time_usec_interval;
			stat->time_usec_work_fse_in_window +=
				stat->interval_window_array[stat->interval_window_index].time_usec_work_fse;
#endif

			stat->time_usec_interval_in_window += stat->time_usec_interval;
			stat->time_usec_work_in_window += stat->time_usec_work;

			if(stat->time_usec_interval_in_window > 1000000){
				lc_time_usec_interval_in_window = stat->time_usec_interval_in_window / 1000;
				lc_time_usec_work_in_window = stat->time_usec_work_in_window / 1000;
			}
			else{
				lc_time_usec_interval_in_window = stat->time_usec_interval_in_window;
				lc_time_usec_work_in_window = stat->time_usec_work_in_window;
			}

#if 0	// MOVING WINDOW
			if(stat->interval_window_index == interval_window_size - 1)
				stat->interval_window_index = 0;
			else
				stat->interval_window_index ++;
#endif

			// DS_GPSCHEDVS_Lx_MIN_WINDOW_LENGTH
			if(ds_counter.elapsed_usec >= stat->time_usec_util_calc_base){
				lc_time_usec_since_last_util_calc =
					ds_counter.elapsed_usec - stat->time_usec_util_calc_base;
			}
			else{
				lc_time_usec_since_last_util_calc =
					ds_counter.elapsed_usec + (1000000 - stat->time_usec_util_calc_base);
			}

			if(lc_time_usec_since_last_util_calc >= interval_window_length){

				/* Calculate OLD x 3 */
				ds_fpmul(stat->utilization_int_ulong, 
						stat->utilization_fra_fp12, 
						DS_AIDVS_MOVING_AVG_WEIGHT, 
						0x0,
						&lc_utilization_int_ulong_old_by_weight, 
						&lc_utilization_fra_fp12_old_by_weight);
	
				/* Calculate NEW, i.e., the current interval window's utilization */
				if(lc_time_usec_interval_in_window == 0){
					stat->utilization_int_ulong = 1;
					stat->utilization_fra_fp12 = 0;
				}
				else{
					if(ds_fpdiv(lc_time_usec_work_in_window,
								0,
								lc_time_usec_interval_in_window,
								0,
								&stat->utilization_int_ulong,
								&stat->utilization_fra_fp12) < 0){
						printk(KERN_INFO "[ds_do_dvs_aidvs] Error: ds_fpdiv failed. \
										 Current DVS scheme is disabled.\n");
						ds_configuration.on_dvs = 0;
						return(-1);
					}
				}

				/* Calculate OLD x 3 + NEW, i.e., the numerator of moving average */
				lc_numerator_int_ulong = 
					lc_utilization_int_ulong_old_by_weight + stat->utilization_int_ulong;
				lc_numerator_fra_fp12 = 
					lc_utilization_fra_fp12_old_by_weight + stat->utilization_fra_fp12;
				if(lc_numerator_fra_fp12 >= 0x1000){
					lc_numerator_int_ulong += 1;
					lc_numerator_fra_fp12 -= 0x1000;
				}

				/* Calculate (OLD x 3 + NEW) / 4, i.e., the moving average */
				ds_fpdiv(lc_numerator_int_ulong, lc_numerator_fra_fp12, 
						DS_AIDVS_MOVING_AVG_WEIGHT+1, 0,
						&lc_moving_avg_int_ulong, &lc_moving_avg_fra_fp12);

				/* Find the CPU_OP_INDEX corresponding to the calculated utilization */
				stat->cpu_op_index = 
					ds_get_next_high_cpu_op_index(lc_moving_avg_int_ulong, lc_moving_avg_fra_fp12);

				stat->time_usec_interval_in_window = 0;
				stat->time_usec_work_in_window = 0;
				stat->interval_window_index = 0;
				stat->time_usec_util_calc_base = ds_counter.elapsed_usec;
			}

			/* Upon every new arrival of a busy half, initialize followings.
			 */
			stat->time_usec_work = 0;
			stat->time_usec_interval = 0;

			/* As GPScheDVS uses a multiple number of AIDVS each of them sees
				a particular priority range of tasks, we have to initialize
				time_usec_work_fse_inc_base OR time_usec_work_inc 
				at every beginning of a new interval.
				Otherwise, all the tasks executed during this particular AIDVS's
				idle half (i.e., not the busy half) will be added into
				stat->interval_window_array[stat->interval_window_index].time_usec_work_fse
				and then stat->time_usec_work_fse_in_window OR
				stat->interval_window_array[stat->interval_window_index].time_usec_work
				and then stat->time_usec_work_in_window. */
			stat->time_usec_work_inc_base = ds_counter.busy_usec;

			/* Upon the end of an interval, set flag_in_busy_half for the next interval. */
			stat->flag_in_busy_half = 1;
		}

		/* Busy -> Idle, i.e., the end of the busy half of the current interval */
		else if(ds_parameter.prev_p->static_prio <= target_static_prio &&
				(ds_parameter.next_p->static_prio > target_static_prio || 
				ds_parameter.next_p->pid == 0))
		{
			stat->time_usec_work_lasting = 0;
			stat->consecutive_speedup_count = 0;
			stat->flag_in_busy_half = 0;
		}
	}

	/* (4) Determine *target_cpu_op_index.
	 */
	*target_cpu_op_index = stat->cpu_op_index;

	return(0);
}
#endif

int ds_do_dvs_aidvs(unsigned int *target_cpu_op_index, 
					DS_AIDVS_STAT_STRUCT *stat,
					int target_static_prio, 
					int interval_window_size,
					unsigned long interval_window_length,
					unsigned long speedup_threshold, 
					unsigned long speedup_interval)
{
	unsigned long lc_time_usec_interval_inc = 0;
	unsigned long lc_time_usec_work_fse_inc = 0;
	unsigned long lc_time_usec_work_fse_in_window = 0;
	unsigned long lc_time_usec_interval_in_window = 0;
	unsigned long lc_time_usec_since_last_util_calc = 0;

	unsigned long lc_utilization_int_ulong_old_by_weight = 0;
	unsigned long lc_utilization_fra_fp12_old_by_weight = 0;
	unsigned long lc_numerator_int_ulong = 0;
	unsigned long lc_numerator_fra_fp12 = 0;
	unsigned long lc_moving_avg_int_ulong = 0;
	unsigned long lc_moving_avg_fra_fp12 = 0;

	/* (1) Update stat->time_usec_interval, 
			stat->time_usec_work_fse,
			stat->time_usec_work_fse_lasting, OR
			stat->time_usec_work,
			stat->time_usec_work_lasting.
	 */
	if(stat->base_initialized == 0 ||
		(stat->base_initialized != 0 &&
		ds_counter.elapsed_sec > stat->time_sec_interval_inc_base + 1))
	{
		stat->time_usec_util_calc_base = ds_counter.elapsed_usec;
		stat->time_usec_interval_inc_base = ds_counter.elapsed_usec;
		stat->time_sec_interval_inc_base = ds_counter.elapsed_sec;
		stat->time_usec_work_fse_inc_base = ds_counter.busy_fse_usec;
		stat->cpu_op_index = DS_CPU_OP_INDEX_MAX;
		stat->base_initialized = 1;
	}

	/* This calaculation assumes that the interval
	   between any consecutive context switches is shorter than 1 sec.
	 */
	if(ds_counter.elapsed_usec >= stat->time_usec_interval_inc_base){
		lc_time_usec_interval_inc =
			ds_counter.elapsed_usec - stat->time_usec_interval_inc_base;
	}
	else{
		lc_time_usec_interval_inc =
			ds_counter.elapsed_usec + (1000000 - stat->time_usec_interval_inc_base);
	}
	stat->time_usec_interval += lc_time_usec_interval_inc;
	stat->time_usec_interval_inc_base = ds_counter.elapsed_usec;
	stat->time_sec_interval_inc_base = ds_counter.elapsed_sec;

	if(stat->flag_in_busy_half == 1){
		if(ds_counter.busy_fse_usec >= stat->time_usec_work_fse_inc_base){
			lc_time_usec_work_fse_inc =
				ds_counter.busy_fse_usec - stat->time_usec_work_fse_inc_base;
		}
		else{
			lc_time_usec_work_fse_inc =
				ds_counter.busy_fse_usec + (1000000 - stat->time_usec_work_fse_inc_base);
		}
		stat->time_usec_work_fse += lc_time_usec_work_fse_inc;
		stat->time_usec_work_fse_lasting += lc_time_usec_work_fse_inc;
		stat->time_usec_work_fse_inc_base = ds_counter.busy_fse_usec;
	}

	/* (2) If a series of workload lasts over speedup_threshold,
			update CPU_OP to cope with this workload burst.
	 */
	if((stat->consecutive_speedup_count == 0 &&
		stat->time_usec_work_fse_lasting >= speedup_threshold) ||
		(stat->consecutive_speedup_count > 0 &&
		stat->time_usec_work_fse_lasting >= speedup_interval))
	{
		/* Rampup stat->cpu_op_index by one available level */
		switch(stat->cpu_op_index){
			case DS_CPU_OP_INDEX_0: break;
			case DS_CPU_OP_INDEX_1: stat->cpu_op_index = DS_CPU_OP_INDEX_0; break;
			case DS_CPU_OP_INDEX_2: stat->cpu_op_index = DS_CPU_OP_INDEX_1; break;
			case DS_CPU_OP_INDEX_3: stat->cpu_op_index = DS_CPU_OP_INDEX_2; break;
			default: stat->cpu_op_index = DS_CPU_OP_INDEX_0; break;
		}

		/* After the speedup, initialize followings */
		stat->time_usec_work_fse_lasting = 0;
		stat->consecutive_speedup_count ++;
	}

	/* (3) Basic AIDVS operation at every context switch.
	 */

	/* If ds_parameter.entry_type == DS_ENTRY_SWITCH_TO */
	if(ds_parameter.entry_type == DS_ENTRY_SWITCH_TO){

		/* Idle -> Busy, i.e. the boundary of two consecutive intervals. */
		if((ds_parameter.prev_p->static_prio > target_static_prio ||
			ds_parameter.prev_p->pid == 0) &&
			ds_parameter.next_p->static_prio <= target_static_prio)
		{

			/* Until all the stat->interval_window_array[] is filled, 
				this calculation is based on the less number of intervals 
				than interval_window_size.
				The following code automatically discards the oldest interval's
				time_usec_interval and time_usec_work_fse OR time_usec_work 
				to keep the number of intervals that are used for 
				the calculation interval_window_size.
			 */
#if 0	// MOVING WINDOW
			if(stat->time_usec_interval_in_window < 
				stat->interval_window_array[stat->interval_window_index].time_usec_interval)
				stat->time_usec_interval_in_window = 0;
			else
				stat->time_usec_interval_in_window -=
					stat->interval_window_array[stat->interval_window_index].time_usec_interval;

			if(stat->time_usec_work_fse_in_window < 
				stat->interval_window_array[stat->interval_window_index].time_usec_work_fse)
				stat->time_usec_work_fse_in_window = 0;
			else
				stat->time_usec_work_fse_in_window -=
					stat->interval_window_array[stat->interval_window_index].time_usec_work_fse;

			stat->interval_window_array[stat->interval_window_index].time_usec_interval = 
				stat->time_usec_interval;
			stat->interval_window_array[stat->interval_window_index].time_usec_work_fse = 
				stat->time_usec_work_fse;

			stat->time_usec_interval_in_window +=
				stat->interval_window_array[stat->interval_window_index].time_usec_interval;
			stat->time_usec_work_fse_in_window +=
				stat->interval_window_array[stat->interval_window_index].time_usec_work_fse;
#endif

			stat->time_usec_interval_in_window += stat->time_usec_interval;
			stat->time_usec_work_fse_in_window += stat->time_usec_work_fse;

			if(stat->time_usec_interval_in_window > 1000000){
				lc_time_usec_interval_in_window = stat->time_usec_interval_in_window / 1000;
				lc_time_usec_work_fse_in_window = stat->time_usec_work_fse_in_window / 1000;
			}
			else{
				lc_time_usec_interval_in_window = stat->time_usec_interval_in_window;
				lc_time_usec_work_fse_in_window = stat->time_usec_work_fse_in_window;
			}

#if 0	// MOVING WINDOW
			if(stat->interval_window_index == interval_window_size - 1)
				stat->interval_window_index = 0;
			else
				stat->interval_window_index ++;
#endif

			// DS_GPSCHEDVS_Lx_MIN_WINDOW_LENGTH
			if(ds_counter.elapsed_usec >= stat->time_usec_util_calc_base){
				lc_time_usec_since_last_util_calc =
					ds_counter.elapsed_usec - stat->time_usec_util_calc_base;
			}
			else{
				lc_time_usec_since_last_util_calc =
					ds_counter.elapsed_usec + (1000000 - stat->time_usec_util_calc_base);
			}

			if(lc_time_usec_since_last_util_calc >= interval_window_length){

				/* Calculate OLD x 3 */
				ds_fpmul(stat->utilization_int_ulong, 
						stat->utilization_fra_fp12, 
						DS_AIDVS_MOVING_AVG_WEIGHT, 
						0x0,
						&lc_utilization_int_ulong_old_by_weight, 
						&lc_utilization_fra_fp12_old_by_weight);
	
				/* Calculate NEW, i.e., the current interval window's utilization */
				if(lc_time_usec_interval_in_window == 0){
					stat->utilization_int_ulong = 1;
					stat->utilization_fra_fp12 = 0;
				}
				else{
					if(ds_fpdiv(lc_time_usec_work_fse_in_window,
								0,
								lc_time_usec_interval_in_window,
								0,
								&stat->utilization_int_ulong,
								&stat->utilization_fra_fp12) < 0){
						printk(KERN_INFO "[ds_do_dvs_aidvs] Error: ds_fpdiv failed. \
										 Current DVS scheme is disabled.\n");
						ds_configuration.on_dvs = 0;
						return(-1);
					}
				}

				/* Calculate OLD x 3 + NEW, i.e., the numerator of moving average */
				lc_numerator_int_ulong = 
					lc_utilization_int_ulong_old_by_weight + stat->utilization_int_ulong;
				lc_numerator_fra_fp12 = 
					lc_utilization_fra_fp12_old_by_weight + stat->utilization_fra_fp12;
				if(lc_numerator_fra_fp12 >= 0x1000){
					lc_numerator_int_ulong += 1;
					lc_numerator_fra_fp12 -= 0x1000;
				}

				/* Calculate (OLD x 3 + NEW) / 4, i.e., the moving average */
				ds_fpdiv(lc_numerator_int_ulong, lc_numerator_fra_fp12, 
						DS_AIDVS_MOVING_AVG_WEIGHT+1, 0,
						&lc_moving_avg_int_ulong, &lc_moving_avg_fra_fp12);

				/* Find the CPU_OP_INDEX corresponding to the calculated utilization */
				stat->cpu_op_index = 
					ds_get_next_high_cpu_op_index(lc_moving_avg_int_ulong, lc_moving_avg_fra_fp12);

				stat->time_usec_interval_in_window = 0;
				stat->time_usec_work_fse_in_window = 0;
				stat->interval_window_index = 0;
				stat->time_usec_util_calc_base = ds_counter.elapsed_usec;
			}

			/* Upon every new arrival of a busy half, initialize followings.
			 */
			stat->time_usec_work_fse = 0;
			stat->time_usec_interval = 0;

			/* As GPScheDVS uses a multiple number of AIDVS each of them sees
				a particular priority range of tasks, we have to initialize
				time_usec_work_fse_inc_base OR time_usec_work_inc 
				at every beginning of a new interval.
				Otherwise, all the tasks executed during this particular AIDVS's
				idle half (i.e., not the busy half) will be added into
				stat->interval_window_array[stat->interval_window_index].time_usec_work_fse
				and then stat->time_usec_work_fse_in_window OR
				stat->interval_window_array[stat->interval_window_index].time_usec_work
				and then stat->time_usec_work_in_window. */
			stat->time_usec_work_fse_inc_base = ds_counter.busy_fse_usec;

			/* Upon the end of an interval, set flag_in_busy_half for the next interval. */
			stat->flag_in_busy_half = 1;
		}

		/* Busy -> Idle, i.e., the end of the busy half of the current interval */
		else if(ds_parameter.prev_p->static_prio <= target_static_prio &&
				(ds_parameter.next_p->static_prio > target_static_prio || 
				ds_parameter.next_p->pid == 0))
		{
			stat->time_usec_work_fse_lasting = 0;
			stat->consecutive_speedup_count = 0;
			stat->flag_in_busy_half = 0;
		}
	}

	/* (4) Determine *target_cpu_op_index.
	 */
	*target_cpu_op_index = stat->cpu_op_index;

	return(0);
}

/*====================================================================
	The functions to perform DVS scheme:
	GPScheDVS.
	====================================================================*/

int ds_do_dvs_gpschedvs(unsigned int *target_cpu_op_index){

	unsigned int lc_target_cpu_op_index_highest = DS_CPU_OP_INDEX_MIN;
	unsigned int lc_target_cpu_op_index_aidvs_l1 = DS_CPU_OP_INDEX_MIN;
	unsigned int lc_target_cpu_op_index_aidvs_l2 = DS_CPU_OP_INDEX_MIN;
	unsigned int lc_target_cpu_op_index_aidvs_l3 = DS_CPU_OP_INDEX_MIN;
	unsigned int lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_MIN;

	if(ds_status.flag_do_post_early_suspend == 0){

		//if(ds_status.touch_timeout_sec == 0){
#if 0
		/* AIDVS for HRT tasks */
		if(ds_do_dvs_aidvs(&lc_target_cpu_op_index_highest,
							 &ds_gpschedvs_status.aidvs_l0_status,
							 DS_HRT_STATIC_PRIO,
							 DS_GPSCHEDVS_L0_INTERVALS_IN_AN_WINDOW,
							 DS_GPSCHEDVS_L0_MIN_WINDOW_LENGTH,
							 DS_GPSCHEDVS_L0_SPEEDUP_THRESHOLD,
							 DS_GPSCHEDVS_L0_SPEEDUP_INTERVAL) < 0)
		{
			printk(KERN_INFO "[ds_do_dvs_gpschedvs] Error: ds_do_dvs_aidvs for level0 failed. \
							 Current DVS scheme is disabled.\n");
			ds_configuration.on_dvs = 0;
			return(-1);
		}
#endif

#if 0
		/* AIDVS for HRT + DBSRT tasks */
		if(ds_do_dvs_aidvs(&lc_target_cpu_op_index_aidvs_l1,
							 &ds_gpschedvs_status.aidvs_l1_status,
							 DS_DBSRT_STATIC_PRIO,
							 DS_GPSCHEDVS_L1_INTERVALS_IN_AN_WINDOW,
							 DS_GPSCHEDVS_L1_MIN_WINDOW_LENGTH,
							 DS_GPSCHEDVS_L1_SPEEDUP_THRESHOLD,
							 DS_GPSCHEDVS_L1_SPEEDUP_INTERVAL) < 0)
		{
			printk(KERN_INFO "[ds_do_dvs_gpschedvs] Error: ds_do_dvs_aidvs for level1 failed. \
							 Current DVS scheme is disabled.\n");
			ds_configuration.on_dvs = 0;
			return(-1);
		}
		if(lc_target_cpu_op_index_aidvs_l1 > lc_target_cpu_op_index_highest)
			lc_target_cpu_op_index_highest = lc_target_cpu_op_index_aidvs_l1;
#endif

#if 1
		/* AIDVS for HRT + DBSRT + RBSRT tasks */
		if(ds_do_dvs_aidvs(&lc_target_cpu_op_index_aidvs_l2,
							 &ds_gpschedvs_status.aidvs_l2_status,
							 DS_RBSRT_STATIC_PRIO,
							 DS_GPSCHEDVS_L2_INTERVALS_IN_AN_WINDOW,
							 DS_GPSCHEDVS_L2_MIN_WINDOW_LENGTH,
							 DS_GPSCHEDVS_L2_SPEEDUP_THRESHOLD,
							 DS_GPSCHEDVS_L2_SPEEDUP_INTERVAL) < 0)
		{
			printk(KERN_INFO "[ds_do_dvs_gpschedvs] Error: ds_do_dvs_aidvs for level2 failed. \
							 Current DVS scheme is disabled.\n");
			ds_configuration.on_dvs = 0;
			return(-1);
		}
		if(lc_target_cpu_op_index_aidvs_l2 > lc_target_cpu_op_index_highest)
			lc_target_cpu_op_index_highest = lc_target_cpu_op_index_aidvs_l2;
#endif

		/* AIDVS for HRT + DBSRT + RBSRT + NRT tasks */
		/* If ds_configuration.gpschedvs_strategy == 1, i.e., CPU power centric
			 strategy, GPScheDVS intentionally applies the lowest CPU_OP to every
			 NRT task so as to reduce CPU energy consumption at the cost of NRT
			 tasks' final completion time and thus the system energy consumption.
		 */
#if 1
		if(ds_configuration.gpschedvs_strategy == 0){
			if(ds_do_dvs_aidvs(&lc_target_cpu_op_index_aidvs_l3,
								 &ds_gpschedvs_status.aidvs_l3_status,
								 DS_NRT_STATIC_PRIO,
								 DS_GPSCHEDVS_L3_INTERVALS_IN_AN_WINDOW,
								 DS_GPSCHEDVS_L3_MIN_WINDOW_LENGTH,
								 DS_GPSCHEDVS_L3_SPEEDUP_THRESHOLD,
								 DS_GPSCHEDVS_L3_SPEEDUP_INTERVAL) < 0)
			{
				printk(KERN_INFO "[ds_do_dvs_gpschedvs] Error: ds_do_dvs_aidvs level3 failed. \
								 Current DVS scheme is disabled.\n");
				ds_configuration.on_dvs = 0;
				return(-1);
			}
		}
		if(lc_target_cpu_op_index_aidvs_l3 > lc_target_cpu_op_index_highest)
			lc_target_cpu_op_index_highest = lc_target_cpu_op_index_aidvs_l3;
#endif
		//}
#if 1
		/* Additional treatment for touch input event.
			Once we got an touch interrupt, we apply 
			a high CPU frequency for a while.
		 */
		if(ds_status.touch_timeout_sec != 0){

			/* If DS_TOUCH_TIMEOUT is over */
			if(ds_counter.elapsed_sec > ds_status.touch_timeout_sec ||
				(ds_counter.elapsed_sec == ds_status.touch_timeout_sec &&
				ds_counter.elapsed_usec > ds_status.touch_timeout_usec))
			{
				switch(ds_status.flag_touch_timeout_count){
					case DS_TOUCH_TIMEOUT_COUNT_MAX:	// 7
						if(ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT < 1000000){
							ds_status.touch_timeout_sec = ds_counter.elapsed_sec;
							ds_status.touch_timeout_usec = ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT;
						}
						else{
							ds_status.touch_timeout_sec = ds_counter.elapsed_sec + 1;
							ds_status.touch_timeout_usec = 
								(ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT) - 1000000;
						}
						ds_status.flag_touch_timeout_count = 6;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_MAX;
						//lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_1;
						break;
					case 6:
						if(ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT < 1000000){
							ds_status.touch_timeout_sec = ds_counter.elapsed_sec;
							ds_status.touch_timeout_usec = ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT;
						}
						else{
							ds_status.touch_timeout_sec = ds_counter.elapsed_sec + 1;
							ds_status.touch_timeout_usec = 
								(ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT) - 1000000;
						}
						ds_status.flag_touch_timeout_count = 5;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_MAX;
						//lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_1;
						break;
					case 5:
						if(ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT < 1000000){
							ds_status.touch_timeout_sec = ds_counter.elapsed_sec;
							ds_status.touch_timeout_usec = ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT;
						}
						else{
							ds_status.touch_timeout_sec = ds_counter.elapsed_sec + 1;
							ds_status.touch_timeout_usec = 
								(ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT) - 1000000;
						}
						ds_status.flag_touch_timeout_count = 4;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_1;
						//lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_2;
						break;
					case 4:
						if(ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT < 1000000){
							ds_status.touch_timeout_sec = ds_counter.elapsed_sec;
							ds_status.touch_timeout_usec = ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT;
						}
						else{
							ds_status.touch_timeout_sec = ds_counter.elapsed_sec + 1;
							ds_status.touch_timeout_usec = 
								(ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT) - 1000000;
						}
						ds_status.flag_touch_timeout_count = 3;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_1;
						//lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_2;
						break;
					case 3:
						if(ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT < 1000000){
							ds_status.touch_timeout_sec = ds_counter.elapsed_sec;
							ds_status.touch_timeout_usec = ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT;
						}
						else{
							ds_status.touch_timeout_sec = ds_counter.elapsed_sec + 1;
							ds_status.touch_timeout_usec = 
								(ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT) - 1000000;
						}
						ds_status.flag_touch_timeout_count = 2;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_2;
						break;
					case 2:
						if(ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT < 1000000){
							ds_status.touch_timeout_sec = ds_counter.elapsed_sec;
							ds_status.touch_timeout_usec = ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT;
						}
						else{
							ds_status.touch_timeout_sec = ds_counter.elapsed_sec + 1;
							ds_status.touch_timeout_usec = 
								(ds_counter.elapsed_usec + DS_TOUCH_TIMEOUT) - 1000000;
						}
						ds_status.flag_touch_timeout_count = 1;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_2;
						//lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_MIN;
						break;
					case 1:
						ds_status.touch_timeout_sec = 0;
						ds_status.touch_timeout_usec = 0;
						ds_status.flag_touch_timeout_count = 0;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_MIN;
						break;
					default:
						ds_status.touch_timeout_sec = 0;
						ds_status.touch_timeout_usec = 0;
						ds_status.flag_touch_timeout_count = 0;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_MIN;
						break;
				}
			}
			/* If DS_TOUCH_TIMEOUT is not over yet */
			else
			{
				switch(ds_status.flag_touch_timeout_count){
					case DS_TOUCH_TIMEOUT_COUNT_MAX:	// 7
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_MAX;
						//lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_1;
						break;
					case 6:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_MAX;
						//lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_1;
						break;
					case 5:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_MAX;
						//lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_1;
						break;
					case 4:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_1;
						//lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_2;
						break;
					case 3:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_1;
						//lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_2;
						break;
					case 2:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_2;
						//lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_2;
						break;
					case 1:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_2;
						//lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_MIN;
						break;
					default:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_MIN;
						break;
				}
			}
		}
		if(lc_target_cpu_op_index_touch > lc_target_cpu_op_index_highest)
			lc_target_cpu_op_index_highest = lc_target_cpu_op_index_touch;
		/* End of the special treatment for touch events */
#endif
	}
	else
		lc_target_cpu_op_index_highest = DS_CPU_OP_INDEX_MIN;

	*target_cpu_op_index = lc_target_cpu_op_index_highest;

	return(0);
}

/*====================================================================
	The functions to initialize dvs_suite.
	====================================================================*/

/*
	 1. Initialize 
			ds_status.flag_time_base_initialized,
			ds_status.cpu_op_index,
			ds_status.cpu_op_sf,
			ds_status.cpu_op_index_nr,
			ds_status.cpu_op_mhz,
			ds_status.cpu_mode.
	 2. Initialize the variables involved with DVS schemes.
 */
int ds_initialize_dvs_suite(int cpu_mode){

	int i;
	unsigned int lc_current_cpu_op_index;

	ds_status.flag_run_dvs = 0;

	/* Reset ds_status.flag_time_base_initialized.
	 * It will be initialized later.
	 */
	ds_status.flag_time_base_initialized = 0;

	/* Initialize ds_status.cpu_op_index, ds_status.cpu_op_sf, ds_status.cpu_op_index_nr,
		 ds_status.cpu_op_mhz, ds_status.cpu_op_dirty, and ds_status.cpu_mode.
	 */
	lc_current_cpu_op_index = DS_CPU_OP_INDEX_MAX;
	ds_status.cpu_op_index = lc_current_cpu_op_index;
	ds_status.cpu_op_sf = DS_INDEX2SF(ds_status.cpu_op_index);
	ds_status.cpu_op_index_nr = DS_INDEX2NR(lc_current_cpu_op_index);
	ds_status.cpu_op_mhz = DS_INDEX2MHZPRECISE(lc_current_cpu_op_index);
	ds_status.cpu_mode = cpu_mode;

	/* Initialize ds_status's fields */
	ds_status.flag_update_cpu_op = 0;
	ds_status.target_cpu_op_index = DS_CPU_OP_INDEX_MAX;
	ds_status.cpu_op_last_update_sec = 0;
	ds_status.cpu_op_last_update_usec = 0;
	ds_status.current_dvs_scheme = 0;
	for(i=0;i<DS_PID_LIMIT;i++){
		ds_status.scheduler[i] = 0;
		ds_status.type[i] = 0;
		ds_status.type_fixed[i] = 0;
		ds_status.type_need_to_be_changed[i] = 0;
		ds_status.tgid[i] = 0;
		ds_status.tgid_type_changed[i] = 0;
		ds_status.tgid_type_change_causer[i] = 0;
		ds_status.ipc_timeout_sec[i] = 0;
		ds_status.ipc_timeout_usec[i] = 0;
	}
	ds_status.flag_touch_timeout_count = 0;
	ds_status.touch_timeout_sec = 0;
	ds_status.touch_timeout_usec = 0;

	ds_status.flag_mutex_lock_on_clock_state = 0;
	ds_status.mutex_lock_on_clock_state_cnt = 0;
	ds_status.flag_correct_cpu_op_update_path = 0;

	ds_status.flag_post_early_suspend = 0;
	ds_status.post_early_suspend_sec = 0;
	ds_status.post_early_suspend_usec = 0;
	ds_status.flag_do_post_early_suspend = 0;

	ds_status.mpu_min_freq_to_lock = 0;
	ds_status.l3_min_freq_to_lock = 0;
	ds_status.iva_min_freq_to_lock = 0;

	/* Initialize ds_counter's fields */
	ds_counter.elapsed_sec = 0;
	ds_counter.elapsed_usec = 0;
#if 0	// Not needed unless we want statistics.
	for(i=0;i<DS_CPU_OP_LIMIT;i++){
		ds_counter.idle_sec[i] = 0;
		ds_counter.idle_usec[i] = 0;
		ds_counter.busy_sec[i] = 0;
		ds_counter.busy_usec[i] = 0;
		ds_counter.busy_task_sec[i] = 0;
		ds_counter.busy_task_usec[i] = 0;
		ds_counter.busy_schedule_sec[i] = 0;
		ds_counter.busy_schedule_usec[i] = 0;
		ds_counter.busy_dvs_suite_sec[i] = 0;
		ds_counter.busy_dvs_suite_usec[i] = 0;
	}
#endif
#if 0	// Not needed unless we want statistics.
	ds_counter.idle_total_sec = 0;
	ds_counter.idle_total_usec = 0;
#endif
	ds_counter.busy_total_sec = 0;
	ds_counter.busy_total_usec = 0;
	ds_counter.busy_fse_sec = 0;
	ds_counter.busy_fse_usec = 0;
	ds_counter.busy_fse_usec_fra_fp12 = 0;
#if 0	// Not needed unless we want statistics.
	ds_counter.busy_task_total_sec = 0;
	ds_counter.busy_task_total_usec = 0;
	ds_counter.busy_task_fse_sec = 0;
	ds_counter.busy_task_fse_usec = 0;
	ds_counter.busy_task_fse_usec_fra_fp12 = 0;
	ds_counter.busy_hrt_task_sec = 0;
	ds_counter.busy_hrt_task_usec = 0;
	ds_counter.busy_hrt_task_fse_sec = 0;
	ds_counter.busy_hrt_task_fse_usec = 0;
	ds_counter.busy_hrt_task_fse_usec_fra_fp12 = 0;
	ds_counter.busy_dbsrt_task_sec = 0;
	ds_counter.busy_dbsrt_task_usec = 0;
	ds_counter.busy_dbsrt_task_fse_sec = 0;
	ds_counter.busy_dbsrt_task_fse_usec = 0;
	ds_counter.busy_dbsrt_task_fse_usec_fra_fp12 = 0;
	ds_counter.busy_rbsrt_task_sec = 0;
	ds_counter.busy_rbsrt_task_usec = 0;
	ds_counter.busy_rbsrt_task_fse_sec = 0;
	ds_counter.busy_rbsrt_task_fse_usec = 0;
	ds_counter.busy_rbsrt_task_fse_usec_fra_fp12 = 0;
	ds_counter.busy_nrt_task_sec = 0;
	ds_counter.busy_nrt_task_usec = 0;
	ds_counter.busy_nrt_task_fse_sec = 0;
	ds_counter.busy_nrt_task_fse_usec = 0;
	ds_counter.busy_nrt_task_fse_usec_fra_fp12 = 0;
	ds_counter.busy_schedule_total_sec = 0;
	ds_counter.busy_schedule_total_usec = 0;
	ds_counter.busy_schedule_fse_sec = 0;
	ds_counter.busy_schedule_fse_usec = 0;
	ds_counter.busy_schedule_fse_usec_fra_fp12 = 0;
	ds_counter.busy_dvs_suite_total_sec = 0;
	ds_counter.busy_dvs_suite_total_usec = 0;
	ds_counter.busy_dvs_suite_fse_sec = 0;
	ds_counter.busy_dvs_suite_fse_usec = 0;
	ds_counter.busy_dvs_suite_fse_usec_fra_fp12 = 0;
	ds_counter.ds_cpu_op_adjustment_no = 0;
	ds_counter.schedule_no = 0;
	ds_counter.ret_from_system_call_no = 0;
#endif

	/* Initialize each DVS scheme related variables */

	/* GPScheDVS */
	ds_configuration.gpschedvs_strategy = 0;
	ds_configuration.aidvs_interval_window_size = DS_AIDVS_INTERVALS_IN_AN_WINDOW;
	ds_configuration.aidvs_speedup_threshold = DS_AIDVS_SPEEDUP_THRESHOLD;
	ds_configuration.aidvs_speedup_interval = DS_AIDVS_SPEEDUP_INTERVAL;

	ds_gpschedvs_status.current_strategy = ds_configuration.gpschedvs_strategy;

	// aidvs_l3_status
	ds_gpschedvs_status.aidvs_l3_status.base_initialized = 0;
	ds_gpschedvs_status.aidvs_l3_status.flag_in_busy_half = 0;

	ds_gpschedvs_status.aidvs_l3_status.time_usec_interval = 0;
	ds_gpschedvs_status.aidvs_l3_status.time_usec_interval_inc_base = 0;
	ds_gpschedvs_status.aidvs_l3_status.time_sec_interval_inc_base = 0;
	ds_gpschedvs_status.aidvs_l3_status.time_usec_work_fse = 0;
	ds_gpschedvs_status.aidvs_l3_status.time_usec_work_fse_inc_base = 0;
	ds_gpschedvs_status.aidvs_l3_status.time_usec_work_fse_lasting = 0;
	ds_gpschedvs_status.aidvs_l3_status.time_usec_work = 0;
	ds_gpschedvs_status.aidvs_l3_status.time_usec_work_inc_base = 0;
	ds_gpschedvs_status.aidvs_l3_status.time_usec_work_lasting = 0;
	for(i=0;i<DS_GPSCHEDVS_L3_INTERVALS_IN_AN_WINDOW;i++){
		ds_gpschedvs_status.aidvs_l3_status.interval_window_array[i].time_usec_interval = 0;
		ds_gpschedvs_status.aidvs_l3_status.interval_window_array[i].time_usec_work_fse = 0;
		ds_gpschedvs_status.aidvs_l3_status.interval_window_array[i].time_usec_work = 0;
	}
	ds_gpschedvs_status.aidvs_l3_status.interval_window_index = 0;
	ds_gpschedvs_status.aidvs_l3_status.time_usec_interval_in_window = 0;
	ds_gpschedvs_status.aidvs_l3_status.time_usec_work_fse_in_window = 0;
	ds_gpschedvs_status.aidvs_l3_status.time_usec_work_in_window = 0;
	ds_gpschedvs_status.aidvs_l3_status.consecutive_speedup_count = 0;
	ds_gpschedvs_status.aidvs_l3_status.utilization_int_ulong = 1;	/* Begin with the max. perf. */
	ds_gpschedvs_status.aidvs_l3_status.utilization_fra_fp12 = 0;
	ds_gpschedvs_status.aidvs_l3_status.time_usec_util_calc_base = 0;
	ds_gpschedvs_status.aidvs_l3_status.time_sec_util_calc_base = 0;
	ds_gpschedvs_status.aidvs_l3_status.cpu_op_index = DS_CPU_OP_INDEX_0;

	// aidvs_l2_status
	ds_gpschedvs_status.aidvs_l2_status.base_initialized = 0;
	ds_gpschedvs_status.aidvs_l2_status.flag_in_busy_half = 0;

	ds_gpschedvs_status.aidvs_l2_status.time_usec_interval = 0;
	ds_gpschedvs_status.aidvs_l2_status.time_usec_interval_inc_base = 0;
	ds_gpschedvs_status.aidvs_l2_status.time_sec_interval_inc_base = 0;
	ds_gpschedvs_status.aidvs_l2_status.time_usec_work_fse = 0;
	ds_gpschedvs_status.aidvs_l2_status.time_usec_work_fse_inc_base = 0;
	ds_gpschedvs_status.aidvs_l2_status.time_usec_work_fse_lasting = 0;
	ds_gpschedvs_status.aidvs_l2_status.time_usec_work = 0;
	ds_gpschedvs_status.aidvs_l2_status.time_usec_work_inc_base = 0;
	ds_gpschedvs_status.aidvs_l2_status.time_usec_work_lasting = 0;
	for(i=0;i<DS_GPSCHEDVS_L3_INTERVALS_IN_AN_WINDOW;i++){
		ds_gpschedvs_status.aidvs_l2_status.interval_window_array[i].time_usec_interval = 0;
		ds_gpschedvs_status.aidvs_l2_status.interval_window_array[i].time_usec_work_fse = 0;
		ds_gpschedvs_status.aidvs_l2_status.interval_window_array[i].time_usec_work = 0;
	}
	ds_gpschedvs_status.aidvs_l2_status.interval_window_index = 0;
	ds_gpschedvs_status.aidvs_l2_status.time_usec_interval_in_window = 0;
	ds_gpschedvs_status.aidvs_l2_status.time_usec_work_fse_in_window = 0;
	ds_gpschedvs_status.aidvs_l2_status.time_usec_work_in_window = 0;
	ds_gpschedvs_status.aidvs_l2_status.consecutive_speedup_count = 0;
	ds_gpschedvs_status.aidvs_l2_status.utilization_int_ulong = 1;	/* Begin with the max. perf. */
	ds_gpschedvs_status.aidvs_l2_status.utilization_fra_fp12 = 0;
	ds_gpschedvs_status.aidvs_l2_status.time_usec_util_calc_base = 0;
	ds_gpschedvs_status.aidvs_l2_status.time_sec_util_calc_base = 0;
	ds_gpschedvs_status.aidvs_l2_status.cpu_op_index = DS_CPU_OP_INDEX_0;

	// aidvs_l1_status
	ds_gpschedvs_status.aidvs_l1_status.base_initialized = 0;
	ds_gpschedvs_status.aidvs_l1_status.flag_in_busy_half = 0;

	ds_gpschedvs_status.aidvs_l1_status.time_usec_interval = 0;
	ds_gpschedvs_status.aidvs_l1_status.time_usec_interval_inc_base = 0;
	ds_gpschedvs_status.aidvs_l1_status.time_sec_interval_inc_base = 0;
	ds_gpschedvs_status.aidvs_l1_status.time_usec_work_fse = 0;
	ds_gpschedvs_status.aidvs_l1_status.time_usec_work_fse_inc_base = 0;
	ds_gpschedvs_status.aidvs_l1_status.time_usec_work_fse_lasting = 0;
	ds_gpschedvs_status.aidvs_l1_status.time_usec_work = 0;
	ds_gpschedvs_status.aidvs_l1_status.time_usec_work_inc_base = 0;
	ds_gpschedvs_status.aidvs_l1_status.time_usec_work_lasting = 0;
	for(i=0;i<DS_GPSCHEDVS_L3_INTERVALS_IN_AN_WINDOW;i++){
		ds_gpschedvs_status.aidvs_l1_status.interval_window_array[i].time_usec_interval = 0;
		ds_gpschedvs_status.aidvs_l1_status.interval_window_array[i].time_usec_work_fse = 0;
		ds_gpschedvs_status.aidvs_l1_status.interval_window_array[i].time_usec_work = 0;
	}
	ds_gpschedvs_status.aidvs_l1_status.interval_window_index = 0;
	ds_gpschedvs_status.aidvs_l1_status.time_usec_interval_in_window = 0;
	ds_gpschedvs_status.aidvs_l1_status.time_usec_work_fse_in_window = 0;
	ds_gpschedvs_status.aidvs_l1_status.time_usec_work_in_window = 0;
	ds_gpschedvs_status.aidvs_l1_status.consecutive_speedup_count = 0;
	ds_gpschedvs_status.aidvs_l1_status.utilization_int_ulong = 1;	/* Begin with the max. perf. */
	ds_gpschedvs_status.aidvs_l1_status.utilization_fra_fp12 = 0;
	ds_gpschedvs_status.aidvs_l1_status.time_usec_util_calc_base = 0;
	ds_gpschedvs_status.aidvs_l1_status.time_sec_util_calc_base = 0;
	ds_gpschedvs_status.aidvs_l1_status.cpu_op_index = DS_CPU_OP_INDEX_0;

	// aidvs_l0_status
	ds_gpschedvs_status.aidvs_l0_status.base_initialized = 0;
	ds_gpschedvs_status.aidvs_l0_status.flag_in_busy_half = 0;

	ds_gpschedvs_status.aidvs_l0_status.time_usec_interval = 0;
	ds_gpschedvs_status.aidvs_l0_status.time_usec_interval_inc_base = 0;
	ds_gpschedvs_status.aidvs_l0_status.time_sec_interval_inc_base = 0;
	ds_gpschedvs_status.aidvs_l0_status.time_usec_work_fse = 0;
	ds_gpschedvs_status.aidvs_l0_status.time_usec_work_fse_inc_base = 0;
	ds_gpschedvs_status.aidvs_l0_status.time_usec_work_fse_lasting = 0;
	ds_gpschedvs_status.aidvs_l0_status.time_usec_work = 0;
	ds_gpschedvs_status.aidvs_l0_status.time_usec_work_inc_base = 0;
	ds_gpschedvs_status.aidvs_l0_status.time_usec_work_lasting = 0;
	for(i=0;i<DS_GPSCHEDVS_L3_INTERVALS_IN_AN_WINDOW;i++){
		ds_gpschedvs_status.aidvs_l0_status.interval_window_array[i].time_usec_interval = 0;
		ds_gpschedvs_status.aidvs_l0_status.interval_window_array[i].time_usec_work_fse = 0;
		ds_gpschedvs_status.aidvs_l0_status.interval_window_array[i].time_usec_work = 0;
	}
	ds_gpschedvs_status.aidvs_l0_status.interval_window_index = 0;
	ds_gpschedvs_status.aidvs_l0_status.time_usec_interval_in_window = 0;
	ds_gpschedvs_status.aidvs_l0_status.time_usec_work_fse_in_window = 0;
	ds_gpschedvs_status.aidvs_l0_status.time_usec_work_in_window = 0;
	ds_gpschedvs_status.aidvs_l0_status.consecutive_speedup_count = 0;
	ds_gpschedvs_status.aidvs_l0_status.utilization_int_ulong = 1;	/* Begin with the max. perf. */
	ds_gpschedvs_status.aidvs_l0_status.utilization_fra_fp12 = 0;
	ds_gpschedvs_status.aidvs_l0_status.time_usec_util_calc_base = 0;
	ds_gpschedvs_status.aidvs_l0_status.time_sec_util_calc_base = 0;
	ds_gpschedvs_status.aidvs_l0_status.cpu_op_index = DS_CPU_OP_INDEX_0;

	ds_status.ds_initialized = 1;

	return(0);
}

/*====================================================================
	Function to change the priority of normal tasks
	NOTE: We apply the RR rt scheduler for HRT and DS_SRT_UI_SERVER_TASK.
		  On the other hand, we apply the normal scheduler for
		  other tasks.
	====================================================================*/
int ds_update_priority_normal(struct task_struct *p){

	int lc_existing_nice = 0;
	int lc_nice_by_type = 0;
	//int lc_static_prio_by_type = 0;
	int lc_resultant_nice = 0;
	//int lc_resultant_static_prio = 0;

	if(ds_status.type_need_to_be_changed[p->pid] == 0) return(0);

	if(p == 0 || p->pid == 0) return(0);

	lc_existing_nice = p->static_prio - 120;

	switch(ds_status.type[p->pid]){
		case DS_HRT_TASK:
			lc_nice_by_type = DS_HRT_NICE;
			//lc_static_prio_by_type = DS_HRT_STATIC_PRIO;
			break;
		case DS_SRT_UI_SERVER_TASK:
			lc_nice_by_type = DS_DBSRT_NICE;
			//lc_static_prio_by_type = DS_DBSRT_STATIC_PRIO;
			break;
		case DS_SRT_UI_CLIENT_TASK:
			lc_nice_by_type = DS_DBSRT_NICE;
			//lc_static_prio_by_type = DS_DBSRT_STATIC_PRIO;
			break;
		case DS_SRT_KERNEL_THREAD:
			lc_nice_by_type = DS_RBSRT_NICE;
			//lc_static_prio_by_type = DS_RBSRT_STATIC_PRIO;
			break;
		case DS_SRT_DAEMON_TASK:
			lc_nice_by_type = DS_RBSRT_NICE;
			//lc_static_prio_by_type = DS_RBSRT_STATIC_PRIO;
			break;
		case DS_NRT_TASK:
			lc_nice_by_type = DS_NRT_NICE;
			//lc_static_prio_by_type = DS_NRT_STATIC_PRIO;
			break;
		case 0:
			lc_nice_by_type = DS_NRT_NICE;
			//lc_static_prio_by_type = DS_NRT_STATIC_PRIO;
			break;
	}

	lc_resultant_nice = lc_nice_by_type + lc_existing_nice;
	if(lc_resultant_nice < -20) lc_resultant_nice = -20;
	if(lc_resultant_nice > 19) lc_resultant_nice = 19;

	set_user_nice(p, lc_resultant_nice);

#if 0
	lc_resultant_static_prio = lc_static_prio_by_type + lc_existing_nice;
	if(lc_resultant_static_prio < 100) lc_resultant_static_prio = 100;
	if(lc_resultant_static_prio > 139) lc_resultant_static_prio = 139;

	p->static_prio = lc_resultant_static_prio;
	p->normal_prio = p->static_prio;
	p->prio = p->static_prio;
	p->se.load.weight = ds_prio_to_weight[lc_resultant_nice+20];
	p->se.load.inv_weight = ds_prio_to_wmult[lc_resultant_nice+20];
#endif

	ds_status.type_need_to_be_changed[p->pid] = 0;

	return(0);
}

/*====================================================================
	Function to change the priority of real-time tasks
	NOTE: We apply the RR rt scheduler for HRT and DS_SRT_UI_SERVER_TASK.
		  On the other hand, we apply the normal scheduler for
		  other tasks.
	====================================================================*/
int ds_update_priority_rt(struct task_struct *p){

	struct sched_param lc_sched_param;

	if(ds_status.type_need_to_be_changed[p->pid] == 0) return(0);

	if(p == 0 || p->pid == 0) return(0);

	switch(ds_status.type[p->pid]){
		case DS_HRT_TASK:
			ds_status.scheduler[p->pid] = DS_SCHED_RR;
			lc_sched_param.sched_priority = DS_HRT_RR_PRIO;
			sched_setscheduler(p, SCHED_RR, &lc_sched_param);
			break;
		case DS_SRT_UI_SERVER_TASK:
			ds_status.scheduler[p->pid] = DS_SCHED_RR;
			lc_sched_param.sched_priority = DS_DBSRT_RR_PRIO;
			sched_setscheduler(p, SCHED_RR, &lc_sched_param);
			break;
		case DS_SRT_UI_CLIENT_TASK:
			//ds_status.scheduler[p->pid] = DS_SCHED_RR;
			//lc_sched_param.sched_priority = DS_DBSRT_RR_PRIO;
			//sched_setscheduler(p, SCHED_RR, &lc_sched_param);
			break;
		case DS_SRT_KERNEL_THREAD:
		case DS_SRT_DAEMON_TASK:
			//ds_status.scheduler[p->pid] = DS_SCHED_NORMAL;
			//lc_sched_param.sched_priority = 0;
			//sched_setscheduler(p, SCHED_NORMAL, &lc_sched_param);
			break;
		case DS_NRT_TASK:
		case 0:
			//ds_status.scheduler[p->pid] = DS_SCHED_NORMAL;
			//lc_sched_param.sched_priority = 0;
			//sched_setscheduler(p, SCHED_NORMAL, &lc_sched_param);
			break;
	}

	ds_status.type_need_to_be_changed[p->pid] = 0;

	return(0);
}

/*====================================================================
	Function to trace every task's type
	====================================================================*/
int ds_detect_task_type(void){

	int old_type = 0;
	int new_type = 0;
	int i = 0;

	struct sched_param lc_sched_param;

	/* Upon ds_parameter.entry_type == DS_ENTRY_SWITCH_TO, do followings.

		 For ds_parameter.next_p:

		(1) Check if ds_status.type[ds_parameter.next_p->pid] is still its initial value.
			If it is, determine it.

		(2) Check if the UI timeout of ds_parameter.next_p has been expired.
			If so, reset ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] and
			ds_status.ipc_timeout_usec[ds_parameter.next_p->pid].
			Then, re-determine the type of this task.
	 */
	if(ds_parameter.entry_type == DS_ENTRY_SWITCH_TO){

		/* For prev_p */
		if(ds_parameter.prev_p != 0){
			if(ds_parameter.prev_p->pid != 0){
				// Nothing to do now.
			}
		}

		/* For next_p */
		if(ds_parameter.next_p != 0){

			if(ds_parameter.next_p->pid != 0){

				if(ds_parameter.next_p->pid == ds_parameter.next_p->tgid){
					for(i=0;i<16;i++)
						ds_status.tg_owner_comm[ds_parameter.next_p->pid][i] = 
						*(ds_parameter.next_p->comm+i);
				}

				ds_status.tgid[ds_parameter.next_p->pid] = ds_parameter.next_p->tgid;

				/* Get old type. */
				old_type = ds_status.type[ds_parameter.next_p->pid];
				if(old_type == 0) old_type = DS_NRT_TASK;

				/* C) Initialize and then determine new type. */
				new_type = old_type;

				switch(*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+0)){
					case 'a':
#if 0	// audio out. Audio related [daemon process].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'u' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == ' ' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'u' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 't'
						)
						{
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// adbd. ADB related [daemon process].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'b' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'd'
						)
						{
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'b':
#if 0	// binder. [kernel thread].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'n' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'r'
						)
						{
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 1	// btld. bluetooth reltated daemon-like [application].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'l' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'd'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 1	// bluetoothd. Bluetooth reltated [daemon process]. HRT.
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'l' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'u' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'h' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'd'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'c':
#if 0	// com.lge.osp. LGE On-Screen Phone [application]. O 4 touch.
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'o' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'm' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == '.' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'l' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'g' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'e' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == '.' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'o' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 's' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'p'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else 
#endif
#if 0	// com.lge.media. [application].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'o' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'm' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == '.' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'l' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'g' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'e' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == '.' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'm' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'e' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'a'
						)
						{
							new_type = DS_SRT_UI_CLIENT_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else 
#endif
#if 0	// com.android.mms. [application].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'n' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 's'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.broadcom.bt.app.pbap. bluetooth. [application].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'b' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'p' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'p' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'p' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'b' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 'p'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'd':
#if 0	// dbus-daemon. bluetooth related [daemon process].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'b' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'u' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 's' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == '-' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'd' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'a' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'e' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'm' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'o' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'n'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 1	// com.android.lgecamera. [application].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'r' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'o' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'i' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'd' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == '.' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'l' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'g' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'e' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'c' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 'a'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// android.process.media. [application].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == '.' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'p' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'r' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'o' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'c' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'e' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 's' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 's' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == '.' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 'a'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.android.bluetooth. [application].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'r' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'o' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'i' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'd' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == '.' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'b' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'l' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'u' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'e' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 'h'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// android.process.acore. Contact, home, and etc. [application].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == '.' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'p' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'r' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'o' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'c' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'e' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 's' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 's' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == '.' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'c' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 'e'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'e':
#if 0	// events/0. [kernel thread].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'v' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'e' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'n' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 't' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == '/'
						)
						{
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.google.process.gapps. [application]. Google services.
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == '.' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'p' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'r' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'o' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'c' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'e' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 's' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 's' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == '.' && 
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'g' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'p' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'p' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 's'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'f':
#if 1	// fw3a_core. camera related [daemon process].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'w' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == '3' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == '_' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'c' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'e'
						)
						{
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'g':
#if 0	// glgps. GPS server [daemon process]. < HRT.
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'l' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'g' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'p' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 's'
						)
						{
							//new_type = DS_HRT_TASK;
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'h':
						break;
					case 'i':
						break;
					case 'j':
#if 0	// jbd2/mmcblk0p*-. [kernel thread].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'b' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == '2' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == '/' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'c' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'b' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'l' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'k'
						)
						{
							new_type = DS_HRT_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'k':
#if 0	// ksoftirqd/0. [kernel thread].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'f' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'q' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'd'
						)
						{
							new_type = DS_SRT_KERNEL_THREAD;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// kmmcd. [kernel thread].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'c' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'd'
						)
						{
							new_type = DS_HRT_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// keystore. Key related [daemon process].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'y' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'e'
						)
						{
							//new_type = DS_HRT_TASK;
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'l':
						break;
					case 'm':
#if 0	// mmcqd. [kernel thread].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'c' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'q' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'd'
						)
						{
							new_type = DS_HRT_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 1	// mediaserver. Android system service [daemon process]. O 4 touch.
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'v' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'r'
						)
						{
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.android.phone. [application].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'n' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'p' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'h' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'n' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 'e'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.android.music. [application].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'n' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'u' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 'c'
						)
						{
							//new_type = DS_SRT_DAEMON_TASK;
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.android.email. [application]. HRT.
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'n' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 'l'
						)
						{
							new_type = DS_HRT_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.broadcom.bt.app.system. bluetooth. [application].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'b' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'p' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'p' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'y' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 'm'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'n':
#if 0	// netd. Network related [daemon process].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'd'
						)
						{
							new_type = DS_HRT_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.android.systemui. [application].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'y' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'u' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 'i'
						)
						{
							new_type = DS_SRT_UI_CLIENT_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.android.settings. [application] X.
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'n' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'g' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 's'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'o':
						break;
					case 'p':
#if 0	// pvr_*. [kernel thread].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'v' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == '_'
						)
						{
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'q':
						break;
					case 'r':
#if 0	// rild. RIL related [daemon process]. < HRT.
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'l' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'd'
						)
						{
							//new_type = DS_HRT_TASK;
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 's':
#if 0	// servicemanager. Android system service [daemon process]
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'v' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'c' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'n' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'g' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'r'
						)
						{
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// system_server. Android system service [application].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'y' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == '_' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'v' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'r'
						)
						{
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 't':
						break;
					case 'u':
#if 0	// ueventd. Polling server [daemon process].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'v' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'n' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'd'
						)
						{
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'v':
#if 0	// vold. Volume (sdcard etc.) server [daemon process].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'l' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'd'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'w':
#if 0	// wpa_supplicant. Wifi related [daemon process]. HRT.
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'p' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == '_' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'u' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'p' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'p' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'l' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'c' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'n' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 't'
						)
						{
							new_type = DS_HRT_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'x':
						break;
					case 'y':
						break;
					case 'z':
#if 0	// zygote. Android process spawning [daemon process].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'y' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'g' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'e'
						)
						{
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'A':
						break;
					case 'B':
						break;
					case 'C':
#if 0	// Camera* threads of mediaserver. camera. [daemon process].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'a'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'D':
						break;
					case 'E':
						break;
					case 'F':
						break;
					case 'G':
						break;
					case 'H':
						break;
					case 'I':
#if 0	// InputDeviceRead. Android input device reader process. [daemon process].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'n' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'p' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'u' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'D' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'v' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'c' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'R' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 'd'
						)
						{
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'J':
						break;
					case 'K':
						break;
					case 'L':
						break;
					case 'M':
#if 0	// Mixer Thread. [daemon process].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'x' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == ' ' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'T' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'h' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'd'
						)
						{
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'N':
						break;
					case 'O':
#if 0	// OMAPVOUT. [kernel thread].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'M' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'A' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'P' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'V' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'O' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'U' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'T'
						)
						{
							new_type = DS_SRT_UI_SERVER_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'P':
						break;
					case 'Q':
						break;
					case 'R':
						break;
					case 'S':
						break;
					case 'T':
						break;
					case 'U':
						break;
					case 'V':
						break;
					case 'W':
						break;
					case 'X':
						break;
					case 'Y':
						break;
					case 'Z':
						break;
					case ' ':
						break;
					case '_':
						break;
					case '/':
						break;
					case '.':
#if 0	// android.process.lghome. [application]. O 4 touch.
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'p' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'c' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'l' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'g' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'h' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 'e'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.lge.cameratest. [application].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'l' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'g' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'c' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'a' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 't' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 't'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.cooliris.media. Gallary related. [application].
						if(
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+1) == 'c' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+2) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+3) == 'o' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+4) == 'l' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+5) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+6) == 'r' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+7) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+8) == 's' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+9) == '.' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+10) == 'm' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+11) == 'e' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+12) == 'd' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+13) == 'i' &&
							*(ds_status.tg_owner_comm[ds_parameter.next_p->pid]+14) == 'a'
						)
						{
							new_type = DS_SRT_DAEMON_TASK;
							ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
							ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;
							ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						//goto DS_UNKNOWN_TASK_NAME;
						break;
				}

DS_UNKNOWN_TASK_NAME:

#if 0
				/* If this task was UI client */
				if(old_type == DS_SRT_UI_CLIENT_TASK)
				{
					/* If DS_SRT_UI_IPC_TIMEOUT is over */
					if(ds_counter.elapsed_sec > ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] ||
						(ds_counter.elapsed_sec == ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] &&
						ds_counter.elapsed_usec > ds_status.ipc_timeout_usec[ds_parameter.next_p->pid]))
					{
						ds_status.ipc_timeout_sec[ds_parameter.next_p->pid] = 0;
						ds_status.ipc_timeout_usec[ds_parameter.next_p->pid] = 0;

						//if(ds_parameter.next_p->parent->pid == 1){
						//	new_type = DS_SRT_DAEMON_TASK;
						//}
						//else{
							new_type = DS_NRT_TASK;
//printk(KERN_WARNING "%16s = DS_NRT_TASK\n", ds_parameter.next_p->comm);
						//}
					}
					/* If DS_SRT_UI_IPC_TIMEOUT is not over yet */
					else{
						new_type = DS_SRT_UI_CLIENT_TASK;
//printk(KERN_WARNING "%16s = DS_SRT_UI_CLIENT_TASK\n", ds_parameter.next_p->comm);
					}
				}
#if 0
				else{
					if(ds_parameter.next_p->pid == 1 ||
						ds_parameter.next_p->pid == 2 ||
						ds_parameter.next_p->parent->pid == 2)
					{
						new_type = DS_SRT_KERNEL_THREAD;
						ds_status.type_fixed[ds_parameter.next_p->pid] = 1;
						goto DS_TASK_TYPE_DETECTION_DONE;
					}
					else if(ds_parameter.next_p->parent->pid == 1){
						new_type = DS_SRT_DAEMON_TASK;
					}
					else{
						new_type = DS_NRT_TASK;
					}
				}
#endif
#endif
new_type = DS_NRT_TASK;

DS_TASK_TYPE_DETECTION_DONE:

				/* D) Thread group consideration */

#if 0
				/* Skip for thread group leader or 
					if the leader's type has not been determined yet. */
				if(ds_parameter.next_p->pid != ds_status.tgid[ds_parameter.next_p->pid])
				{
					/* If thread group leader's type is different from next_p's */
					if(new_type != ds_status.type[ds_status.tgid[ds_parameter.next_p->pid]])
					{
						/* If thread group leader's type is higher than next_p's */
						if(new_type > ds_status.type[ds_status.tgid[ds_parameter.next_p->pid]])
						{
							/* If thread group leader's type change causer is next_p,
								decrease the leader's type down to next_p's.
								Then, reset the leader's type change causer pid. */
							if(ds_status.tgid_type_change_causer[ds_status.tgid[ds_parameter.next_p->pid]] == 
								ds_parameter.next_p->pid)
							{
								switch(new_type){
									case DS_HRT_TASK:
										ds_status.type[ds_status.tgid[ds_parameter.next_p->pid]] = 
											DS_HRT_TASK;
										break;
									case DS_SRT_UI_SERVER_TASK:
										ds_status.type[ds_status.tgid[ds_parameter.next_p->pid]] = 
											DS_SRT_UI_SERVER_TASK;
										break;
									case DS_SRT_UI_CLIENT_TASK:
										ds_status.type[ds_status.tgid[ds_parameter.next_p->pid]] = 
											DS_SRT_UI_CLIENT_TASK;
										break;
									case DS_SRT_KERNEL_THREAD:
										ds_status.type[ds_status.tgid[ds_parameter.next_p->pid]] = 
											DS_SRT_KERNEL_THREAD;
										break;
									case DS_SRT_DAEMON_TASK:
										ds_status.type[ds_status.tgid[ds_parameter.next_p->pid]] = 
											DS_SRT_DAEMON_TASK;
										break;
									case DS_NRT_TASK:
									default:
										ds_status.type[ds_status.tgid[ds_parameter.next_p->pid]] = 
											DS_NRT_TASK;
										break;
								}
								ds_status.tgid_type_changed[ds_status.tgid[ds_parameter.next_p->pid]] = 1;
								ds_status.tgid_type_change_causer[ds_status.tgid[ds_parameter.next_p->pid]] = 0;
							}
							/* If thread group leader's type change causer is 
								either the leader itself or another task,
								increase next_p's type up to the leader's.
								The leader's type change causer pid should be remained. */
							else
							{
								switch(ds_status.type[ds_status.tgid[ds_parameter.next_p->pid]]){
									case DS_HRT_TASK:
										new_type = DS_HRT_TASK;
										break;
									case DS_SRT_UI_SERVER_TASK:
										new_type = DS_SRT_UI_SERVER_TASK;
										break;
									case DS_SRT_UI_CLIENT_TASK:
										new_type = DS_SRT_UI_CLIENT_TASK;
										break;
									case DS_SRT_KERNEL_THREAD:
										new_type = DS_SRT_KERNEL_THREAD;
										break;
									case DS_SRT_DAEMON_TASK:
										new_type = DS_SRT_DAEMON_TASK;
										break;
									case DS_NRT_TASK:
									default:
										new_type = DS_NRT_TASK;
										break;
								}
							}
						}
						/* If next_p's type is higher than thread group leader's */
						else
						{
							/* In this case, unconditionally increase the leader's type up to the next_p's */
							switch(new_type){
								case DS_HRT_TASK:
									ds_status.type[ds_status.tgid[ds_parameter.next_p->pid]] = 
											DS_HRT_TASK;
									break;
								case DS_SRT_UI_SERVER_TASK:
									ds_status.type[ds_status.tgid[ds_parameter.next_p->pid]] = 
											DS_SRT_UI_SERVER_TASK;
									break;
								case DS_SRT_UI_CLIENT_TASK:
									ds_status.type[ds_status.tgid[ds_parameter.next_p->pid]] = 
											DS_SRT_UI_CLIENT_TASK;
									break;
								case DS_SRT_KERNEL_THREAD:
									ds_status.type[ds_status.tgid[ds_parameter.next_p->pid]] = 
											DS_SRT_KERNEL_THREAD;
									break;
								case DS_SRT_DAEMON_TASK:
									ds_status.type[ds_status.tgid[ds_parameter.next_p->pid]] = 
											DS_SRT_DAEMON_TASK;
									break;
								case DS_NRT_TASK:
								default:
									ds_status.type[ds_status.tgid[ds_parameter.next_p->pid]] = 
											DS_NRT_TASK;
									break;
							}
							ds_status.tgid_type_changed[ds_status.tgid[ds_parameter.next_p->pid]] = 1;
							ds_status.tgid_type_change_causer[ds_status.tgid[ds_parameter.next_p->pid]] = 
								ds_parameter.next_p->pid;
						}
					}
				}
#endif

				/* E) Check type change. */
				if(old_type != new_type){
					ds_status.type[ds_parameter.next_p->pid] = new_type;
					ds_status.type_need_to_be_changed[ds_parameter.next_p->pid] = 1;
				}
#if 0
				else if(ds_status.tgid_type_changed[ds_parameter.next_p->pid] == 1){
					ds_status.type_need_to_be_changed[ds_parameter.next_p->pid] = 1;
					ds_status.tgid_type_changed[ds_parameter.next_p->pid] = 0;
				}
#endif
			}
		}
	}

	return(0);
}

/*====================================================================
	The function which updates the CPU operating point.
	====================================================================*/
asmlinkage void ds_update_cpu_op(void){

	struct device *mpu_dev;	// Moved from cpu-omap.c
	struct device *l3_dev;	// Moved from cpu-omap.c
	struct device *iva_dev; // Moved from cpu-omap.c

	unsigned long lc_min_cpu_op_update_interval = 0;

	if(ds_status.flag_run_dvs == 1){

		if(ds_status.flag_update_cpu_op == 1){

			if(ds_status.target_cpu_op_index > ds_status.cpu_op_index)
				lc_min_cpu_op_update_interval = DS_MIN_CPU_OP_UPDATE_INTERVAL_U;
			else
				lc_min_cpu_op_update_interval = DS_MIN_CPU_OP_UPDATE_INTERVAL_D;

			/* If this is the first time */
			if(ds_status.cpu_op_last_update_sec == 0 && ds_status.cpu_op_last_update_usec == 0){
				goto update_cpu_op;
			}
			else{
				/* If lc_min_cpu_op_update_interval is over since the last update */
				if(ds_counter.elapsed_sec > ds_status.cpu_op_last_update_sec + 1){
					goto update_cpu_op;
				}
				else if(ds_counter.elapsed_sec > ds_status.cpu_op_last_update_sec){
					if(ds_counter.elapsed_usec > ds_status.cpu_op_last_update_usec){
						goto update_cpu_op;
					}
					else if(ds_counter.elapsed_usec + 1000000
						> ds_status.cpu_op_last_update_usec + lc_min_cpu_op_update_interval){
						goto update_cpu_op;
					}
					else
						goto do_not_update;
				}
				else{
					if(ds_counter.elapsed_usec
						> ds_status.cpu_op_last_update_usec + lc_min_cpu_op_update_interval){
						goto update_cpu_op;
					}
					else
						goto do_not_update;
				}
			}
		}
		else
			goto do_not_update;
	}

update_cpu_op:

	if(ds_status.flag_run_dvs == 1){
		ds_update_time_counter();

		ds_status.flag_correct_cpu_op_update_path = 1;

		/* A treatment to integrate DSP 800MHz patch into LG-DVFS */
		if(ds_status.mpu_min_freq_to_lock != 0){
			if(ds_status.target_cpu_op_index < ds_status.mpu_min_freq_to_lock)
				ds_status.target_cpu_op_index = ds_status.mpu_min_freq_to_lock;
			//printk(KERN_WARNING "mpu locked %lu\n", ds_status.mpu_min_freq_to_lock);
		}

		mpu_dev = omap2_get_mpuss_device();
		l3_dev = omap2_get_l3_device();
		iva_dev = omap2_get_iva_device();

		switch(ds_status.target_cpu_op_index){
			case DS_CPU_OP_INDEX_0:
				omap_pm_cpu_set_freq(1000000000);	// VDD1_OPP4	1GHz
				omap_device_set_rate(mpu_dev, mpu_dev, 1000000000);
				omap_device_set_rate(iva_dev, iva_dev, 800000000);
				/* A treatment to integrate DSP 800MHz patch into LG-DVFS */
				if(ds_status.l3_min_freq_to_lock == 0){
					if(ds_status.flag_do_post_early_suspend == 0)
						omap_device_set_rate(l3_dev, l3_dev, 200000000);
					else
						omap_device_set_rate(l3_dev, l3_dev, 100000000);
				}
				else{
					omap_device_set_rate(l3_dev, l3_dev, ds_status.l3_min_freq_to_lock);
//printk(KERN_WARNING "l3 locked %lu\n", ds_status.l3_min_freq_to_lock);
				}

				break;
			case DS_CPU_OP_INDEX_1:
				omap_pm_cpu_set_freq(800000000);	// VDD1_OPP3	800MHz
				omap_device_set_rate(mpu_dev, mpu_dev, 800000000);
				omap_device_set_rate(iva_dev, iva_dev, 660000000);
				/* A treatment to integrate DSP 800MHz patch into LG-DVFS */
				if(ds_status.l3_min_freq_to_lock == 0){
					if(ds_status.flag_do_post_early_suspend == 0)
						omap_device_set_rate(l3_dev, l3_dev, 200000000);
					else
						omap_device_set_rate(l3_dev, l3_dev, 100000000);
				}
				else{
					omap_device_set_rate(l3_dev, l3_dev, ds_status.l3_min_freq_to_lock);
//printk(KERN_WARNING "l3 locked %lu\n", ds_status.l3_min_freq_to_lock);
				}

				break;
			case DS_CPU_OP_INDEX_2:
				omap_pm_cpu_set_freq(600000000);	// VDD1_OPP2	600MHz
				omap_device_set_rate(mpu_dev, mpu_dev, 600000000);
				omap_device_set_rate(iva_dev, iva_dev, 520000000);
				/* A treatment to integrate DSP 800MHz patch into LG-DVFS */
				if(ds_status.l3_min_freq_to_lock == 0){
					if(ds_status.flag_do_post_early_suspend == 0)
						omap_device_set_rate(l3_dev, l3_dev, 200000000);
					else
						omap_device_set_rate(l3_dev, l3_dev, 100000000);
				}
				else{
					omap_device_set_rate(l3_dev, l3_dev, ds_status.l3_min_freq_to_lock);
//printk(KERN_WARNING "l3 locked %lu\n", ds_status.l3_min_freq_to_lock);
				}

				break;
			case DS_CPU_OP_INDEX_3:
			default:
				omap_pm_cpu_set_freq(300000000);	// VDD1_OPP1	300MHz
				omap_device_set_rate(mpu_dev, mpu_dev, 300000000);
				omap_device_set_rate(iva_dev, iva_dev, 260000000);
				/* A treatment to integrate DSP 800MHz patch into LG-DVFS */
				if(ds_status.l3_min_freq_to_lock == 0){
					if(ds_status.flag_do_post_early_suspend == 0)
						omap_device_set_rate(l3_dev, l3_dev, 200000000);
					else
						omap_device_set_rate(l3_dev, l3_dev, 100000000);
				}
				else{
					omap_device_set_rate(l3_dev, l3_dev, ds_status.l3_min_freq_to_lock);
//printk(KERN_WARNING "l3 locked %lu\n", ds_status.l3_min_freq_to_lock);
				}

				break;
		}

		ds_status.flag_correct_cpu_op_update_path = 0;
		ds_status.cpu_op_index = ds_status.target_cpu_op_index;
		ds_status.cpu_op_last_update_sec = ds_counter.elapsed_sec;
		ds_status.cpu_op_last_update_usec = ds_counter.elapsed_usec;
		ds_status.flag_update_cpu_op = 0;
	}

do_not_update:

	return;
}
EXPORT_SYMBOL(ds_update_cpu_op);

/*====================================================================
	The main dynamic voltage scaling and
	performance evaluation kernel function.
	====================================================================*/

/* This function is called at the end of context_swtich().
 */
void do_dvs_suite(void){

	unsigned int lc_target_cpu_op_index = 0;

	/* Update ds_counter and ds_status.cpu_mode.
	 */
#if 0
	ds_update_time_counter();
	ds_status.cpu_mode = DS_CPU_MODE_DVS_SUITE;
#endif

#if 0	// WARNING: This code will cause a severe performance degradation!!!
printk(KERN_WARNING "t%d %5d %5d %16s p%3d s%3d\n", 
ds_status.type[ds_parameter.next_p->pid],
ds_parameter.next_p->pid,
ds_parameter.next_p->tgid,
ds_parameter.next_p->comm,
ds_parameter.next_p->prio,
ds_parameter.next_p->static_prio
);
#endif

	/* Trace task type.
	 */
	if(ds_status.flag_run_dvs == 1)
		if(ds_status.type_fixed[ds_parameter.next_p->pid] == 0)
			ds_detect_task_type();

	/* If ds_status.flag_run_dvs == 1. This is set in binder.c */
	if(ds_status.flag_run_dvs == 1){

		ds_status.current_dvs_scheme = ds_configuration.dvs_scheme;

		/* Perform the selected Dynamic Voltage Scaling Scheme to adjust CPU_OP.
		 */
		switch(ds_configuration.dvs_scheme){
			case DS_DVS_NO_DVS:
				lc_target_cpu_op_index = DS_CPU_OP_INDEX_MAX;
				break;
			case DS_DVS_MIN:
				lc_target_cpu_op_index = DS_CPU_OP_INDEX_MIN;
				break;
			case DS_DVS_GPSCHEDVS:
				ds_do_dvs_gpschedvs(&lc_target_cpu_op_index);
				break;
			case DS_DVS_MANUAL:
				lc_target_cpu_op_index = ds_status.cpu_op_index;
				break;
		}

		if(lc_target_cpu_op_index != ds_status.cpu_op_index){
			ds_status.flag_update_cpu_op = 1;
			ds_status.target_cpu_op_index = lc_target_cpu_op_index;
		}
	}

	/* Initialize some instant information fields in ds_status.
	 */

	/* Update ds_counter.ret_from_system_call_no and ds_status.cpu_mode.
	 */
#if 0
	ds_update_time_counter();
	if(ds_parameter.entry_type == DS_ENTRY_RET_FROM_SYSTEM_CALL){
		//ds_counter.ret_from_system_call_no ++;	// Not needed unless we want statistics.
		/* If current is the idle process, i.e. the swapper.
			 During the booting time, the PID of 0 is not necessarily for swapper.
			 So, we check name rather than PID. */
		if(current->pid == 0){
			ds_status.cpu_mode = DS_CPU_MODE_IDLE;
		}
		else{
			ds_status.cpu_mode = DS_CPU_MODE_TASK;
		}
	}
	else{
		ds_status.cpu_mode = DS_CPU_MODE_SCHEDULE;
	}
#endif

	/* In provision for calling do_dvs_suite() at ENTRY(ret_from_system_call),
		 initialize ds_parameter.
	 */
	ds_parameter.entry_type = DS_ENTRY_RET_FROM_SYSTEM_CALL;
	ds_parameter.prev_p = 0;
	ds_parameter.next_p = 0;

	/* Delay the activation of LG-DVFS
	 */
	if(ds_configuration.on_dvs == 1 && ds_status.flag_run_dvs == 0){
		if(ds_counter.elapsed_sec > DS_INIT_DELAY_SEC){
			ds_status.flag_run_dvs = 1;
			printk(KERN_INFO "[LG-DVFS] LG-DVFS was activated.\n");
		}
	}

	/* Delay the enforcement of the min. freq. upon early suspend.
	 */
	if(ds_status.flag_post_early_suspend == 0){
		ds_status.flag_do_post_early_suspend = 0;
	}
	else if((ds_status.post_early_suspend_sec * 1000000 + 
			ds_status.post_early_suspend_usec + 
			DS_POST_EARLY_SUSPEND_TIMEOUT_USEC) >
			(ds_counter.elapsed_sec * 1000000 + 
			ds_counter.elapsed_usec))
	{
		ds_status.flag_do_post_early_suspend = 0;
	}
	else{
		ds_status.flag_do_post_early_suspend = 1;
	}

	return;
}

/* LKM START ***********************/
static int __init lg_dvfs_init(void)
{
	printk(KERN_INFO "LG-DVFS init\n");
	return 0;
}

static void __exit lg_dvfs_exit(void)
{
	printk(KERN_INFO "LG-DVFS exit\n");
}

module_init(lg_dvfs_init);
module_exit(lg_dvfs_exit);

//MODULE_LICENSE("GPL");
MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
//MODULE_SUPPORTED_DEVICE("cpu");
/* LKM END *************************/
