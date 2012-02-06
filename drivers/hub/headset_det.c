
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/string.h>
#include <asm/gpio.h>
#include <asm/system.h>
#include <linux/interrupt.h>
#include <linux/switch.h>
#include <linux/workqueue.h>
#include <mach/hub_headset_det.h>
#include <linux/input.h>		/* LGE_CHANGE_S [luckyjun77@lge.com] 2009-11-25, hub rev A hook key */    

#include <linux/earlysuspend.h>	// 20100603 junyeop.kim@lge.com, headset suspend/resume [START_LGE]
// 20100825 junyeop.kim@lge.com, mic bias LDO control test [START_LGE]
#if 1
#include "../mux.h"
#define MIC_BIAS_LDO	60
#endif
// 20100825 junyeop.kim@lge.com, mic bias LDO control test [END_LGE]

struct headset_switch_data	*headset_sw_data;

unsigned int headset_status = 0;
headset_type_enum headset_type = HUB_NONE;
unsigned int hook_status = HOOK_RELEASED;

struct headset_switch_data {
	struct switch_dev sdev;
	unsigned gpio;
	unsigned hook_gpio;
	const char *name_on;
	const char *name_off;
	const char *state_on;
	const char *state_off;
	int irq;
	int hook_irq;
	struct work_struct work;
	struct delayed_work delayed_work;
	struct delayed_work hook_delayed_work;
	struct input_dev *ip_dev;		/* LGE_CHANGE_S [luckyjun77@lge.com] 2009-11-25, heaven rev A hook key */    
	struct early_suspend early_suspend;		// 20100603 junyeop.kim@lge.com, headset suspend/resume [START_LGE]
	int is_suspend;					// 20100603 junyeop.kim@lge.com, headset suspend/resume [START_LGE]
#if defined(CONFIG_MACH_LGE_HUB_REV_A)
	//nothing
#else
	unsigned dmb_ant_gpio;				// 20100814 junyeop.kim@lge.com, dmb ant gpio [START_LGE]
    int dmb_ant_irq;
    int dmb_ant_detected;
#endif
};

// 20100831 jh.koo@lge.com for TEST MODE [START_LGE]
extern int get_test_mode(void);
extern void write_gkpd_value(int value);
// 20100831 jh.koo@lge.com for TEST MODE [END_LGE]

#if defined(CONFIG_HUB_AMP_WM9093)
extern unsigned int get_wm9093_mode(void);		//20101004 junyeop.kim@lge.com, ear sense wakable setting
#endif

// 20101128 junyeop.kim@lge.com, dmb ant status[START_LGE]
extern void set_dmb_status(int state);
// 20101128 junyeop.kim@lge.com, dmb ant status[END_LGE]

// 20100603 junyeop.kim@lge.com, headset suspend/resume [START_LGE]

#ifdef CONFIG_HAS_EARLYSUSPEND
static void headsetdet_early_suspend(struct early_suspend *h);
static void headsetdet_late_resume(struct early_suspend *h);
extern int get_twl4030_status(void);
#endif
// 20100603 junyeop.kim@lge.com, headset suspend/resume [END_LGE]


// 20100608 junyeop.kim@lge.com, headset event inform for hdmi [START_LGE]
#if 1
#if defined(CONFIG_NXP_HDMI)
extern int Hdmi_getEnable(void);
extern int Hdmi_setAudioMute(bool audioMute);
extern int is_hdmi_enabled();	//junyeop.kim@lge.com
#endif
#endif
// 20100608 junyeop.kim@lge.com, headset event inform for hdmi [END_LGE]

unsigned int get_headset_type(void)
{
    return (unsigned int)headset_type;
}
EXPORT_SYMBOL(get_headset_type);

int get_dmb_status(void)
{
#if defined(CONFIG_MACH_LGE_HUB_REV_A)
	return 0;
#else
    return (int)headset_sw_data->dmb_ant_detected;
#endif    
}
EXPORT_SYMBOL(get_dmb_status);

int type_detection_tim = 100; 	

extern void hub_headsetdet_bias(int bias);

