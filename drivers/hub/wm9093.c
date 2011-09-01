
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <asm/gpio.h>
#include <asm/system.h>
#include <mach/wm9093.h>
#include <mach/hub_headset_det.h> 

struct wm9093_i2c_device *wm9093_i2c_dev = NULL;
struct wm9093_device *wm9093_amp_dev = NULL;

#include <linux/i2c/twl.h>
#include <linux/regulator/consumer.h>

#define VPLL2_DEV_GRP       0x33  

static struct regulator *wm9093_reg; 
static wm9093_fmvolume_enum s_volume =  LEVEL_4;

int wm9093_control_status = 0;		

static const wm9093_reg_type wm9093_in1_to_out_tab[] =
{
  	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0002},
	{WM9093_CMD ,0x19, 0x0002},
	{WM9093_CMD ,0x18, 0x0102},
	{WM9093_CMD ,0x36, 0x0050},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0050},
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x25, 0x0160},
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};

static const wm9093_reg_type wm9093_in1_to_hp_tab[] =
{
#if 1	
	{WM9093_DELAY,0x39, 0x0050},
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_DELAY,0x00, 50}, 
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0100},	
	{WM9093_CMD ,0x19, 0x0100},	
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x1C, 0x0133},	
	{WM9093_CMD ,0x1D, 0x0135},	
//	{WM9093_CMD ,0x1C, 0x0131},	
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
#else
    {WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0000},	
	{WM9093_CMD ,0x19, 0x0000},	
	{WM9093_CMD ,0x18, 0x0101},
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x1C, 0x0036},	
	{WM9093_CMD ,0x1D, 0x0036},	
	{WM9093_CMD ,0x1C, 0x0136},	
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
#endif    
};

static const wm9093_reg_type wm9093_in1_to_FMhp_tab[] =
{
       {WM9093_DELAY,0x39, 0x0050},
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0100},	
	{WM9093_CMD ,0x19, 0x0100},	
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x16, 0x0000},
	//{WM9093_CMD ,0x1C, 0x011C},	//initial value
	//{WM9093_CMD ,0x1D, 0x011C},	//initial value
	//{WM9093_CMD ,0x1C, 0x0135},	
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
       {WM9093_END_SEQ,0x00,0x00}
};
static const wm9093_reg_type wm9093_in1_to_FMout_tab[] =
{
  	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0100},		
	{WM9093_CMD ,0x1B, 0x0100},		
	{WM9093_CMD ,0x36, 0x0005},	
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},		
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x25, 0x0178},		
	{WM9093_CMD ,0x24, 0x0010},			
	//{WM9093_CMD ,0x26, 0x011C},	//initial value	
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},	
    {WM9093_END_SEQ,0x00,0x00}
};


static const wm9093_reg_type wm9093_in1_to_hp_voip_tab[] =
{
	{WM9093_DELAY,0x39, 0x0050},
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0100},	
	{WM9093_CMD ,0x19, 0x0100},	
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x1C, 0x0033},	
	{WM9093_CMD ,0x1D, 0x0033},	
	{WM9093_CMD ,0x1C, 0x0133},	
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
};



static const wm9093_reg_type wm9093_in2_to_out_hp_tab[] =
{
#if 0	
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x100B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0002},
	{WM9093_CMD ,0x1B, 0x0002},
	{WM9093_CMD ,0x1A, 0x0102},
	{WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x22, 0x0005},
	{WM9093_CMD ,0x25, 0x0178},
