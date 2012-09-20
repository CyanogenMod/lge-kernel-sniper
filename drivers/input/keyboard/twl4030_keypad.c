/*
 * twl4030_keypad.c - driver for 8x8 keypad controller in twl4030 chips
 *
 * Copyright (C) 2007 Texas Instruments, Inc.
 * Copyright (C) 2008 Nokia Corporation
 *
 * Code re-written for 2430SDP by:
 * Syed Mohammed Khasim <x0khasim@ti.com>
 *
 * Initial Code:
 * Manjunatha G K <manjugk@ti.com>
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
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/platform_device.h>
#include <linux/i2c/twl.h>
#include <linux/slab.h>

// 20100727 jh.koo@lge.com for CP reset Pop up [START_LGE]
#include <asm/gpio.h>
#include <linux/delay.h>
#include "../mux.h"
// 20100727 jh.koo@lge.com for CP reset Pop up [END_LGE]
// 20101109 jh.koo@lge.com wake lock for Test Mode of Key [START_LGE]
#include <linux/wakelock.h>
// 20101109 jh.koo@lge.com wake lock for Test Mode of Key [END_LGE]

/*
 * The TWL4030 family chips include a keypad controller that supports
 * up to an 8x8 switch matrix.  The controller can issue system wakeup
 * events, since it uses only the always-on 32KiHz oscillator, and has
 * an internal state machine that decodes pressed keys, including
 * multi-key combinations.
 *
 * This driver lets boards define what keycodes they wish to report for
 * which scancodes, as part of the "struct twl4030_keypad_data" used in
 * the probe() routine.
 *
 * See the TPS65950 documentation; that's the general availability
 * version of the TWL5030 second generation part.
 */
#define TWL4030_MAX_ROWS	8	/* TWL4030 hard limit */
#define TWL4030_MAX_COLS	8
/*
 * Note that we add space for an extra column so that we can handle
 * row lines connected to the gnd (see twl4030_col_xlate()).
 */
#define TWL4030_ROW_SHIFT	4
#define TWL4030_KEYMAP_SIZE	(TWL4030_MAX_ROWS << TWL4030_ROW_SHIFT)

// 20100727 jh.koo@lge.com for CP reset Pop up [START_LGE]
#define CP_DOWN_INT		176
// 20100727 jh.koo@lge.com for CP reset Pop up [END_LGE]
// 20100831 jh.koo@lge.com for TEST MODE [START_LGE]
/*LGSI_GKPD_AT_CMD_snehal.shinde@lge.com_27_Aug_2011_Start*/
#define GKPD_BUF_MAX		20		
static unsigned int test_mode = 0;
static int test_code, gkpd_last_index = 0;
//static int gkpd_state; // 20120213 taeju.park@lge.com To delete compile warning, unused variable.
static unsigned char gkpd_value[GKPD_BUF_MAX+1];
/*LGSI_GKPD_AT_CMD_snehal.shinde@lge.com_27_Aug_2011_End*/

static struct wake_lock key_wake_lock;
// 20100831 jh.koo@lge.com for TEST MODE [END_LGE]

struct twl4030_keypad {
	unsigned short	keymap[TWL4030_KEYMAP_SIZE];
	u16		kp_state[TWL4030_MAX_ROWS];
	unsigned	n_rows;
	unsigned	n_cols;
	unsigned	irq;

	struct device *dbg_dev;
	struct input_dev *input;
};
/*----------------------------------------------------------------------*/

/* arbitrary prescaler value 0..7 */
#define PTV_PRESCALER			4

/* Register Offsets */
#define KEYP_CTRL			0x00
#define KEYP_DEB			0x01
#define KEYP_LONG_KEY			0x02
#define KEYP_LK_PTV			0x03
#define KEYP_TIMEOUT_L			0x04
#define KEYP_TIMEOUT_H			0x05
#define KEYP_KBC			0x06
#define KEYP_KBR			0x07
#define KEYP_SMS			0x08
#define KEYP_FULL_CODE_7_0		0x09	/* row 0 column status */
#define KEYP_FULL_CODE_15_8		0x0a	/* ... row 1 ... */
#define KEYP_FULL_CODE_23_16		0x0b
#define KEYP_FULL_CODE_31_24		0x0c
#define KEYP_FULL_CODE_39_32		0x0d
#define KEYP_FULL_CODE_47_40		0x0e
#define KEYP_FULL_CODE_55_48		0x0f
#define KEYP_FULL_CODE_63_56		0x10
#define KEYP_ISR1			0x11
#define KEYP_IMR1			0x12
#define KEYP_ISR2			0x13
#define KEYP_IMR2			0x14
#define KEYP_SIR			0x15
#define KEYP_EDR			0x16	/* edge triggers */
#define KEYP_SIH_CTRL			0x17

