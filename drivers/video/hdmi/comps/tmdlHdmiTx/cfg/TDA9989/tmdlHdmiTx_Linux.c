/**
 * Copyright (C) 2009 NXP N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of NXP N.V. and is confidential in nature. Under no circumstances
 * is this software to be  exposed to or placed under an Open Source License of
 * any type without the expressed written permission of NXP N.V.
 *
 * \file          tmdlHdmiTx_LinuxCfg.c
 *
 * \version       Revision: 1
 *
 * \date          Date: October 1st, 2009
 *
 * \brief         Linux devlib driver interface for the TDA998x HDMI Transmitters
 *
 * \section info  Change Information
 *
 * \verbatim

   History:       tmdlHdmiTx_LinuxCfg.c
 *
 * *****************  Version 1  *****************
 * User: A. Lepine Date: October 1st, 2009
 * initial version
 *

   \endverbatim
 *
*/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/delay.h>
// == 2011.05.11 === hycho@ubiquix.com START
#include <linux/slab.h>
#include <linux/semaphore.h>
// == 2011.05.11 === hycho@ubiquix.com END

#include "tmdlHdmiTx_IW.h"
#include "tmdlHdmiTx.h"
#include "tmdlHdmiTx_cfg.h"

#ifdef TMFL_CEC_AVAILABLE

#include "tmdlHdmiCEC_functions.h"
#define CEC_UNIT_I2C_ADDRESS_0 0x34
#define CEC_UNIT_I2C_ADDRESS_1 0x34

#endif

struct i2c_client *GetThisI2cClient(void);
unsigned char  my_i2c_data[255];

/*============================================================================*/
/*                                MACRO                                       */
/*============================================================================*/

/* macro for quick error handling */
#define RETIF(cond, rslt) if ((cond)){return (rslt);}
#define I2C_M_WR 0


/******************************************************************************
 ******************************************************************************
 *                 THIS PART CAN BE MODIFIED BY CUSTOMER                      *
 ******************************************************************************
 *****************************************************************************/

tmErrorCode_t TxI2cReadFunction(tmbslHdmiTxSysArgs_t *pSysArgs);
tmErrorCode_t TxI2cWriteFunction(tmbslHdmiTxSysArgs_t *pSysArgs);

/* I2C adress of the unit                                                     */
/* Put there the I2C slave adress of the Tx transmitter IC                    */
#define UNIT_I2C_ADDRESS_0 0x70

/* Priority of the command task                                               */
/* Command task is an internal task that handles incoming event from the IC   */
/* put there a value that will ensure a response time of ~20ms in your system */
#define COMMAND_TASK_PRIORITY_0  250
#define COMMAND_TASK_PRIORITY_1  250

/* Priority of the hdcp check tasks */
/* HDCP task is an internal task that handles periodical HDCP processing      */
/* put there a value that will ensure a response time of ~20ms in your system */
#define HDCP_CHECK_TASK_PRIORITY_0  250

/* Stack size of the command tasks */
/* This value depends of the type of CPU used, and also from the length of    */
/* the customer callbacks. Increase this value if you are making a lot of     */
/* processing (function calls & local variables) and that you experience      */
/* stack overflows                                                            */
#define COMMAND_TASK_STACKSIZE_0 128
#define COMMAND_TASK_STACKSIZE_1 128

/* stack size of the hdcp check tasks */
/* This value depends of the type of CPU used, default value should be enough */
/* for all configuration                                                      */
#define HDCP_CHECK_TASK_STACKSIZE_0 128

/* Size of the message queues for command tasks                               */
/* This value defines the size of the message queue used to link the          */
/* the tmdlHdmiTxHandleInterrupt function and the command task. The default   */
/* value below should fit any configuration                                   */
#define COMMAND_TASK_QUEUESIZE_0 128
#define COMMAND_TASK_QUEUESIZE_1 128

/* HDCP key seed                                                              */
/* HDCP key are stored encrypted into the IC, this value allows the IC to     */
/* decrypt them. This value is provided to the customer by NXP customer       */
/* support team.                                                              */
#define KEY_SEED 0xCAFE

