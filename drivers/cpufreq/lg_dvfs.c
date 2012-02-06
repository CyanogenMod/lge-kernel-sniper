/*
 *	linux/drivers/cpufreq/lg_dvfs.c
 *
 *	Copyright (C) 2010 Sookyoung Kim <sookyoung.kim@lge.com>
 */

/* LKM START ***********************/
//#define __KERNEL__
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

/****************************************************************************************
 * Variables and data structures
 ****************************************************************************************/

/****************************************************************************************
 * Function definitions
 ****************************************************************************************/

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
		return(DS_CPU_OP_INDEX_MAX);
	}
	else{
		switch(perf_requirement_fra_fp12 & 0xf00){
			case 0xf00: goto LC_LT_0XF00;	/* > 0.9375 and < 1 */
			case 0xe00: goto LC_LT_0XE00;	/* > 0.875 and < 0.9375 */
			case 0xd00: goto LC_LT_0XD00;	/* > 0.8125 and < 0.875 */
			case 0xc00: goto LC_LT_0XC00;	/* > 0.75 and < 0.8125 */
			case 0xb00: goto LC_LT_0XB00;	/* > 0.6875 and < 0.75 */
			case 0xa00: goto LC_LT_0XA00;	/* > 0.625 and < 0.6875 */
			case 0x900: goto LC_LT_0X900;	/* > 0.5625 and < 0.625 */
			case 0x800: goto LC_LT_0X800;	/* > 0.5 and < 0.5625 */
			case 0x700: goto LC_LT_0X700;	/* > 0.4375 and < 0.5 */
			case 0x600: goto LC_LT_0X600;	/* > 0.375 and < 0.4375 */
			case 0x500: goto LC_LT_0X500;	/* > 0.3125 and < 0.375 */
			case 0x400: goto LC_LT_0X400;	/* > 0.25 and < 0.3125 */
			case 0x300: goto LC_LT_0X300;	/* > 0.1875 and < 0.25 */
			case 0x200: goto LC_LT_0X200;	/* > 0.125 and < 0.1875 */
			case 0x100: goto LC_LT_0X100;	/* > 0.0625 and < 0.125 */
			default: goto LC_LT_0X000;	/* < 0.0625 */
		}
	}

LC_LT_0XF00:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9961 and < 1 */
		case 0x0e0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9922 and < 0.9961 */
		case 0x0d0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9883 and < 0.9922 */
		case 0x0c0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9844 and < 0.9883 */
		case 0x0b0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9805 and < 0.9844 */
		case 0x0a0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9766 and < 0.9805 */
		case 0x090: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9727 and < 0.9766 */
		case 0x080: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9688 and < 0.9727 */
		case 0x070: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9648 and < 0.9688 */
		case 0x060: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9609 and < 0.9648 */
		case 0x050: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9570 and < 0.9609 */
		case 0x040: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9531 and < 0.9570 */
		case 0x030: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9492 and < 0.9531 */
		case 0x020: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9453 and < 0.9492 */
		case 0x010: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9414 and < 0.9453 */
		default: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9375 and < 0.9414 */
	}
LC_LT_0XE00:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9336 and < 0.9375 */
		case 0x0e0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9297 and < 0.9336 */
		case 0x0d0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9258 and < 0.9297 */
		case 0x0c0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9219 and < 0.9258 */
		case 0x0b0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9180 and < 0.9219 */
		case 0x0a0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9141 and < 0.9180 */
		case 0x090: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9102 and < 0.9141 */
		case 0x080: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9063 and < 0.9102 */
		case 0x070: return(DS_CPU_OP_INDEX_MAX);	/* > 0.9023 and < 0.9063 */
		case 0x060: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8984 and < 0.9023 */
		case 0x050: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8945 and < 0.8984 */
		case 0x040: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8906 and < 0.8945 */
		case 0x030: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8867 and < 0.8906 */
		case 0x020: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8828 and < 0.8867 */
		case 0x010: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8789 and < 0.8828 */
		default: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8750 and < 0.8789 */
	}
LC_LT_0XD00:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8711 and < 0.8750 */
		case 0x0e0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8672 and < 0.8711 */
		case 0x0d0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8633 and < 0.8672 */
		case 0x0c0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8594 and < 0.8633 */
		case 0x0b0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8555 and < 0.8594 */
		case 0x0a0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8516 and < 0.8555 */
		case 0x090: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8477 and < 0.8516 */
		case 0x080: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8438 and < 0.8477 */
		case 0x070: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8398 and < 0.8438 */
		case 0x060: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8359 and < 0.8398 */
		case 0x050: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8320 and < 0.8359 */
		case 0x040: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8281 and < 0.8320 */
		case 0x030: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8242 and < 0.8281 */
		case 0x020: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8203 and < 0.8242 */
		case 0x010: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8164 and < 0.8203 */
		default: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8125 and < 0.8164 */
	}
LC_LT_0XC00:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8086 and < 0.8125 */
		case 0x0e0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8047 and < 0.8086 */
		case 0x0d0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.8008 and < 0.8047 */
		case 0x0c0: return(DS_CPU_OP_INDEX_MAX);	/* > 0.7969 and < 0.8008 */
		case 0x0b0: return(DS_CPU_OP_INDEX_1);	/* > 0.7930 and < 0.7969 */
		case 0x0a0: return(DS_CPU_OP_INDEX_1);	/* > 0.7891 and < 0.7930 */
		case 0x090: return(DS_CPU_OP_INDEX_1);	/* > 0.7852 and < 0.7891 */
		case 0x080: return(DS_CPU_OP_INDEX_1);	/* > 0.7813 and < 0.7852 */
		case 0x070: return(DS_CPU_OP_INDEX_1);	/* > 0.7773 and < 0.7813 */
		case 0x060: return(DS_CPU_OP_INDEX_1);	/* > 0.7734 and < 0.7773 */
		case 0x050: return(DS_CPU_OP_INDEX_1);	/* > 0.7695 and < 0.7734 */
		case 0x040: return(DS_CPU_OP_INDEX_1);	/* > 0.7656 and < 0.7695 */
		case 0x030: return(DS_CPU_OP_INDEX_1);	/* > 0.7617 and < 0.7656 */
		case 0x020: return(DS_CPU_OP_INDEX_1);	/* > 0.7578 and < 0.7617 */
		case 0x010: return(DS_CPU_OP_INDEX_1);	/* > 0.7539 and < 0.7578 */
		default: return(DS_CPU_OP_INDEX_1);	/* > 0.7500 and < 0.7539 */
	}
LC_LT_0XB00:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_1);	/* > 0.7461 and < 0.7500 */
		case 0x0e0: return(DS_CPU_OP_INDEX_1);	/* > 0.7422 and < 0.7461 */
		case 0x0d0: return(DS_CPU_OP_INDEX_1);	/* > 0.7383 and < 0.7422 */
		case 0x0c0: return(DS_CPU_OP_INDEX_1);	/* > 0.7344 and < 0.7383 */
		case 0x0b0: return(DS_CPU_OP_INDEX_1);	/* > 0.7305 and < 0.7344 */
		case 0x0a0: return(DS_CPU_OP_INDEX_1);	/* > 0.7266 and < 0.7305 */
		case 0x090: return(DS_CPU_OP_INDEX_1);	/* > 0.7227 and < 0.7266 */
		case 0x080: return(DS_CPU_OP_INDEX_1);	/* > 0.7188 and < 0.7227 */
		case 0x070: return(DS_CPU_OP_INDEX_1);	/* > 0.7148 and < 0.7188 */
		case 0x060: return(DS_CPU_OP_INDEX_1);	/* > 0.7109 and < 0.7148 */
		case 0x050: return(DS_CPU_OP_INDEX_1);	/* > 0.7070 and < 0.7109 */
		case 0x040: return(DS_CPU_OP_INDEX_1);	/* > 0.7031 and < 0.7070 */
		case 0x030: return(DS_CPU_OP_INDEX_1);	/* > 0.6992 and < 0.7031 */
		case 0x020: return(DS_CPU_OP_INDEX_1);	/* > 0.6953 and < 0.6992 */
		case 0x010: return(DS_CPU_OP_INDEX_1);	/* > 0.6914 and < 0.6953 */
		default: return(DS_CPU_OP_INDEX_1);	/* > 0.6875 and < 0.6914 */
	}
LC_LT_0XA00:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_1);	/* > 0.6836 and < 0.6875 */
		case 0x0e0: return(DS_CPU_OP_INDEX_1);	/* > 0.6797 and < 0.6836 */
		case 0x0d0: return(DS_CPU_OP_INDEX_1);	/* > 0.6758 and < 0.6797 */
		case 0x0c0: return(DS_CPU_OP_INDEX_1);	/* > 0.6719 and < 0.6758 */
		case 0x0b0: return(DS_CPU_OP_INDEX_1);	/* > 0.6680 and < 0.6719 */
		case 0x0a0: return(DS_CPU_OP_INDEX_1);	/* > 0.6641 and < 0.6680 */
		case 0x090: return(DS_CPU_OP_INDEX_1);	/* > 0.6602 and < 0.6641 */
		case 0x080: return(DS_CPU_OP_INDEX_1);	/* > 0.6563 and < 0.6602 */
		case 0x070: return(DS_CPU_OP_INDEX_1);	/* > 0.6523 and < 0.6563 */
		case 0x060: return(DS_CPU_OP_INDEX_1);	/* > 0.6484 and < 0.6523 */
		case 0x050: return(DS_CPU_OP_INDEX_1);	/* > 0.6445 and < 0.6484 */
		case 0x040: return(DS_CPU_OP_INDEX_1);	/* > 0.6406 and < 0.6445 */
		case 0x030: return(DS_CPU_OP_INDEX_1);	/* > 0.6367 and < 0.6406 */
		case 0x020: return(DS_CPU_OP_INDEX_1);	/* > 0.6328 and < 0.6367 */
		case 0x010: return(DS_CPU_OP_INDEX_1);	/* > 0.6289 and < 0.6328 */
		default: return(DS_CPU_OP_INDEX_1);	/* > 0.6250 and < 0.6289 */
	}
