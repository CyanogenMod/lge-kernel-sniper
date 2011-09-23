/*
 *   arch/arm/mach-omap2/lge/lge_diag_class.c
 *
 *   kiwone creates this file for audio test mode, and the use of another function to send framework.
*/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/err.h>
#include "lge_diag_communication.h"

/* LGE_CHANGE_S [lgetaewon.kim@lge.com] [2011.05.05] [B project] added for accelerometer testmode [START] */
#define ACCEL_INIT_VALUE 1000
/* LGE_CHANGE_E [lgetaewon.kim@lge.com] [2011.05.05] [B project] added for accelerometer testmode [END] */

#if 1 // for sleep_flight
#include <plat/control.h>
#endif

struct class *lg_fw_diag_class;
static atomic_t device_count;
// LGE_CHANGE_S [myeonggyu.son@lge.com] [2011.02.21] [gelato] add diag sysfs [START]
extern char process_status[10];
extern char process_value[300];
// LGE_CHANGE_E [myeonggyu.son@lge.com] [2011.02.21] [gelato] add diag sysfs [END]
char process_status[10];
EXPORT_SYMBOL(process_status);

char process_value[300];
EXPORT_SYMBOL(process_value);

static ssize_t state_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct diagcmd_dev *sdev = (struct diagcmd_dev *)dev_get_drvdata(dev);
	printk("\n%s:%d\n", __func__, sdev->state);
	return sprintf(buf, "%d\n", sdev->state);
}

static ssize_t name_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct diagcmd_dev *sdev = (struct diagcmd_dev *)dev_get_drvdata(dev);
	printk("\n%s:%s\n", __func__, sdev->name);
	return sprintf(buf, "%s\n", sdev->name);
}

// LGE_CHANGE_S [myeonggyu.son@lge.com] [2011.02.21] [gelato] add diag sysfs [START]
static ssize_t result_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	printk("\n%s:%s\n", __func__,buf);
	strcpy(process_status, buf);
	return count;
}

static ssize_t return_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	printk("\n%s:%s\n", __func__,buf);
	strcpy(process_value, buf);
	return count;
}
static ssize_t result_show(struct device *dev, struct device_attribute *attr,		char *buf)
{
	printk("\n%s:%s\n", __func__,process_status);

	memcpy(buf,process_status,sizeof(process_status));
	return sizeof(process_status);

}

static ssize_t return_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk("\n%s:%s\n", __func__,process_value);
	memcpy(buf,process_value,sizeof(process_value));
	return sizeof(process_value);
	
}

// LGE_CHANGE_E [myeonggyu.son@lge.com] [2011.02.21] [gelato] add diag sysfs [END]

static DEVICE_ATTR(state, S_IRUGO | S_IWUSR, state_show, NULL);
static DEVICE_ATTR(name, S_IRUGO | S_IWUSR, name_show, NULL);
// LGE_CHANGE_S [myeonggyu.son@lge.com] [2011.02.21] [gelato] add diag sysfs [START]
static DEVICE_ATTR(result, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH, result_show, result_store);
static DEVICE_ATTR(return, S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH, return_show, return_store);
// LGE_CHANGE_E [myeonggyu.son@lge.com] [2011.02.21] [gelato] add diag sysfs [END]

extern unsigned int system_rev;
static ssize_t system_rev_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	printk("%s: system_rev %u\n", __func__, system_rev);
	return sprintf(buf, "%u\n", system_rev);
}
static DEVICE_ATTR(system_rev, S_IRUGO, system_rev_show, NULL);

static ssize_t ftm_boot_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int val = 0;

	// MMC Read
	// mbk_temp
	//val = 1;
	printk("%s: ftm_boot %d\n", __func__, val);
	return sprintf(buf, "%u\n", val);
}
static DEVICE_ATTR(ftm_boot, S_IRUGO, ftm_boot_show, NULL);