/* Video port configuration for YUV444 input                                  */
/* You can specify in this table how are connected video ports in case of     */
/* YUV444 input signal. Each line of the array corresponds to a quartet of    */
/* pins of one video port (see comment on the left to identify them). Just    */
/* change the enum to specify which signal you connected to it. See file      */
/* tmdlHdmiTx_cfg.h to get the list of possible values                        */
const tmdlHdmiTxCfgVideoSignal444 videoPortMapping_YUV444[MAX_UNITS][6] = {
    {
        TMDL_HDMITX_VID444_BU_0_TO_3,   /* Signals connected to VPA[0..3] */
        TMDL_HDMITX_VID444_BU_4_TO_7,   /* Signals connected to VPA[4..7] */
        TMDL_HDMITX_VID444_GY_0_TO_3,   /* Signals connected to VPB[0..3] */
        TMDL_HDMITX_VID444_GY_4_TO_7,   /* Signals connected to VPB[4..7] */
        TMDL_HDMITX_VID444_VR_0_TO_3,   /* Signals connected to VPC[0..3] */
        TMDL_HDMITX_VID444_VR_4_TO_7    /* Signals connected to VPC[4..7] */
    }
};

/* Video port configuration for RGB444 input                                  */
/* You can specify in this table how are connected video ports in case of     */
/* RGB444 input signal. Each line of the array corresponds to a quartet of    */
/* pins of one video port (see comment on the left to identify them). Just    */
/* change the enum to specify which signal you connected to it. See file      */
/* tmdlHdmiTx_cfg.h to get the list of possible values                        */
const tmdlHdmiTxCfgVideoSignal444 videoPortMapping_RGB444[MAX_UNITS][6] = {
    {
// changhoony.lee@lge.com 2010-4-5 changed.

        TMDL_HDMITX_VID444_BU_0_TO_3,   /* Signals connected to VPA[0..3] */
        TMDL_HDMITX_VID444_BU_4_TO_7,   /* Signals connected to VPA[4..7] */
        TMDL_HDMITX_VID444_GY_0_TO_3,   /* Signals connected to VPB[0..3] */
        TMDL_HDMITX_VID444_GY_4_TO_7,   /* Signals connected to VPB[4..7] */
        TMDL_HDMITX_VID444_VR_0_TO_3,   /* Signals connected to VPC[0..3] */
        TMDL_HDMITX_VID444_VR_4_TO_7    /* Signals connected to VPC[4..7] */

     //   TMDL_HDMITX_VID444_VR_0_TO_3,   /* Signals connected to VPB[0..3] */
     //   TMDL_HDMITX_VID444_VR_4_TO_7,   /* Signals connected to VPB[4..7] */
     //   TMDL_HDMITX_VID444_GY_0_TO_3,   /* Signals connected to VPA[0..3] */
     //   TMDL_HDMITX_VID444_GY_4_TO_7,   /* Signals connected to VPA[4..7] */
     //   TMDL_HDMITX_VID444_BU_0_TO_3,   /* Signals connected to VPC[0..3] */
     //   TMDL_HDMITX_VID444_BU_4_TO_7    /* Signals connected to VPC[4..7] */

/*         TMDL_HDMITX_VID444_BU_0_TO_3,   /\* Signals connected to VPA[0..3] *\/ */
/*         TMDL_HDMITX_VID444_BU_4_TO_7,   /\* Signals connected to VPA[4..7] *\/ */
/*         TMDL_HDMITX_VID444_GY_0_TO_3,   /\* Signals connected to VPB[0..3] *\/ */
/*         TMDL_HDMITX_VID444_GY_4_TO_7,   /\* Signals connected to VPB[4..7] *\/ */
/*         TMDL_HDMITX_VID444_VR_0_TO_3,   /\* Signals connected to VPC[0..3] *\/ */
/*         TMDL_HDMITX_VID444_VR_4_TO_7    /\* Signals connected to VPC[4..7] *\/ */
    }
};

/* Video port configuration for YUV422 input                                  */
/* You can specify in this table how are connected video ports in case of     */
/* YUV422 input signal. Each line of the array corresponds to a quartet of    */
/* pins of one video port (see comment on the left to identify them). Just    */
/* change the enum to specify which signal you connected to it. See file      */
/* tmdlHdmiTx_cfg.h to get the list of possible values                        */
const tmdlHdmiTxCfgVideoSignal422 videoPortMapping_YUV422[MAX_UNITS][6] = {
    {
        TMDL_HDMITX_VID422_Y_4_TO_7,           /* Signals connected to VPA[0..3] */    
        TMDL_HDMITX_VID422_Y_8_TO_11,          /* Signals connected to VPA[4..7] */    
        TMDL_HDMITX_VID422_UV_4_TO_7,          /* Signals connected to VPB[0..3] */
        TMDL_HDMITX_VID422_UV_8_TO_11,         /* Signals connected to VPB[4..7] */
        TMDL_HDMITX_VID422_NOT_CONNECTED,      /* Signals connected to VPC[0..3] */
        TMDL_HDMITX_VID422_NOT_CONNECTED       /* Signals connected to VPC[4..7] */
    }
};