LC_LT_0X900:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_1);	/* > 0.6211 and < 0.6250 */
		case 0x0e0: return(DS_CPU_OP_INDEX_1);	/* > 0.6172 and < 0.6211 */
		case 0x0d0: return(DS_CPU_OP_INDEX_1);	/* > 0.6133 and < 0.6172 */
		case 0x0c0: return(DS_CPU_OP_INDEX_1);	/* > 0.6093 and < 0.6133 */
		case 0x0b0: return(DS_CPU_OP_INDEX_1);	/* > 0.6055 and < 0.6093 */
		case 0x0a0: return(DS_CPU_OP_INDEX_1);	/* > 0.6016 and < 0.6055 */
		case 0x090: return(DS_CPU_OP_INDEX_1);	/* > 0.5977 and < 0.6016 */
		case 0x080: return(DS_CPU_OP_INDEX_2);	/* > 0.5938 and < 0.5977 */
		case 0x070: return(DS_CPU_OP_INDEX_2);	/* > 0.5898 and < 0.5938 */
		case 0x060: return(DS_CPU_OP_INDEX_2);	/* > 0.5859 and < 0.5898 */
		case 0x050: return(DS_CPU_OP_INDEX_2);	/* > 0.5820 and < 0.5859 */
		case 0x040: return(DS_CPU_OP_INDEX_2);	/* > 0.5781 and < 0.5820 */
		case 0x030: return(DS_CPU_OP_INDEX_2);	/* > 0.5742 and < 0.5781 */
		case 0x020: return(DS_CPU_OP_INDEX_2);	/* > 0.5703 and < 0.5742 */
		case 0x010: return(DS_CPU_OP_INDEX_2);	/* > 0.5664 and < 0.5703 */
		default: return(DS_CPU_OP_INDEX_2);	/* > 0.5625 and < 0.5664 */
	}
LC_LT_0X800:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_2);	/* > 0.5586 and < 0.5625 */
		case 0x0e0: return(DS_CPU_OP_INDEX_2);	/* > 0.5547 and < 0.5586 */
		case 0x0d0: return(DS_CPU_OP_INDEX_2);	/* > 0.5508 and < 0.5547 */
		case 0x0c0: return(DS_CPU_OP_INDEX_2);	/* > 0.5469 and < 0.5508 */
		case 0x0b0: return(DS_CPU_OP_INDEX_2);	/* > 0.5430 and < 0.5469 */
		case 0x0a0: return(DS_CPU_OP_INDEX_2);	/* > 0.5391 and < 0.5430 */
		case 0x090: return(DS_CPU_OP_INDEX_2);	/* > 0.5352 and < 0.5391 */
		case 0x080: return(DS_CPU_OP_INDEX_2);	/* > 0.5313 and < 0.5352 */
		case 0x070: return(DS_CPU_OP_INDEX_2);	/* > 0.5273 and < 0.5313 */
		case 0x060: return(DS_CPU_OP_INDEX_2);	/* > 0.5234 and < 0.5273 */
		case 0x050: return(DS_CPU_OP_INDEX_2);	/* > 0.5195 and < 0.5234 */
		case 0x040: return(DS_CPU_OP_INDEX_2);	/* > 0.5156 and < 0.5195 */
		case 0x030: return(DS_CPU_OP_INDEX_2);	/* > 0.5117 and < 0.5156 */
		case 0x020: return(DS_CPU_OP_INDEX_2);	/* > 0.5078 and < 0.5117 */
		case 0x010: return(DS_CPU_OP_INDEX_2);	/* > 0.5039 and < 0.5078 */
		default: return(DS_CPU_OP_INDEX_2);	/* > 0.5000 and < 0.5039 */
	}
LC_LT_0X700:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_2);	/* > 0.4961 and < 0.5000 */
		case 0x0e0: return(DS_CPU_OP_INDEX_2);	/* > 0.4922 and < 0.4961 */
		case 0x0d0: return(DS_CPU_OP_INDEX_2);	/* > 0.4883 and < 0.4922 */
		case 0x0c0: return(DS_CPU_OP_INDEX_2);	/* > 0.4844 and < 0.4883 */
		case 0x0b0: return(DS_CPU_OP_INDEX_2);	/* > 0.4805 and < 0.4844 */
		case 0x0a0: return(DS_CPU_OP_INDEX_2);	/* > 0.4766 and < 0.4805 */
		case 0x090: return(DS_CPU_OP_INDEX_2);	/* > 0.4727 and < 0.4766 */
		case 0x080: return(DS_CPU_OP_INDEX_2);	/* > 0.4688 and < 0.4727 */
		case 0x070: return(DS_CPU_OP_INDEX_2);	/* > 0.4648 and < 0.4688 */
		case 0x060: return(DS_CPU_OP_INDEX_2);	/* > 0.4609 and < 0.4648 */
		case 0x050: return(DS_CPU_OP_INDEX_2);	/* > 0.4570 and < 0.4609 */
		case 0x040: return(DS_CPU_OP_INDEX_2);	/* > 0.4531 and < 0.4570 */
		case 0x030: return(DS_CPU_OP_INDEX_2);	/* > 0.4492 and < 0.4531 */
		case 0x020: return(DS_CPU_OP_INDEX_2);	/* > 0.4453 and < 0.4492 */
		case 0x010: return(DS_CPU_OP_INDEX_2);	/* > 0.4414 and < 0.4453 */
		default: return(DS_CPU_OP_INDEX_2);	/* > 0.4375 and < 0.4414 */
	}
LC_LT_0X600:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_2);	/* > 0.4336 and < 0.4375 */
		case 0x0e0: return(DS_CPU_OP_INDEX_2);	/* > 0.4297 and < 0.4336 */
		case 0x0d0: return(DS_CPU_OP_INDEX_2);	/* > 0.4258 and < 0.4297 */
		case 0x0c0: return(DS_CPU_OP_INDEX_2);	/* > 0.4219 and < 0.4258 */
		case 0x0b0: return(DS_CPU_OP_INDEX_2);	/* > 0.4180 and < 0.4219 */
		case 0x0a0: return(DS_CPU_OP_INDEX_2);	/* > 0.4141 and < 0.4180 */
		case 0x090: return(DS_CPU_OP_INDEX_2);	/* > 0.4102 and < 0.4141 */
		case 0x080: return(DS_CPU_OP_INDEX_2);	/* > 0.4063 and < 0.4102 */
		case 0x070: return(DS_CPU_OP_INDEX_2);	/* > 0.4023 and < 0.4063 */
		case 0x060: return(DS_CPU_OP_INDEX_2);	/* > 0.3984 and < 0.4023 */
		case 0x050: return(DS_CPU_OP_INDEX_2);	/* > 0.3945 and < 0.3984 */
		case 0x040: return(DS_CPU_OP_INDEX_2);	/* > 0.3906 and < 0.3945 */
		case 0x030: return(DS_CPU_OP_INDEX_2);	/* > 0.3867 and < 0.3906 */
		case 0x020: return(DS_CPU_OP_INDEX_2);	/* > 0.3828 and < 0.3867 */
		case 0x010: return(DS_CPU_OP_INDEX_2);	/* > 0.3789 and < 0.3828 */
		default: return(DS_CPU_OP_INDEX_2);	/* > 0.3750 and < 0.3789 */
	}
LC_LT_0X500:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_2);	/* > 0.3711 and < 0.3750 */
		case 0x0e0: return(DS_CPU_OP_INDEX_2);	/* > 0.3672 and < 0.3711 */
		case 0x0d0: return(DS_CPU_OP_INDEX_2);	/* > 0.3633 and < 0.3672 */
		case 0x0c0: return(DS_CPU_OP_INDEX_2);	/* > 0.3594 and < 0.3633 */
		case 0x0b0: return(DS_CPU_OP_INDEX_2);	/* > 0.3555 and < 0.3594 */
		case 0x0a0: return(DS_CPU_OP_INDEX_2);	/* > 0.3516 and < 0.3555 */
		case 0x090: return(DS_CPU_OP_INDEX_2);	/* > 0.3477 and < 0.3516 */
		case 0x080: return(DS_CPU_OP_INDEX_2);	/* > 0.3438 and < 0.3477 */
		case 0x070: return(DS_CPU_OP_INDEX_2);	/* > 0.3398 and < 0.3438 */
		case 0x060: return(DS_CPU_OP_INDEX_2);	/* > 0.3359 and < 0.3398 */
		case 0x050: return(DS_CPU_OP_INDEX_2);	/* > 0.3320 and < 0.3359 */
		case 0x040: return(DS_CPU_OP_INDEX_2);	/* > 0.3281 and < 0.3320 */
		case 0x030: return(DS_CPU_OP_INDEX_2);	/* > 0.3242 and < 0.3281 */
		case 0x020: return(DS_CPU_OP_INDEX_2);	/* > 0.3203 and < 0.3242 */
		case 0x010: return(DS_CPU_OP_INDEX_2);	/* > 0.3164 and < 0.3203 */
		default: return(DS_CPU_OP_INDEX_2);	/* > 0.3125 and < 0.3164 */
	}