int alarm_rtc_disable_by_flight_test_mode = 0;
static int sleep_flight = 0;
extern int lcd_off_boot;			// to check factory mode
extern void omap_mux_set_gpio(u16 val, int gpio);
extern u16 omap_mux_get_gpio(int gpio);
#define OFF_WE          ((1 << 15) | (1 << 14) | (1 << 10) | (1 << 9))
static ssize_t sleep_flight_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	int i;
	u16 gpio_val;
	printk("\n%s:%s\n", __func__,buf);
	sscanf(buf, "%d", &sleep_flight);

	if( sleep_flight && lcd_off_boot ) {
		printk("%s: ===============================Wake Up Condietion Disable\n", __func__);

		alarm_rtc_disable_by_flight_test_mode = 1;
#if 0
		/* GPIO_175 GAUGE_INT */
		gpio_val = omap_mux_get_gpio(175); 
		gpio_val &= ~OMAP44XX_PADCONF_WAKEUPENABLE0;
		omap_mux_set_gpio(gpio_val, 175);

		/* GPIO_176 MODEM_SEND */
		gpio_val = omap_mux_get_gpio(176); 
		gpio_val &= ~OMAP44XX_PADCONF_WAKEUPENABLE0;
		omap_mux_set_gpio(gpio_val, 176);

		/* GOIO_0 SYS_IRQ_N */
		gpio_val = omap_mux_get_gpio(0); 
		gpio_val &= ~OMAP44XX_PADCONF_WAKEUPENABLE0;
		omap_mux_set_gpio(gpio_val, 0);

		/* GOIO_14 PROX_OUT */
		gpio_val = omap_mux_get_gpio(14); 
		gpio_val &= ~OMAP44XX_PADCONF_WAKEUPENABLE0;
		omap_mux_set_gpio(gpio_val, 14);
		
		/* GOIO_15 CHG_STATUS_N_OMAP */
		gpio_val = omap_mux_get_gpio(15); 
		gpio_val &= ~OMAP44XX_PADCONF_WAKEUPENABLE0;
		omap_mux_set_gpio(gpio_val, 15);

		/* GOIO_21 IPC_CPRDY */
		gpio_val = omap_mux_get_gpio(21); 
		gpio_val &= ~OMAP44XX_PADCONF_WAKEUPENABLE0;
		omap_mux_set_gpio(gpio_val, 21);
#else
		for(i=0;i<192;i++) {
			gpio_val = omap_mux_get_gpio(i); 
			//printk(KERN_INFO "gpio %d, gpio val %x\n", i, gpio_val);
			gpio_val &= ~OFF_WE;
			//printk(KERN_INFO "   After gpio val %x\n\n", gpio_val);
			omap_mux_set_gpio(gpio_val, i);
		}
#endif
	}

	return count;
}
static ssize_t sleep_flight_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	//printk("%s: sleep_flight %d\n", __func__, sleep_flight);
	return sprintf(buf, "%u\n", sleep_flight);
}
static DEVICE_ATTR(sleep_flight, S_IRUGO | S_IWUSR, sleep_flight_show, sleep_flight_store);

#if 1 /* BT_DUN mbk */
char bt_dun_by_cp_buffer[160];
static ssize_t bt_dun_by_cp_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	//sscanf(buf, "%s", bt_dun_by_cp_buffer);
	memset(bt_dun_by_cp_buffer, 0x00, sizeof(bt_dun_by_cp_buffer));
	memcpy(bt_dun_by_cp_buffer, buf, count);

#if 0
	printk("buffer from app: %s, cnt %d\n", buf, count);
	printk("Kernel Buffer  : %s\n", bt_dun_by_cp_buffer);
#endif
	return count;
}
static ssize_t bt_dun_by_cp_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
#if 0
	printk("Kernel Buffer  : %s\n", bt_dun_by_cp_buffer);
#endif

	return sprintf(buf, "%s\n", bt_dun_by_cp_buffer);
}
static DEVICE_ATTR(bt_dun_by_cp, 0777 /* S_IRUGO | S_IWUSR */, bt_dun_by_cp_show, bt_dun_by_cp_store);
#endif

/* LGE_MERGE_S [sunmyoung.lee@lge.com] 2010-07-15. SMS UTS Test */
udm_sms_status_new lg_diag_req_udm_sms_status_new = {0};

static ssize_t read_sms_status_new(struct device *dev, struct device_attribute *attr,
	char *buf)
{
  int udm_sms_statu_len = sizeof(udm_sms_status_new);
  
  memcpy(buf, &lg_diag_req_udm_sms_status_new, udm_sms_statu_len);
  return udm_sms_statu_len;
}