/* Video port configuration for CCIR656 input                                 */
/* You can specify in this table how are connected video ports in case of     */
/* CCIR656 input signal. Each line of the array corresponds to a quartet of   */
/* pins of one video port (see comment on the left to identify them). Just    */
/* change the enum to specify which signal you connected to it. See file      */
/* tmdlHdmiTx_cfg.h to get the list of possible values                        */
const tmdlHdmiTxCfgVideoSignalCCIR656 videoPortMapping_CCIR656[MAX_UNITS][6] = {
    {
        TMDL_HDMITX_VIDCCIR_4_TO_7,         /* Signals connected to VPA[0..3] */
        TMDL_HDMITX_VIDCCIR_8_TO_11,        /* Signals connected to VPA[4..7] */
        TMDL_HDMITX_VIDCCIR_NOT_CONNECTED,  /* Signals connected to VPB[0..3] */
        TMDL_HDMITX_VIDCCIR_NOT_CONNECTED,  /* Signals connected to VPB[4..7] */
        TMDL_HDMITX_VIDCCIR_NOT_CONNECTED,  /* Signals connected to VPC[0..3] */
        TMDL_HDMITX_VIDCCIR_NOT_CONNECTED   /* Signals connected to VPC[4..7] */
    }
};

/* Audio port configuration for SPDIF                                         */
/* Here you can specify the audio port routing configuration for SPDIF input. */
/* enableAudioPortSPDIF and groundAudioPortSPDIF should be filled with a      */
/* value build as follows : each bit represent an audio port, LSB is port 0.  */
/* enableAudioClockPortSPDIF and groundAudioClockPortSPDIF can be configured  */
/* with the corresponding enums (See file tmdlHdmiTx_cfg.h for more details). */
UInt8 enableAudioPortSPDIF[MAX_UNITS]      = {0x02};
UInt8 enableAudioClockPortSPDIF[MAX_UNITS] = {DISABLE_AUDIO_CLOCK_PORT};
UInt8 groundAudioPortSPDIF[MAX_UNITS]      = {0xFD};
UInt8 groundAudioClockPortSPDIF[MAX_UNITS] = {ENABLE_AUDIO_CLOCK_PORT_PULLDOWN};

/* Audio port configuration for I2S                                           */
/* Here you can specify the audio port routing configuration for SPDIF input. */
/* enableAudioPortI2S and groundAudioPortI2S should be filled with a          */
/* value build as follows : each bit represent an audio port, LSB is port 0.  */
/* enableAudioClockPortI2S and groundAudioClockPortI2S can be configured      */
/* with the corresponding enums (See file tmdlHdmiTx_cfg.h for more details). */
UInt8 enableAudioPortI2S[MAX_UNITS]        = {0x03};
UInt8 enableAudioPortI2S8C[MAX_UNITS]      = {0x1f};
UInt8 enableAudioClockPortI2S[MAX_UNITS]   = {ENABLE_AUDIO_CLOCK_PORT};
UInt8 groundAudioPortI2S[MAX_UNITS]        = {0xfc};
UInt8 groundAudioPortI2S8C[MAX_UNITS]      = {0xe0};
UInt8 groundAudioClockPortI2S[MAX_UNITS]   = {DISABLE_AUDIO_CLOCK_PORT_PULLDOWN};