//	{WM9093_CMD ,0x26, 0x003a},		
	{WM9093_CMD ,0x2D, 0x0004},
	{WM9093_CMD ,0x2E, 0x0001},
	{WM9093_CMD ,0x03, 0x0138},
	{WM9093_CMD ,0x1C, 0x001c},	
	{WM9093_CMD ,0x1D, 0x011c},	
	{WM9093_CMD ,0x1C, 0x011c},	
	{WM9093_CMD ,0x2F, 0x0000},	
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
#else
#if 0	
 	{WM9093_CMD ,0x39, 0x000D},
    {WM9093_CMD ,0x01, 0x130B},
	{WM9093_CMD ,0x02, 0x60c0},
    {WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0002},	
	{WM9093_CMD ,0x19, 0x0002},	
	{WM9093_CMD ,0x18, 0x0102},	
    {WM9093_CMD ,0x36, 0x0050},	
	{WM9093_CMD ,0x22, 0x0050},    
	{WM9093_CMD ,0x25, 0x0178},  
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
    {WM9093_CMD ,0x03, 0x0138},
	{WM9093_CMD ,0x1C, 0x001c},	
	{WM9093_CMD ,0x1D, 0x011c},	
	{WM9093_CMD ,0x1C, 0x011c},	
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
    {WM9093_CMD ,0x16, 0x0000},    
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
#else	
 	{WM9093_CMD ,0x39, 0x000D},
    {WM9093_CMD ,0x01, 0x130B},
	{WM9093_CMD ,0x02, 0x60f0},
    {WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0002},	
	{WM9093_CMD ,0x19, 0x0002},	
	{WM9093_CMD ,0x18, 0x0102},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0002},
	{WM9093_CMD ,0x1B, 0x0002},
	{WM9093_CMD ,0x1A, 0x0102},
    {WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x22, 0x0005},
	{WM9093_CMD ,0x25, 0x0178},
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
    {WM9093_CMD ,0x03, 0x0138},	
	{WM9093_CMD ,0x1C, 0x0023},	
	{WM9093_CMD ,0x1D, 0x0123},	
	{WM9093_CMD ,0x1C, 0x0123},	
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
    {WM9093_CMD ,0x16, 0x0000},    
	{WM9093_CMD ,0x17, 0x0000},
    {WM9093_CMD ,0x46, 0x0100},
    {WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}

#endif
#endif    
};


static const wm9093_reg_type wm9093_in2_to_out_call_tab[] =	//spk call
{
#if 1	
	{WM9093_CMD ,0x39, 0x000D},		
 	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6020},
//	{WM9093_CMD ,0x1A, 0x0000},
//	{WM9093_CMD ,0x1B, 0x0000},
	{WM9093_CMD ,0x1A, 0x0102},     
	{WM9093_CMD ,0x1B, 0x0102},		
	{WM9093_CMD ,0x36, 0x0004},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},		
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x24, 0x0010},
	{WM9093_CMD ,0x25, 0x0168},		
	{WM9093_CMD ,0x26, 0x0137},		
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
#else	//spk call stereo
	{WM9093_CMD ,0x39, 0x000D},		
 	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x1A, 0x0005},		
	{WM9093_CMD ,0x1B, 0x0005},		
	{WM9093_CMD ,0x1A, 0x0105},		
	{WM9093_CMD ,0x1B, 0x0105},		
	{WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0011},		
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x24, 0x0010},
	{WM9093_CMD ,0x25, 0x0170},		
	{WM9093_CMD ,0x26, 0x0039},		
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}

#endif
};

static const wm9093_reg_type wm9093_in2_to_out_voip_tab[] =
{
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0100},		
	{WM9093_CMD ,0x1B, 0x0100},		
	{WM9093_CMD ,0x36, 0x0005},	
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},		
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x25, 0x0178},		
	{WM9093_CMD ,0x24, 0x0010},		
//	{WM9093_CMD ,0x26, 0x003F},		
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},	
    {WM9093_END_SEQ,0x00,0x00}	
};


static const wm9093_reg_type wm9093_in2_to_out_tab[] =
{
#if 1 
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0100},		
	{WM9093_CMD ,0x1B, 0x0100},		
	{WM9093_CMD ,0x36, 0x0005},	
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},		
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x25, 0x0178},		
	{WM9093_CMD ,0x24, 0x0010},		
	{WM9093_CMD ,0x26, 0x0137},		
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},	
    {WM9093_END_SEQ,0x00,0x00}	
#else
 	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x1A, 0x0000},		
	{WM9093_CMD ,0x1B, 0x0000},		
	{WM9093_CMD ,0x1A, 0x0100},		
	{WM9093_CMD ,0x1B, 0x0100},		
	{WM9093_CMD ,0x36, 0x0005},
	{WM9093_CMD ,0x03, 0x0008},
	{WM9093_CMD ,0x22, 0x0000},		
	{WM9093_CMD ,0x03, 0x0108},
	{WM9093_CMD ,0x24, 0x0010},
	{WM9093_CMD ,0x25, 0x0170},		
	{WM9093_CMD ,0x26, 0x0039},		
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
#endif    
};