static ssize_t write_sms_status_new(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf, size_t size)
{
  int udm_sms_statu_len = sizeof(udm_sms_status_new);

  memcpy((void*)&lg_diag_req_udm_sms_status_new, buf, udm_sms_statu_len);
//  printk( KERN_DEBUG "LG_FW : write_cmd_pkt_length = %d\n",lg_diag_rsp_pkt_length);  
  return udm_sms_statu_len;
}
/* LGE_MERGE_E [sunmyoung.lee@lge.com] 2010-07-15. SMS UTS Test */

/* LGE_MERGE_S [sunmyoung.lee@lge.com] 2010-07-15. SMS UTS Test */
static DEVICE_ATTR(get_sms, S_IRUGO | S_IWUSR,read_sms_status_new, write_sms_status_new);
static DEVICE_ATTR(set_sms, S_IRUGO | S_IWUSR,read_sms_status_new, write_sms_status_new);
static DEVICE_ATTR(sms_status, S_IRUGO | S_IWUSR,read_sms_status_new, write_sms_status_new);
static DEVICE_ATTR(rsp_get_sms, S_IRUGO | S_IWUSR,read_sms_status_new, write_sms_status_new);
static DEVICE_ATTR(rsp_set_sms, S_IRUGO | S_IWUSR,read_sms_status_new, write_sms_status_new);
static DEVICE_ATTR(rsp_sms_status, S_IRUGO | S_IWUSR,read_sms_status_new, write_sms_status_new);
/* LGE_MERGE_E [sunmyoung.lee@lge.com] 2010-07-15. SMS UTS Test */

/* LGE_CHANGE_S [lgetaewon.kim@lge.com] [2011.05.05] [B project] added for accelerometer testmode [START] */
static signed short pitch = ACCEL_INIT_VALUE;
static signed short roll = ACCEL_INIT_VALUE;

//20110519 yongman.kwon@lge.com [LS855] adding value for gyro self test.
static signed char GyroTestResult = -1;

static ssize_t read_accel_pitch (struct device *dev, struct device_attribute *attr, const char *buf) {
//	printk("[%s] %d\n", __func__, pitch);

	memcpy(buf, &pitch, sizeof(signed short));
	
	return sizeof(signed short);
}
static ssize_t write_accel_pitch (struct device *dev, struct device_attribute *attr, const char *buf, size_t size) {

//	printk("%s\n", buf);
	
	sscanf(buf, "%d", &pitch);

//	printk("[%s]%d\n", __func__, pitch);

	return 0;
}
static DEVICE_ATTR(accel_pitch, S_IRUGO | S_IWUSR, read_accel_pitch, write_accel_pitch);

static ssize_t read_accel_roll (struct device *dev, struct device_attribute *attr, const char *buf) {
//	printk("[%s] %d\n", __func__, roll);

	memcpy(buf, &roll, sizeof(signed short));
	
	return sizeof(signed short);
}
static ssize_t write_accel_roll (struct device *dev, struct device_attribute *attr, const char *buf, size_t size) {
	
//	printk("%s\n", buf);
	
	sscanf(buf, "%d", &roll);

//	printk("[%s]%d\n", __func__, roll);

	return 0;
}
static DEVICE_ATTR(accel_roll, S_IRUGO | S_IWUSR, read_accel_roll, write_accel_roll);
/* LGE_CHANGE_E [lgetaewon.kim@lge.com] [2011.05.05] [B project] added for accelerometer testmode [END] */


//20110519 yongman.kwon@lge.com [LS855] Write description here in detail [START]
static ssize_t read_gyro_test_result (struct device *dev, struct device_attribute *attr, const char *buf) {
//	printk("[%s] %d\n", __func__, pitch);

	memcpy(buf, &GyroTestResult, sizeof(signed char));
	
	return sizeof(signed short);
}
static ssize_t write_gyro_test_result (struct device *dev, struct device_attribute *attr, const char *buf, size_t size) {

//	printk("%s\n", buf);
	
	sscanf(buf, "%d", &GyroTestResult);

//	printk("[%s]%d\n", __func__, pitch);

	return 0;
}
static DEVICE_ATTR(gyro_test, S_IRUGO | S_IWUSR, read_gyro_test_result, write_gyro_test_result);
//20110519 yongman.kwon@lge.com [LS855] Write description here in detail [END]