/* KEYP_CTRL_REG Fields */
#define KEYP_CTRL_SOFT_NRST		BIT(0)
#define KEYP_CTRL_SOFTMODEN		BIT(1)
#define KEYP_CTRL_LK_EN			BIT(2)
#define KEYP_CTRL_TOE_EN		BIT(3)
#define KEYP_CTRL_TOLE_EN		BIT(4)
#define KEYP_CTRL_RP_EN			BIT(5)
#define KEYP_CTRL_KBD_ON		BIT(6)

/* KEYP_DEB, KEYP_LONG_KEY, KEYP_TIMEOUT_x*/
#define KEYP_PERIOD_US(t, prescale)	((t) / (31 << (prescale + 1)) - 1)

/* KEYP_LK_PTV_REG Fields */
#define KEYP_LK_PTV_PTV_SHIFT		5

/* KEYP_{IMR,ISR,SIR} Fields */
#define KEYP_IMR1_MIS			BIT(3)
#define KEYP_IMR1_TO			BIT(2)
#define KEYP_IMR1_LK			BIT(1)
#define KEYP_IMR1_KP			BIT(0)

/* KEYP_EDR Fields */
#define KEYP_EDR_KP_FALLING		0x01
#define KEYP_EDR_KP_RISING		0x02
#define KEYP_EDR_KP_BOTH		0x03
#define KEYP_EDR_LK_FALLING		0x04
#define KEYP_EDR_LK_RISING		0x08
#define KEYP_EDR_TO_FALLING		0x10
#define KEYP_EDR_TO_RISING		0x20
#define KEYP_EDR_MIS_FALLING		0x40
#define KEYP_EDR_MIS_RISING		0x80

// [START_LGE]
int key_row;
int key_col;
int key_pressed;
// [END_LGE]

/*----------------------------------------------------------------------*/

// 20100831 jh.koo@lge.com for TEST MODE [START_LGE]
int get_test_mode(void)
{
	return test_mode;
}
EXPORT_SYMBOL(get_test_mode);

/*LGSI_GKPD_AT_CMD_snehal.shinde@lge.com_27_Aug_2011_Start*/
#if 1// 20101226 changhyun.han@lge.com for GKPD command HARD key map, refer to feature phone models.
typedef struct
{
	char         out;
	unsigned char in;

} Conv;

 Conv GKPD_table[]=
 {
 #if 0
	 {'#',  UTA_KBD_KEY_HASH}
	,{'*',  UTA_KBD_KEY_STAR}
	,{'0',  UTA_KBD_KEY_0}
	,{'1',  UTA_KBD_KEY_1}
	,{'2',  UTA_KBD_KEY_2}
	,{'3',  UTA_KBD_KEY_3}
	,{'4',  UTA_KBD_KEY_4}
	,{'5',  UTA_KBD_KEY_5}
	,{'6',  UTA_KBD_KEY_6}
	,{'7',  UTA_KBD_KEY_7}
	,{'8',  UTA_KBD_KEY_8}
	,{'9',  UTA_KBD_KEY_9}
	,{'^',  UTA_KBD_KEY_UP}
	,{'V',  UTA_KBD_KEY_DOWN}
	,{'L',  UTA_KBD_KEY_LEFT}
	,{'R',  UTA_KBD_KEY_RIGHT}
	,{'E',  UTA_KBD_KEY_ONOFF}
	,{'[',  UTA_KBD_KEY_LSO}
	,{']',  UTA_KBD_KEY_RSO}
	,{'S',  UTA_KBD_KEY_DIAL}
	,{'Y',  UTA_KBD_KEY_CLEAR}
	,{'O',  UTA_KBD_KEY_OK} // Key_Reassign winneck test 080717
    ,{'U',  UTA_KBD_KEY_VOL_UP}
	,{'M',  UTA_KBD_CUSTOMER_KEY_1}
#endif	
	{'D',  KEY_VOLUMEDOWN}
	,{'A',  KEY_VOLUMEUP}
	 ,{'F',  KEY_KPJPCOMMA}
	,{'H',  KEY_HOOK} 
	,{0, 0}
};