static const wm9093_reg_type wm9093_test_tab[] =
{
	{WM9093_DELAY,0x39, 0x0050},
	{WM9093_CMD ,0x39, 0x000D},
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_DELAY,0x00, 50}, 
	{WM9093_CMD ,0x02, 0x6030},
	{WM9093_CMD ,0x17, 0x0001},
	{WM9093_CMD ,0x1A, 0x0100},		
	{WM9093_CMD ,0x1B, 0x0100},	
	{WM9093_CMD ,0x2D, 0x0004},
	{WM9093_CMD ,0x2E, 0x0001},
	{WM9093_CMD ,0x03, 0x0030},
	{WM9093_CMD ,0x2F, 0x0000},
	{WM9093_CMD ,0x30, 0x0000},
	{WM9093_CMD ,0x17, 0x0000},
	{WM9093_CMD ,0x1C, 0x0135},	
	{WM9093_CMD ,0x1D, 0x0135},	
	{WM9093_CMD ,0x1C, 0x0135},	
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}

};



static const wm9093_reg_type wm9093_in2_to_hp_tab[] =	// headset call
{
#if 0	
    {WM9093_CMD ,0x39, 0x000D},
    {WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x60C0},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0101},	
	{WM9093_CMD ,0x19, 0x0101},	
	{WM9093_CMD ,0x2D, 0x0040},
	{WM9093_CMD ,0x2E, 0x0010},
    {WM9093_CMD ,0x03, 0x0030},
    {WM9093_CMD ,0x2F, 0x0040},	
    {WM9093_CMD ,0x30, 0x0010},	
    {WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x1C, 0x002f},	
	{WM9093_CMD ,0x1D, 0x002f},	
	{WM9093_CMD ,0x1C, 0x012f},	
    {WM9093_CMD ,0x46, 0x0100},
    {WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
#else	
    {WM9093_CMD ,0x39, 0x000D},
    {WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x02, 0x6040},
	{WM9093_CMD ,0x16, 0x0001},
	{WM9093_CMD ,0x18, 0x0100},	
	{WM9093_CMD ,0x19, 0x0100},	
	{WM9093_CMD ,0x2D, 0x0010},
	{WM9093_CMD ,0x2E, 0x0010},
	{WM9093_CMD ,0x03, 0x0030},
    {WM9093_CMD ,0x2F, 0x0000},	
    {WM9093_CMD ,0x30, 0x0000},	
	{WM9093_CMD ,0x16, 0x0000},
	{WM9093_CMD ,0x1C, 0x0133},	
	{WM9093_CMD ,0x1D, 0x0133},	
	{WM9093_CMD ,0x1C, 0x0133},	
	{WM9093_CMD ,0x46, 0x0100},
	{WM9093_CMD ,0x49, 0x0100},
    {WM9093_END_SEQ,0x00,0x00}
#endif
};

static const wm9093_reg_type wm9093_in3_to_out_tab[] =
{
	{WM9093_CMD ,0x39, 0x000D},		
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x22, 0x1100},
	{WM9093_CMD ,0x24, 0x0020},
	{WM9093_CMD ,0x15, 0x0000},
//	{WM9093_CMD ,0x25, 0x01e0},     
	{WM9093_CMD ,0x25, 0x01d8},     
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};


static const wm9093_reg_type wm9093_in3_to_out_voip_tab[] =
{
	{WM9093_CMD ,0x39, 0x000D},		
	{WM9093_CMD ,0x01, 0x000B},
	{WM9093_CMD ,0x22, 0x1100},
	{WM9093_CMD ,0x24, 0x0020},
	{WM9093_CMD ,0x15, 0x0000},
	{WM9093_CMD ,0x25, 0x01d8},
	{WM9093_CMD ,0x01, 0x100B},
    {WM9093_END_SEQ,0x00,0x00}
};


static const wm9093_reg_type wm9093_pwroff_tab[] =
{
#if 1 	//shut down sequence
    {WM9093_CMD ,0x46, 0x0100},
    {WM9093_CMD ,0x49, 0x0110},
    {WM9093_CMD ,0x00, 0x0000}, 
    {WM9093_CMD ,0x02, 0x6000},
    {WM9093_CMD ,0x39, 0x0000},

#else
    {WM9093_CMD ,0x00, 0x9093},
#endif    
    {WM9093_END_SEQ,0x00,0x00}};