//LGE_CHANGE_START [jungsoo1221.lee] - AllAutoTest
char autotest[10] = "0";
static ssize_t audio_autotest_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{

	printk("\n%s:%s\n", __func__,buf);
	strcpy(autotest, buf);
	return count;
}

static ssize_t audio_autotest_show(struct device *dev, struct device_attribute *attr,		char *buf)
{
	printk("\n%s:%s\n", __func__,autotest);

	memcpy(buf,autotest,sizeof(autotest));
	return sizeof(autotest);

}

static DEVICE_ATTR(audio_autotest, 660, audio_autotest_show, audio_autotest_store);

//LGE_CHANGE_END [jungsoo1221.lee] - AllAutoTest

void update_diagcmd_state(struct diagcmd_dev *sdev, char *cmd, int state)
{
	char name_buf[120];
	char state_buf[120];
	char *prop_buf;
	char *envp[3];
	int env_offset = 0;
	int length;

	/* 
	 * 2010-08-12, jinkyu.choi@lge.com, Do not check the state
	 * Now, each command has own state number which is the sub command number of testmode tools.
	 * The sub commands can be same though the major commands are different.
	 * It is result in not sending the commnad to Android Diag application
	 */

	//if (sdev->state != state) {
	sdev->state = state;
	sdev->name = cmd;

	prop_buf = (char *)get_zeroed_page(GFP_KERNEL);
	if (prop_buf) {
		length = name_show(sdev->dev, NULL, prop_buf);
		if (length > 0) {
			if (prop_buf[length - 1] == '\n')
				prop_buf[length - 1] = 0;
			snprintf(name_buf, sizeof(name_buf),"DIAG_NAME=%s", prop_buf);
			printk(KERN_INFO "DIAG_NAME%s \n",prop_buf);
			envp[env_offset++] = name_buf;
		}
		length = state_show(sdev->dev, NULL, prop_buf);
		if (length > 0) {
			if (prop_buf[length - 1] == '\n')
				prop_buf[length - 1] = 0;
			snprintf(state_buf, sizeof(state_buf),"DIAG_STATE=%s", prop_buf);
			envp[env_offset++] = state_buf;
			printk(KERN_INFO "DIAG_STATE%s \n",prop_buf);
		}
		envp[env_offset] = NULL;
		kobject_uevent_env(&sdev->dev->kobj, KOBJ_CHANGE, envp);
		free_page((unsigned long)prop_buf);
	} else {
		printk(KERN_ERR "out of memory in update_diagcmd_state\n");
		kobject_uevent(&sdev->dev->kobj, KOBJ_CHANGE);
	}
	//}
}
EXPORT_SYMBOL_GPL(update_diagcmd_state);

static int create_lg_fw_diag_class(void)
{
	if (!lg_fw_diag_class) {
		lg_fw_diag_class = class_create(THIS_MODULE, "lg_fw_diagclass");
		if (IS_ERR(lg_fw_diag_class))
			return PTR_ERR(lg_fw_diag_class);
		atomic_set(&device_count, 0);
	}

	return 0;
}

