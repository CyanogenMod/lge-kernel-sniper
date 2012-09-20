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
#if defined(CONFIG_PRODUCT_LGE_LU6800)
#undef     USE_SUBPM
#else
#define     USE_SUBPM 
#endif

static bool g_bAmpEnabled = false;

#ifdef USE_SUBPM
#if defined(CONFIG_REGULATOR_LP8720)
#include <linux/regulator/lp8720.h>
#endif
#endif

#define HUB_VIBE_GPIO_EN			57
#define HUB_VIBE_PWM				56
#define HUB_VIBE_GPTIMER_NUM		10


/* LGE_CHANGE_S, ryu.seeyeol@lge.com, 2011-03-21, Change the PWM Clock Source (GB Source)*/
//[LGE_CHANGE_S] seungmoon.lee@lge.com, 2012-02-29, enable USE_SYS_CLK in ICS source(it was disabled in GB source)
#if 1
//[LGE_CHANGE_E] seungmoon.lee@lge.com, 2012-02-29, enable USE_SYS_CLK in ICS source(it was disabled in GB source)
#define USE_SYS_CLK
#undef USE_32_CLK
#else
#define USE_32_CLK
#undef USE_SYS_CLK
#endif // if 0

//[LGE_CHANGE_S] seungmoon.lee@lge.com, 2012-02-29, change PWM_DUTY_MAX as 1158 in ICS source(roll-back in ICS code)
#if 1
// #define PWM_DUTY_MAX	 1158  /*1158 /* 22.43 kHz */ 
#define PWM_DUTY_MAX	 1158  /*1158 22.43 kHz */ // 20120213 taeju.park@lge.com To delete compile warning,  
//[LGE_CHANGE_E] seungmoon.lee@lge.com, 2012-02-29, change PWM_DUTY_MAX as 1158 in ICS source(roll-back in ICS code)
#else
#ifdef USE_32_CLK
#define PWM_DUTY_MAX	 0xAEFFFFFF//use 32k clock source ok
#else
#define PWM_DUTY_MAX	 0x387638//use sys clock 26MHz source
#endif // USE_32_CLK
#endif // if 0
/* LGE_CHANGE_E, ryu.seeyeol@lge.com, 2011-03-21, Change the PWM Clock Source (GB Source)*/


#define PLTR_VALUE		(0xFFFFFFFF - PWM_DUTY_MAX)
#define PWM_DUTY_HALF	(0xFFFFFFFF - (PWM_DUTY_MAX >> 1))
static struct omap_dm_timer *omap_vibrator_timer = NULL;


#define CLK_COUNT 38400000
#define MOTOR_RESONANCE_HZ 227

#define MOTOR_RESONANCE_COUTER_VALUE     (0xFFFFFFFE - ((CLK_COUNT/MOTOR_RESONANCE_HZ)/128))
#define PWM_DUTY_MAX     ((CLK_COUNT/MOTOR_RESONANCE_HZ)/128)
#define DUTY_HALF   (PWM_DUTY_MAX >> 1)

#ifdef USE_SUBPM
#if defined(CONFIG_REGULATOR_LP8720)
extern void subpm_set_output(subpm_output_enum outnum, int onoff);
extern void subpm_output_enable(void);
#endif
#else
static struct regulator *vibe_regulator = NULL;
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
#else
	if (val == 1){
		regulator_enable(vibe_regulator);
	}else{
		regulator_disable(vibe_regulator);
	}
#endif
}

// static void vib_enable(int on )
static int vib_enable(int on ) // 20120213 taeju.park@lge.com To delete compile warning, not void return
{
	hub_vibrator_gpio_enable(on);
	hub_vibrator_LDO_enable(on);

	return VIBE_S_SUCCESS;
}

