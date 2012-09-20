/*
 * Charging IC driver (RT9524)
 *
 * Copyright (C) 2010 LGE, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
#define CHG_EN_SET_N_OMAP 		83
#define CHG_STATUS_N_OMAP 		51

//#define DEBUG
// LGE_CHANGE [euiseop.shin@lge.com] 2011-04-13, LGE_P940, Bring in Cosmo.--->
#define OMAP_SEND				122  //for fuel gauge reset on CP.


#ifdef DEBUG
#define D(fmt, args...) printk(fmt " :: file=%s, func=%s, line=%d\n", ##args, __FILE__, __func__, __LINE__ ) 
//#define D(fmt, args...) printk(fmt, ##args) 
#else
#define D(fmt, args...)
#endif

#define BAT_TEMP_OVER

#define TEMP_LOW_NO_BAT			-300
#define TEMP_LOW_DISCHARGING		-100
#define TEMP_HIGH_DISCHARGING		550

#define TEMP_LOW_RECHARGING		-50
#define TEMP_HIGH_RECHARGING		420

#define TEMP_CHANGE_CHARGING_MODE	450

#define RECHARGING_BAT_SOC_CON		97

// LGE_CHANGE_S [byoungcheol.lee@lge.com]  2011-07-07, From cosmo. cause discharging while Not complete charging process. 
#define RECHARGING_BAT_VOLT_LOW		4185
#define RECHARGING_BAT_VOLT_HIGH	4216
// LGE_CHANGE_E [byoungcheol.lee@lge.com]  2011-07-07, From cosmo. cause discharging while Not complete charging process. 

#if 0
typedef enum {
	CHARGING_IC_DEACTIVE,    		/* 0  */
	CHARGING_IC_ACTIVE_DEFAULT,
	CHARGING_IC_TA_MODE,
	CHARGING_IC_FACTORY_MODE,

	CHARGING_IC_MAX_MODE			/* 4  */
}charger_status;
#endif

typedef enum {
	FACTORY_CHARGER_ENABLE,
	FACTORY_CHARGER_DISABLE,
}charge_factory_cmd;


typedef enum {
	CHARGER_DISABLE,
	BATTERY_NO_CHARGER,
	CHARGER_NO_BATTERY,
	CHARGER_AND_BATTERY,
}charge_enable_state_t ;

typedef enum {
	RECHARGING_WAIT_UNSET,
	RECHARGING_WAIT_SET,
}recharging_state_t;

typedef enum {
	CHARGER_LOGO_STATUS_UNKNOWN,
	CHARGER_LOGO_STATUS_STARTED,
	CHARGER_LOGO_STATUS_END,
}charger_logo_state_t;


/*/this code should be included from muic.h  until then this code remains.
typedef enum {
	MUIC_UNKNOWN,       // 0 - Error in detection or unsupported accessory.
	MUIC_NONE,      // 1 - No accessory is plugged in.
	MUIC_NA_TA,     // 2 - Not used actually. Special TA for North America.
	MUIC_LG_TA,     // 3
	MUIC_HCHH,      // 4 - Not used actually.
	MUIC_INVALID_CHG,   // 5
	MUIC_AP_UART,       // 6
	MUIC_CP_UART,       // 7
	MUIC_AP_USB,        // 8
	MUIC_CP_USB,        // 9 - Not defined yet.
	MUIC_TV_OUT_NO_LOAD,    // 10 - Not used.
	MUIC_EARMIC,        // 11
	MUIC_TV_OUT_LOAD,   // 12 - Not used.
	MUIC_OTG,       // 13 - Not used.
	MUIC_RESERVE1,      // 14
	MUIC_RESERVE2,      // 15
	MUIC_RESERVE3,      // 16
	MUIC_MODE_NO,       // 17
}TYPE_MUIC_MODE;
//this code should be included from muic.h  until then this code remains.
*/


/* Function Prototype */

extern enum power_supply_type get_charging_ic_status(void);

extern void charging_ic_active_default(void);
extern void charging_ic_set_ta_mode(void);
extern void charging_ic_set_usb_mode(void);
extern void charging_ic_set_factory_mode(void);
extern void charging_ic_deactive(void);
// LGE_CHANGE [euiseop.shin@lge.com] 2011-04-13, LGE_P940, Bring in Cosmo.
int get_temp(void);

typedef enum {
	CHARG_FSM_CAUSE_ANY = 0,
	CHARG_FSM_CAUSE_CHARGING_TIMER_EXPIRED,
}charger_fsm_cause;
void charger_fsm(charger_fsm_cause reason);

// [jongho3.lee@lge.com] export temperature func.
int twl6030battery_temperature(void);
int get_bat_soc(void);
struct delayed_work* get_charger_work(void);

void charger_schedule_delayed_work(struct delayed_work *work, unsigned long delay);

/// max17043 fuel gauge..
void set_boot_charging_mode(int charging_mode);
