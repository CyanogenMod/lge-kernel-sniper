/*
 *  arch/arm/mach-omap2/lge/lge_diag_communication.c 
 *
 *   the use of another function to send framework.
*/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include "lge_diag_communication.h"
#include <linux/miscdevice.h>
#include <asm/uaccess.h>
#include <linux/fs.h>

#define DEBUG_DIAG 1

#if DEBUG_DIAG
#define D(fmt, args...) printk(fmt, ##args)
#else
#define D(fmt, args...) do () while(0)
#endif

struct diagcmd_data {
	struct diagcmd_dev sdev;
};
static struct diagcmd_data *diagcmd_data;

struct diagcmd_dev *diagcmd_get_dev(void)
{
	return &(diagcmd_data->sdev);
}
EXPORT_SYMBOL(diagcmd_get_dev);

static struct daigcmd_dev *diagpdev;

/* Misc device file operations */
static int lge_diag_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static int lge_diag_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static int lge_diag_ioctl(struct inode *inode, struct file *filp,
		unsigned int cmd, unsigned long arg)
{
	struct diag_cmd_data ctrl_buf;
	int ret;

	diagpdev = diagcmd_get_dev();		
	switch (cmd) {
		
		case DIAG_IOCTL_UPDATE: 
			if(copy_from_user(&ctrl_buf , arg ,sizeof(struct diag_cmd_data))){
				printk(KERN_INFO "%s: copy_from_user error.\n", __func__);
				ret = -EFAULT;
			}
			printk(KERN_INFO "%d: ctrl_buf.sub_cmd_code\n", ctrl_buf.sub_cmd_code);
			printk(KERN_INFO "%d: ctrl_buf.state\n", ctrl_buf.state);
			switch(ctrl_buf.sub_cmd_code){
				/* Test Mode */	
				case TEST_MODE_MOTOR:
					update_diagcmd_state(diagpdev,"MOTOR", ctrl_buf.state); 
					break;
				
				case TEST_MODE_ACOUSTIC:
					update_diagcmd_state(diagpdev,"ACOUSTIC", ctrl_buf.state); 
					break;

				case TEST_MODE_CAM:
					update_diagcmd_state(diagpdev,"CAMERA", ctrl_buf.state); 
					break;

				case TEST_MODE_BLUETOOTH_TEST:
					update_diagcmd_state(diagpdev,"BT_TEST_MODE", ctrl_buf.state); 
					break;

				case TEST_MODE_MP3_TEST:
					update_diagcmd_state(diagpdev,"MP3", ctrl_buf.state); 
					break;

/* LGE_CHANGE_S [lgetaewon.kim@lge.com] [2011.05.05] [B project] added for accelerometer testmode [START] */
				case TEST_MODE_ACCEL_SENSOR_TEST:
					update_diagcmd_state(diagpdev,"ACCEL", ctrl_buf.state); 
					break;
/* LGE_CHANGE_E [lgetaewon.kim@lge.com] [2011.05.05] [B project] added for accelerometer testmode [END] */

				case TEST_MODE_FM_TRANCEIVER_TEST:
					update_diagcmd_state(diagpdev,"FM_TRANCEIVER", ctrl_buf.state); 
					break;

				case TEST_MODE_SLEEP_MODE_TEST:
					update_diagcmd_state(diagpdev,"SLEEP_MODE", ctrl_buf.state); 
					break;

				case TEST_MODE_SPEAKER_PHONE_TEST:
					update_diagcmd_state(diagpdev,"SPEAKERPHONE", ctrl_buf.state); 
					break;

				case TEST_MODE_VOLUME_TEST:
					update_diagcmd_state(diagpdev,"VOLUMELEVEL", ctrl_buf.state); 
					break;

				case TEST_MODE_HFA_TEST:
					update_diagcmd_state(diagpdev,"HFA", ctrl_buf.state); 
					break;
		
				case TEST_MODE_DB_INTEGRITY_TEST:
					update_diagcmd_state(diagpdev,"DBCHECK", ctrl_buf.state); 
					break;

//20110519 yongman.kwon@lge.com [LS855] adding cod for gyro test [START]
				case TEST_MODE_GYRO_SENSOR_TEST:
					update_diagcmd_state(diagpdev,"GYRO", ctrl_buf.state); 
					break;
//20110519 yongman.kwon@lge.com [LS855] adding cod for gyro test [END]

				case TEST_MODE_MANUAL_MODE_TEST://20110608
					update_diagcmd_state(diagpdev,"MANUALMODE", ctrl_buf.state); 
                    break;

// 20110611 jihang.chung@lge.com test script on -> go to Idle START
				case TEST_MODE_TEST_SCRIPT_MODE:
					update_diagcmd_state(diagpdev,"TESTSCRIPTMODE", ctrl_buf.state); 
					break;
// 20110611 jihang.chung@lge.com test script on -> go to Idle END
						
				/* for slate */	
				case ICD_EXTENDEDVERSIONINFO_REQ_IOCTL:
					update_diagcmd_state(diagpdev,"ICD_GETEXTENDEDVERSION", ctrl_buf.state); 
					break;
				
				case ICD_GETAIRPLANEMODE_REQ_IOCTL:
					update_diagcmd_state(diagpdev,"ICD_GETAIRPLANEMODE", ctrl_buf.state); 
					break;
				
				case ICD_SETAIRPLANEMODE_REQ_IOCTL:
					update_diagcmd_state(diagpdev,"ICD_SETAIRPLANEMODE", ctrl_buf.state); 
					break;
				
				case ICD_GETBACKLIGHTSETTING_REQ_IOCTL:
					update_diagcmd_state(diagpdev,"ICD_GETBACKLIGHTSETTING", ctrl_buf.state); 
					break;
				
				case ICD_SETBACKLIGHTSETTING_REQ_IOCTL:
					update_diagcmd_state(diagpdev,"ICD_SETBACKLIGHTSETTING", ctrl_buf.state); 
					break;
				
				case ICD_GETBLUETOOTHSTATUS_REQ_IOCTL:
					update_diagcmd_state(diagpdev,"ICD_GETBLUETOOTH", ctrl_buf.state); 
					break;
				
				case ICD_SETBLUETOOTHSTATUS_REQ_IOCTL:
					update_diagcmd_state(diagpdev,"ICD_SETBLUETOOTH", ctrl_buf.state); 
					break;
				
				case ICD_GETUISCREENID_REQ_IOCTL:
					update_diagcmd_state(diagpdev,"ICD_GETUISCREENID", ctrl_buf.state); 
					break;
					
				case ICD_GETWIFISTATUS_REQ_IOCTL:
					update_diagcmd_state(diagpdev,"ICD_GETWIFISTATUS", ctrl_buf.state); 
					break;
					
				case ICD_SETWIFISTATUS_REQ_IOCTL:
					update_diagcmd_state(diagpdev,"ICD_SETWIFISTATUS", ctrl_buf.state); 
					break;
					
				case ICD_SETSCREENORIENTATIONLOCK_REQ_IOCTL:
					update_diagcmd_state(diagpdev,"ICD_SETORIENTATIONLOCK", ctrl_buf.state); 
					break;

			}

		break;
		
		default: 
			break;
	}
	
	return 0;
}