static void headset_det_work(struct work_struct *work)
{
	headset_status = gpio_get_value(headset_sw_data->gpio); 

	if(headset_status == 0)
	{
#if 1	// 20100825 junyeop.kim@lge.com, mic bias LDO control test [START_LGE]
		gpio_direction_output(MIC_BIAS_LDO, 0);
		gpio_set_value(MIC_BIAS_LDO, 0);
#endif	// 20100825 junyeop.kim@lge.com, mic bias LDO control test [END_LGE]
	
		//hub_headsetdet_bias(0);
		printk("[LUCKYJUN77] headset_det_work : headset_type = HUB_NONE\n");
		headset_type = HUB_NONE;
		switch_set_state(&headset_sw_data->sdev, headset_type);
		#if defined(CONFIG_NXP_HDMI)
		if(is_hdmi_enabled() == 1)
		{
			printk("(headset_det_work == Hdmi_setAudioMute(0)\n");
			Hdmi_setAudioMute(0);
		}
	    #endif
		return;		
	}
	
	msleep(100);
#if 1	// 20100825 junyeop.kim@lge.com, mic bias LDO control test [START_LGE]
	gpio_direction_output(MIC_BIAS_LDO, 1);
	gpio_set_value(MIC_BIAS_LDO, 1);
#endif	// 20100825 junyeop.kim@lge.com, mic bias LDO control test [END_LGE]
	
	//hub_headsetdet_bias(1);
        
#if defined(CONFIG_MACH_LGE_HUB_REV_A)	// 20100814 junyeop.kim@lge.com, dmb ant detect [START_LGE]
	//nothing
#else
   if(system_rev > 3 && gpio_get_value(headset_sw_data->dmb_ant_gpio) == 1) 
    {
		printk("[LUCKYJUN77] headset_det_work : dmb_ant detected\n");
		headset_sw_data->dmb_ant_detected = 1;
		set_dmb_status(headset_sw_data->dmb_ant_detected);	//junyeop.kim@lge.com
		headset_type = HUB_NONE;
		return;
	}
#endif	
	schedule_delayed_work(&headset_sw_data->delayed_work,	msecs_to_jiffies(type_detection_tim));
	
}
#if defined(CONFIG_MACH_LGE_HEAVEN_EVB2) || defined(CONFIG_MACH_LGE_HEAVEN_REV_A) /* LGE_CHANGE_S [iggikim@lge.com] 2009-09-13, heaven evb b */
struct twl4030_madc_request req;
static void type_det_work(struct work_struct *work)
{
	int val = 0;
	
	req.channels = 0x08; 
	req.do_avg	= 1;
	req.method	= TWL4030_MADC_SW1;
	req.func_cb	= NULL;
	
	for(val=0;val<16;val++)
		req.rbuf[val]=0;

	val = twl4030_madc_conversion(&req);
    
       if(headset_status == 1)
	{
	    if(req.rbuf[3] > 300)
		    headset_type = HUB_HEADSET;
	    else
		    headset_type = HUB_HEADPHONE;
	}
	else
		headset_type = HUB_NONE;

    if(headset_type != HUB_HEADSET){
	    //hub_headsetdet_bias(0);
	    gpio_direction_output(MIC_BIAS_LDO, 0);
		gpio_set_value(MIC_BIAS_LDO, 0);
    }
	switch_set_state(&headset_sw_data->sdev, headset_type);
}