LC_LT_0X400:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_2);	/* > 0.3086 and < 0.3125 */
		case 0x0e0: return(DS_CPU_OP_INDEX_2);	/* > 0.3047 and < 0.3086 */
		case 0x0d0: return(DS_CPU_OP_INDEX_2);	/* > 0.3008 and < 0.3047 */
		case 0x0c0: return(DS_CPU_OP_INDEX_2);	/* > 0.2969 and < 0.3008 */
		case 0x0b0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2930 and < 0.2969 */
		case 0x0a0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2891 and < 0.2930 */
		case 0x090: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2852 and < 0.2891 */
		case 0x080: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2813 and < 0.2852 */
		case 0x070: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2773 and < 0.2813 */
		case 0x060: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2734 and < 0.2773 */
		case 0x050: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2695 and < 0.2734 */
		case 0x040: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2656 and < 0.2695 */
		case 0x030: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2617 and < 0.2656 */
		case 0x020: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2578 and < 0.2617 */
		case 0x010: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2539 and < 0.2578 */
		default: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2500 and < 0.2539 */
	}
LC_LT_0X300:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2461 and < 0.2500 */
		case 0x0e0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2422 and < 0.2461 */
		case 0x0d0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2383 and < 0.2422 */
		case 0x0c0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2344 and < 0.2383 */
		case 0x0b0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2305 and < 0.2344 */
		case 0x0a0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2266 and < 0.2305 */
		case 0x090: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2227 and < 0.2266 */
		case 0x080: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2188 and < 0.2227 */
		case 0x070: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2148 and < 0.2188 */
		case 0x060: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2109 and < 0.2148 */
		case 0x050: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2070 and < 0.2109 */
		case 0x040: return(DS_CPU_OP_INDEX_MIN);	/* > 0.2031 and < 0.2070 */
		case 0x030: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1992 and < 0.2031 */
		case 0x020: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1953 and < 0.1992 */
		case 0x010: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1914 and < 0.1953 */
		default: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1875 and < 0.1914 */
	}
LC_LT_0X200:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1836 and < 0.1875 */
		case 0x0e0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1797 and < 0.1836 */
		case 0x0d0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1758 and < 0.1797 */
		case 0x0c0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1719 and < 0.1758 */
		case 0x0b0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1680 and < 0.1719 */
		case 0x0a0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1641 and < 0.1680 */
		case 0x090: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1602 and < 0.1641 */
		case 0x080: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1563 and < 0.1602 */
		case 0x070: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1523 and < 0.1563 */
		case 0x060: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1484 and < 0.1523 */
		case 0x050: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1445 and < 0.1484 */
		case 0x040: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1406 and < 0.1445 */
		case 0x030: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1367 and < 0.1406 */
		case 0x020: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1328 and < 0.1367 */
		case 0x010: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1289 and < 0.1328 */
		default: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1250 and < 0.1289 */
	}
LC_LT_0X100:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1211 and < 0.1250 */
		case 0x0e0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1172 and < 0.1211 */
		case 0x0d0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1133 and < 0.1172 */
		case 0x0c0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1094 and < 0.1133 */
		case 0x0b0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1055 and < 0.1094 */
		case 0x0a0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.1016 and < 0.1055 */
		case 0x090: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0977 and < 0.1016 */
		case 0x080: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0938 and < 0.0977 */
		case 0x070: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0898 and < 0.0938 */
		case 0x060: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0859 and < 0.0898 */
		case 0x050: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0820 and < 0.0859 */
		case 0x040: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0781 and < 0.0820 */
		case 0x030: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0742 and < 0.0781 */
		case 0x020: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0703 and < 0.0742 */
		case 0x010: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0664 and < 0.0703 */
		default: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0625 and < 0.0664 */
	}
LC_LT_0X000:
	switch(perf_requirement_fra_fp12 & 0x0f0){
		case 0x0f0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0586 and < 0.0625 */
		case 0x0e0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0547 and < 0.0586 */
		case 0x0d0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0508 and < 0.0547 */
		case 0x0c0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0469 and < 0.0508 */
		case 0x0b0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0430 and < 0.0469 */
		case 0x0a0: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0391 and < 0.0430 */
		case 0x090: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0352 and < 0.0391 */
		case 0x080: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0313 and < 0.0352 */
		case 0x070: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0273 and < 0.0313 */
		case 0x060: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0234 and < 0.0273 */
		case 0x050: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0195 and < 0.0234 */
		case 0x040: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0156 and < 0.0195 */
		case 0x030: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0117 and < 0.0156 */
		case 0x020: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0078 and < 0.0117 */
		case 0x010: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0039 and < 0.0078 */
		default: return(DS_CPU_OP_INDEX_MIN);	/* > 0.0000 and < 0.0039 */
	}
}

/*====================================================================
	The function which updates the fractions of
	elapsed and busy time both in wall time and full speed equivalent time.
	====================================================================*/

int ds_update_time_counter(int ds_cpu){

	cputime64_t lc_usec_wall_cur = 0;
	cputime64_t lc_usec_idle_cur = 0;
	cputime64_t lc_usec_iowait_cur = 0;

	unsigned long lc_usec_interval = 0;
	unsigned long lc_usec_idle = 0;
	unsigned long lc_usec_iowait = 0;
	unsigned long lc_usec_busy = 0;

	if(per_cpu(ds_counter, ds_cpu).counter_mutex != 0) return(0);
	per_cpu(ds_counter, ds_cpu).counter_mutex ++;

	lc_usec_wall_cur = jiffies64_to_cputime64(get_jiffies_64());
	lc_usec_idle_cur = get_cpu_idle_time_us(ds_cpu, &lc_usec_wall_cur);
	lc_usec_iowait_cur = get_cpu_iowait_time_us(ds_cpu, &lc_usec_wall_cur);

	if(per_cpu(ds_counter, ds_cpu).flag_counter_initialized == 0){
		per_cpu(ds_counter, ds_cpu).flag_counter_initialized = 1;
	}
	else{
		lc_usec_interval = 
			(unsigned long)(lc_usec_wall_cur - per_cpu(ds_counter, ds_cpu).wall_usec_base);
		lc_usec_idle = 
			(unsigned long)(lc_usec_idle_cur - per_cpu(ds_counter, ds_cpu).idle_usec_base);
		lc_usec_iowait = 
			(unsigned long)(lc_usec_iowait_cur - per_cpu(ds_counter, ds_cpu).iowait_usec_base);

#if 0
		if(lc_usec_idle > lc_usec_iowait)
			lc_usec_idle -= lc_usec_iowait;
		else
			lc_usec_idle = 0;
#endif

		if(lc_usec_interval > lc_usec_idle)
			lc_usec_busy = lc_usec_interval - lc_usec_idle;
		else
			lc_usec_busy = 0;

		/* Elapsed */
		per_cpu(ds_counter, ds_cpu).elapsed_sec += (lc_usec_interval / 1000000);
		per_cpu(ds_counter, ds_cpu).elapsed_usec += (lc_usec_interval % 1000000);
		if(per_cpu(ds_counter, ds_cpu).elapsed_usec >= 1000000){
			per_cpu(ds_counter, ds_cpu).elapsed_sec += 1;
			per_cpu(ds_counter, ds_cpu).elapsed_usec -= 1000000;
				}

		/* Busy */
		per_cpu(ds_counter, ds_cpu).busy_sec += (lc_usec_busy / 1000000);
		per_cpu(ds_counter, ds_cpu).busy_usec += (lc_usec_busy % 1000000);
		if(per_cpu(ds_counter, ds_cpu).busy_usec >= 1000000){
			per_cpu(ds_counter, ds_cpu).busy_sec += 1;
			per_cpu(ds_counter, ds_cpu).busy_usec -= 1000000;
				}
				}

	per_cpu(ds_counter, ds_cpu).wall_usec_base = lc_usec_wall_cur;
	per_cpu(ds_counter, ds_cpu).idle_usec_base = lc_usec_idle_cur;
	per_cpu(ds_counter, ds_cpu).iowait_usec_base = lc_usec_iowait_cur;

	per_cpu(ds_counter, ds_cpu).counter_mutex --;

	return(0);
}

/*====================================================================
	The functions to perform DVS scheme:
	AIDVS.
	====================================================================*/