static unsigned int wm9093_read_reg(struct i2c_client *client, unsigned char reg)
{
	struct i2c_msg xfer[2];
	u16 data = 0xffff;
	int ret;

	/* Write register */
	xfer[0].addr = client->addr;
	xfer[0].flags = 0;
	xfer[0].len = 1;
	xfer[0].buf = &reg;

	/* Read data */
	xfer[1].addr = client->addr;
	xfer[1].flags = I2C_M_RD;
	xfer[1].len = 2;
	xfer[1].buf = (u8 *)&data;

	ret = i2c_transfer(client->adapter, xfer, 2);

	return (data >> 8) | ((data & 0xff) << 8);
}

static void wm9093_write_reg(struct i2c_client *client, u8 reg, int val)
{
	int err;

	struct i2c_msg	msg;
        u8 buf[3];

	msg.addr = (u16)client->addr;
	msg.flags =0;
	msg.len =3;

    buf[0]= reg;
	buf[1]= (val & 0xFF00) >> 8;
    buf[2]= val & 0x00FF;

	msg.buf = &buf[0];

	if ((err = i2c_transfer(client->adapter, &msg, 1)) < 0) {
		dev_err(&client->dev, "i2c write error\n");
	}

	return;
}

void wm_delay_msec(int msec)
{
    unsigned long start;
	start = jiffies;
	while (time_before(jiffies, start+(msec*HZ)/1000))
	{
        udelay(10);
	}
}

void wm9093_write_table(wm9093_reg_type* table)
{
    int i;

	for(i=0; table[i].irc!= WM9093_END_SEQ; i++) {
        if(table[i].irc == WM9093_DELAY){
			wm_delay_msec(table[i].data);
	    }
        else{
			if(wm9093_i2c_dev != NULL)
			    wm9093_write_reg(wm9093_i2c_dev->client, table[i].address, table[i].data);
			else
				printk(KERN_ERR "wm9093 i2c_dev is null");
	    }
	}
}

int wm9093_get_curmode(void)
{
    return (int)wm9093_amp_dev->wm9093_mode;
}
int boot_cnt =0;
void wm9093_configure_path(wm9093_mode_enum mode)
{
	printk("[LUCKYJUN77] wm9093_configure_path : %d\n", mode);

    if(wm9093_amp_dev->wm9093_mode == mode)
		return;


	if(wm9093_control_status == 1)
	{
		printk("[LUCKYJUN77] wm9093_control_status : %d\n", wm9093_control_status);
		return;
	}

	
#if 0
	if(mode == OFF_MODE){
		if(wm9093_amp_dev->wm9093_pstatus > 0 )
            wm9093_amp_dev->wm9093_pstatus = wm9093_amp_dev->wm9093_pstatus - 1;
	}
	else{
        wm9093_amp_dev->wm9093_pstatus = wm9093_amp_dev->wm9093_pstatus + 1;
	}
#endif


	   	if(wm9093_amp_dev->wm9093_mode != OFF_MODE && mode != OFF_MODE) 	
            wm9093_write_table((wm9093_reg_type*)&wm9093_pwroff_tab[0]);


    switch(mode){
#if 0    
        case OFF_MODE : if(wm9093_amp_dev->wm9093_pstatus == 0){
			                wm9093_write_table((wm9093_reg_type*)&wm9093_pwroff_tab[0]);
						wm9093_amp_dev->wm9093_mode = OFF_MODE;
        	            }
						break;
#else
        case OFF_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_pwroff_tab[0]);
						wm9093_amp_dev->wm9093_mode = OFF_MODE;
						wm_delay_msec(100);								
						break;

#endif						
	    case HEADSET_AUDIO_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_hp_tab[0]);
						    wm9093_amp_dev->wm9093_mode = HEADSET_AUDIO_MODE;
							break;
	    case SPEAKER_AUDIO_MODE :
			                if(boot_cnt < 10)
								boot_cnt++;
			                if((get_headset_type() != 0) && (boot_cnt < 4)){
								wm9093_write_table((wm9093_reg_type*)&wm9093_test_tab[0]);
								printk(KERN_INFO "@@WM9093@@ BOOT SOUND HEADSETv\n");
							}
							else{
			                    wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_tab[0]);
							}
	                        wm9093_amp_dev->wm9093_mode = SPEAKER_AUDIO_MODE;
							break;
	    case SPEAKER_HEADSET_DUAL_AUDIO_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_hp_tab[0]);
						 wm9093_amp_dev->wm9093_mode = SPEAKER_HEADSET_DUAL_AUDIO_MODE;
			             break;
	    case RECEIVER_CALL_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in3_to_out_tab[0]);
						   wm9093_amp_dev->wm9093_mode = RECEIVER_CALL_MODE;
						   break;
		case SPEAKER_CALL_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_call_tab[0]);
						   wm9093_amp_dev->wm9093_mode = SPEAKER_CALL_MODE;
						   break;
		case HEADSET_CALL_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_hp_tab[0]);
						   wm9093_amp_dev->wm9093_mode = HEADSET_CALL_MODE;
						   break;
	    case RECEIVER_VOIP_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in3_to_out_voip_tab[0]);
						   wm9093_amp_dev->wm9093_mode = RECEIVER_VOIP_MODE;
						   break;
		case SPEAKER_VOIP_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_voip_tab[0]);
						   wm9093_amp_dev->wm9093_mode = SPEAKER_VOIP_MODE;
						   break;
		case HEADSET_VOIP_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_hp_voip_tab[0]);
						   wm9093_amp_dev->wm9093_mode = HEADSET_VOIP_MODE;
						   break;
        case SPEAKER_FMR_MODE : wm9093_fmradio_volume(s_volume);
			               wm_delay_msec(50);
			  	           wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_FMout_tab[0]);
			               wm9093_amp_dev->wm9093_mode = SPEAKER_FMR_MODE;
							break;								 
		case HEADSET_FMR_MODE : wm9093_fmradio_volume(s_volume); 
		                   wm_delay_msec(50);
			               wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_FMhp_tab[0]);
		                   wm9093_amp_dev->wm9093_mode = HEADSET_FMR_MODE;
							break;				   
		default :
			     break;
    }

}

