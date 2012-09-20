/*
 * arch/arm/mach-omap2/omap_ion.h
 *
 * Copyright (C) 2011 Texas Instruments
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _OMAP_ION_H
#define _OMAP_ION_H


// LGE_CHANGE_S kibum.lee@lge.com 512M settings
/*
 ___      _______    _______  __   __  _______  _______  _______    _______  ____   _______  __   __   
|   |    |       |  |       ||  |_|  ||   _   ||       ||       |  |       ||    | |       ||  |_|  |  
|   |    |    ___|  |   _   ||       ||  |_|  ||    _  ||___    |  |   ____| |   | |____   ||       |  
|   |    |   | __   |  | |  ||       ||       ||   |_| | ___|   |  |  |____  |   |  ____|  ||       |  
|   |___ |   ||  |  |  |_|  ||       ||       ||    ___||___    |  |_____  | |   | | ______||       |  
|       ||   |_| |  |       || ||_|| ||   _   ||   |     ___|   |   _____| | |   | | |_____ | ||_|| |  
|_______||_______|  |_______||_|   |_||__| |__||___|    |_______|  |_______| |___| |_______||_|   |_|  

 _______  _______  _______  _______  ___   __    _  _______  _______ 
|       ||       ||       ||       ||   | |  |  | ||       ||       |
|  _____||    ___||_     _||_     _||   | |   |_| ||    ___||  _____|
| |_____ |   |___   |   |    |   |  |   | |       ||   | __ | |_____ 
|_____  ||    ___|  |   |    |   |  |   | |  _    ||   ||  ||_____  |
 _____| ||   |___   |   |    |   |  |   | | | |   ||   |_| | _____| |
|_______||_______|  |___|    |___|  |___| |_|  |__||_______||_______|


------------------------------------- 0XA0000000(2G+512M) // End Addr
|         PHYS_ADDR_SMC(3M)         |
-------------------------------------
|     OMAP3_ION(100M) -> (30M)      |
-------------------------------------
|/////// free 67M -> 137M  /////////| 
-------------------------------------
|        OMAP3_RAMCONSOL (2M)       |
-------------------------------------
|////////// Android 340M ///////////|
------------------------------------- 0x80000000(2G) // Start Addr
*/
#if defined(CONFIG_MACH_LGE_HUB) 

#define OMAP_RAM_CONSOLE_START	(PLAT_PHYS_OFFSET + SZ_1M * 340)
#define OMAP_RAM_CONSOLE_SIZE	SZ_2M

//#define OMAP3_ION_HEAP_CARVEOUT_INPUT_SIZE       (SZ_1M * 100)
//#define OMAP3_ION_HEAP_CARVEOUT_INPUT_SIZE       (SZ_1M * 30)
#define OMAP3_ION_HEAP_CARVEOUT_INPUT_SIZE       (SZ_1M * 35)			// youmi.jun@lge.com , workaround: increase size for ARTTOOL test (30MB -> 35MB)

#define PHYS_ADDR_SMC_SIZE	(SZ_1M * 3)
#define PHYS_ADDR_SMC_MEM	(0x80000000 + SZ_1G - PHYS_ADDR_SMC_SIZE)
#define OMAP3_PHYS_ADDR_SMC_MEM		(0x80000000 + SZ_512M - PHYS_ADDR_SMC_SIZE)
#else	//CONFIG_MACH_LGE_HUB

#define OMAP4_ION_HEAP_SECURE_INPUT_SIZE	(SZ_1M * 90)
#define OMAP4_ION_HEAP_TILER_SIZE		(SZ_128M - SZ_32M)
#define OMAP4_ION_HEAP_NONSECURE_TILER_SIZE	SZ_32M

//#define OMAP3_ION_HEAP_CARVEOUT_INPUT_SIZE       (SZ_1M * 100)
//#define OMAP3_ION_HEAP_CARVEOUT_INPUT_SIZE       (SZ_1M * 30)
#define OMAP3_ION_HEAP_CARVEOUT_INPUT_SIZE       (SZ_1M * 35)			// youmi.jun@lge.com , workaround: increase size for ARTTOOL test (30MB -> 35MB)

#define PHYS_ADDR_SMC_SIZE	(SZ_1M * 3)
#define PHYS_ADDR_SMC_MEM	(0x80000000 + SZ_1G - PHYS_ADDR_SMC_SIZE)
#define OMAP3_PHYS_ADDR_SMC_MEM		(0x80000000 + SZ_512M - PHYS_ADDR_SMC_SIZE)
#define PHYS_ADDR_DUCATI_SIZE	(SZ_1M * 105)
#define PHYS_ADDR_DUCATI_MEM	(PHYS_ADDR_SMC_MEM - PHYS_ADDR_DUCATI_SIZE - \
				OMAP4_ION_HEAP_SECURE_INPUT_SIZE)

#endif	// CONFIG_MACH_LGE_HUB
// LGE_CHANGE_E kibum.lee@lge.com 512M settings


#ifdef CONFIG_ION_OMAP
void omap_ion_init(void);
void omap_register_ion(void);
#else
static inline void omap_ion_init(void) { return; }
static inline void omap_register_ion(void) { return; }
#endif

#endif