int ds_do_dvs_aidvs(int ds_cpu, unsigned int *target_cpu_op_index, DS_AIDVS_STAT_STRUCT *stat)
{
	unsigned long lc_time_usec_interval_inc = 0;
	unsigned long lc_time_usec_work_inc = 0;

	unsigned long lc_old_moving_avg_int_ulong = 0;
	unsigned long lc_old_moving_avg_fra_fp12 = 0x0;
	unsigned long lc_old_utilization_int_ulong = 0;
	unsigned long lc_old_utilization_fra_fp12 = 0x0;
	unsigned long lc_old_moving_avg_int_ulong_by_weight = 0;
	unsigned long lc_old_moving_avg_fra_fp12_by_weight = 0x0;
	unsigned long lc_numerator_int_ulong = 0;
	unsigned long lc_numerator_fra_fp12 = 0x0;

	/* Calc interval */
	if(per_cpu(ds_counter, ds_cpu).elapsed_usec >= stat->time_usec_interval_inc_base){
		lc_time_usec_interval_inc =
			per_cpu(ds_counter, ds_cpu).elapsed_usec - stat->time_usec_interval_inc_base;
	}
	else{
		lc_time_usec_interval_inc =
			per_cpu(ds_counter, ds_cpu).elapsed_usec + (1000000 - stat->time_usec_interval_inc_base);
	}
	stat->time_usec_interval += lc_time_usec_interval_inc;
	stat->time_sec_interval_inc_base = per_cpu(ds_counter, ds_cpu).elapsed_sec;
	stat->time_usec_interval_inc_base = per_cpu(ds_counter, ds_cpu).elapsed_usec;

	/* Calc work */
	if(per_cpu(ds_counter, ds_cpu).busy_usec >= stat->time_usec_work_inc_base){
			lc_time_usec_work_inc =
			per_cpu(ds_counter, ds_cpu).busy_usec - stat->time_usec_work_inc_base;
		}
		else{
			lc_time_usec_work_inc =
			per_cpu(ds_counter, ds_cpu).busy_usec + (1000000 - stat->time_usec_work_inc_base);
		}
		stat->time_usec_work += lc_time_usec_work_inc;
	stat->time_sec_work_inc_base = per_cpu(ds_counter, ds_cpu).busy_sec;
	stat->time_usec_work_inc_base = per_cpu(ds_counter, ds_cpu).busy_usec;

	/* Determine interval_length to use */
	if(per_cpu(ds_sys_status, 0).flag_do_post_early_suspend == 0){
		stat->interval_length = ds_control.aidvs_interval_length;
			}
			else{
		stat->interval_length = DS_AIDVS_PE_INTERVAL_LENGTH;
			}

	/* Calc cpu_op if we reached interval_window_legnth */
	if(stat->time_usec_interval >= stat->interval_length)
	{
		/* SAVE OLD */
		lc_old_utilization_int_ulong = stat->utilization_int_ulong;
		lc_old_utilization_fra_fp12 = stat->utilization_fra_fp12;
		lc_old_moving_avg_int_ulong = stat->moving_avg_int_ulong;
		lc_old_moving_avg_fra_fp12 = stat->moving_avg_fra_fp12;
	
				/* Calculate NEW, i.e., the current interval window's utilization */
		/* In wall clock time */
		if(stat->time_usec_interval == 0){
					stat->utilization_int_ulong = 1;
			stat->utilization_fra_fp12 = 0x0;
		}
		else{
			if(ds_fpdiv(stat->time_usec_work,
					0x0,
					stat->time_usec_interval,
					0x0,
					&(stat->utilization_int_ulong),
					&(stat->utilization_fra_fp12)) < 0)
	{
				printk(KERN_INFO "[ds_do_dvs_aidvs 1] Error: ds_fpdiv failed. \
						dvs_suite gonna be off.\n");
				ds_control.on_dvs = 0;
				ds_control.flag_run_dvs = 0;
				return(-1);
		}
	}

		/* Determine WEIGHT */
		if(per_cpu(ds_sys_status, 0).flag_do_post_early_suspend == 0){
			stat->moving_avg_weight = ds_control.aidvs_moving_avg_weight;
			}
			else
			stat->moving_avg_weight = DS_AIDVS_PE_MOVING_AVG_WEIGHT;

		/* Calculate OLD x WEIGHT */
		if(ds_fpmul(lc_old_moving_avg_int_ulong, 
					lc_old_moving_avg_fra_fp12, 
					stat->moving_avg_weight,
						0x0,
					&lc_old_moving_avg_int_ulong_by_weight, 
					&lc_old_moving_avg_fra_fp12_by_weight) < 0)
		{
			printk(KERN_INFO "[ds_do_dvs_aidvs 4] Error: ds_fpmul failed. \
					dvs_suite gonna be off.\n");
			ds_control.on_dvs = 0;
			ds_control.flag_run_dvs = 0;
						return(-1);
					}

		/* Calculate OLD x WEIGHT + NEW, i.e., the numerator of moving average */
				lc_numerator_int_ulong = 
			lc_old_moving_avg_int_ulong_by_weight + stat->utilization_int_ulong;
				lc_numerator_fra_fp12 = 
			lc_old_moving_avg_fra_fp12_by_weight + stat->utilization_fra_fp12;
				if(lc_numerator_fra_fp12 >= 0x1000){
					lc_numerator_int_ulong += 1;
					lc_numerator_fra_fp12 -= 0x1000;
				}

		/* Calculate (OLD x WEIGHT + NEW) / (WEIGHT + 1), i.e., the moving average */
		if(ds_fpdiv(lc_numerator_int_ulong, 
					lc_numerator_fra_fp12, 
					stat->moving_avg_weight+1, 
					0x0,
					&stat->moving_avg_int_ulong, 
					&stat->moving_avg_fra_fp12) < 0)
		{
			printk(KERN_INFO "[ds_do_dvs_aidvs 5] Error: ds_fpdiv failed. \
					dvs_suite gonna be off.\n");
			ds_control.on_dvs = 0;
			ds_control.flag_run_dvs = 0;
			return(-1);
		}

				/* Find the CPU_OP_INDEX corresponding to the calculated utilization */
				stat->cpu_op_index = 
			ds_get_next_high_cpu_op_index(stat->moving_avg_int_ulong, stat->moving_avg_fra_fp12);
		/* TODO: Tune for OMAP */
		if(ds_cpu){
			switch(stat->cpu_op_index){
				case DS_CPU_OP_INDEX_0:	stat->cpu_op_index = DS_CPU_OP_INDEX_0; break;
				case DS_CPU_OP_INDEX_1:	stat->cpu_op_index = DS_CPU_OP_INDEX_1; break;
				case DS_CPU_OP_INDEX_2:	stat->cpu_op_index = DS_CPU_OP_INDEX_2; break;
				case DS_CPU_OP_INDEX_3:	stat->cpu_op_index = DS_CPU_OP_INDEX_3; break;
				default:	stat->cpu_op_index = DS_CPU_OP_INDEX_0; break;
			}
			}

			stat->time_usec_interval = 0;
		stat->time_usec_work = 0;
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
int ds_do_dvs_gpschedvs(int ds_cpu, unsigned int *target_cpu_op_index){

	unsigned int lc_target_cpu_op_index_highest = DS_CPU_OP_INDEX_MIN;
	unsigned int lc_target_cpu_op_index_aidvs = DS_CPU_OP_INDEX_MIN;
	unsigned int lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_MIN;

	/* Calc target_cpu_op based on workload */
	ds_do_dvs_aidvs(ds_cpu, &lc_target_cpu_op_index_aidvs, &(per_cpu(ds_aidvs_status, ds_cpu)));

	lc_target_cpu_op_index_highest = lc_target_cpu_op_index_aidvs;

	/* LCD is on (i.e., before early suspend) */
	if(per_cpu(ds_sys_status, 0).flag_do_post_early_suspend == 0){

		/* Touch event occurred and being processed - Begin */
		if(per_cpu(ds_sys_status, 0).flag_touch_timeout_count != 0){

			/* If DS_TOUCH_CPU_OP_UP_INTERVAL is over */
			if((per_cpu(ds_counter, ds_cpu).elapsed_sec * 1000000 + 
				per_cpu(ds_counter, ds_cpu).elapsed_usec) >= 
				(per_cpu(ds_sys_status, 0).touch_timeout_sec * 1000000 +
				per_cpu(ds_sys_status, 0).touch_timeout_usec))
			{
				if(per_cpu(ds_counter, ds_cpu).elapsed_usec + DS_TOUCH_CPU_OP_UP_INTERVAL < 1000000){
					per_cpu(ds_sys_status, 0).touch_timeout_sec = 
						per_cpu(ds_counter, ds_cpu).elapsed_sec;
					per_cpu(ds_sys_status, 0).touch_timeout_usec = 
						per_cpu(ds_counter, ds_cpu).elapsed_usec + DS_TOUCH_CPU_OP_UP_INTERVAL;
			}
				else{
					per_cpu(ds_sys_status, 0).touch_timeout_sec = 
						per_cpu(ds_counter, ds_cpu).elapsed_sec + 1;
					per_cpu(ds_sys_status, 0).touch_timeout_usec = 
						(per_cpu(ds_counter, ds_cpu).elapsed_usec + DS_TOUCH_CPU_OP_UP_INTERVAL) - 1000000;
		}

				switch(per_cpu(ds_sys_status, 0).flag_touch_timeout_count){
					case DS_TOUCH_CPU_OP_UP_CNT_MAX:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 39;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH39;
						break;
					case 39:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 38;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH38;
						break;
					case 38:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 37;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH37;
						break;
					case 37:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 36;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH36;
						break;
					case 36:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 35;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH35;
						break;
					case 35:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 34;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH34;
						break;
					case 34:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 33;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH33;
						break;
					case 33:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 32;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH32;
						break;
					case 32:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 31;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH31;
						break;
					case 31:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 30;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH30;
						break;
					case 30:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 29;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH29;
						break;
					case 29:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 28;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH28;
						break;
					case 28:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 27;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH27;
						break;
					case 27:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 26;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH26;
						break;
					case 26:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 25;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH25;
						break;
					case 25:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 24;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH24;
						break;
					case 24:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 23;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH23;
						break;
					case 23:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 22;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH22;
						break;
					case 22:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 21;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH21;
						break;
					case 21:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 20;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH20;
						break;
					case 20:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 19;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH19;
						break;
					case 19:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 18;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH18;
						break;
					case 18:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 17;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH17;
						break;
					case 17:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 16;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH16;
						break;
					case 16:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 15;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH15;
						break;
					case 15:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 14;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH14;
						break;
					case 14:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 13;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH13;
						break;
					case 13:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 12;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH12;
						break;
					case 12:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 11;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH11;
						break;
					case 11:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 10;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH10;
						break;
					case 10:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 9;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH9;
						break;
					case 9:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 8;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH8;
						break;
					case 8:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 7;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH7;
						break;
					case 7:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 6;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH6;
						break;
					case 6:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 5;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH5;
						break;
					case 5:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 4;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH4;
						break;
					case 4:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 3;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH3;
						break;
					case 3:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 2;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH2;
						break;
					case 2:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 1;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH1;
						break;
					case 1:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 0;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_MIN;
						per_cpu(ds_sys_status, 0).touch_timeout_sec = 0;
						per_cpu(ds_sys_status, 0).touch_timeout_usec = 0;
						break;
					default:
						per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 0;
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_MIN;
						per_cpu(ds_sys_status, 0).touch_timeout_sec = 0;
						per_cpu(ds_sys_status, 0).touch_timeout_usec = 0;
						break;
				}
			}
			/* If DS_TOUCH_CPU_OP_UP_INTERVAL is not over yet */
			else
			{
				switch(per_cpu(ds_sys_status, 0).flag_touch_timeout_count){
					case DS_TOUCH_CPU_OP_UP_CNT_MAX:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH40;
						break;
					case 39:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH39;
						break;
					case 38:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH38;
						break;
					case 37:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH37;
						break;
					case 36:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH36;
						break;
					case 35:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH35;
						break;
					case 34:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH34;
						break;
					case 33:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH33;
						break;
					case 32:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH32;
						break;
					case 31:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH31;
						break;
					case 30:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH30;
						break;
					case 29:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH29;
						break;
					case 28:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH28;
						break;
					case 27:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH27;
						break;
					case 26:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH26;
						break;
					case 25:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH25;
						break;
					case 24:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH24;
						break;
					case 23:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH23;
						break;
					case 22:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH22;
						break;
					case 21:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH21;
						break;
					case 20:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH20;
						break;
					case 19:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH19;
						break;
					case 18:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH18;
						break;
					case 17:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH17;
						break;
					case 16:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH16;
						break;
					case 15:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH15;
						break;
					case 14:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH14;
						break;
					case 13:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH13;
						break;
					case 12:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH12;
						break;
					case 11:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH11;
						break;
					case 10:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH10;
						break;
					case 9:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH9;
						break;
					case 8:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH8;
						break;
					case 7:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH7;
						break;
					case 6:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH6;
						break;
					case 5:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH5;
						break;
					case 4:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH4;
						break;
					case 3:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH3;
						break;
					case 2:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH2;
						break;
					case 1:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_TOUCH1;
						break;
					default:
						lc_target_cpu_op_index_touch = DS_CPU_OP_INDEX_MIN;
						break;
				}
			}

			/* Ramping up cpu op upon touch events */
			if(lc_target_cpu_op_index_highest < lc_target_cpu_op_index_touch)
			lc_target_cpu_op_index_highest = lc_target_cpu_op_index_touch;
	}
		/* Touch event occurred and being processed - End */

		/* Special treatment upon frequency ceiling or flooring - Begin */
		/* Ceiled by long consecutive touches */
		if(per_cpu(ds_sys_status, 0).flag_long_consecutive_touches == 1){
			if(lc_target_cpu_op_index_highest > DS_CPU_OP_INDEX_CONT_TOUCH)
				lc_target_cpu_op_index_highest = DS_CPU_OP_INDEX_CONT_TOUCH;
		}
		/* Ceiled by cpufreq sysfs */
		if(per_cpu(ds_sys_status, 0).locked_max_cpu_op_index < DS_CPU_OP_INDEX_MAX){
			if(lc_target_cpu_op_index_highest > per_cpu(ds_sys_status, 0).locked_max_cpu_op_index)
				lc_target_cpu_op_index_highest = per_cpu(ds_sys_status, 0).locked_max_cpu_op_index;
		}
		/* Floored by cpufreq sysfs */
		if(per_cpu(ds_sys_status, 0).locked_min_cpu_op_index > DS_CPU_OP_INDEX_MIN){
			per_cpu(ds_sys_status, 0).locked_min_cpu_op_release_sec = 
				per_cpu(ds_counter, ds_cpu).elapsed_sec + DS_CPU_OP_LOCK_SUSTAIN_SEC;
			per_cpu(ds_sys_status, 0).flag_locked_min_cpu_op = 1;
			if(per_cpu(ds_sys_status, 0).locked_min_cpu_op_index > DS_CPU_OP_INDEX_LOCKED_MIN){
				if(lc_target_cpu_op_index_highest < DS_CPU_OP_INDEX_LOCKED_MIN)
					lc_target_cpu_op_index_highest = DS_CPU_OP_INDEX_LOCKED_MIN;
			}
			else{
				if(lc_target_cpu_op_index_highest < per_cpu(ds_sys_status, 0).locked_min_cpu_op_index)
					lc_target_cpu_op_index_highest = per_cpu(ds_sys_status, 0).locked_min_cpu_op_index;
			}
		}
		/* Frequency had been floored. But not now */
		else{
			if(per_cpu(ds_sys_status, 0).locked_min_cpu_op_release_sec >
				per_cpu(ds_counter, ds_cpu).elapsed_sec)
			{
				per_cpu(ds_sys_status, 0).flag_locked_min_cpu_op = 1;
				if(lc_target_cpu_op_index_highest < DS_CPU_OP_INDEX_LOCKED_MIN)
					lc_target_cpu_op_index_highest = DS_CPU_OP_INDEX_LOCKED_MIN;
			}
			else{
				per_cpu(ds_sys_status, 0).locked_min_cpu_op_release_sec = 0;
				per_cpu(ds_sys_status, 0).flag_locked_min_cpu_op = 0;
			}
		}
		/* Special care for the frequency locking through cpufreq sysfs - End */
	}
	/* LCD is off (i.e., after early suspend) */
	else{
		if(lc_target_cpu_op_index_highest < DS_CPU_OP_INDEX_MAX)
		lc_target_cpu_op_index_highest = DS_CPU_OP_INDEX_MIN;
	}

	*target_cpu_op_index = lc_target_cpu_op_index_highest;

	return(0);
}

/*====================================================================
	The functions to initialize dvs_suite_system_status structure.
	====================================================================*/

int ds_initialize_ds_control(void){

	ds_control.aidvs_moving_avg_weight = DS_AIDVS_NM_MOVING_AVG_WEIGHT;
	ds_control.aidvs_interval_length = DS_AIDVS_NM_INTERVAL_LENGTH;

	return(0);
}

/*====================================================================
	The functions to initialize dvs_suite_system_status structure.
	====================================================================*/

int ds_initialize_ds_sys_status(void){

	per_cpu(ds_sys_status, 0).locked_min_cpu_op_index = DS_CPU_OP_INDEX_MIN;
	per_cpu(ds_sys_status, 0).locked_min_cpu_op_release_sec = 0;
	per_cpu(ds_sys_status, 0).flag_locked_min_cpu_op = 0;
	per_cpu(ds_sys_status, 0).locked_max_cpu_op_index = DS_CPU_OP_INDEX_MAX;
	per_cpu(ds_sys_status, 0).locked_min_iva_freq = 0;
	per_cpu(ds_sys_status, 0).locked_min_l3_freq = 0;

	per_cpu(ds_sys_status, 0).flag_touch_timeout_count = 0;
	per_cpu(ds_sys_status, 0).touch_timeout_sec = 0;
	per_cpu(ds_sys_status, 0).touch_timeout_usec = 0;

	per_cpu(ds_sys_status, 0).flag_consecutive_touches = 0;
	per_cpu(ds_sys_status, 0).new_touch_sec = 0;
	per_cpu(ds_sys_status, 0).new_touch_usec = 0;
	per_cpu(ds_sys_status, 0).first_consecutive_touch_sec = 0;
	per_cpu(ds_sys_status, 0).flag_long_consecutive_touches = 0;

	per_cpu(ds_sys_status, 0).flag_post_early_suspend = 0;
	per_cpu(ds_sys_status, 0).flag_do_post_early_suspend = 0;
	per_cpu(ds_sys_status, 0).do_post_early_suspend_sec = 0;

	return(0);
}

/*====================================================================
	The functions to initialize dvs_suite_cpu_status structure.
	====================================================================*/

int ds_initialize_ds_cpu_status(int ds_cpu, int cpu_mode){

	per_cpu(ds_cpu_status, ds_cpu).cpu_mode = cpu_mode;
	per_cpu(ds_cpu_status, ds_cpu).dvs_suite_mutex = 0;

	per_cpu(ds_cpu_status, ds_cpu).flag_update_cpu_op = 0;
	per_cpu(ds_cpu_status, ds_cpu).cpu_op_mutex = 0;

	per_cpu(ds_cpu_status, ds_cpu).current_cpu_op_index = DS_CPU_OP_INDEX_INI;
	per_cpu(ds_cpu_status, ds_cpu).target_cpu_op_index = DS_CPU_OP_INDEX_INI;
	per_cpu(ds_cpu_status, ds_cpu).cpu_op_last_update_sec = 0;
	per_cpu(ds_cpu_status, ds_cpu).cpu_op_last_update_usec = 0;

	return(0);
}

/*====================================================================
	The functions to initialize dvs_suite_counter structure.
	====================================================================*/

int ds_initialize_ds_counter(int ds_cpu, int mode){

	if(mode == 0) per_cpu(ds_counter, ds_cpu).flag_counter_initialized = 0;
	per_cpu(ds_counter, ds_cpu).counter_mutex = 0;

	if(mode == 0){
		per_cpu(ds_counter, ds_cpu).wall_usec_base = 0;
		per_cpu(ds_counter, ds_cpu).idle_usec_base = 0;
		per_cpu(ds_counter, ds_cpu).iowait_usec_base = 0;

		per_cpu(ds_counter, ds_cpu).elapsed_sec = 0;
		per_cpu(ds_counter, ds_cpu).elapsed_usec = 0;

		per_cpu(ds_counter, ds_cpu).busy_sec = 0;
		per_cpu(ds_counter, ds_cpu).busy_usec = 0;
	}
	else{
		per_cpu(ds_counter, ds_cpu).wall_usec_base = per_cpu(ds_counter, 0).wall_usec_base;
		per_cpu(ds_counter, ds_cpu).idle_usec_base = per_cpu(ds_counter, 0).idle_usec_base;
		per_cpu(ds_counter, ds_cpu).iowait_usec_base = per_cpu(ds_counter, 0).iowait_usec_base;

		per_cpu(ds_counter, ds_cpu).elapsed_sec = per_cpu(ds_counter, 0).elapsed_sec;
		per_cpu(ds_counter, ds_cpu).elapsed_usec = per_cpu(ds_counter, 0).elapsed_usec;

		per_cpu(ds_counter, ds_cpu).busy_sec = per_cpu(ds_counter, 0).busy_sec;
		per_cpu(ds_counter, ds_cpu).busy_usec = per_cpu(ds_counter, 0).busy_usec;
	}

	return(0);
}

/*====================================================================
	The functions to initialize ds_aidvs_status structure.
	====================================================================*/

int ds_initialize_aidvs(int ds_cpu, int mode){

	per_cpu(ds_aidvs_status, ds_cpu).moving_avg_weight = ds_control.aidvs_moving_avg_weight;
	per_cpu(ds_aidvs_status, ds_cpu).interval_length = ds_control.aidvs_interval_length;

	if(mode == 0){
		per_cpu(ds_aidvs_status, ds_cpu).time_usec_interval = 0;
		per_cpu(ds_aidvs_status, ds_cpu).time_sec_interval_inc_base =
			per_cpu(ds_counter, ds_cpu).elapsed_sec;
		per_cpu(ds_aidvs_status, ds_cpu).time_usec_interval_inc_base =
			per_cpu(ds_counter, ds_cpu).elapsed_usec;

		per_cpu(ds_aidvs_status, ds_cpu).time_usec_work = 0;
		per_cpu(ds_aidvs_status, ds_cpu).time_sec_work_inc_base =
			per_cpu(ds_counter, ds_cpu).busy_sec;
		per_cpu(ds_aidvs_status, ds_cpu).time_usec_work_inc_base =
			per_cpu(ds_counter, ds_cpu).busy_usec;
	}
	else{
		per_cpu(ds_aidvs_status, ds_cpu).time_usec_interval =
			per_cpu(ds_aidvs_status, 0).time_usec_interval;
		per_cpu(ds_aidvs_status, ds_cpu).time_sec_interval_inc_base =
			per_cpu(ds_aidvs_status, 0).time_sec_interval_inc_base;
		per_cpu(ds_aidvs_status, ds_cpu).time_usec_interval_inc_base =
			per_cpu(ds_aidvs_status, 0).time_usec_interval_inc_base;

		per_cpu(ds_aidvs_status, ds_cpu).time_usec_work =
			per_cpu(ds_aidvs_status, 0).time_usec_work;
		per_cpu(ds_aidvs_status, ds_cpu).time_sec_work_inc_base =
			per_cpu(ds_aidvs_status, 0).time_sec_work_inc_base;
		per_cpu(ds_aidvs_status, ds_cpu).time_usec_work_inc_base =
			per_cpu(ds_aidvs_status, 0).time_usec_work_inc_base;
	}

	per_cpu(ds_aidvs_status, ds_cpu).utilization_int_ulong = 1;
	per_cpu(ds_aidvs_status, ds_cpu).utilization_fra_fp12 = 0x0;

	per_cpu(ds_aidvs_status, ds_cpu).moving_avg_int_ulong = 1;
	per_cpu(ds_aidvs_status, ds_cpu).moving_avg_fra_fp12 = 0x0;

	per_cpu(ds_aidvs_status, ds_cpu).cpu_op_index = DS_CPU_OP_INDEX_INI;

	return(0);
}

/*====================================================================
	Function to change the priority of normal tasks
	NOTE: We apply the RR rt scheduler for HRT and DS_SRT_UI_SERVER_TASK.
		  On the other hand, we apply the normal scheduler for
		  other tasks.
	====================================================================*/

int ds_update_priority_normal(int ds_cpu, struct task_struct *p){

	int lc_existing_nice = 0;
	int lc_nice_by_type = 0;
	int lc_static_prio_by_type = 0;
	int lc_resultant_nice = 0;
	int lc_resultant_static_prio = 0;

	if(p == 0 || p->pid == 0) return(0);
	if((per_cpu(ds_sys_status, 0).type[p->pid] & DS_TYPE_2B_CHANGED_M) == 0) return(0);

	lc_existing_nice = p->static_prio - 120;

	switch(per_cpu(ds_sys_status, 0).type[p->pid] & DS_TYPE_M){
		case DS_HRT_TASK:
			lc_nice_by_type = DS_HRT_NICE;
			lc_static_prio_by_type = DS_HRT_STATIC_PRIO;
			break;
		case DS_SRT_UI_SERVER_TASK:
			lc_nice_by_type = DS_DBSRT_NICE;
			lc_static_prio_by_type = DS_DBSRT_STATIC_PRIO;
			break;
		case DS_SRT_UI_CLIENT_TASK:
			lc_nice_by_type = DS_DBSRT_NICE;
			lc_static_prio_by_type = DS_DBSRT_STATIC_PRIO;
			break;
		case DS_SRT_KERNEL_THREAD:
			lc_nice_by_type = DS_RBSRT_NICE;
			lc_static_prio_by_type = DS_RBSRT_STATIC_PRIO;
			break;
		case DS_SRT_DAEMON_TASK:
			lc_nice_by_type = DS_RBSRT_NICE;
			lc_static_prio_by_type = DS_RBSRT_STATIC_PRIO;
			break;
		case DS_NRT_TASK:
			lc_nice_by_type = DS_NRT_NICE;
			lc_static_prio_by_type = DS_NRT_STATIC_PRIO;
			break;
		default:
			lc_nice_by_type = DS_NRT_NICE;
			lc_static_prio_by_type = DS_NRT_STATIC_PRIO;
			break;
	}

	lc_resultant_nice = lc_nice_by_type + lc_existing_nice;
	if(lc_resultant_nice < -20) lc_resultant_nice = -20;
	if(lc_resultant_nice > 19) lc_resultant_nice = 19;

#if 1
	lc_resultant_static_prio = lc_static_prio_by_type + lc_existing_nice;
	if(lc_resultant_static_prio < 100) lc_resultant_static_prio = 100;
	if(lc_resultant_static_prio > 139) lc_resultant_static_prio = 139;

	p->static_prio = lc_resultant_static_prio;
	p->normal_prio = p->static_prio;
	p->prio = p->static_prio;
#endif

	set_user_nice(p, lc_resultant_nice);

	per_cpu(ds_sys_status, 0).type[p->pid] &= DS_TYPE_2B_CHANGED_N;

	return(0);
}

/*====================================================================
	Function to trace every task's type
	====================================================================*/
int ds_detect_task_type(int ds_cpu){

	int old_type = 0;
	int new_type = 0;
	int i = 0;

	int next_pid;

	/* Upon ds_parameter.entry_type == DS_ENTRY_SWITCH_TO, do followings.

		 For ds_parameter.next_p:

		(1) Check if per_cpu(ds_sys_status, 0).type[next_pid] & DS_TYPE_M is still its initial value.
			If it is, determine it.
	 */
	if(ds_parameter.entry_type == DS_ENTRY_SWITCH_TO)
	{

		/* For prev_p */
#if 0
		if(ds_parameter.prev_p != 0){
			if(ds_parameter.prev_p->pid != 0){
				// Nothing to do now.
			}
		}
#endif

		/* For next_p */
		if(ds_parameter.next_p != 0)
		{

			next_pid = ds_parameter.next_p->pid;

			if(next_pid != 0){

				if(next_pid == ds_parameter.next_p->tgid)
				{
					for(i=0;i<16;i++)
						per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid][i] = 
						*(ds_parameter.next_p->comm+i);
				}

				/* Get old type. */
				old_type = per_cpu(ds_sys_status, 0).type[next_pid] & DS_TYPE_M;
				if(old_type == 0) old_type = DS_NRT_TASK;

				/* C) Initialize and then determine new type. */
				new_type = old_type;

				/*----------------------------------------------------------------------
				 * Manual type detection
				 ----------------------------------------------------------------------*/
				switch(*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+0)){
					case 'a':
#if 0	// youtube. [application process].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'n' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'd' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 'r' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'd' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'y' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'u' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 't' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'u' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'b' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'e'
						)
						{
							//new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							//new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'b':
#if 0	// /system/bin/dbus-daemon. bluetooth related [daemon process].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'i' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'n' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == '/' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'd' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'b' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'u' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 's' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == '-' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'd' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'a' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'm' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'n'
						)
						{
							new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							//new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// /system/bin/mediaserver. [daemon process].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'n' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == '/' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'm' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'd' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'a' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'r' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'v' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'r'
						)
						{
							//new_type = DS_HRT_TASK;
							new_type = DS_SRT_UI_SERVER_TASK;
							//new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'c':
#if 1	// com.lge.camera. LGE On-Screen Phone [application]. O 4 touch.
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'o' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'm' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == '.' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'l' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'g' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'e' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == '.' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'c' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'a' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'm' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'r' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'a'
						)
						{
							//new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.lge.media. [application].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'o' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'm' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == '.' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'l' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'g' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'e' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == '.' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'm' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'e' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'd' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'a'
						)
						{
							//new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.broadcom.bt.app.pbap. bluetooth. [application].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'm' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'b' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 't' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 'a' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'p' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'p' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'p' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'b' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'a' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'p'
						)
						{
							new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							//new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'd':
#if 1	// com.android.lgecamera. [application].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'r' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'o' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 'i' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'd' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == '.' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'l' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 'g' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'e' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'c' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'a' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'm' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'r' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'a'
						)
						{
							//new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else 
#endif
#if 0	// android.process.media. [application].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == '.' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'p' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 'r' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'o' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'c' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'e' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 's' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 's' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == '.' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'm' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'd' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'a'
						)
						{
							//new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else 
#endif
#if 0	// com.android.bluetooth. [application].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'r' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'o' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 'i' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'd' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == '.' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'b' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 'l' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'u' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'e' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 't' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 't' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'h'
						)
						{
							new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							//new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 1	// android.process.acore. Contact, home, and etc. [application].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == '.' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'p' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 'r' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'o' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'c' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'e' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 's' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 's' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == '.' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'a' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'c' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'r' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'e'
						)
						{
							//new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_SRT_KERNEL_TASK;
							//new_type = DS_NRT_TASK;
#if 0
if(per_cpu(ds_sys_status, 0).flag_touch_timeout_count != 0){
printk(KERN_DEBUG "\n");
}
#endif
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'e':
#if 0	// com.google.process.gapps. [application]. Google services.
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == '.' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'p' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 'r' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'o' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'c' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'e' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 's' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 's' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == '.' && 
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'g' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'a' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'p' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'p' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 's'
						)
						{
							//new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							//new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'f':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'g':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'h':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'i':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'j':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'k':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'l':
#if 0	// com.lge.videoplayer. [application process].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'g' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'v' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'd' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'p' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'l' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'a' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'y' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'r'
						)
						{
							//new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'm':
#if 0	// com.lge.launcher2. [application process].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'l' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 'g' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'l' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 'a' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'u' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'n' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'c' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'h' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'r' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == '2'
						)
						{
							//new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							//per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.android.phone. [application].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'a' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 'n' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'd' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'r' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'd' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'p' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'h' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'n' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'e'
						)
						{
							new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							//new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.android.music. [application].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'a' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 'n' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'd' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'r' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'd' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'm' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'u' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'c'
						)
						{
							//new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.broadcom.bt.app.system. bluetooth. [application].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'b' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 't' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'a' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'p' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 'p' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'y' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 't' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'm'
						)
						{
							new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							//new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'n':
#if 0	// com.android.systemui. [application].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'd' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'r' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'd' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'y' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 't' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'm' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'u' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'i'
						)
						{
							//new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'o':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'p':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'q':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'r':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 's':
#if 1	// system_server. Android system service [application].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'y' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 't' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'm' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == '_' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'r' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'v' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'r'
						)
						{
							//new_type = DS_HRT_TASK;
							new_type = DS_SRT_UI_SERVER_TASK;
							//new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// /system/bin/netd. Network related [daemon process].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'y' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 't' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'm' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == '/' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 'b' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'n' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == '/' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'n' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 't' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'd'
						)
						{
							new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							//new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// /system/bin/rild. RIL related [daemon process].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'y' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 't' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'm' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == '/' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 'b' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'n' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == '/' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'r' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'l' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'd'
						)
						{
							new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							//new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// /system/bin/vold. Volume (sdcard etc.) server [daemon process].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'y' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 't' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'm' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == '/' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 'b' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'n' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == '/' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'v' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'l' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'd'
						)
						{
							new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							//new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 't':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'u':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'v':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'w':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'x':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'y':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'z':
#if 0	// zygote. Android process spawning [daemon process].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'y' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'g' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 't' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'e'
						)
						{
							//new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							//new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'A':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'B':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'C':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'D':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'E':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'F':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'G':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'H':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'I':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'J':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'K':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'L':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'M':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'N':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'O':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'P':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'Q':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'R':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'S':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'T':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'U':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'V':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'W':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'X':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'Y':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case 'Z':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case ' ':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case '_':
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case '/':
#if 0	// /system/bin/servicemanager. Android system service [daemon process]
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 'r' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'v' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'c' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'm' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'a' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'n' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'a' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'g' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'r'
						)
						{
							//new_type = DS_HRT_TASK;
							new_type = DS_SRT_UI_SERVER_TASK;
							//new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// /sbin/ueventd. Polling server [daemon process].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'b' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'n' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == '/' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'u' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 'v' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'n' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 't' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'd'
						)
						{
							new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							//new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						goto DS_UNKNOWN_TASK_NAME;
						break;
					case '.':
#if 0	// android.process.lghome. [application].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'p' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'r' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'c' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == 'l' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'g' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'h' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'm' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'e'
						)
						{
							//new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
#if 0	// com.cooliris.media. Gallary related. [application].
						if(
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+1) == 'c' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+2) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+3) == 'o' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+4) == 'l' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+5) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+6) == 'r' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+7) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+8) == 's' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+9) == '.' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+10) == 'm' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+11) == 'e' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+12) == 'd' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+13) == 'i' &&
							*(per_cpu(ds_sys_status, 0).tg_owner_comm[next_pid]+14) == 'a'
						)
						{
							//new_type = DS_HRT_TASK;
							//new_type = DS_SRT_UI_SERVER_TASK;
							new_type = DS_SRT_UI_CLIENT_TASK;
							//new_type = DS_SRT_KERNEL_THREAD;
							//new_type = DS_SRT_DAEMON_TASK;
							//new_type = DS_NRT_TASK;
							per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
							goto DS_TASK_TYPE_DETECTION_DONE;
						}
						else