#else
static void type_det_work(struct work_struct *work)
{
	//printk("[JIWON] type_det_work start\n");

	if(headset_status == 1)
	{
	    if(gpio_get_value(headset_sw_data->hook_gpio) == 0) 
	    {
			//printk("[LUCKYJUN77] type_det_work : HUB_HEADPHONE\n");
			headset_type = HUB_HEADPHONE;
		}
		else
		{
			//printk("[LUCKYJUN77] type_det_work : HUB_HEADSET\n");		
			headset_type = HUB_HEADSET;
	}
	}
	else
	{
		headset_type = HUB_NONE;
#if defined(CONFIG_MACH_LGE_HUB_REV_A) // 20100814 junyeop.kim@lge.com, dmb ant detect [START_LGE]		
	//nothing
#else
		headset_sw_data->dmb_ant_detected = 0;
		set_dmb_status(headset_sw_data->dmb_ant_detected);	//junyeop.kim@lge.com		
#endif		
	}


#if defined(CONFIG_MACH_LGE_HUB_REV_A) 	// 20100814 junyeop.kim@lge.com, dmb ant detect [START_LGE]
	//nothing
#else
//	msleep(100);
    if(system_rev > 3 && gpio_get_value(headset_sw_data->dmb_ant_gpio) == 1) 
    {
		printk("[LUCKYJUN77] type_det_work : dmb_ant detected\n");
		headset_type = HUB_NONE;
		headset_sw_data->dmb_ant_detected = 1;
		set_dmb_status(headset_sw_data->dmb_ant_detected);	//junyeop.kim@lge.com		
//		switch_set_state(&headset_sw_data->sdev, headset_type);		
//		return;
	}
#endif	// 20100814 junyeop.kim@lge.com, dmb ant detect [START_LGE]

#if defined(CONFIG_MACH_LGE_HUB_REV_A) // 20100814 junyeop.kim@lge.com, dmb ant detect [START_LGE]
    if(headset_type == HUB_NONE){
		//hub_headsetdet_bias(0);
		gpio_direction_output(MIC_BIAS_LDO, 0);
		gpio_set_value(MIC_BIAS_LDO, 0);
    }
#else
	if(system_rev <= 3)	//revB case
	{
    if(headset_type == HUB_NONE)
		//hub_headsetdet_bias(0);
		gpio_direction_output(MIC_BIAS_LDO, 0);
		gpio_set_value(MIC_BIAS_LDO, 0);
	}
	else 
	{
	if(headset_type == HUB_NONE && headset_sw_data->dmb_ant_detected == 0)
		//hub_headsetdet_bias(0);	
		gpio_direction_output(MIC_BIAS_LDO, 0);
		gpio_set_value(MIC_BIAS_LDO, 0);
	}
#endif	// 20100814 junyeop.kim@lge.com, dmb ant detect [START_LGE]

	switch_set_state(&headset_sw_data->sdev, headset_type);

// 20100608 junyeop.kim@lge.com, headset event inform for hdmi [START_LGE]
#if 1
#if defined(CONFIG_NXP_HDMI)
	if(is_hdmi_enabled() == 1)
	{
		if(headset_type == HUB_NONE)
			Hdmi_setAudioMute(0);
		else
			Hdmi_setAudioMute(1);
	}
#endif
#endif
// 20100608 junyeop.kim@lge.com, headset event inform for hdmi [END_LGE]
	
}