/* Audio port configuration for OBA                                           */
/* Here you can specify the audio port routing configuration for SPDIF input. */
/* enableAudioPortOBA and groundAudioPortOBA should be filled with a          */
/* value build as follows : each bit represent an audio port, LSB is port 0.  */
/* enableAudioClockPortOBA and groundAudioClockPortOBA can be configured      */
/* with the corresponding enums (See file tmdlHdmiTx_cfg.h for more details). */
UInt8 enableAudioPortOBA[MAX_UNITS]        = {0xFF};
UInt8 enableAudioClockPortOBA[MAX_UNITS]   = {ENABLE_AUDIO_CLOCK_PORT};
UInt8 groundAudioPortOBA[MAX_UNITS]        = {0x00};
UInt8 groundAudioClockPortOBA[MAX_UNITS]   = {DISABLE_AUDIO_CLOCK_PORT_PULLDOWN};

/* Audio port configuration for DST                                           */
/* Here you can specify the audio port routing configuration for SPDIF input. */
/* enableAudioPortDST and groundAudioPortDST should be filled with a          */
/* value build as follows : each bit represent an audio port, LSB is port 0.  */
/* enableAudioClockPortDST and groundAudioClockPortDST can be configured      */
/* with the corresponding enums (See file tmdlHdmiTx_cfg.h for more details). */
UInt8 enableAudioPortDST[MAX_UNITS]        = {0xFF};
UInt8 enableAudioClockPortDST[MAX_UNITS]   = {ENABLE_AUDIO_CLOCK_PORT};
UInt8 groundAudioPortDST[MAX_UNITS]        = {0x00};
UInt8 groundAudioClockPortDST[MAX_UNITS]   = {DISABLE_AUDIO_CLOCK_PORT_PULLDOWN};

/* Audio port configuration for HBR                                           */
/* Here you can specify the audio port routing configuration for SPDIF input. */
/* enableAudioPortHBR and groundAudioPortHBR should be filled with a          */
/* value build as follows : each bit represent an audio port, LSB is port 0.  */
/* enableAudioClockPortHBR and groundAudioClockPortHBR can be configured      */
/* with the corresponding enums (See file tmdlHdmiTx_cfg.h for more details). */
UInt8 enableAudioPortHBR[MAX_UNITS]        = {0x1f};
UInt8 enableAudioClockPortHBR[MAX_UNITS]   = {ENABLE_AUDIO_CLOCK_PORT};
UInt8 groundAudioPortHBR[MAX_UNITS]        = {0xe0};
UInt8 groundAudioClockPortHBR[MAX_UNITS]   = {DISABLE_AUDIO_CLOCK_PORT_PULLDOWN};


/*
 *
 * Linux wrapping starts here...............................
 *
 */

/* 
 *  Write a bloc to a register in Tx device.
 */
int blockwrite_reg(struct i2c_client *client,
		   u8 reg, u16 alength, u8 *val)
{
    int err = 0, i;
    struct i2c_msg msg[1];

    if (!client->adapter) {
        printk(KERN_ERR "<%s> ERROR: No HDMI Device\n", __func__);
        return -ENODEV;
    }

    msg->addr = client->addr;
    msg->flags = I2C_M_WR;
    msg->len = alength+1;
    msg->buf = my_i2c_data;

    msg->buf[0] = reg;   
    for (i=1; i<=alength; i++) msg->buf[i] = (*val++);

    err = i2c_transfer(client->adapter, msg, 1);
    udelay(50);
    
    if(err<0)
        printk(KERN_INFO "DBG blockwrite_reg addr:%x reg:%d data:%x %s\n",msg->addr,reg,val,(err<0?"ERROR":""));

/*    dev_dbg(&client->dev, "<%s> i2c Block write at 0x%x, " */
/*            "*val=%d flags=%d byte[%d] err=%d\n", */
/*            __func__, data[0], data[1], msg->flags, i, err); */
    return (err < 0?err:0);
}

/* 
 *  Read a bloc to a register in Tx device.
 */