void wm9093_fmradio_volume(wm9093_fmvolume_enum volume)
{      
 s_volume = volume;
        if(wm9093_i2c_dev != NULL)
        {
           switch(volume)
           {
               case OFF : 
				   printk(KERN_INFO "FMvolume_OFF");
				   wm_delay_msec(50);
				   wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x113);
				   wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x113);	  
		                 wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x113);
				   break;
		 case LEVEL_1: 
		 	          wm_delay_msec(50);
		 	          wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x113);
				   wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x113);	  
		                 wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x113);
				   printk(KERN_INFO "FMvolume_1");	
				   break;
		 case LEVEL_2: 
		 	          wm_delay_msec(50);
		 	          wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x116);
				   wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x116);	  
				   wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x116);
				   printk(KERN_INFO "FMvolume_2");
				   break;
		 case LEVEL_3: 
		 	          wm_delay_msec(50);
		 	          wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x119);
				   wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x119);	  	  
				   wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x119); 
				   printk(KERN_INFO "FMvolume_3");
				   break;
		 case LEVEL_4: 
		 	          wm_delay_msec(50);
		 	          wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x11C);
				   wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x11C);	 	  
				   wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x11C);	 
				   printk(KERN_INFO "FMvolume_4");
				   break;
		 case LEVEL_5:
		 	           wm_delay_msec(50);
		 	           wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x11E);
				    wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x11E);	 	   
				    wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x11E);	 
				   printk(KERN_INFO "FMvolume_5");	
				   break;
		 case LEVEL_6: 
		 	            wm_delay_msec(50);
		 	            wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x122);
				     wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x122);	 		
				     wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x122);
				    printk(KERN_INFO "FMvolume_6");
				    break;
		 case LEVEL_7: 
		 	            wm_delay_msec(50);
		 	            wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x124);
				     wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x124);	 		
				     wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x124);
				     printk(KERN_INFO "FMvolume_7");
				    break;	 
			 		 
		 case LEVEL_8: 
		 	             wm_delay_msec(50);
		 	             wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x126);
				      wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x126);	 		 
				      wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x126);
				     printk(KERN_INFO "FMvolume_8");
                                 break;					  
		 case LEVEL_9: 
		 	             wm_delay_msec(50);
		 	             wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x129);
				      wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x129);	 		 
				      wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x129);
				      printk(KERN_INFO "FMvolume_9");
				      break; 
		 case LEVEL_10: 
		 	             wm_delay_msec(50);
		 	             wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x12A);
					wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x12A);	 	 
				      wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x12A);	
				      printk(KERN_INFO "FMvolume_10");
				      break;	  
		 case LEVEL_11: 
		 	             wm_delay_msec(50);
		 	             wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x12C);
				      wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x12C);		 
				      wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x12C);	
				      printk(KERN_INFO "FMvolume_11");
				      break;	  
		 case LEVEL_12: 
		 	             wm_delay_msec(50);
		 	             wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x12E);
				      wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x12E);	 	 
				      wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x12E);	
				      printk(KERN_INFO "FMvolume_12");
				      break;	   
		 case LEVEL_13: 
		 	              wm_delay_msec(50);
		 	              wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x132);
					wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x132);	 	  
					wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x132);
					printk(KERN_INFO "FMvolume_13");
					break;
		 case LEVEL_14: 
		 	              wm_delay_msec(50);
		 	              wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x134);
				       wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x134);	 		  
					wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x134);	
					printk(KERN_INFO "FMvolume_14");
					break;
		 case LEVEL_15: 
		 	              wm_delay_msec(50);
		 	              wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x137);
					wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x137);	 	  
					wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x137);	
					printk(KERN_INFO "FMvolume_15");
					break;
		 case LEVEL_reset : 
		 	             wm_delay_msec(50);
		 	              wm9093_write_reg(wm9093_i2c_dev->client, 0x1C,0x12d);
					wm9093_write_reg(wm9093_i2c_dev->client, 0x1D,0x12d);	 	  
					wm9093_write_reg(wm9093_i2c_dev->client, 0x26,0x139);
					printk(KERN_INFO "FMRadop_reset");
					break;
		 	
           }
        }
	 else
	    printk(KERN_ERR "wm9093 i2c_dev is null");
}