int gkpd_KeyConvert(int key)
{
	u16 indexCount = 40;
	int i = 0;
	
	while ((i < indexCount) && (key != 0xFF))
	{
		if (GKPD_table[i].in == key)
			return GKPD_table[i].out;
		i++;
	}
	return key;
}
#endif

/*LGSI_GKPD_AT_CMD_snehal.shinde@lge.com_27_Aug_2011_End*/
/*LGSI_GKPD_AT_CMD_snehal.shinde@lge.com_27_Aug_2011_Start*/
void write_gkpd_value(int value)
{
	int i;
	int con ;

	con=gkpd_KeyConvert(value);// 20101226 changhyun.han@lge.com for GKPD command HARD key map, refer to feature phone models.
	value = con;

	//printk(KERN_INFO "[GKPD] Input : %c , Index : %d\n",value,gkpd_last_index);

	if (gkpd_last_index == GKPD_BUF_MAX) {
		gkpd_value[gkpd_last_index] = value;
		for ( i = 0; i < GKPD_BUF_MAX ; i++) {
			gkpd_value[i] = gkpd_value[i + 1];
		}			
		gkpd_value[gkpd_last_index] = '\n';
	}
	else {
		gkpd_value[gkpd_last_index] = value;
		gkpd_value[gkpd_last_index + 1] = '\n';
		gkpd_last_index++;
	}		
}
/*LGSI_GKPD_AT_CMD_snehal.shinde@lge.com_27_Aug_2011_End*/
EXPORT_SYMBOL(write_gkpd_value);
// 20100831 jh.koo@lge.com for TEST MODE [END_LGE]

static int twl4030_kpread(struct twl4030_keypad *kp,
		u8 *data, u32 reg, u8 num_bytes)
{
	int ret = twl_i2c_read(TWL4030_MODULE_KEYPAD, data, reg, num_bytes);

	if (ret < 0)
		dev_warn(kp->dbg_dev,
			"Couldn't read TWL4030: %X - ret %d[%x]\n",
			 reg, ret, ret);

	return ret;
}

static int twl4030_kpwrite_u8(struct twl4030_keypad *kp, u8 data, u32 reg)
{
	int ret = twl_i2c_write_u8(TWL4030_MODULE_KEYPAD, data, reg);

	if (ret < 0)
		dev_warn(kp->dbg_dev,
			"Could not write TWL4030: %X - ret %d[%x]\n",
			 reg, ret, ret);

	return ret;
}

static inline u16 twl4030_col_xlate(struct twl4030_keypad *kp, u8 col)
{
	/* If all bits in a row are active for all coloumns then
	 * we have that row line connected to gnd. Mark this
	 * key on as if it was on matrix position n_cols (ie
	 * one higher than the size of the matrix).
	 */
	if (col == 0xFF)
		return 1 << kp->n_cols;
	else
		return col & ((1 << kp->n_cols) - 1);
}

static int twl4030_read_kp_matrix_state(struct twl4030_keypad *kp, u16 *state)
{
	u8 new_state[TWL4030_MAX_ROWS];
	int row;
	int ret = twl4030_kpread(kp, new_state,
				 KEYP_FULL_CODE_7_0, kp->n_rows);
	if (ret >= 0)
		for (row = 0; row < kp->n_rows; row++)
			state[row] = twl4030_col_xlate(kp, new_state[row]);

	return ret;
}

static bool twl4030_is_in_ghost_state(struct twl4030_keypad *kp, u16 *key_state)
{
	int i;
	u16 check = 0;

	for (i = 0; i < kp->n_rows; i++) {
		u16 col = key_state[i];

		if ((col & check) && hweight16(col) > 1)
			return true;

		check |= col;
	}

	return false;
}