int blockread_reg(struct i2c_client *client,
		  u8 reg, u16 alength, u8 *val)
{
   int err = 0;
   struct i2c_msg msg[1];
   u8 data[2];

   if (!client->adapter) {
      printk(KERN_ERR "<%s> ERROR: No HDMI Device\n", __func__);
      //dev_err(&client->dev, "<%s> ERROR: No HDMI Device\n", __func__);
      return -ENODEV;
   }

   msg->addr = client->addr;
   msg->flags = I2C_M_WR;
   msg->len = 1;
   msg->buf = data;
   data[0] = reg; /* High byte goes out first */
   err = i2c_transfer(client->adapter, msg, 1);
/*    printk(KERN_INFO "DBG blockread_reg #1 addr:%x len:%d buf:%02x%02x%02x%02x %s\n",msg->addr,msg->len,\ */
/*           msg->buf[0],msg->buf[1],msg->buf[2],msg->buf[3],(err<0?"ERROR":"")); */
   if (err<0) goto BLOCK_READ_OUPS;

   msg->flags = I2C_M_RD;
   msg->len = alength; 
   msg->buf = val;
   err = i2c_transfer(client->adapter, msg, 1);
/*    printk(KERN_INFO "DBG blockread_reg #2 addr:%x len:%d buf:%02x%02x%02x%02x %s\n",msg->addr,msg->len,\ */
/*           msg->buf[0],msg->buf[1],msg->buf[2],msg->buf[3],(err<0?"ERROR":"")); */

   if (err<0) goto BLOCK_READ_OUPS;

   return 0;
   
 BLOCK_READ_OUPS:
    /*
   dev_err(&client->dev, "<%s> ERROR:  i2c Read at 0x%x, "
           "*val=%d flags=%d bytes err=%d\n",
    */
  printk(KERN_ERR "<%s> ERROR:  i2c Read at 0x%x, *val=%d flags=%d bytes err=%d\n",
           __func__, reg, *val, msg->flags, err);
   return err;
}

/* 
 *  Write a byte to a register in Tx device.
 */
int write_reg(struct i2c_client *client, u8 reg, u8 val)
{
   int err = 0;
   struct i2c_msg msg[1];
   u8 data[2];
   int retries = 0;

   if (!client->adapter) {
      //dev_err(&client->dev, "<%s> ERROR: No HDMI Device\n", __func__);
      printk(KERN_ERR "<%s> ERROR: No HDMI Device\n", __func__);
      return -ENODEV;
   }

 retry:
   msg->addr = client->addr;
   msg->flags = I2C_M_WR;
   msg->len = 2;
   msg->buf = data;

   data[0] = reg;
   data[1] = val;

   err = i2c_transfer(client->adapter, msg, 1);
    #if 0
   dev_dbg(&client->dev, "<%s> i2c write at=%x "
	   "val=%x flags=%d err=%d\n",
	   __func__, data[0], data[1], msg->flags, err);
    #endif
   udelay(50);

   //2012.07.24 goochang.jeong@lge.com i2c error fix
   if (retries != 0)
	set_current_state(TASK_RUNNING);  
//    printk(KERN_INFO "DBG write_reg addr:%x reg:%d data:%x %s\n",msg->addr,reg,val,(err<0?"ERROR":"")); 

   if (err >= 0)
      return 0;
    #if 0
   dev_err(&client->dev, "<%s> ERROR: i2c write at=%x "
	   "val=%x flags=%d err=%d\n",
	   __func__, data[0], data[1], msg->flags, err);
    #endif
   printk(KERN_ERR "<%s> ERROR: i2c write at=%x "
	   "val=%x flags=%d err=%d\n",
	   __func__, data[0], data[1], msg->flags, err);   
   if (retries <= 5) {
      //dev_info(&client->dev, "Retrying I2C... %d\n", retries);
      printk(KERN_ERR "<%s> ERROR: Retrying I2C... %d\n", __func__, retries);
      retries++;
      set_current_state(TASK_UNINTERRUPTIBLE);
      schedule_timeout(msecs_to_jiffies(20));
      goto retry;
   }
   return err;
}

/*
 *  Read a byte from a register in Tx device.
 */
int read_reg(struct i2c_client *client, u16 data_length, u8 reg, u8 *val)
{
   int err = 0;
   struct i2c_msg msg[1];
   u8 data[2];

   if (!client->adapter) {
      //dev_err(&client->dev, "<%s> ERROR: No HDMI Device\n", __func__);
      printk(KERN_ERR "<%s> ERROR: No HDMI Device\n", __func__);
      return -ENODEV;
   }

   msg->addr = client->addr;
   msg->flags = I2C_M_WR;
   msg->len = 1;
   msg->buf = data;

   data[0] = reg;
   err = i2c_transfer(client->adapter, msg, 1);
//   dev_dbg(&client->dev, "<%s> i2c Read1 reg=%x val=%d "
//	   "flags=%d err=%d\n",
//	   __func__, reg, data[1], msg->flags, err);

   if (err >= 0) {
      mdelay(3);
      msg->flags = I2C_M_RD;
      msg->len = data_length;
      err = i2c_transfer(client->adapter, msg, 1);
   }

   if (err >= 0) {
      *val = 0;
      if (data_length == 1)
	 *val = data[0];
      else if (data_length == 2)
	 *val = data[1] + (data[0] << 8);
//      dev_dbg(&client->dev, "<%s> i2c Read2 at 0x%x, *val=%d "
//	      "flags=%d err=%d\n",
//	      __func__, reg, *val, msg->flags, err);
      return 0;
   }

  printk(KERN_ERR "<%s> ERROR: i2c Read at 0x%x, *val=%d flags=%d err=%d\n",
	   __func__, reg, *val, msg->flags, err);
//   dev_err(&client->dev, "<%s> ERROR: i2c Read at 0x%x, "
//	   "*val=%d flags=%d err=%d\n",
//	   __func__, reg, *val, msg->flags, err);
   return err;
}