int wm9093_ext_suspend()
{
	wm9093_write_table((wm9093_reg_type*)&wm9093_pwroff_tab[0]);
	return 0;
}

int wm9093_ext_resume()
{
	switch(wm9093_amp_dev->wm9093_mode){
        case OFF_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_pwroff_tab[0]);
						break;
	    case HEADSET_AUDIO_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_hp_tab[0]);
						    	break;
	    case SPEAKER_AUDIO_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_tab[0]);
	                        	break;
	    case SPEAKER_HEADSET_DUAL_AUDIO_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_hp_tab[0]);
						 		             break;
	    case RECEIVER_CALL_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in3_to_out_tab[0]);
						   		   break;
		case SPEAKER_CALL_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_call_tab[0]);
						   		   break;
		case HEADSET_CALL_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_hp_tab[0]);
						   		   break;
	    case RECEIVER_VOIP_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in3_to_out_voip_tab[0]);
								break;
		case SPEAKER_VOIP_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in2_to_out_voip_tab[0]);
								break;
		case HEADSET_VOIP_MODE : wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_hp_voip_tab[0]);
								break;
        case SPEAKER_FMR_MODE : wm9093_fmradio_volume(s_volume);
			                                    wm_delay_msec(100);
			  	                             wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_FMout_tab[0]);
								break;								 
		case HEADSET_FMR_MODE : wm9093_fmradio_volume(s_volume); 
		                                           wm_delay_msec(100);
			                                   wm9093_write_table((wm9093_reg_type*)&wm9093_in1_to_FMhp_tab[0]);
								break;		
		default :
			     break;
    }

	return 0;
}


EXPORT_SYMBOL(wm9093_get_curmode);
EXPORT_SYMBOL(wm9093_configure_path);
EXPORT_SYMBOL(wm9093_ext_suspend);
EXPORT_SYMBOL(wm9093_ext_resume);
EXPORT_SYMBOL(wm9093_fmradio_volume);


ssize_t wm9093_show_level(struct device *dev, struct device_attribute *attr, char *buf)
{
    struct wm9093_i2c_device *wm9093_dev;
	int r = 0;

	wm9093_dev = dev_get_drvdata(dev);

	r += sprintf(buf+r, "wm9093 mode is : %d\n",wm9093_dev->wm9093_dev->wm9093_mode);

	return r;

}
ssize_t wm9093_store_level(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int level;

	level = simple_strtoul(buf, NULL, 10);

	switch(level){
		//turn off
		case 0 : wm9093_configure_path(OFF_MODE);
			    break;
		//set spk mode
		case 1 : wm9093_configure_path(SPEAKER_AUDIO_MODE);
			    break;
		//set headset mode
		case 2 : wm9093_configure_path(HEADSET_AUDIO_MODE);
			    break;
		//set bypass mode
		case 3 : wm9093_configure_path(RECEIVER_CALL_MODE);
			    break;
		//set dual mode
		case 4 : wm9093_configure_path(SPEAKER_HEADSET_DUAL_AUDIO_MODE);
			    break;
		case 5 : wm9093_configure_path(SPEAKER_CALL_MODE);
			    break;
		case 6 : wm9093_configure_path(HEADSET_CALL_MODE);
			    break;
		default :
			    break;
	}

	return count;
}

