/*
 * hub Charging IC driver (MAX8922)
 *
 * Copyright (C) 2009 LGE, Inc.
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
#ifndef __HUB_CHARGING_IC_H__
#define __HUB_CHARGING_IC_H__

#define CHG_EN_SET_N_OMAP 		25
#define CHG_STATUS_N_OMAP 		15

#ifdef CONFIG_LGE_CHARGE_CONTROL_BATTERY_FET
#define CHAR_CONTROL 			39
#endif 

typedef enum {
  CHARGING_IC_DEACTIVE,    		/* 0  */
  CHARGING_IC_ACTIVE_DEFAULT,
  CHARGING_IC_TA_MODE,
  CHARGING_IC_FACTORY_MODE,

  CHARGING_IC_MAX_MODE			/* 4  */
} max8922_status;

/* Function Prototype */
void charging_ic_active_default(void);
void charging_ic_set_ta_mode(void);
void charging_ic_set_usb_mode(void);
void charging_ic_set_factory_mode(void);
void charging_ic_deactive(void);
/* Shutdown issue at the case of USB booting [kyungyoon.kim@lge.com] 2010-12-25 */
void charging_ic_set_usb_mode_from_ta_mode(void);
/* Shutdown issue at the case of USB booting [kyungyoon.kim@lge.com] 2010-12-25 */
max8922_status get_charging_ic_status(void);

#ifdef CONFIG_LGE_CHARGE_CONTROL_BATTERY_FET
extern void lge_battery_fet_onoff(int on);
#endif 

#endif /* __HUB_CHARGING_IC_H__ */