static struct file_operations lge_diag_fops = {
	.owner	 = THIS_MODULE,
	.open	 = lge_diag_open,
	.release = lge_diag_release,
	.ioctl	 = lge_diag_ioctl,
};

static struct miscdevice lge_diag_misc_dev = {
	.minor	= MISC_DYNAMIC_MINOR,
	.name	= "lge_diag_ctl",
	.fops	= &lge_diag_fops,
};


static int diagcmd_probe(struct platform_device *pdev)
{
	struct diagcmd_platform_data *pdata = pdev->dev.platform_data;
	int ret = 0;

	if (!pdata){
		D("diagcmd_probe pdata err:%s\n", pdata->name);
		return -EBUSY;
	}

	D("%s:%s\n", __func__, pdata->name);
	diagcmd_data = kzalloc(sizeof(struct diagcmd_data), GFP_KERNEL);
	if (!diagcmd_data){
		D("diagcmd_probe data err:%s\n", pdata->name);
		return -ENOMEM;
	}

	diagcmd_data->sdev.name = pdata->name;
    ret = diagcmd_dev_register(&diagcmd_data->sdev);
	if (ret < 0)
		goto err_diagcmd_dev_register;

	/* Misc device register */
	ret = misc_register(&lge_diag_misc_dev);
	if (ret) {
		printk(KERN_ERR "%s: failed to register misc device.\n", __func__);
		goto misc_register_err1;
	}

	return 0;

err_diagcmd_dev_register:
	kfree(diagcmd_data);
misc_register_err1:
	misc_deregister(&lge_diag_misc_dev);

	return ret;
}

static int __devexit diagcmd_remove(struct platform_device *pdev)
{
	struct diagcmd_data *atcmd = platform_get_drvdata(pdev);

    diagcmd_dev_unregister(&atcmd->sdev);
	kfree(diagcmd_data);
	misc_deregister(&lge_diag_misc_dev);

	return 0;
}

static struct platform_driver diagcmd_driver = {
	.probe		= diagcmd_probe,
	.remove		= __devexit_p(diagcmd_remove),
	.driver		= {
		.name	= "lg_fw_diagcmd",
		.owner	= THIS_MODULE,
	},
};

static int __init diagcmd_init(void)
{
	return platform_driver_register(&diagcmd_driver);
}

static void __exit diagcmd_exit(void)
{
	platform_driver_unregister(&diagcmd_driver);
}

module_init(diagcmd_init);
module_exit(diagcmd_exit);

MODULE_AUTHOR("kiwone.seo@lge.com");
MODULE_DESCRIPTION("lg_fw_diagcmd driver");
MODULE_LICENSE("GPL");