#endif
						goto DS_UNKNOWN_TASK_NAME;
						break;
				}

DS_UNKNOWN_TASK_NAME:

				/*----------------------------------------------------------------------
				 * Autonomous type detection
				 ----------------------------------------------------------------------*/
#if 0
				if(next_pid == 1 ||
					next_pid == 2 ||
						ds_parameter.next_p->parent->pid == 2)
					{
						new_type = DS_SRT_KERNEL_THREAD;
					per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
						goto DS_TASK_TYPE_DETECTION_DONE;
					}
					else if(ds_parameter.next_p->parent->pid == 1){
						new_type = DS_SRT_DAEMON_TASK;
					}
					else{
						new_type = DS_NRT_TASK;
					}
#else	/* Apply this without autonomous type detection */
new_type = DS_NRT_TASK;
per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_FIXED_M;
goto DS_TASK_TYPE_DETECTION_DONE;
#endif

DS_TASK_TYPE_DETECTION_DONE:

				/* D) Check type change. */
				if(old_type != new_type){
					per_cpu(ds_sys_status, 0).type[next_pid] &= DS_TYPE_N;
					per_cpu(ds_sys_status, 0).type[next_pid] |= new_type;
					per_cpu(ds_sys_status, 0).type[next_pid] |= DS_TYPE_2B_CHANGED_M;
				}
			}
		}
	}

	return(0);
}