#if 0	//junyeop.kim@lge.com for long/short hook key 
static void hook_det_work(struct work_struct *work)
{
    if(headset_type != HEAVEN_HEADSET)
		return;
	
	if(hook_status == HOOK_RELEASED){
		if(gpio_get_value(headset_sw_data->hook_gpio) == 0){ 
		    hook_status = HOOK_PRESSED; 
		    input_report_key(headset_sw_data->ip_dev, KEY_HOOK, 1);
			input_sync(headset_sw_data->ip_dev);
			schedule_delayed_work(&headset_sw_data->delayed_work,	msecs_to_jiffies(type_detection_tim));			
		}
	}
	else{
		if(gpio_get_value(headset_sw_data->hook_gpio) == 1){
		    hook_status = HOOK_RELEASED; 
		    input_report_key(headset_sw_data->ip_dev, KEY_HOOK, 0);
			input_sync(headset_sw_data->ip_dev);
		}
	}
}
#else
static void hook_det_work(struct work_struct *work)
{
	//printk("[LUCKYJUN77] hook_det_work\n");
//[LGSI]Saravanan START TD116383-hook key issue in deep sleep
/* Dont skip Hook key event eventhough you are in Sleep
we are not holding any state in get_twl4030_status , making same as froyo*/
#if 0
// prime@sdcmicro.com Make the following code be conditional since get_twl4030_status() is included to CONFIG_SND_SOC_TWL4030 feature [START]
#ifdef CONFIG_SND_SOC_TWL4030
	if(headset_sw_data->is_suspend == 1 && get_twl4030_status() == 0)
#else
	if(headset_sw_data->is_suspend == 1)
#endif
// prime@sdcmicro.com Make the following code be conditional since get_twl4030_status() is included to CONFIG_SND_SOC_TWL4030 feature [END]
	{
		printk("[LUCKYJUN77] suspend status \n");	
		return;
	}
#endif
//[LGSI]Saravanan END TD116383-hook key issue in deep sleep
    if(headset_type == HUB_HEADPHONE)	//detect type error case
    {
    	//printk("[LUCKYJUN77] hook_det_work : headphone detect\n");
//		schedule_delayed_work(&headset_sw_data->delayed_work,	msecs_to_jiffies(type_detection_tim*3));
		headset_type = HUB_HEADSET;
		switch_set_state(&headset_sw_data->sdev, headset_type);
    }


    if(headset_type != HUB_HEADSET)
		return;

	if(hook_status == HOOK_RELEASED){
		if(gpio_get_value(headset_sw_data->hook_gpio) == 0){ 	//threshold area for hook key glitch
			//printk("[LUCKYJUN77] threshold area for hook key glitch\n");		
		    hook_status = HOOK_PRESSED; 
//		    input_report_key(headset_sw_data->ip_dev, KEY_HOOK, 1);
			schedule_delayed_work(&headset_sw_data->hook_delayed_work,	msecs_to_jiffies(type_detection_tim/8));
		}
	}
	else{
		if(gpio_get_value(headset_sw_data->hook_gpio) == 0){ 
			printk("[LUCKYJUN77] HOOK_PRESSED\n");
		    input_report_key(headset_sw_data->ip_dev, KEY_HOOK, 1);		    
			input_sync(headset_sw_data->ip_dev);		    
			schedule_delayed_work(&headset_sw_data->hook_delayed_work,	msecs_to_jiffies(type_detection_tim));			
		}
		else {
			    input_report_key(headset_sw_data->ip_dev, KEY_HOOK, 0);	
				input_sync(headset_sw_data->ip_dev);
				printk("[LUCKYJUN77] HOOK_RELEASED\n");				
			    hook_status = HOOK_RELEASED;	
// 20100831 jh.koo@lge.com for TEST MODE [START_LGE]
			if(get_test_mode())
			{
				write_gkpd_value(KEY_HOOK);
			}
// 20100831 jh.koo@lge.com for TEST MODE [END_LGE]				
		}
	}
}
#endif

#endif

static irqreturn_t headset_int_handler(int irq, void *dev_id)
{
	struct headset_switch_data *switch_data =
	    (struct headset_switch_data *)dev_id;

	schedule_work(&switch_data->work);
	return IRQ_HANDLED;
}

static irqreturn_t headset_hook_int_handler(int irq, void *dev_id)
{
	struct headset_switch_data	*switch_data =
	    (struct headset_switch_data *)dev_id;
	
	schedule_delayed_work(&switch_data->hook_delayed_work,	msecs_to_jiffies(type_detection_tim));
	
	return IRQ_HANDLED;
}

#if defined(CONFIG_MACH_LGE_HUB_REV_A) // 20100814 junyeop.kim@lge.com, dmb ant detect [START_LGE]
	//nothing
#else
static irqreturn_t dmb_ant_int_handler(int irq, void *dev_id)
{
	struct headset_switch_data	*switch_data =
	    (struct headset_switch_data *)dev_id;

	printk("[LUCKYJUN77]dmb_ant_int_handler\n");
#if 0	
	if(gpio_get_value(headset_sw_data->dmb_ant_gpio) == 0)
	{
		printk("[LUCKYJUN77]dmb_ant_int_handler : headset/headphone detected\n");
		schedule_delayed_work(&headset_sw_data->delayed_work,	msecs_to_jiffies(type_detection_tim));
	}
	else
	{
		printk("[LUCKYJUN77]dmb_ant_int_handler : dmb ant detected\n");		
		if(headset_type != HUB_NONE)
		{
			headset_type = HUB_NONE;		
			switch_set_state(&headset_sw_data->sdev, headset_type);
		}
	}
#endif	
	schedule_delayed_work(&headset_sw_data->delayed_work,	msecs_to_jiffies(500));
	
	return IRQ_HANDLED;
}
#endif	// 20100814 junyeop.kim@lge.com, dmb ant detect [END_LGE]