/* The following function must be rewritten by the customer to fit its own    */
/* SW infrastructure. This function allows reading through I2C bus.           */
/* tmbslHdmiTxSysArgs_t definition is located into tmbslHdmiTx_type.h file.   */
tmErrorCode_t TxI2cReadFunction(tmbslHdmiTxSysArgs_t *pSysArgs)
{

   tmErrorCode_t errCode = TM_OK;
   struct i2c_client *client=GetThisI2cClient();
   u32 client_main_addr=client->addr;

   /* DevLib needs address control, so let it be */ 
   client->addr=pSysArgs->slaveAddr;

   if (pSysArgs->lenData == 1) {
      /* single byte */
      errCode = read_reg(GetThisI2cClient(),1,pSysArgs->firstRegister,pSysArgs->pData);
   }
   else {
      /* block */
      errCode = blockread_reg(GetThisI2cClient(), \
			      pSysArgs->firstRegister, \
			      pSysArgs->lenData, \
			      pSysArgs->pData);
   }
   
   /* restore default client address */
   client->addr=client_main_addr;

   return errCode;
}

/* The following function must be rewritten by the customer to fit its own    */
/* SW infrastructure. This function allows writing through I2C bus.           */
/* tmbslHdmiTxSysArgs_t definition is located into tmbslHdmiTx_type.h file.   */
tmErrorCode_t TxI2cWriteFunction(tmbslHdmiTxSysArgs_t *pSysArgs)
{

   tmErrorCode_t errCode = TM_OK;
   struct i2c_client *client=GetThisI2cClient();
   u32 client_main_addr=client->addr;

   /* DevLib needs address control, so let it be */ 
   client->addr=pSysArgs->slaveAddr;

//	printk(KERN_INFO "[hycho1] -- TxI2cWriteFunction\n");		
   
   if (pSysArgs->lenData == 1) {
      /* single byte */
      errCode = write_reg(GetThisI2cClient(),pSysArgs->firstRegister,*pSysArgs->pData);
//	printk(KERN_INFO "[hycho2] -- TxI2cWriteFunction = %ld\n", errCode);		
   }
   else {
      /* block */
      errCode = blockwrite_reg(GetThisI2cClient(),  \
                               pSysArgs->firstRegister, \
                               pSysArgs->lenData,       \
                               pSysArgs->pData);
   }
   
   /* restore default client address */
   client->addr=client_main_addr;
//	printk(KERN_INFO "[hycho3] -- TxI2cWriteFunction = %ld\n", errCode);		
   return errCode;
}

/******************************************************************************
    \brief  This function blocks the current task for the specified amount time. 
            This is a passive wait.

    \param  Duration    Duration of the task blocking in milliseconds.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_NO_RESOURCES: the resource is not available

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWWait
(
    UInt16 duration
)
{

    mdelay((unsigned long)duration);

    return(TM_OK);
}

/******************************************************************************
    \brief  This function creates a semaphore.

    \param  pHandle Pointer to the handle buffer.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_NO_RESOURCES: the resource is not available
            - TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS: an input parameter is
              inconsistent

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreCreate
(
    tmdlHdmiTxIWSemHandle_t *pHandle
)
{
    struct semaphore * mutex;
    
    /* check that input pointer is not NULL */
    RETIF(pHandle == Null, TMDL_ERR_DLHDMITX_INCONSISTENT_PARAMS)

    mutex = (struct semaphore *)kmalloc(sizeof(struct semaphore),GFP_KERNEL);
    if (!mutex) {
       printk(KERN_ERR "malloc failed in %s\n",__func__);
       return TMDL_ERR_DLHDMITX_NO_RESOURCES;
    }
    
    init_MUTEX(mutex);
    *pHandle = (tmdlHdmiTxIWSemHandle_t)mutex;

    RETIF(pHandle == NULL, TMDL_ERR_DLHDMITX_NO_RESOURCES)

    return(TM_OK);
}