static void twl4030_kp_scan(struct twl4030_keypad *kp, bool release_all)
{
	struct input_dev *input = kp->input;
	u16 new_state[TWL4030_MAX_ROWS];
	int col, row;

	if (release_all){
		key_pressed = key_col = key_row = 0; //20100918 ks.kwon@lge.com for key detection in muic.
		memset(new_state, 0, sizeof(new_state));
	}else {
		/* check for any changes */
		int ret = twl4030_read_kp_matrix_state(kp, new_state);

		key_pressed = 1;
		if (ret < 0)	/* panic ... */
			return;

		if (twl4030_is_in_ghost_state(kp, new_state))
			return;
	}

	/* check for changes and print those */
	for (row = 0; row < kp->n_rows; row++) {
		int changed = new_state[row] ^ kp->kp_state[row];

		if (!changed)
			continue;

		/* Extra column handles "all gnd" rows */
		for (col = 0; col < kp->n_cols + 1; col++) {
			int code;

			if (!(changed & (1 << col)))
				continue;

			dev_dbg(kp->dbg_dev, "key [%d:%d] %s\n", row, col,
				(new_state[row] & (1 << col)) ?
				"press" : "release");

			code = MATRIX_SCAN_CODE(row, col, TWL4030_ROW_SHIFT);
// 20100831 jh.koo@lge.com for TEST MODE [START_LGE]
			test_code = code;
// 20100831 jh.koo@lge.com for TEST MODE [END_LGE]
			input_event(input, EV_MSC, MSC_SCAN, code);
			input_report_key(input, kp->keymap[code],
					 new_state[row] & (1 << col));
		}
		kp->kp_state[row] = new_state[row];
		//20100918 ks.kwon@lge.com for key detection in muic. 
		if(key_pressed){
			key_row = row;
			key_col = col;
		} else {
			key_row = 0;
			key_col = 0;
		}
	}
	input_sync(input);
}

/*
 * Keypad interrupt handler
 */
static irqreturn_t do_kp_irq(int irq, void *_kp)
{
	struct twl4030_keypad *kp = _kp;
	u8 reg;
	int ret;

	/* Read & Clear TWL4030 pending interrupt */
	ret = twl4030_kpread(kp, &reg, KEYP_ISR1, 1);

	/* Release all keys if I2C has gone bad or
	 * the KEYP has gone to idle state */
	if (ret >= 0 && (reg & KEYP_IMR1_KP))
		twl4030_kp_scan(kp, false);
	else
	{
		twl4030_kp_scan(kp, true);

/*LGSI_GKPD_AT_CMD_snehal.shinde@lge.com_08_Oct_2011_Start*/
// 20100831 jh.koo@lge.com for TEST MODE [START_LGE]
		if(test_mode == 1)	
			write_gkpd_value(kp->keymap[test_code]);	
// 20100831 jh.koo@lge.com for TEST MODE [END_LGE]	
/*LGSI_GKPD_AT_CMD_snehal.shinde@lge.com_08_Oct_2011_Start*/
	}

	return IRQ_HANDLED;
}

static int __devinit twl4030_kp_program(struct twl4030_keypad *kp)
{
	u8 reg;
	int i;

	/* Enable controller, with hardware decoding but not autorepeat */
	reg = KEYP_CTRL_SOFT_NRST | KEYP_CTRL_SOFTMODEN
		| KEYP_CTRL_TOE_EN | KEYP_CTRL_KBD_ON;
	if (twl4030_kpwrite_u8(kp, reg, KEYP_CTRL) < 0)
		return -EIO;

	/* NOTE:  we could use sih_setup() here to package keypad
	 * event sources as four different IRQs ... but we don't.
	 */

	/* Enable TO rising and KP rising and falling edge detection */
	reg = KEYP_EDR_KP_BOTH | KEYP_EDR_TO_RISING;
	if (twl4030_kpwrite_u8(kp, reg, KEYP_EDR) < 0)
		return -EIO;

	/* Set PTV prescaler Field */
	reg = (PTV_PRESCALER << KEYP_LK_PTV_PTV_SHIFT);
	if (twl4030_kpwrite_u8(kp, reg, KEYP_LK_PTV) < 0)
		return -EIO;

	/* Set key debounce time to 20 ms */
	i = KEYP_PERIOD_US(20000, PTV_PRESCALER);
	if (twl4030_kpwrite_u8(kp, i, KEYP_DEB) < 0)
		return -EIO;

	/* Set timeout period to 100 ms */
	i = KEYP_PERIOD_US(200000, PTV_PRESCALER);
	if (twl4030_kpwrite_u8(kp, (i & 0xFF), KEYP_TIMEOUT_L) < 0)
		return -EIO;

	if (twl4030_kpwrite_u8(kp, (i >> 8), KEYP_TIMEOUT_H) < 0)
		return -EIO;

	/*
	 * Enable Clear-on-Read; disable remembering events that fire
	 * after the IRQ but before our handler acks (reads) them,
	 */
	reg = TWL4030_SIH_CTRL_COR_MASK | TWL4030_SIH_CTRL_PENDDIS_MASK;
	if (twl4030_kpwrite_u8(kp, reg, KEYP_SIH_CTRL) < 0)
		return -EIO;

	/* initialize key state; irqs update it from here on */
	if (twl4030_read_kp_matrix_state(kp, kp->kp_state) < 0)
		return -EIO;

	return 0;
}