static ssize_t switch_gpio_print_state(struct switch_dev *sdev, char *buf)
{
	const char *state;
	if (switch_get_state(sdev))
		state = headset_sw_data->state_on;
	else
		state = headset_sw_data->state_off;

	if (state)
		return sprintf(buf, "%s\n", state);
	return -1;
}

static int headsetdet_probe(struct platform_device *pdev)
{
	struct gpio_switch_platform_data *pdata = pdev->dev.platform_data;
	struct headset_switch_data *switch_data;
	int ret = 0;
	struct input_dev *ip_dev;  

	if (!pdata)
		return -EBUSY;
	switch_data = kzalloc(sizeof(struct headset_switch_data), GFP_KERNEL);
	if (!switch_data)
		return -ENOMEM;

	switch_data->sdev.name = pdata->name;
	switch_data->gpio = pdata->gpio;
	switch_data->name_on = pdata->name_on;
	switch_data->name_off = pdata->name_off;
	switch_data->state_on = pdata->state_on;
	switch_data->state_off = pdata->state_off;
	switch_data->sdev.print_state = switch_gpio_print_state;

    switch_data->hook_gpio = 163;
#if defined(CONFIG_MACH_LGE_HUB_REV_A)	// 20100814 junyeop.kim@lge.com, dmb ant detect [START_LGE]
	//nothing
#else
	switch_data->dmb_ant_gpio = 164;
	switch_data->dmb_ant_detected = 0;
#endif	// 20100814 junyeop.kim@lge.com, dmb ant detect [END_LGE]

    ret = switch_dev_register(&switch_data->sdev);
	if (ret < 0)
		goto err_switch_dev_register;

	omap_mux_init_gpio(switch_data->gpio, OMAP_PIN_INPUT_PULLDOWN | OMAP_PIN_OFF_WAKEUPENABLE);	//20101004 junyeop.kim@lge.com, ear sense wakable setting

#if 1	//20101109 junyeop.kim@lge.com, hook key wakable setting
	omap_mux_init_gpio(switch_data->hook_gpio, OMAP_PIN_INPUT_PULLUP | OMAP_PIN_OFF_WAKEUPENABLE);	
#endif

	ret = gpio_request(switch_data->gpio, pdev->name);

	ret = gpio_request(switch_data->hook_gpio, pdev->name);

#if defined(CONFIG_MACH_LGE_HUB_REV_A) // 20100814 junyeop.kim@lge.com, dmb ant detect [START_LGE]
	//nothing
#else
	ret = gpio_request(switch_data->dmb_ant_gpio, pdev->name);
#endif	// 20100814 junyeop.kim@lge.com, dmb ant detect [END_LGE]

	if (ret < 0)
		goto err_request_gpio;

	ret = gpio_direction_input(switch_data->gpio);

	ret = gpio_direction_input(switch_data->hook_gpio);

#if defined(CONFIG_MACH_LGE_HUB_REV_A)	|| defined(CONFIG_MACH_LGE_HUB_REV_B)// 20100814 junyeop.kim@lge.com, dmb ant detect [START_LGE]
	//nothing
#else
	ret = gpio_direction_input(switch_data->dmb_ant_gpio);
#endif	// 20100814 junyeop.kim@lge.com, dmb ant detect [END_LGE]

	if (ret < 0)
		goto err_set_gpio_input;

    ip_dev = input_allocate_device();
    
	switch_data->ip_dev = ip_dev;

	switch_data->ip_dev->name = "Hookkey";	//20100830, junyeop.kim@lge.com, define the input dev name
	
	set_bit(EV_SYN, switch_data->ip_dev->evbit);
	set_bit(EV_KEY, switch_data->ip_dev->evbit);
	set_bit(KEY_HOOK, switch_data->ip_dev->keybit); 	//for short pressed hook key
		
	ret = input_register_device(switch_data->ip_dev);
	    
	INIT_WORK(&switch_data->work, headset_det_work);
    INIT_DELAYED_WORK(&switch_data->delayed_work, type_det_work);
	INIT_DELAYED_WORK(&switch_data->hook_delayed_work, hook_det_work);

	switch_data->irq = gpio_to_irq(switch_data->gpio);
	switch_data->hook_irq = gpio_to_irq(switch_data->hook_gpio);
// 20100814 junyeop.kim@lge.com, dmb ant detect [START_LGE]	
#if defined(CONFIG_MACH_LGE_HUB_REV_A) 
	//nothing
#else
	switch_data->dmb_ant_irq = gpio_to_irq(switch_data->dmb_ant_gpio);	// 20100814 junyeop.kim@lge.com, dmb ant gpio [START_LGE]
#endif	
// 20100814 junyeop.kim@lge.com, dmb ant detect [END_LGE]
	
	if (switch_data->irq < 0) {
		ret = switch_data->irq;
		goto err_detect_irq_num_failed;
	}

#if 1		//20101004 junyeop.kim@lge.com, ear sense wakable setting
	/* Make the interrupt on wake up OMAP which is in suspend mode */		
	ret = enable_irq_wake(switch_data->irq);		
	if(ret < 0){
		printk(KERN_INFO "[LUCKYJUN77] EAR SENSE 170 wake up source setting failed!\n");
		disable_irq_wake(switch_data->irq);
		return -ENOSYS;
	}
#endif			

#if 1		//20101109 junyeop.kim@lge.com, hook key wakable setting
	/* Make the interrupt on wake up OMAP which is in suspend mode */		
	ret = enable_irq_wake(switch_data->hook_irq);		
	if(ret < 0){
		printk(KERN_INFO "[LUCKYJUN77] Hook key 163 wake up source setting failed!\n");
		disable_irq_wake(switch_data->hook_irq);
		return -ENOSYS;
	}
#endif			



	ret = request_irq(switch_data->irq, headset_int_handler,
			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, pdev->name, switch_data);
	if (ret < 0)
		goto err_request_irq;

    ret = request_irq(switch_data->hook_irq, headset_hook_int_handler,
			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "headset_hook", switch_data);
	if (ret < 0)
		goto err_request_irq;
			  
// 20100825 junyeop.kim@lge.com, mic bias LDO control test [START_LGE]
#if 1
	omap_mux_init_gpio(MIC_BIAS_LDO, OMAP_PIN_OUTPUT);
	ret = gpio_request(MIC_BIAS_LDO,  pdev->name);
	if(ret < 0) {	
		printk("can't get hub Mic bias LDO enable GPIO\n");
		goto err_request_gpio;
	}
#endif
// 20100825 junyeop.kim@lge.com, mic bias LDO control test [END_LGE]

// 20100814 junyeop.kim@lge.com, dmb ant detect [START_LGE]
#if defined(CONFIG_MACH_LGE_HUB_REV_A) 
	//nothing
#else
    ret = request_irq(switch_data->dmb_ant_irq, dmb_ant_int_handler,
			  IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "dmb_ant", switch_data);
#endif
// 20100814 junyeop.kim@lge.com, dmb ant detect [END_LGE]

	/* Perform initial detection */
	headset_sw_data = switch_data;

	headset_sw_data->is_suspend = 0;	//resume
	
	headset_det_work(&switch_data->work);

// 20100603 junyeop.kim@lge.com, headset suspend/resume [START_LGE]
#ifdef CONFIG_HAS_EARLYSUSPEND
	switch_data->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 2;
	switch_data->early_suspend.suspend = headsetdet_early_suspend;
	switch_data->early_suspend.resume = headsetdet_late_resume;
	register_early_suspend(&switch_data->early_suspend);
#endif
// 20100603 junyeop.kim@lge.com, headset suspend/resume [END_LGE]	

	return 0;

err_request_irq:
err_detect_irq_num_failed:
err_set_gpio_input:
	gpio_free(switch_data->gpio);
err_request_gpio:
    switch_dev_unregister(&switch_data->sdev);
err_switch_dev_register:
	kfree(switch_data);

	return ret;
}