/******************************************************************************
    \brief  This function destroys an existing semaphore.

    \param  Handle  Handle of the semaphore to be destroyed.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreDestroy
(
    tmdlHdmiTxIWSemHandle_t handle
)
{
   RETIF(handle == False, TMDL_ERR_DLHDMITX_BAD_HANDLE);
   
   if (atomic_read((atomic_t*)&((struct semaphore *)handle)->count) < 1) {
      printk(KERN_ERR "release catched semaphore");
   }
   
   kfree((void*)handle);
   
   return(TM_OK);
}

/******************************************************************************
    \brief  This function acquires the specified semaphore.

    \param  Handle  Handle of the semaphore to be acquired.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreP
(
    tmdlHdmiTxIWSemHandle_t handle
)
{

    down((struct semaphore *)handle);

    return(TM_OK);
}

/******************************************************************************
    \brief  This function releases the specified semaphore.

    \param  Handle  Handle of the semaphore to be released.

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMITX_BAD_HANDLE: the handle number is wrong

******************************************************************************/
tmErrorCode_t tmdlHdmiTxIWSemaphoreV
(
    tmdlHdmiTxIWSemHandle_t handle
)
{
    up((struct semaphore *)handle);

    return(TM_OK);
}

/*
 *
 * Linux wrapping starts here...............................
 *
 */

/*****************************************************************************
******************************************************************************
*                THIS PART MUST NOT BE MODIFIED BY CUSTOMER                  *
******************************************************************************
*****************************************************************************/

