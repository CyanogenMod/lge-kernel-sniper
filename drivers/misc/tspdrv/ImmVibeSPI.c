/*
** =========================================================================
** File:
**     ImmVibeSPI.c
**
** Description: 
**     Device-dependent functions called by Immersion TSP API
**     to control PWM duty cycle, amp enable/disable, save IVT file, etc...
**
** Portions Copyright (c) 2008-2009 Immersion Corporation. All Rights Reserved. 
**
** This file contains Original Code and/or Modifications of Original Code 
** as defined in and that are subject to the GNU Public License v2 - 
** (the 'License'). You may not use this file except in compliance with the 
** License. You should have received a copy of the GNU General Public License 
** along with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA or contact 
** TouchSenseSales@immersion.com.
**
** The Original Code and all software distributed under the License are 
** distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER 
** EXPRESS OR IMPLIED, AND IMMERSION HEREBY DISCLAIMS ALL SUCH WARRANTIES, 
** INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS 
** FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT. Please see 
** the License for the specific language governing rights and limitations 
** under the License.
** =========================================================================
*/

#ifdef 	IMMVIBESPIAPI
#undef 	IMMVIBESPIAPI
#endif
#define 	IMMVIBESPIAPI static

#include <linux/platform_device.h>
#include <mach/gpio.h>
//#include <linux/delay.h>
//#include "../staging/android/timed_output.h"
//#include <linux/hrtimer.h>
#include <plat/dmtimer.h>
#include <linux/regulator/consumer.h>

/* This SPI supports only one actuator. */
#define 	NUM_ACTUATORS   	1

#define 	VIB_DEBUG 		1
#define     USE_SUBPM 

static bool g_bAmpEnabled = false;

#ifdef USE_SUBPM
#if defined(CONFIG_REGULATOR_LP8720)
#include <linux/regulator/lp8720.h>
#endif
#endif

#define HUB_VIBE_GPIO_EN			57
#define HUB_VIBE_PWM				56
#define HUB_VIBE_GPTIMER_NUM		10

#if 0
#define PWM_DUTY_MAX	 1158 /*1158 /* 22.43 kHz */
#endif
#define PWM_DUTY_MAX	 96336

#if 0
#define USE_SYS_CLK
#undef USE_32_CLK
#else
#define USE_32_CLK
#undef USE_SYS_CLK
#endif // if 0

#if 0
#define PWM_DUTY_MAX	 1158 /*1158 /* 22.43 kHz */
#else
#ifdef USE_32_CLK
#define PWM_DUTY_MAX	 0xAEFFFFFF//use 32k clock source ok
#else
#define PWM_DUTY_MAX	 0x387638//use sys clock 26MHz source
#endif // USE_32_CLK
#endif // if 0


#define PLTR_VALUE		(0xFFFFFFFF - PWM_DUTY_MAX)
#define PWM_DUTY_HALF	(0xFFFFFFFF - (PWM_DUTY_MAX >> 1))
static struct omap_dm_timer *omap_vibrator_timer = NULL;


#ifdef USE_SUBPM
#if defined(CONFIG_REGULATOR_LP8720)
extern void subpm_set_output(subpm_output_enum outnum, int onoff);
extern void subpm_output_enable(void);
#endif
#endif

static void hub_vibrator_gpio_enable (int enable)
{
	if (enable)
		gpio_set_value(HUB_VIBE_GPIO_EN, 1);
	else 	
		gpio_set_value(HUB_VIBE_GPIO_EN, 0);
}

static void hub_vibrator_LDO_enable(int val)
{

#ifdef USE_SUBPM
	subpm_set_output(1, val);
	subpm_output_enable();
#endif
}

static void vib_enable(int on )
{
	hub_vibrator_gpio_enable(on);
	hub_vibrator_LDO_enable(on);

	return VIBE_S_SUCCESS;
}

static void vib_generatePWM(int on)
{
	if(on) {
		/* Select clock */
		omap_dm_timer_enable(omap_vibrator_timer);

#ifdef USE_32_CLK
		omap_dm_timer_set_source(omap_vibrator_timer, OMAP_TIMER_SRC_32_KHZ);
#else
		omap_dm_timer_set_source(omap_vibrator_timer, OMAP_TIMER_SRC_SYS_CLK);
#endif

		/* set a period */
		omap_dm_timer_set_load(omap_vibrator_timer, 1, PLTR_VALUE);

		/* set a duty */
		omap_dm_timer_set_match(omap_vibrator_timer, 1, PWM_DUTY_HALF);
		omap_dm_timer_set_pwm(omap_vibrator_timer, 0, 1, OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE);
		//omap_dm_timer_set_pwm(omap_vibrator_timer, 1, 1, OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE);
		omap_dm_timer_start(omap_vibrator_timer);		
	}
	else {
		omap_dm_timer_stop(omap_vibrator_timer);
		omap_dm_timer_disable(omap_vibrator_timer);
	}

	return VIBE_S_SUCCESS;
}