static int headsetdet_remove(struct platform_device *pdev)
{
    struct headset_switch_data *switch_data = platform_get_drvdata(pdev);

	cancel_work_sync(&switch_data->work);
	cancel_delayed_work_sync(&switch_data->delayed_work);
	gpio_free(switch_data->gpio);
	gpio_free(switch_data->hook_gpio);
#if defined(CONFIG_MACH_LGE_HUB_REV_A)
	//nothing
#else	
	gpio_free(switch_data->dmb_ant_gpio);	// 20100814 junyeop.kim@lge.com, dmb ant gpio [START_LGE]	
#endif
    switch_dev_unregister(&switch_data->sdev);
	input_unregister_device(switch_data->ip_dev);
	kfree(switch_data);

	return 0;
}

// 20100603 junyeop.kim@lge.com, headset suspend/resume [START_LGE]
#ifdef CONFIG_HAS_EARLYSUSPEND
static void headsetdet_early_suspend(struct early_suspend *h)
{
//For_Resume_Speed	printk("[LUCKYJUN77] headsetdet_early_suspend\n");
//	disable_irq(headset_sw_data->gpio);	
//	disable_irq(headset_sw_data->hook_irq);
	headset_sw_data->is_suspend = 1;	//suspend flag
}

static void headsetdet_late_resume(struct early_suspend *h)
{
//For_Resume_Speed	printk("[LUCKYJUN77] headsetdet_late_resume\n");

	headset_det_work(&headset_sw_data->work);

//	enable_irq(headset_sw_data->gpio);		    
//	enable_irq(headset_sw_data->hook_irq);
	headset_sw_data->is_suspend = 0;	//resume flag
}
#endif
// 20100603 junyeop.kim@lge.com, headset suspend/resume [END_LGE]