int diagcmd_dev_register(struct diagcmd_dev *sdev)
{
	int ret;

	if (!lg_fw_diag_class) {
		ret = create_lg_fw_diag_class();
		if (ret < 0)
			return ret;
	}

	sdev->index = atomic_inc_return(&device_count);
	sdev->dev = device_create(lg_fw_diag_class, NULL,
		MKDEV(0, sdev->index), NULL, sdev->name);
	if (IS_ERR(sdev->dev))
		return PTR_ERR(sdev->dev);

	ret = device_create_file(sdev->dev, &dev_attr_state);
	if (ret < 0)
		goto err_create_file_1;
	ret = device_create_file(sdev->dev, &dev_attr_name);
	if (ret < 0)
		goto err_create_file_2;
	// LGE_CHANGE_S [myeonggyu.son@lge.com] [2011.02.21] [gelato] add diag sysfs [START]
	ret = device_create_file(sdev->dev, &dev_attr_result);
	if (ret < 0)
		goto err_create_file_3;
	ret = device_create_file(sdev->dev, &dev_attr_return);
	if (ret < 0)
		goto err_create_file_4;
	// LGE_CHANGE_E [myeonggyu.son@lge.com] [2011.02.21] [gelato] add diag sysfs [END]

	ret = device_create_file(sdev->dev, &dev_attr_system_rev);
	if (ret < 0)
		goto err_create_file_5;

	ret = device_create_file(sdev->dev, &dev_attr_ftm_boot);
	if (ret < 0)
		goto err_create_file_6;

	ret = device_create_file(sdev->dev, &dev_attr_sleep_flight);
	if (ret < 0)
		goto err_create_file_7;

#if 1 /* BT_DUN mbk */
	ret = device_create_file(sdev->dev, &dev_attr_bt_dun_by_cp);
	if (ret < 0)
		goto err_create_file_8;
#endif

/* LGE_MERGE_S [sunmyoung.lee@lge.com] 2010-07-15. SMS UTS Test */
	ret = device_create_file(sdev->dev, &dev_attr_sms_status);
	if (ret) {
	  printk( KERN_DEBUG "LG_FW : diag device file3 create fail\n");
	  device_remove_file(sdev->dev, &dev_attr_sms_status);
	  return ret;
	}
  
	ret = device_create_file(sdev->dev, &dev_attr_get_sms);
	if (ret) {
	  printk( KERN_DEBUG "LG_FW : diag device file3 create fail\n");
	  device_remove_file(sdev->dev, &dev_attr_get_sms);
	  return ret;
	}
  
	ret = device_create_file(sdev->dev, &dev_attr_set_sms);
	if (ret) {
	  printk( KERN_DEBUG "LG_FW : diag device file3 create fail\n");
	  device_remove_file(sdev->dev, &dev_attr_set_sms);
	  return ret;
	}
  
	ret = device_create_file(sdev->dev, &dev_attr_rsp_sms_status);
	if (ret) {
	  printk( KERN_DEBUG "LG_FW : diag device file3 create fail\n");
	  device_remove_file(sdev->dev, &dev_attr_rsp_sms_status);
	  return ret;
	}
  
	ret = device_create_file(sdev->dev, &dev_attr_rsp_get_sms);
	if (ret) {
	  printk( KERN_DEBUG "LG_FW : diag device file3 create fail\n");
	  device_remove_file(sdev->dev, &dev_attr_rsp_get_sms);
	  return ret;
	}
  
	ret = device_create_file(sdev->dev, &dev_attr_rsp_set_sms);
	if (ret) {
	  printk( KERN_DEBUG "LG_FW : diag device file3 create fail\n");
	  device_remove_file(sdev->dev, &dev_attr_rsp_set_sms);
	  return ret;
	}
/* LGE_MERGE_E [sunmyoung.lee@lge.com] 2010-07-15. SMS UTS Test */

/* LGE_CHANGE_S [lgetaewon.kim@lge.com] [2011.05.05] [B project] added for accelerometer testmode [START] */
	ret = device_create_file(sdev->dev, &dev_attr_accel_pitch);
	if(ret) {
		printk(KERN_DEBUG "LG_FW : diag device file3 create fail\n");
		device_remove_file(sdev->dev, &dev_attr_accel_pitch);
		return ret;
	}

	ret = device_create_file(sdev->dev, &dev_attr_accel_roll);
	if(ret) {
		printk(KERN_DEBUG "LG_FW : diag device file3 create fail\n");
		device_remove_file(sdev->dev, &dev_attr_accel_roll);
		return ret;
	}
/* LGE_CHANGE_E [lgetaewon.kim@lge.com] [2011.05.05] [B project] added for accelerometer testmode [END] */
	
//LGE_CHANGE_START [jungsoo1221.lee] - AllAutoTest	
        ret = device_create_file(sdev->dev, &dev_attr_audio_autotest);
        if(ret) {
                  printk(KERN_DEBUG "LG_FW : diag device file3 create fail\n");
                  device_remove_file(sdev->dev, &dev_attr_audio_autotest);
                  return ret;
        }
//LGE_CHANGE_END [jungsoo1221.lee] - AllAutoTest

	dev_set_drvdata(sdev->dev, sdev);
	sdev->state = 0;
	return 0;

#if 1 /* BT_DUN mbk */
	device_remove_file(sdev->dev, &dev_attr_bt_dun_by_cp);
err_create_file_8:
#endif
	device_remove_file(sdev->dev, &dev_attr_sleep_flight);
err_create_file_7:
	device_remove_file(sdev->dev, &dev_attr_ftm_boot);
err_create_file_6:
	device_remove_file(sdev->dev, &dev_attr_system_rev);
err_create_file_5:
// LGE_CHANGE_S [myeonggyu.son@lge.com] [2011.02.21] [gelato] add diag sysfs [START]
	device_remove_file(sdev->dev, &dev_attr_return);
err_create_file_4:
	device_remove_file(sdev->dev, &dev_attr_result);
err_create_file_3:
	device_remove_file(sdev->dev, &dev_attr_name);
// LGE_CHANGE_E [myeonggyu.son@lge.com] [2011.02.21] [gelato] add diag sysfs [END]
err_create_file_2:
	device_remove_file(sdev->dev, &dev_attr_state);
err_create_file_1:
	device_destroy(lg_fw_diag_class, MKDEV(0, sdev->index));
	printk(KERN_ERR "lg_fw_diagcmd: Failed to register driver %s\n", sdev->name);

	return ret;
}
EXPORT_SYMBOL_GPL(diagcmd_dev_register);