// 20100727 jh.koo@lge.com for CP reset Pop up [START_LGE]
extern void hub_reboot_device(void);
extern char reset_mode;
extern int hidden_reset_enabled;
//LGE_TELECA_JAVA_RIL_RECOVERY_264  -START
//20110329, ramesh.chandrasekaran@teleca.com, RIL RECOVERY
//Description: ISR for modem restart handling
extern void modem_restart(void);
static irqreturn_t hub_cp_int_handler(int irq, void *_kp)
{
	struct twl4030_keypad *kp = _kp;

    pr_err("modem crash!\n");
#if 0 //20110317,suchul.lee@lge.com Request_Kill_RILD [START]
	if (hidden_reset_enabled) {
#if 0 //2011-02-19, for ril recovery
		reset_mode = 'h';
		hub_reboot_device();
#else
        //modem_restart();
#endif
	} else {
		input_report_key(kp->input, KEY_PROG3, 1);
		mdelay(10);
		input_report_key(kp->input, KEY_PROG3, 0);
		input_sync(kp->input);
	}
#else
    input_report_key(kp->input, KEY_PROG3, 1);
    mdelay(10);
    input_report_key(kp->input, KEY_PROG3, 0);
    input_sync(kp->input);
#endif //20110317,suchul.lee@lge.com Request_Kill_RILD [END]
	
	return IRQ_HANDLED;
}
// 20100727 jh.koo@lge.com for CP reset Pop up [END_LGE]
//LGE_TELECA_JAVA_RIL_RECOVERY_264  -END

/*
 * Registers keypad device with input subsystem
 * and configures TWL4030 keypad registers
 */

// 20100831 jh.koo@lge.com for TEST MODE [START_LGE]
static ssize_t hub_keypad_test_mode_show(struct device *dev,  struct device_attribute *attr,  char *buf)
{
	int i;
	int r = 0;
	for(i = 0; i < gkpd_last_index; i++)
	{
		printk(KERN_WARNING"[!] %s() code value : %d\n", __func__, gkpd_value[i]);
/*LGSI_GKPD_AT_CMD_snehal.shinde@lge.com_27_Aug_2011_Start*/
		r += sprintf(buf+r, "%c", gkpd_value[i]);
/*LGSI_GKPD_AT_CMD_snehal.shinde@lge.com_27_Aug_2011_end*/
#if 0		
		if(i == gkpd_last_index - 1) 
			r += sprintf(buf+r, "%2x\n", gkpd_value[i]);
		else
			r += sprintf(buf+r, "%2x, ", gkpd_value[i]);
#endif
	}
//	r += sprintf(buf+r, "%d", '\n');
/*LGSI_GKPD_AT_CMD_snehal.shinde@lge.com_27_Aug_2011_Start*/
	gkpd_last_index = 0;
	memset(gkpd_value, 0, sizeof(gkpd_value));
/*LGSI_GKPD_AT_CMD_snehal.shinde@lge.com_27_Aug_2011_end*/
	return r;
//	return (ssize_t)(&gkpd_value);
#if 0
	return sprintf(buf, "%d\n", gkpd_value);
#else
#endif
}