static int headsetdet_suspend(struct platform_device *pdev, pm_message_t state)
{
//For_Resume_Speed	printk("[LUCKYJUN77] headsetdet_suspend\n");
	return 0;
}

static int headsetdet_resume(struct platform_device *pdev)
{
//For_Resume_Speed	printk("[LUCKYJUN77] headsetdet_resume\n");

#if defined(CONFIG_HUB_AMP_WM9093)
	unsigned int cur_device = get_wm9093_mode();
#if 1
	if(headset_type == HUB_HEADSET && cur_device == 6 && gpio_get_value(headset_sw_data->hook_gpio) == 0)
	{
		input_report_key(headset_sw_data->ip_dev, KEY_HOOK, 1);		    
		input_report_key(headset_sw_data->ip_dev, KEY_HOOK, 0);
		input_sync(headset_sw_data->ip_dev);
	}
#endif

	if(cur_device == 4 || cur_device == 5 || cur_device == 6)  //call case
		headset_det_work(&headset_sw_data->work);
#endif
	return 0;
}

static struct platform_driver headsetdet_driver = {
	.probe		= headsetdet_probe,
	.remove		= __devexit_p(headsetdet_remove),
//#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= headsetdet_suspend,
	.resume		= headsetdet_resume,
//#endif
	
	.driver		= {
		.name	= "hub_headset",
		.owner	= THIS_MODULE,
	},
};

static int __init headsetdet_init(void)
{
	return platform_driver_register(&headsetdet_driver);
}

static void __exit headsetdet_exit(void)
{
	platform_driver_unregister(&headsetdet_driver);
}

late_initcall(headsetdet_init); /* LGE_CHANGE_S [iggikim@lge.com] 2009-11-10, rev a headset */
module_exit(headsetdet_exit);

MODULE_AUTHOR("LG Electronics");
MODULE_DESCRIPTION("hub headset Detection Driver");
MODULE_LICENSE("GPL");