/* Called to disable amp (disable output force) */
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_AmpDisable( VibeUInt8 nActuatorIndex )
{
	DbgOut(( "[ImmVibeSPI] : ImmVibeSPI_ForceOut_AmpDisable[%d]\n", g_bAmpEnabled ));
	//printk("[ImmVibeSPI] : ImmVibeSPI_ForceOut_AmpDisable[%d]\n", g_bAmpEnabled);
    	if ( g_bAmpEnabled ) {
        	g_bAmpEnabled = false;
		vib_enable(false);
		vib_generatePWM(false);
       }
	return VIBE_S_SUCCESS;
}


/* Called to enable amp (enable output force) */
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_AmpEnable( VibeUInt8 nActuatorIndex)
{
	DbgOut(( "[ImmVibeSPI] : ImmVibeSPI_ForceOut_AmpEnabled[%d]\n", g_bAmpEnabled ));
	if ( ! g_bAmpEnabled ) {
		g_bAmpEnabled = true;
		vib_generatePWM(true);
		vib_enable(true);
	}
   	return VIBE_S_SUCCESS;
}


/* Called at initialization time to set PWM freq, disable amp, etc... */
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_Initialize( void)
{
	int status = 0;
	int ret = 0;
	
   	DbgOut(( "[ImmVibeSPI] : ImmVibeSPI_ForceOut_Initialize\n" ));
	//g_bAmpEnabled = true;   /* to force ImmVibeSPI_ForceOut_AmpDisable disabling the amp */
	ret = gpio_request(HUB_VIBE_GPIO_EN, "Hub Vibrator Enable");
	if (ret < 0) {
		printk(KERN_ERR "%s: Failed to request GPIO_%d for vibrator\n", __func__, HUB_VIBE_GPIO_EN);
	}
	gpio_direction_output(HUB_VIBE_GPIO_EN, 0);

	omap_vibrator_timer = omap_dm_timer_request_specific(HUB_VIBE_GPTIMER_NUM);
	if (omap_vibrator_timer == NULL) {
		printk(KERN_ERR "%s: failed to request omap pwm timer.\n", __func__);
		ret = -ENODEV;
	}
	omap_dm_timer_disable(omap_vibrator_timer); 
   	//ImmVibeSPI_ForceOut_AmpDisable( 0 );

    return VIBE_S_SUCCESS;
}


/* Called at termination time to set PWM freq, disable amp, etc... */
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_Terminate( void )
{
   	DbgOut(( "[ImmVibeSPI] : ImmVibeSPI_ForceOut_Terminate\n" ));

    	ImmVibeSPI_ForceOut_AmpDisable(0);

    	return VIBE_S_SUCCESS;
}

bool bInTestMode = 0;
/*** Called by the real-time loop to set PWM duty cycle, and enable amp if required*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_Set( VibeUInt8 nActuatorIndex, VibeInt8 nForce )
{

#if 0 
	unsigned int nTmp;
	
	DbgOut(( "[ImmVibeSPI] ImmVibeSPI_ForceOut_Set nForce =  %d \n", nForce ));

	/* Check the Force value with Max and Min force value */
	if (nForce > 127) nForce = 127;
	if (nForce < -127) nForce = -127;
	
	if(bInTestMode)
		   {		   
			 if(nForce > 0 && nForce < 125) 
			 	nForce = 125; 
    	}
	if (nForce == 0) {
		hub_vibrator_gpio_enable(0);
		omap_dm_timer_stop(omap_vibrator_timer);		
	} else {
		hub_vibrator_gpio_enable(1);
		nTmp = 0xFFFFFFF7 - (((127 - nForce) * 9) >> 1);
		omap_dm_timer_set_match(omap_vibrator_timer, 1, nTmp);
		omap_dm_timer_start(omap_vibrator_timer);		
	}
#endif

	return VIBE_S_SUCCESS;
}


/*** Called by the real-time loop to set force output, and enable amp if required*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_SetSamples( VibeUInt8 nActuatorIndex, VibeUInt16 nOutputSignalBitDepth, VibeUInt16 nBufferSizeInBytes, VibeInt8* pForceOutputBuffer )
{
    VibeStatus status = VIBE_E_FAIL;

    /* nOutputSignalBitDepth should always be 8 */

    if (1 == nBufferSizeInBytes)
    {
        status = ImmVibeSPI_ForceOut_Set(nActuatorIndex, pForceOutputBuffer[0]);
    }
    else
    {
        /* Send 'nBufferSizeInBytes' bytes of data to HW */
        /* Will get here only if configured to handle Piezo actuators */
		printk( "[ImmVibeSPI] ImmVibeSPI_ForceOut_SetSamples nBufferSizeInBytes =  %d \n", nBufferSizeInBytes );
    }

    return status;
}


/*** Called to set force output frequency parameters*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_SetFrequency( VibeUInt8 nActuatorIndex, VibeUInt16 nFrequencyParameterID, VibeUInt32 nFrequencyParameterValue )
{
    /* This function is not called for ERM device */
	return VIBE_S_SUCCESS;
}


/*** Called to get the device name (device name must be returned as ANSI char)*/
IMMVIBESPIAPI VibeStatus ImmVibeSPI_Device_GetName( VibeUInt8 nActuatorIndex, char *szDevName, int nSize)
{
    return VIBE_S_SUCCESS;
}