/*====================================================================
	The function which updates the CPU operating point.
	====================================================================*/
asmlinkage void ds_update_cpu_op(int ds_cpu)
{
	struct device *mpu_dev;	// Moved from cpu-omap.c
	struct device *l3_dev;	// Moved from cpu-omap.c
	struct device *iva_dev; // Moved from cpu-omap.c

	unsigned long lc_min_cpu_op_update_interval = 0;

	if(!cpu_active(ds_cpu)) return;
	if(!ds_control.flag_run_dvs) return;
	if(!per_cpu(ds_cpu_status, ds_cpu).flag_update_cpu_op) return;
	if(per_cpu(ds_cpu_status, ds_cpu).cpu_op_mutex) return;
	per_cpu(ds_cpu_status, ds_cpu).cpu_op_mutex ++;

	if(per_cpu(ds_cpu_status, ds_cpu).target_cpu_op_index > 
		per_cpu(ds_cpu_status, ds_cpu).current_cpu_op_index)
				lc_min_cpu_op_update_interval = DS_MIN_CPU_OP_UPDATE_INTERVAL_U;
			else
				lc_min_cpu_op_update_interval = DS_MIN_CPU_OP_UPDATE_INTERVAL_D;

			/* If this is the first time */
	if(per_cpu(ds_cpu_status, ds_cpu).cpu_op_last_update_sec == 0 && 
		per_cpu(ds_cpu_status, ds_cpu).cpu_op_last_update_usec == 0)
	{
				goto update_cpu_op;
			}
			else{
				/* If lc_min_cpu_op_update_interval is over since the last update */
		if(per_cpu(ds_counter, ds_cpu).elapsed_sec > 
			per_cpu(ds_cpu_status, ds_cpu).cpu_op_last_update_sec + 1)
		{
					goto update_cpu_op;
				}
		else if(per_cpu(ds_counter, ds_cpu).elapsed_sec > 
			per_cpu(ds_cpu_status, ds_cpu).cpu_op_last_update_sec)
		{
			if(per_cpu(ds_counter, ds_cpu).elapsed_usec > 
				per_cpu(ds_cpu_status, ds_cpu).cpu_op_last_update_usec)
			{
						goto update_cpu_op;
					}
			else if(per_cpu(ds_counter, ds_cpu).elapsed_usec + 1000000
				> per_cpu(ds_cpu_status, ds_cpu).cpu_op_last_update_usec + 
				lc_min_cpu_op_update_interval)
			{
						goto update_cpu_op;
					}
			else{
						goto do_not_update;
				}
		}
				else{
			if(per_cpu(ds_counter, ds_cpu).elapsed_usec
				> per_cpu(ds_cpu_status, ds_cpu).cpu_op_last_update_usec + 
				lc_min_cpu_op_update_interval)
			{
						goto update_cpu_op;
					}
			else{
						goto do_not_update;
				}
			}
		}

update_cpu_op:

		mpu_dev = omap2_get_mpuss_device();
		l3_dev = omap2_get_l3_device();
		iva_dev = omap2_get_iva_device();

		switch(per_cpu(ds_cpu_status, ds_cpu).target_cpu_op_index){
			case DS_CPU_OP_INDEX_0:
				omap_pm_cpu_set_freq(1000000000);	// VDD1_OPP4	1GHz
				omap_device_set_rate(mpu_dev, mpu_dev, 1000000000);
				if(per_cpu(ds_sys_status, 0).locked_min_iva_freq == 0){
				omap_device_set_rate(iva_dev, iva_dev, 800000000);
				}
				else{
					omap_device_set_rate(iva_dev, iva_dev, per_cpu(ds_sys_status, 0).locked_min_iva_freq);
				}
				if(per_cpu(ds_sys_status, 0).locked_min_l3_freq == 0){
					if(per_cpu(ds_sys_status, 0).flag_do_post_early_suspend == 0)
						omap_device_set_rate(l3_dev, l3_dev, 200000000);
					else
						omap_device_set_rate(l3_dev, l3_dev, 100000000);
				}
				else{
					omap_device_set_rate(l3_dev, l3_dev, per_cpu(ds_sys_status, 0).locked_min_l3_freq);
				}
				break;
			case DS_CPU_OP_INDEX_1:
				omap_pm_cpu_set_freq(800000000);	// VDD1_OPP3	800MHz
				omap_device_set_rate(mpu_dev, mpu_dev, 800000000);
				if(per_cpu(ds_sys_status, 0).locked_min_iva_freq == 0){
				omap_device_set_rate(iva_dev, iva_dev, 660000000);
				}
				else{
					omap_device_set_rate(iva_dev, iva_dev, per_cpu(ds_sys_status, 0).locked_min_iva_freq);
				}
				if(per_cpu(ds_sys_status, 0).locked_min_l3_freq == 0){
					if(per_cpu(ds_sys_status, 0).flag_do_post_early_suspend == 0)
						omap_device_set_rate(l3_dev, l3_dev, 200000000);
					else
						omap_device_set_rate(l3_dev, l3_dev, 100000000);
				}
				else{
					omap_device_set_rate(l3_dev, l3_dev, per_cpu(ds_sys_status, 0).locked_min_l3_freq);
				}
				break;
			case DS_CPU_OP_INDEX_2:
				omap_pm_cpu_set_freq(600000000);	// VDD1_OPP2	600MHz
				omap_device_set_rate(mpu_dev, mpu_dev, 600000000);
				if(per_cpu(ds_sys_status, 0).locked_min_iva_freq == 0){
				omap_device_set_rate(iva_dev, iva_dev, 520000000);
				}
				else{
					omap_device_set_rate(iva_dev, iva_dev, per_cpu(ds_sys_status, 0).locked_min_iva_freq);
				}
				if(per_cpu(ds_sys_status, 0).locked_min_l3_freq == 0){
					if(per_cpu(ds_sys_status, 0).flag_do_post_early_suspend == 0)
						omap_device_set_rate(l3_dev, l3_dev, 200000000);
					else
						omap_device_set_rate(l3_dev, l3_dev, 100000000);
				}
				else{
					omap_device_set_rate(l3_dev, l3_dev, per_cpu(ds_sys_status, 0).locked_min_l3_freq);
				}
				break;
			case DS_CPU_OP_INDEX_3:
			default:
				omap_pm_cpu_set_freq(300000000);	// VDD1_OPP1	300MHz
				omap_device_set_rate(mpu_dev, mpu_dev, 300000000);
				if(per_cpu(ds_sys_status, 0).locked_min_iva_freq == 0){
				omap_device_set_rate(iva_dev, iva_dev, 260000000);
				}
				else{
					omap_device_set_rate(iva_dev, iva_dev, per_cpu(ds_sys_status, 0).locked_min_iva_freq);
				}
				if(per_cpu(ds_sys_status, 0).locked_min_l3_freq == 0){
#if 0
					if(per_cpu(ds_sys_status, 0).flag_do_post_early_suspend == 0)
						omap_device_set_rate(l3_dev, l3_dev, 200000000);
					else
#endif
						omap_device_set_rate(l3_dev, l3_dev, 100000000);
				}
				else{
					omap_device_set_rate(l3_dev, l3_dev, per_cpu(ds_sys_status, 0).locked_min_l3_freq);
				}
				break;
		}

	per_cpu(ds_cpu_status, ds_cpu).current_cpu_op_index = 
		per_cpu(ds_cpu_status, ds_cpu).target_cpu_op_index;

	per_cpu(ds_cpu_status, ds_cpu).cpu_op_last_update_sec = 
		per_cpu(ds_counter, ds_cpu).elapsed_sec;
	per_cpu(ds_cpu_status, ds_cpu).cpu_op_last_update_usec = 
		per_cpu(ds_counter, ds_cpu).elapsed_usec;

	per_cpu(ds_cpu_status, ds_cpu).flag_update_cpu_op = 0;

do_not_update:

	per_cpu(ds_cpu_status, ds_cpu).cpu_op_mutex --;

	return;
}
EXPORT_SYMBOL(ds_update_cpu_op);