void diagcmd_dev_unregister(struct diagcmd_dev *sdev)
{
	device_remove_file(sdev->dev, &dev_attr_name);
	device_remove_file(sdev->dev, &dev_attr_state);
	// LGE_CHANGE_S [myeonggyu.son@lge.com] [2011.02.21] [gelato] add diag sysfs [START]
	device_remove_file(sdev->dev, &dev_attr_result);
	device_remove_file(sdev->dev, &dev_attr_return);
	// LGE_CHANGE_E [myeonggyu.son@lge.com] [2011.02.21] [gelato] add diag sysfs [END]
	device_remove_file(sdev->dev, &dev_attr_system_rev);
	device_remove_file(sdev->dev, &dev_attr_ftm_boot);
	device_remove_file(sdev->dev, &dev_attr_sleep_flight);
#if 1 /* BT_DUN mbk */
	device_remove_file(sdev->dev, &dev_attr_bt_dun_by_cp);
#endif

/* LGE_MERGE_S [sunmyoung.lee@lge.com] 2010-07-15. SMS UTS Test */
	device_remove_file(sdev->dev, &dev_attr_sms_status);
	device_remove_file(sdev->dev, &dev_attr_get_sms);
	device_remove_file(sdev->dev, &dev_attr_set_sms);
	device_remove_file(sdev->dev, &dev_attr_rsp_sms_status);
	device_remove_file(sdev->dev, &dev_attr_rsp_get_sms);
	device_remove_file(sdev->dev, &dev_attr_rsp_set_sms);
/* LGE_MERGE_E [sunmyoung.lee@lge.com] 2010-07-15. SMS UTS Test */	

/* LGE_CHANGE_S [lgetaewon.kim@lge.com] [2011.05.05] [B project] added for accelerometer testmode [START] */
	device_remove_file(sdev->dev, &dev_attr_accel_pitch);
	device_remove_file(sdev->dev, &dev_attr_accel_roll);
/* LGE_CHANGE_E [lgetaewon.kim@lge.com] [2011.05.05] [B project] added for accelerometer testmode [END] */

	device_destroy(lg_fw_diag_class, MKDEV(0, sdev->index));
	dev_set_drvdata(sdev->dev, NULL);
}
EXPORT_SYMBOL_GPL(diagcmd_dev_unregister);

static int __init lg_fw_diag_class_init(void)
{
	return create_lg_fw_diag_class();
}

static void __exit lg_fw_diag_class_exit(void)
{
	class_destroy(lg_fw_diag_class);
}

module_init(lg_fw_diag_class_init);
module_exit(lg_fw_diag_class_exit);

MODULE_AUTHOR("kiwone.seo@lge.com");
MODULE_DESCRIPTION("lg_fw_diag class driver");
MODULE_LICENSE("GPL");