// static void vib_generatePWM(int on)
static int vib_generatePWM(int on)  // 20120213 taeju.park@lge.com To delete compile warning, not void return
{
#if 0
	if(on) {
		/* Select clock */
		omap_dm_timer_enable(omap_vibrator_timer);

/* LGE_CHANGE_S, ryu.seeyeol@lge.com, 2011-03-21, Change the PWM Clock Source */
#ifdef USE_32_CLK
		omap_dm_timer_set_source(omap_vibrator_timer, OMAP_TIMER_SRC_32_KHZ);
#else
		omap_dm_timer_set_source(omap_vibrator_timer, OMAP_TIMER_SRC_SYS_CLK);
#endif
/* LGE_CHANGE_E, ryu.seeyeol@lge.com, 2011-03-21, Change the PWM Clock Source */

		/* set a period */
		omap_dm_timer_set_load(omap_vibrator_timer, 1, PLTR_VALUE);

		/* set a duty */
		omap_dm_timer_set_match(omap_vibrator_timer, 1, PWM_DUTY_HALF);
/* LGE_CHANGE_S, ryu.seeyeol@lge.com, 2011-03-21, Change the PWM Clock Source */
		omap_dm_timer_set_pwm(omap_vibrator_timer, 0, 1, OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE);
		//omap_dm_timer_set_pwm(omap_vibrator_timer, 1, 1, OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE);
/* LGE_CHANGE_E, ryu.seeyeol@lge.com, 2011-03-21, Change the PWM Clock Source */
		omap_dm_timer_start(omap_vibrator_timer);		
	}
	else {
		omap_dm_timer_stop(omap_vibrator_timer);
		omap_dm_timer_disable(omap_vibrator_timer);
	}

	return VIBE_S_SUCCESS;
#else
	if (on) {
		omap_dm_timer_enable(omap_vibrator_timer);
		omap_dm_timer_set_match(omap_vibrator_timer, 1, PWM_DUTY_HALF);
		omap_dm_timer_set_pwm(omap_vibrator_timer, 0, 1, OMAP_TIMER_TRIGGER_OVERFLOW_AND_COMPARE);
		omap_dm_timer_set_load_start(omap_vibrator_timer, 1, MOTOR_RESONANCE_COUTER_VALUE);
	}
	else {
		omap_dm_timer_stop(omap_vibrator_timer);	
		omap_dm_timer_disable(omap_vibrator_timer);
	}
#endif

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
	// int status = 0; // 20120213 taeju.park@lge.com To delete compile warning, unused variable.
	int ret = 0;
	
#if defined(CONFIG_PRODUCT_LGE_LU6800)
	vibe_regulator = regulator_get(NULL, "vaux1");
	if (vibe_regulator == NULL) {
		printk("LGE: vaux1 regulator get fail\n");
		return VIBE_E_FAIL;
	}
#endif
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

#ifdef USE_32_CLK
		omap_dm_timer_set_source(omap_vibrator_timer, OMAP_TIMER_SRC_32_KHZ);
#else
		omap_dm_timer_set_source(omap_vibrator_timer, OMAP_TIMER_SRC_SYS_CLK);
#endif
	
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

/*
** Called by the real-time loop to set PWM duty cycle
*/
//[LGE_CHANGE_S] seungmoon.lee@lge.com, 2012-02-29
IMMVIBESPIAPI VibeStatus ImmVibeSPI_ForceOut_SetSamples(VibeUInt8 nActuatorIndex, VibeUInt16 nOutputSignalBitDepth, VibeUInt16 nBufferSizeInBytes, VibeInt8* pForceOutputBuffer)
{
//AnandKumar_EUR_LGP970_TD114610_Start_LGE_Changes
/* LGE_CHANGE_S, ryu.seeyeol@lge.com, 2011-05-13, This function executes twice.. duplicate vib_generatePWM() */
#if 1 
	unsigned int nTmp;
    VibeInt8 nForce;
	


    switch (nOutputSignalBitDepth)
    {
        case 8:
            /* pForceOutputBuffer is expected to contain 1 byte */
            if (nBufferSizeInBytes != 1) return VIBE_E_FAIL;

            nForce = pForceOutputBuffer[0];
            break;
        case 16:
            /* pForceOutputBuffer is expected to contain 2 byte */
            if (nBufferSizeInBytes != 2) return VIBE_E_FAIL;

            /* Map 16-bit value to 8-bit */
            nForce = ((VibeInt16*)pForceOutputBuffer)[0] >> 8;
            break;
        default:
            /* Unexpected bit depth */
            return VIBE_E_FAIL;
    }
//[LGE_CHANGE_E] seungmoon.lee@lge.com, 2012-02-29
//#if 1
	/* Check the Force value with Max and Min force value */
	if (nForce > 127) nForce = 127;
	if (nForce < -127) nForce = -127;
	
	if (nForce == 0) {
		hub_vibrator_gpio_enable(0);
		omap_dm_timer_stop(omap_vibrator_timer);		
	} else {
		hub_vibrator_gpio_enable(1);
		nTmp = 0xFFFFFFF7 - (((127 - nForce) * 9) >> 1);
		DbgOut(( "[ImmVibeSPI] ImmVibeSPI_ForceOut_SetSamples nForce =  %d / nTmp = %d \n", nForce, nTmp ));
		omap_dm_timer_set_match(omap_vibrator_timer, 1, nTmp);
		omap_dm_timer_start(omap_vibrator_timer);		
	}
#endif
/* LGE_CHANGE_E, ryu.seeyeol@lge.com, 2011-05-13, This function executes twice.. duplicate vib_generatePWM() */
//AnandKumar_EUR_LGP970_TD114610_End_LGE Changes
	return VIBE_S_SUCCESS;
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

