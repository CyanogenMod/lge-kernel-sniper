/*
 * linux/kernel/irq/pm.c
 *
 * Copyright (C) 2011 LG Electronics Inc. 
 *
 * This file contains irq printing functions related to android wake up.
 */

#include <linux/module.h>

#define WAKEUP_IRQ_RECORD_COUNT 10

int wakeup_irq_record_index;
unsigned int wakeup_irq_record_array[WAKEUP_IRQ_RECORD_COUNT] = {0,};

/**
 * wakeup_irq_record_reset
 *
 */
void wakeup_irq_record_reset(void)
{
	unsigned int i = 0;

	wakeup_irq_record_index = 0;

	for (i = 0; i < WAKEUP_IRQ_RECORD_COUNT; i++) {
		wakeup_irq_record_array[i] = 0;
	}

	return;
}
EXPORT_SYMBOL_GPL(wakeup_irq_record_reset);

/**
 * wakeup_irq_record_one
 *
 */
void wakeup_irq_record_one(unsigned int irq)
{
	if (wakeup_irq_record_index < WAKEUP_IRQ_RECORD_COUNT) {
		if( irq != 56 ) /* i2c_omap */
			if( irq != 57 ) /* i2c_omap */
				if( irq != 61 ) /* i2c_omap */
					if( irq != 37 ) /* gp timer */
						if( irq != 86 ) /* mmc0 */
							if( irq != 83 ) /* mmc1 */
		wakeup_irq_record_array[wakeup_irq_record_index++] = irq;
	}

	return;
}
EXPORT_SYMBOL_GPL(wakeup_irq_record_one);

/**
 * wakeup_irq_record_print
 *
 */
void wakeup_irq_record_print(void)
{
	int i;

	printk("############## Checking irq");
	for (i = 0; i < WAKEUP_IRQ_RECORD_COUNT; i++) {
		printk(", %d", wakeup_irq_record_array[i]);
	}
	printk("\n");

	return;
}
EXPORT_SYMBOL_GPL(wakeup_irq_record_print);