/* DO NOT MODIFY, those tables are filled dynamically by                      */
/* dlHdmiTxGenerateCfgVideoPortTables API                                     */
UInt8 mirrorTableCCIR656[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 swapTableCCIR656[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 enableVideoPortCCIR656[MAX_UNITS][3] = {{0x00, 0x00, 0x00}};
UInt8 groundVideoPortCCIR656[MAX_UNITS][3] = {{0xFF, 0xFF, 0xFF}};
UInt8 mirrorTableYUV422[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 swapTableYUV422[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 enableVideoPortYUV422[MAX_UNITS][3] = {{0x00, 0x00, 0x00}};
UInt8 groundVideoPortYUV422[MAX_UNITS][3] = {{0xFF, 0xFF, 0xFF}};
UInt8 mirrorTableYUV444[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 swapTableYUV444[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 enableVideoPortYUV444[MAX_UNITS][3] = {{0x00, 0x00, 0x00}};
UInt8 groundVideoPortYUV444[MAX_UNITS][3] = {{0xFF, 0xFF, 0xFF}};
UInt8 mirrorTableRGB444[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 swapTableRGB444[MAX_UNITS][6] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
UInt8 enableVideoPortRGB444[MAX_UNITS][3] = {{0x00, 0x00, 0x00}};
UInt8 groundVideoPortRGB444[MAX_UNITS][3] = {{0xFF, 0xFF, 0xFF}};

/* DO NOT MODIFY, this table is used for transmission of the configuration to */
/* the core driver                                                            */
tmdlHdmiTxDriverConfigTable_t driverConfigTableTx[MAX_UNITS] = {
    {
        COMMAND_TASK_PRIORITY_0,
        COMMAND_TASK_STACKSIZE_0,
        COMMAND_TASK_QUEUESIZE_0,
        HDCP_CHECK_TASK_PRIORITY_0,
        HDCP_CHECK_TASK_STACKSIZE_0,
        UNIT_I2C_ADDRESS_0,
        TxI2cReadFunction,
        TxI2cWriteFunction,
        Null,                           /* filled dynamically, do not modify */
        &mirrorTableCCIR656[0][0],      /* filled dynamically, do not modify */
        &swapTableCCIR656[0][0],        /* filled dynamically, do not modify */
        &enableVideoPortCCIR656[0][0],  /* filled dynamically, do not modify */
        &groundVideoPortCCIR656[0][0],  /* filled dynamically, do not modify */
        &mirrorTableYUV422[0][0],       /* filled dynamically, do not modify */
        &swapTableYUV422[0][0],         /* filled dynamically, do not modify */
        &enableVideoPortYUV422[0][0],   /* filled dynamically, do not modify */
        &groundVideoPortYUV422[0][0],   /* filled dynamically, do not modify */
        &mirrorTableYUV444[0][0],       /* filled dynamically, do not modify */
        &swapTableYUV444[0][0],         /* filled dynamically, do not modify */
        &enableVideoPortYUV444[0][0],   /* filled dynamically, do not modify */
        &groundVideoPortYUV444[0][0],   /* filled dynamically, do not modify */
        &mirrorTableRGB444[0][0],       /* filled dynamically, do not modify */
        &swapTableRGB444[0][0],         /* filled dynamically, do not modify */
        &enableVideoPortRGB444[0][0],   /* filled dynamically, do not modify */
        &groundVideoPortRGB444[0][0],   /* filled dynamically, do not modify */
        &enableAudioPortSPDIF[0],
        &groundAudioPortSPDIF[0],
        &enableAudioClockPortSPDIF[0],
        &groundAudioClockPortSPDIF[0],
        &enableAudioPortI2S[0],
        &groundAudioPortI2S[0],
        &enableAudioPortI2S8C[0],
        &groundAudioPortI2S8C[0],
        &enableAudioClockPortI2S[0],
        &groundAudioClockPortI2S[0],
        &enableAudioPortOBA[0],
        &groundAudioPortOBA[0],
        &enableAudioClockPortOBA[0],
        &groundAudioClockPortOBA[0],
        &enableAudioPortDST[0],
        &groundAudioPortDST[0],
        &enableAudioClockPortDST[0],
        &groundAudioClockPortDST[0],
        &enableAudioPortHBR[0],
        &groundAudioPortHBR[0],
        &enableAudioClockPortHBR[0],
        &groundAudioClockPortHBR[0],
        KEY_SEED,
        TMDL_HDMITX_PATTERN_BLUE
    }
 };


#ifdef TMFL_CEC_AVAILABLE

tmdlHdmiCecCapabilities_t CeccapabilitiesList = {TMDL_HDMICEC_DEVICE_UNKNOWN, CEC_VERSION_1_3a};

/**
 * \brief Configuration Tables. This table can be modified by the customer 
            to choose its prefered configuration
 */

tmdlHdmiCecDriverConfigTable_t CecdriverConfigTable[MAX_UNITS] = {
    {
    COMMAND_TASK_PRIORITY_0,
    COMMAND_TASK_STACKSIZE_0,
    COMMAND_TASK_QUEUESIZE_0,
    CEC_UNIT_I2C_ADDRESS_0,
    TxI2cReadFunction,
    TxI2cWriteFunction,
    &CeccapabilitiesList
    }
};


/******************************************************************************
******************************************************************************
*                THIS PART MUST NOT BE MODIFIED BY CUSTOMER                  *
******************************************************************************
*****************************************************************************/

/**
    \brief This function allows to the main driver to retrieve its
           configuration parameters.

    \param pConfig Pointer to the config structure

    \return The call result:
            - TM_OK: the call was successful
            - TMDL_ERR_DLHDMICEC_BAD_UNIT_NUMBER: the unit number is wrong or
              the receiver instance is not initialised
            - TMDL_ERR_DLHDMICEC_INCONSISTENT_PARAMS: an input parameter is
              inconsistent

******************************************************************************/
tmErrorCode_t tmdlHdmiCecCfgGetConfig
(
    tmUnitSelect_t                 unit,
    tmdlHdmiCecDriverConfigTable_t *pConfig
)
{
    /* check if unit number is in range */
    if((unit < 0) || (unit >= MAX_UNITS))
	  	return TMDL_ERR_DLHDMICEC_BAD_UNIT_NUMBER;

    /* check if pointer is Null */
    if(pConfig == Null)
	 	return TMDL_ERR_DLHDMICEC_INCONSISTENT_PARAMS;

    *pConfig = CecdriverConfigTable[unit];

    return TM_OK;
}

#endif

/*============================================================================*/
/*                            END OF FILE                                     */
/*============================================================================*/