static ssize_t hub_keypad_test_mode_store(struct device *dev,  struct device_attribute *attr,  const char *buf, size_t count)
{
//	int val;
    int ret;
	int i;

//	val = simple_strtoul(buf, NULL, 10);
    ret = sscanf(buf, "%d", &test_mode);

	if(test_mode == 1) {

// 20101109 jh.koo@lge.com wake lock for Test Mode of Key [START_LGE]		
		wake_lock(&key_wake_lock);
// 20101109 jh.koo@lge.com wake lock for Test Mode of Key [END_LGE]

		for(i = 0; i < gkpd_last_index; i++)
			gkpd_value[i] = 0;

		gkpd_last_index = 0;
	}
// 20101109 jh.koo@lge.com wake lock for Test Mode of Key [START_LGE]	
	else if(test_mode == 0) {
		wake_unlock(&key_wake_lock);
	}
// 20101109 jh.koo@lge.com wake lock for Test Mode of Key [END_LGE]	
//	test_mode = ret;

	return ret;
}
static DEVICE_ATTR(key_test_mode, 0664, hub_keypad_test_mode_show, hub_keypad_test_mode_store);
// 20100831 jh.koo@lge.com for TEST MODE [END_LGE]

static int __devinit twl4030_kp_probe(struct platform_device *pdev)
{
// 20100727 jh.koo@lge.com for CP reset Pop up [START_LGE]
	int ret = 0;
	omap_mux_init_gpio(CP_DOWN_INT, OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_WAKEUPENABLE);
// 20100727 jh.koo@lge.com for CP reset Pop up [END_LGE]	
	
	struct twl4030_keypad_data *pdata = pdev->dev.platform_data;
	const struct matrix_keymap_data *keymap_data = pdata->keymap_data;
	struct twl4030_keypad *kp;
	struct input_dev *input;
	u8 reg;
	int error;

	if (!pdata || !pdata->rows || !pdata->cols ||
	    pdata->rows > TWL4030_MAX_ROWS || pdata->cols > TWL4030_MAX_COLS) {
		dev_err(&pdev->dev, "Invalid platform_data\n");
		return -EINVAL;
	}

	kp = kzalloc(sizeof(*kp), GFP_KERNEL);
	input = input_allocate_device();
	if (!kp || !input) {
		return -ENOMEM;
	}

	/* Get the debug Device */
	kp->dbg_dev = &pdev->dev;
	kp->input = input;

	kp->n_rows = pdata->rows;
	kp->n_cols = pdata->cols;
	kp->irq = platform_get_irq(pdev, 0);
	/* setup input device */
	__set_bit(EV_KEY, input->evbit);

	/* Enable auto repeat feature of Linux input subsystem */
	if (pdata->rep)
		__set_bit(EV_REP, input->evbit);

	input_set_capability(input, EV_MSC, MSC_SCAN);

	input->name		= "TWL4030_Keypad";
	input->phys		= "TWL4030_Keypad/input0";
	input->dev.parent	= &pdev->dev;

	input->id.bustype	= BUS_HOST;
	input->id.vendor	= 0x0001;
	input->id.product	= 0x0001;
	input->id.version	= 0x0003;

	input->keycode		= kp->keymap;
	input->keycodesize	= sizeof(kp->keymap[0]);
	input->keycodemax	= ARRAY_SIZE(kp->keymap);

	matrix_keypad_build_keymap(keymap_data, TWL4030_ROW_SHIFT,
				   input->keycode, input->keybit);

	error = input_register_device(input);
	if (error) {
		dev_err(kp->dbg_dev,
			"Unable to register twl4030 keypad device\n");
		goto err1;
	}

	error = twl4030_kp_program(kp);
	if (error)
		goto err2;

	/*
	 * This ISR will always execute in kernel thread context because of
	 * the need to access the TWL4030 over the I2C bus.
	 *
	 * NOTE:  we assume this host is wired to TWL4040 INT1, not INT2 ...
	 */
	error = request_threaded_irq(kp->irq, NULL, do_kp_irq,
			0, pdev->name, kp);
	if (error) {
		dev_info(kp->dbg_dev, "request_irq failed for irq no=%d\n",
			kp->irq);
		goto err2;
	}

	/* Enable KP and TO interrupts now. */
	reg = (u8) ~(KEYP_IMR1_KP | KEYP_IMR1_TO);
	if (twl4030_kpwrite_u8(kp, reg, KEYP_IMR1)) {
		error = -EIO;
		goto err3;
	}

// 20101109 jh.koo@lge.com wake lock for Test Mode of Key [START_LGE]
	wake_lock_init(&key_wake_lock, WAKE_LOCK_SUSPEND, "TWL4030_Keypad");
// 20101109 jh.koo@lge.com wake lock for Test Mode of Key [END_LGE]

// 20100727 jh.koo@lge.com for CP reset Pop up [START_LGE]
	ret = gpio_request(CP_DOWN_INT, "cp int gpio");

	if(ret < 0) {
			printk(KERN_WARNING"%s() can't get hub GPIO\n", __func__);
			kzfree(kp);
			return -ENOSYS;
		}
	
	ret = gpio_direction_input(CP_DOWN_INT);

	ret = request_irq(gpio_to_irq(CP_DOWN_INT), hub_cp_int_handler, IRQF_TRIGGER_FALLING /*| IRQF_TRIGGER_RISING*/, "CP_INT", kp);
	if (ret < 0){
			printk(KERN_INFO "[CP Reset] GPIO 176 IRQ line set up failed!\n");
			free_irq(gpio_to_irq(CP_DOWN_INT), kp);
			return -ENOSYS;
		}	
		/* Make the interrupt on wake up OMAP which is in suspend mode */
		ret = enable_irq_wake(gpio_to_irq(CP_DOWN_INT));
		if(ret < 0){
			printk(KERN_INFO "[CP Reset] GPIO 176 CP Switching wake up source setting failed!\n");
			disable_irq_wake(gpio_to_irq(CP_DOWN_INT));
			return -ENOSYS;
		}
//	omap_mux_init_gpio(CP_DOWN_INT, OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_WAKEUPENABLE);
// 20100727 jh.koo@lge.com for CP reset Pop up [END_LGE]
// 20100831 jh.koo@lge.com for TEST MODE [START_LGE]
	ret = device_create_file(&pdev->dev, &dev_attr_key_test_mode);
	if (ret) {
		printk( "Hub-keypad: keypad_probe: Fail\n");
		device_remove_file(&pdev->dev, &dev_attr_key_test_mode);
		return ret;
	}
// 20100831 jh.koo@lge.com for TEST MODE [END_LGE]

	platform_set_drvdata(pdev, kp);
	return 0;

err3:
	/* mask all events - we don't care about the result */
	(void) twl4030_kpwrite_u8(kp, 0xff, KEYP_IMR1);
	free_irq(kp->irq, NULL);
err2:
	input_unregister_device(input);
	input = NULL;
err1:
	input_free_device(input);
	kfree(kp);
	return error;
}