DEVICE_ATTR(wm9093_path, 0664, wm9093_show_level, wm9093_store_level);

static ssize_t wm9093_reg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    int r =0;
	int r_data;
	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 0);
	r += sprintf(buf+r, "wm9093 reg 0 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 1);
	r += sprintf(buf+r, "wm9093 reg 1 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 2);
	r += sprintf(buf+r, "wm9093 reg 2 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 3);
	r += sprintf(buf+r, "wm9093 reg 3 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 6);
	r += sprintf(buf+r, "wm9093 reg 6 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 21);
	r += sprintf(buf+r, "wm9093 reg 15 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 22);
	r += sprintf(buf+r, "wm9093 reg 16 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 23);
	r += sprintf(buf+r, "wm9093 reg 17 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 24);
	r += sprintf(buf+r, "wm9093 reg 18 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 25);
	r += sprintf(buf+r, "wm9093 reg 19 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 26);
	r += sprintf(buf+r, "wm9093 reg 1a : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 27);
	r += sprintf(buf+r, "wm9093 reg 1b : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 28);
	r += sprintf(buf+r, "wm9093 reg 1c : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 29);
	r += sprintf(buf+r, "wm9093 reg 1d : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 34);
	r += sprintf(buf+r, "wm9093 reg 22 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 35);
	r += sprintf(buf+r, "wm9093 reg 23 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 36);
	r += sprintf(buf+r, "wm9093 reg 24 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 37);
	r += sprintf(buf+r, "wm9093 reg 25 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 38);
	r += sprintf(buf+r, "wm9093 reg 26 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 40);
	r += sprintf(buf+r, "wm9093 reg 28 : 0x%4x\n",r_data);
	
	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 45);
	r += sprintf(buf+r, "wm9093 reg 2d : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 46);
	r += sprintf(buf+r, "wm9093 reg 2e : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 47);
	r += sprintf(buf+r, "wm9093 reg 2f : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 48);
	r += sprintf(buf+r, "wm9093 reg 30 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 54);
	r += sprintf(buf+r, "wm9093 reg 36 : 0x%4x\n",r_data);

#if 1
	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 57);
	r += sprintf(buf+r, "wm9093 reg 39 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 76);
	r += sprintf(buf+r, "wm9093 reg 4c : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 84);
	r += sprintf(buf+r, "wm9093 reg 54 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 96);
	r += sprintf(buf+r, "wm9093 reg 60 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 98);
	r += sprintf(buf+r, "wm9093 reg 62 : 0x%4x\n",r_data);

	r_data = wm9093_read_reg(wm9093_i2c_dev->client, 99);
	r += sprintf(buf+r, "wm9093 reg 63 : 0x%4x\n",r_data);
	
#endif
	
	return r;
}

ssize_t wm9093_reg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int reg, data;
	char *r, *d;

	printk("[LUCKYJUN77] wm9093_reg_store : %s\n", buf);
	
	r= &buf[0];
	printk("[LUCKYJUN77] r : %s\n", r);	
	
	d= &buf[5];
	printk("[LUCKYJUN77] d : %s\n", d);		
	
	reg = simple_strtoul(r, NULL, 16);
	data = simple_strtoul(d, NULL, 16);

	printk("[LUCKYJUN77] reg: %d, data : %d\n", reg, data);			

    wm9093_write_reg(wm9093_i2c_dev->client, (u8)reg, data);

	return count;
}

#if 1	
static DEVICE_ATTR(wm9093_data, 0644, wm9093_reg_show, wm9093_reg_store);
#else
static DEVICE_ATTR(wm9093_data, 0666, wm9093_reg_show, wm9093_reg_store);
#endif	


ssize_t wm9093_show_status(struct device *dev, struct device_attribute *attr, char *buf)
{

    struct wm9093_i2c_device *wm9093_dev;
	int r = 0;

	wm9093_dev = dev_get_drvdata(dev);

	r += sprintf(buf+r, "wm9093 status is : %d\n", wm9093_control_status);

	return r;

}
ssize_t wm9093_store_status(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int status;

	status = simple_strtoul(buf, NULL, 10);

	printk("[LUCKYJUN77] status: %d\n", status);

	if(status == 0 || status == 1)
	{
		wm9093_control_status = status;
	}

	return count;
}

DEVICE_ATTR(wm9093_status, 0644, wm9093_show_status, wm9093_store_status);


static void wm9093_init(struct i2c_client *client)
{

    wm9093_write_table((wm9093_reg_type*)&wm9093_pwroff_tab[0]);
	wm9093_amp_dev->wm9093_mode = OFF_MODE;
    wm9093_amp_dev->wm9093_pstatus = 0;
	return;
}

static int __init wm9093_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	int err;
	if (i2c_get_clientdata(client))
		return -EBUSY;
    
	wm9093_i2c_dev = kzalloc(sizeof(struct wm9093_i2c_device), GFP_KERNEL);
    wm9093_amp_dev = kzalloc(sizeof(struct wm9093_device), GFP_KERNEL);

	wm9093_i2c_dev->client = client;
	wm9093_i2c_dev->wm9093_dev = wm9093_amp_dev;


    struct device *wm9093_dev = &client->dev;
    wm9093_reg = regulator_get(wm9093_dev,"vpll2");
    if (wm9093_reg == NULL) {
        printk("wm9093 : Failed to get amp power regulator !! \n");
	}

    regulator_enable(wm9093_reg);


	i2c_set_clientdata(client, wm9093_i2c_dev);

    wm9093_init(client);

    err = device_create_file(&client->dev, &dev_attr_wm9093_path);
	err = device_create_file(&client->dev, &dev_attr_wm9093_data);
	err = device_create_file(&client->dev, &dev_attr_wm9093_status);	

	return 0;

}

static int wm9093_remove(struct i2c_client *client)
{
	i2c_set_clientdata(client, NULL);
    device_remove_file(&client->dev, &dev_attr_wm9093_path);
	device_remove_file(&client->dev, &dev_attr_wm9093_data);
	device_remove_file(&client->dev, &dev_attr_wm9093_status);		

	kfree(wm9093_amp_dev);
	kfree(wm9093_i2c_dev);
	return 0;
}


static int wm9093_suspend(struct i2c_client *client, pm_message_t mesg)
{
	printk("[LUCKYJUN77]wm9093_suspend\n");
	if(wm9093_amp_dev->wm9093_mode == OFF_MODE)	
	{
    regulator_disable(wm9093_reg);	
    }
    else
    {
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0xee, VPLL2_DEV_GRP );   
		twl_i2c_write_u8(TWL4030_MODULE_PM_RECEIVER, 0x0e, 0x35 );  
	}

	return 0;

}

static int wm9093_resume(struct i2c_client *client)
{
	printk("[LUCKYJUN77]wm9093_resume\n");
	if(wm9093_amp_dev->wm9093_mode == OFF_MODE)	
    regulator_enable(wm9093_reg);		

	return 0;
}



unsigned int get_wm9093_mode(void)
{    
	return (unsigned int)wm9093_amp_dev->wm9093_mode;
}
EXPORT_SYMBOL_GPL(get_wm9093_mode);


static const struct i2c_device_id wm9093_ids[] = {
	{ WM9093_I2C_NAME, 0 },	/*wm9093*/
	{ /* end of list */ },
};

static struct i2c_driver wm9093_i2c_driver = {
	.probe = wm9093_probe,
	.remove = wm9093_remove,
	.suspend	= wm9093_suspend,
	.resume		= wm9093_resume,
	.id_table	= wm9093_ids,
	.driver = {
		.name = WM9093_I2C_NAME,
		.owner = THIS_MODULE,
    },
};

static int __init ext_amp_wm9093_init(void)
{
    int ret;
	ret = i2c_add_driver(&wm9093_i2c_driver);
	if (ret != 0)
		printk(KERN_ERR "can't add i2c driver");
	return ret;
}

static void __exit ext_amp_wm9093_exit(void)
{
	i2c_del_driver(&wm9093_i2c_driver);
}

module_init(ext_amp_wm9093_init);
module_exit(ext_amp_wm9093_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("wm9093 audio ext amp Driver");
MODULE_LICENSE("GPL");