/*====================================================================
	The functions which up or down the auxiliary cores.
	====================================================================*/
#if 0	// Need CONFIG_HOTPLUG_CPU
void ds_up_aux_cpu(int ds_cpu, int cpu){

	if(cpu == 0) return;

	if(ds_cpu == 0 && !cpu_active(cpu)){
		cpu_hotplug_driver_lock();
		cpu_up(cpu);
		cpu_hotplug_driver_unlock();
	}

	return;
}

void ds_down_aux_cpu(int ds_cpu, int cpu){

	if(cpu == 0) return;

	if(ds_cpu == 0 && cpu_active(cpu)){
		cpu_hotplug_driver_lock();
		cpu_down(cpu);
		cpu_hotplug_driver_unlock();
	}

	return;
}
#endif

/*====================================================================
	The main dynamic voltage scaling and
	performance evaluation kernel function.
	====================================================================*/

/* This function is called at the end of context_swtich()
 * and update_process_times().
	 */
void do_dvs_suite(int ds_cpu){

	unsigned int lc_target_cpu_op_index = 0;

	if(per_cpu(ds_cpu_status, ds_cpu).dvs_suite_mutex != 0) return;
	per_cpu(ds_cpu_status, ds_cpu).dvs_suite_mutex ++;

	per_cpu(ds_cpu_status, ds_cpu).cpu_mode = DS_CPU_MODE_DVS_SUITE;

	/* dvs_suite has been activated. */
	if(ds_control.flag_run_dvs == 1)
	{
		/* Delayed application of the special treatment upon early suspend */
		if(ds_cpu == 0){
			if(per_cpu(ds_sys_status, 0).flag_post_early_suspend == 1){
				if(per_cpu(ds_sys_status, 0).flag_do_post_early_suspend == 0){
					if(per_cpu(ds_sys_status, 0).do_post_early_suspend_sec <
						per_cpu(ds_counter, ds_cpu).elapsed_sec)
					{
						per_cpu(ds_sys_status, 0).flag_do_post_early_suspend = 1;
					}
		}
		}
	}

		/* Detect task type, set task priority, and determine cpu frequency */
		if(ds_parameter.entry_type == DS_ENTRY_SWITCH_TO)
		{
			if((per_cpu(ds_sys_status, 0).type[ds_parameter.next_p->pid] & 
				DS_TYPE_FIXED_M) == 0)
			{
				ds_detect_task_type(ds_cpu);
			}
		}
		else{	// DS_ENTRY_TIMER_IRQ

			/* Check if consecutive touches have been ended - Begin */
			if(ds_cpu == 0){
				if(per_cpu(ds_sys_status, 0).flag_consecutive_touches == 1){
					if((per_cpu(ds_counter, ds_cpu).elapsed_sec - 
						per_cpu(ds_sys_status, 0).new_touch_sec) * 1000000 +
						(per_cpu(ds_counter, ds_cpu).elapsed_usec - 
						per_cpu(ds_sys_status, 0).new_touch_usec) > DS_CONT_TOUCH_THRESHOLD_USEC)
					{
						per_cpu(ds_sys_status, 0).flag_consecutive_touches = 0;
						per_cpu(ds_sys_status, 0).flag_long_consecutive_touches = 0;
		}
		}
	}
			/* Check if consecutive touches have been ended - End */

			ds_do_dvs_gpschedvs(ds_cpu, &lc_target_cpu_op_index);

			/* Schedule the actual CPU frequency and voltage changes. */
			if(lc_target_cpu_op_index != per_cpu(ds_cpu_status, ds_cpu).current_cpu_op_index)
			{
				per_cpu(ds_cpu_status, ds_cpu).flag_update_cpu_op = 1;
				per_cpu(ds_cpu_status, ds_cpu).target_cpu_op_index = lc_target_cpu_op_index;
			}

			/* Control the auxiliary cores - Begin */
#if 0
			if(ds_cpu == 0){
				if(per_cpu(ds_sys_status, 0).flag_long_consecutive_touches == 1)
					ds_down_aux_cpu(ds_cpu, 1);
	}
#endif
			/* Control the auxiliary cores - End */
		}
	}

	/* dvs_suite has not been activated yet. */
	else{

		/* Delayed activation of LG-DVFS */
		if(ds_cpu == 0){
			if(per_cpu(ds_counter, ds_cpu).elapsed_sec > DS_INIT_DELAY_SEC){
				ds_control.flag_run_dvs = 1;
				printk(KERN_INFO "[LG-DVFS] LG-DVFS starts running.\n");
		}
	}
	}

	if(ds_parameter.entry_type == DS_ENTRY_TIMER_IRQ)
	{
		if(current->pid == 0)
			per_cpu(ds_cpu_status, ds_cpu).cpu_mode = DS_CPU_MODE_IDLE;
		else
			per_cpu(ds_cpu_status, ds_cpu).cpu_mode = DS_CPU_MODE_TASK;
	}
	else{	// DS_ENTRY_SWITCH_TO
		per_cpu(ds_cpu_status, ds_cpu).cpu_mode = DS_CPU_MODE_SCHEDULE;
	}

	per_cpu(ds_cpu_status, ds_cpu).dvs_suite_mutex --;

	return;
}

void do_dvs_suite_timer(struct work_struct *work){

	int ds_cpu = smp_processor_id();

	ds_update_cpu_op(ds_cpu);

	return;
}

inline void dvs_suite_timer_init(void){

	INIT_WORK(&dvs_suite_work, do_dvs_suite_timer);

	return;
}

inline void dvs_suite_timer_exit(void){

	cancel_work_sync(&dvs_suite_work);

	return;
}

static int __init lg_dvfs_init(void)
{
	printk(KERN_WARNING "lg_dvfs_init\n");

	return 0;
}

static void __exit lg_dvfs_exit(void)
{
	printk(KERN_WARNING "lg_dvfs_exit\n");
}

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_SUPPORTED_DEVICE("cpu");
//MODULE_LICENSE("GPL");

module_init(lg_dvfs_init);
module_exit(lg_dvfs_exit);