static int __devexit twl4030_kp_remove(struct platform_device *pdev)
{
	struct twl4030_keypad *kp = platform_get_drvdata(pdev);

	free_irq(kp->irq, kp);
	input_unregister_device(kp->input);
	platform_set_drvdata(pdev, NULL);
	kfree(kp);
// 20101109 jh.koo@lge.com wake lock for Test Mode of Key [START_LGE]
	wake_lock_destroy(&key_wake_lock);
// 20101109 jh.koo@lge.com wake lock for Test Mode of Key [END_LGE]

	return 0;
}

/*
 * NOTE: twl4030 are multi-function devices connected via I2C.
 * So this device is a child of an I2C parent, thus it needs to
 * support unplug/replug (which most platform devices don't).
 */

static struct platform_driver twl4030_kp_driver = {
	.probe		= twl4030_kp_probe,
	.remove		= __devexit_p(twl4030_kp_remove),
	.driver		= {
		.name	= "twl4030_keypad",
		.owner	= THIS_MODULE,
	},
};

static int __init twl4030_kp_init(void)
{
	return platform_driver_register(&twl4030_kp_driver);
}
module_init(twl4030_kp_init);

static void __exit twl4030_kp_exit(void)
{
	platform_driver_unregister(&twl4030_kp_driver);
}
module_exit(twl4030_kp_exit);

MODULE_AUTHOR("Texas Instruments");
MODULE_DESCRIPTION("TWL4030 Keypad Driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:twl4030_keypad");

