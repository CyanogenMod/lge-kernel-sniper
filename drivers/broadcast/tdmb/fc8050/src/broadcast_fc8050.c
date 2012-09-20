#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/spi/spi.h>
#include <linux/spi/spidev.h>
#include <linux/interrupt.h>

#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/workqueue.h>
#include <linux/wakelock.h> 		/* wake_lock, unlock */

#include "../../broadcast_tdmb_drv_ifdef.h"
#include "../inc/broadcast_fc8050.h"
#include "../inc/fci_types.h"
#include "../inc/bbm.h"

/* DEFINE set for TDMB driver { */
/* SPI Data read using workqueue if this is not defined, using irq_thread */
//#define FEATURE_DMB_USE_WORKQUEUE
#define PM_QOS
/* DEFINE set for TDMB driver } */

#ifdef PM_QOS
#include <linux/pm_qos_params.h>
#endif /* PM_QOS */

/* external function */
extern int broadcast_drv_if_isr(void);
extern void fc8050_isr_control(fci_u8 onoff);

/* proto type declare */
static int broadcast_tdmb_fc8050_probe(struct spi_device *spi);
static int broadcast_tdmb_fc8050_remove(struct spi_device *spi);
static int broadcast_tdmb_fc8050_suspend(struct spi_device *spi, pm_message_t mesg);
static int broadcast_tdmb_fc8050_resume(struct spi_device *spi);

//--[[ LGE_UBIQUIX_MODIFIED_START : ymjun@mnbt.co.kr - TDMB 
#if defined(CONFIG_PRODUCT_LGE_LU6800)
#define DMB_EN 			28 //GPIO28
#define DMB_INT_N 		29 //GPIO29
#define DMB_RESET_N 	62 //GPIO62
#define DMB_EAR_ANT_SEL	51 //GPIO51
#elif defined(CONFIG_PRODUCT_LGE_KU5900)
#define DMB_EN 			92 //GPIO92
#define DMB_INT_N 		93 //GPIO93
#define DMB_RESET_N 	160 //GPIO160
#define DMB_EAR_ANT_SEL	59 //GPIO59
#else
#error need DMB_EN, DMB_INT_N, DMB_REST_N, DMB_EAR_ANT_SEL define.
#endif
//--]] LGE_UBIQUIX_MODIFIED_END : ymjun@mnbt.co.kr - TDMB 

/************************************************************************/
/* LINUX Driver Setting                                                 */
/************************************************************************/
static uint32 user_stop_flg = 0;
struct tdmb_fc8050_ctrl_blk
{
	boolean 					TdmbPowerOnState;
	struct spi_device* 			spi_ptr;
#ifdef FEATURE_DMB_USE_WORKQUEUE	
	struct work_struct 			spi_work;
	struct workqueue_struct* 	spi_wq;
#endif
	struct mutex				mutex;
	struct wake_lock 			wake_lock;	/* wake_lock,wake_unlock */
	//test
	boolean 					spi_irq_status;
	spinlock_t				spin_lock;
#ifdef PM_QOS
	struct pm_qos_request_list 		pm_req_list;
#endif /* PM_QOS */
};


static struct tdmb_fc8050_ctrl_blk fc8050_ctrl_info;

struct spi_device *tdmb_fc8050_get_spi_device(void)
{
	return fc8050_ctrl_info.spi_ptr;
}

void tdmb_fc8050_set_userstop(int mode)
{
	user_stop_flg = mode;
}

int tdmb_fc8050_mdelay(int32 ms)
{
	int32	wait_loop =0;
	int32	wait_ms = ms;
	int		rc = 1;  /* 0 : false, 1 : ture */

	if(ms > 100)
	{
		wait_loop = (ms /100);   /* 100, 200, 300 more only , Otherwise this must be modified e.g (ms + 40)/50 */
		wait_ms = 100;
	}

	do
	{
		mdelay(wait_ms);//msleep(wait_ms);
		if(user_stop_flg == 1)
		{
			printk("~~~~~~~~ Ustop flag is set so return false ms =(%d)~~~~~~~~\n", ms);
			rc = 0;
			break;
		}
	}while((--wait_loop) > 0);

	return rc;
}

void tdmb_fc8050_Must_mdelay(int32 ms)
{
	mdelay(ms);
}

int tdmb_fc8050_tdmb_is_on(void)
{
	return (int)fc8050_ctrl_info.TdmbPowerOnState;
}

/* EXPORT_SYMBOL() : when we use external symbol             */
/* which is not included in current module - over kernel 2.6    */
/* EXPORT_SYMBOL(tdmb_fc8050_tdmb_is_on);                    */

int tdmb_fc8050_power_on(void)
{
	printk("tdmb_fc8050_power_on \n");

	// DMB_INT = GPIO29
	// DMB_EN = GPIO28(1.2V) , 1.8V_VIO(alyways on)
	// DMB_RESET = GPIO175
	if ( fc8050_ctrl_info.TdmbPowerOnState == FALSE )
	{
#ifdef PM_QOS
		if(pm_qos_request_active(&fc8050_ctrl_info.pm_req_list)) {
			pm_qos_update_request(&fc8050_ctrl_info.pm_req_list, 20);
		}
#endif  /* PM_QOS */

		wake_lock(&fc8050_ctrl_info.wake_lock);
		
		gpio_direction_output(DMB_EAR_ANT_SEL,true);
		gpio_set_value(DMB_EAR_ANT_SEL, 0);
		gpio_direction_input(DMB_INT_N);
		gpio_direction_output(DMB_RESET_N, false);
		gpio_direction_output(DMB_EN, true);
		gpio_set_value(DMB_EN, 1);
		gpio_set_value(DMB_RESET_N, 1);
		udelay(1000); //500us
		udelay(1000);
		udelay(1000);
		gpio_set_value(DMB_RESET_N, 0);
		udelay(5); //500us
		gpio_set_value(DMB_RESET_N, 1);
		tdmb_fc8050_interrupt_free();
		fc8050_ctrl_info.TdmbPowerOnState = TRUE;

		printk("tdmb_fc8050_power_on OK\n");
		
	}
	else
	{
		printk("tdmb_fc8050_power_on the power already turn on \n");
	}

	printk("tdmb_fc8050_power_on completed \n");

	return TRUE;
}

int tdmb_fc8050_power_off(void)
{
	if ( fc8050_ctrl_info.TdmbPowerOnState == TRUE )
	{
		tdmb_fc8050_interrupt_lock();
		fc8050_ctrl_info.TdmbPowerOnState = FALSE;
		gpio_set_value(DMB_RESET_N, 0);
		gpio_set_value(DMB_EN, 0);
		gpio_direction_output(DMB_INT_N, false);
		gpio_set_value(DMB_INT_N, 0);		
		gpio_set_value(DMB_EAR_ANT_SEL,0);
		wake_unlock(&fc8050_ctrl_info.wake_lock);

#ifdef PM_QOS		/* QoS release */
		if(pm_qos_request_active(&fc8050_ctrl_info.pm_req_list)) {
			pm_qos_update_request(&fc8050_ctrl_info.pm_req_list, PM_QOS_DEFAULT_VALUE);
		}
#endif /* PM_QOS */
	}
	else
	{
		printk("tdmb_fc8050_power_on the power already turn off \n");
	}	

	printk("tdmb_fc8050_power_off completed \n");
	
	return TRUE;
}

int tdmb_fc8050_select_antenna(unsigned int sel)
{
	if(LGE_BROADCAST_TDMB_ANT_TYPE_INTENNA == sel)
	{
		gpio_set_value(DMB_EAR_ANT_SEL, 0);
	}
	else if(LGE_BROADCAST_TDMB_ANT_TYPE_EARANT == sel)
	{
		gpio_set_value(DMB_EAR_ANT_SEL, 1);
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

static struct spi_driver broadcast_tdmb_driver = {
	.probe = broadcast_tdmb_fc8050_probe,
	.remove	= __devexit_p(broadcast_tdmb_fc8050_remove),
	.suspend = broadcast_tdmb_fc8050_suspend,
	.resume  = broadcast_tdmb_fc8050_resume,
	.driver = {
		.name = "tdmb_fc8050",
		.bus	= &spi_bus_type,
		.owner = THIS_MODULE,
	},
};

void tdmb_fc8050_interrupt_lock(void)
{
	if (fc8050_ctrl_info.spi_ptr == NULL)
	{
		printk("tdmb_fc8050_interrupt_lock fail\n");
		return; // by hskim@mnbt.co.kr
	}

	disable_irq(fc8050_ctrl_info.spi_ptr->irq);
}

void tdmb_fc8050_interrupt_free(void)
{
	if (fc8050_ctrl_info.spi_ptr == NULL)
	{
		printk("tdmb_fc8050_interrupt_free fail\n");
		return; // by hskim@mnbt.co.kr
	}

	enable_irq(fc8050_ctrl_info.spi_ptr->irq);
}

int tdmb_fc8050_spi_write_read(uint8* tx_data, int tx_length, uint8 *rx_data, int rx_length)
{
	int rc;
	
	struct spi_transfer	t = {
			.tx_buf		= tx_data,
			.rx_buf		= rx_data,
			.len		= tx_length+rx_length,
		};
	struct spi_message	m;	

	if (fc8050_ctrl_info.spi_ptr == NULL)
	{
		printk("tdmb_fc8050_spi_write_read error txdata=0x%x, length=%d\n", (unsigned int)tx_data, tx_length+rx_length);
		return FALSE;
	}

	mutex_lock(&fc8050_ctrl_info.mutex);

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	rc = spi_sync(fc8050_ctrl_info.spi_ptr, &m);
	if ( rc < 0 )
	{
		printk("tdmb_fc8050_spi_read_burst result(%d), actual_len=%d\n",rc, m.actual_length);
	}

	mutex_unlock(&fc8050_ctrl_info.mutex);

	return TRUE;
}

#ifdef FEATURE_DMB_USE_WORKQUEUE
static irqreturn_t broadcast_tdmb_spi_isr(int irq, void *handle)
{
	struct tdmb_fc8050_ctrl_blk* fc8050_info_p;
	unsigned long flag;

	fc8050_info_p = (struct tdmb_fc8050_ctrl_blk *)handle;	
	if ( fc8050_info_p && fc8050_info_p->TdmbPowerOnState )
	{
		if (fc8050_info_p->spi_irq_status)
		{			
			printk("######### spi read function is so late skip #########\n");			
			return IRQ_HANDLED;
		}
		spin_lock_irqsave(&fc8050_info_p->spin_lock, flag);
		queue_work(fc8050_info_p->spi_wq, &fc8050_info_p->spi_work);    
		spin_unlock_irqrestore(&fc8050_info_p->spin_lock, flag);
	}
	else
	{
		printk("broadcast_tdmb_spi_isr is called, but device is off state\n");
	}

	return IRQ_HANDLED; 
}

static void broacast_tdmb_spi_work(struct work_struct *tdmb_work)
{
	struct tdmb_fc8050_ctrl_blk *pTdmbWorkData;

	pTdmbWorkData = container_of(tdmb_work, struct tdmb_fc8050_ctrl_blk, spi_work);
	if ( pTdmbWorkData )
	{
		fc8050_isr_control(0);
		pTdmbWorkData->spi_irq_status = TRUE;
		broadcast_drv_if_isr();
		pTdmbWorkData->spi_irq_status = FALSE;
		fc8050_isr_control(1);
	}
	else
	{
		printk("~~~~~~~broadcast_tdmb_spi_work call but pTdmbworkData is NULL ~~~~~~~\n");
	}
}
#else
static irqreturn_t broadcast_tdmb_spi_event_handler(int irq, void *handle)
{	
	struct tdmb_fc8050_ctrl_blk* fc8050_info_p;

	fc8050_info_p = (struct tdmb_fc8050_ctrl_blk *)handle;	
	if ( fc8050_info_p && fc8050_info_p->TdmbPowerOnState )
	{
		if (fc8050_info_p->spi_irq_status)
		{			
			printk("######### spi read function is so late skip #########\n");			
			return IRQ_HANDLED;
		}		

		fc8050_isr_control(0);
		fc8050_info_p->spi_irq_status = TRUE;
		broadcast_drv_if_isr();
		fc8050_info_p->spi_irq_status = FALSE;
		fc8050_isr_control(1);
	}
	else
	{
		printk("broadcast_tdmb_spi_isr is called, but device is off state\n");
	}

	return IRQ_HANDLED; 
}
#endif


static int broadcast_tdmb_fc8050_probe(struct spi_device *spi)
{
	int rc;

	fc8050_ctrl_info.TdmbPowerOnState = FALSE;

	fc8050_ctrl_info.spi_ptr 				= spi;
	fc8050_ctrl_info.spi_ptr->mode 			= SPI_MODE_0;
	fc8050_ctrl_info.spi_ptr->bits_per_word 	= 8;
	fc8050_ctrl_info.spi_ptr->max_speed_hz 	= ( 24000*1000 );
	rc = spi_setup(spi);
	printk("broadcast_tdmb_fc8050_probe spi_setup=%d\n", rc);
	BBM_HOSTIF_SELECT(NULL, 1);

#ifdef FEATURE_DMB_USE_WORKQUEUE
	INIT_WORK(&fc8050_ctrl_info.spi_work, broacast_tdmb_spi_work);
	fc8050_ctrl_info.spi_wq = create_singlethread_workqueue("tdmb_spi_wq");
	if(fc8050_ctrl_info.spi_wq == NULL){
		printk("Failed to setup tdmb spi workqueue \n");
		return -ENOMEM;
	}
#endif

	//--[[ LGE_UBIQUIX_MODIFIED_START : ymjun@mnbt.co.kr [2011.07.08] - TDMB 
	gpio_request(DMB_RESET_N, "dmb_reset");
	gpio_request(DMB_EN, "dmb_en");
	gpio_request(DMB_INT_N, "dmb_int");
	gpio_request(DMB_EAR_ANT_SEL, "dmb_ear_ant");
	//--]] LGE_UBIQUIX_MODIFIED_END : ymjun@mnbt.co.kr  [2011.07.08] - TDMB 
	
#ifdef FEATURE_DMB_USE_WORKQUEUE
	rc = request_irq(spi->irq, broadcast_tdmb_spi_isr, IRQF_DISABLED | IRQF_TRIGGER_FALLING, 
	                   spi->dev.driver->name, &fc8050_ctrl_info);
#else
	rc = request_threaded_irq(spi->irq, NULL, broadcast_tdmb_spi_event_handler, IRQF_DISABLED | IRQF_TRIGGER_FALLING,
	                      spi->dev.driver->name, &fc8050_ctrl_info);
#endif
	printk("broadcast_tdmb_fc8050_probe request_irq=%d\n", rc);
	//enable_irq_wake(spi->irq);

	// DMB_EN = GPIO28(1.2V) , 1.8V_VIO(alyways on)
	// DMB_RESET = GPIO62

	//gpio_free(DMB_RESET_N);
	//gpio_direction_output(DMB_RESET_N, false);
	gpio_direction_output(DMB_EN, false);
	gpio_direction_output(DMB_INT_N, false);
	tdmb_fc8050_interrupt_lock();

	mutex_init(&fc8050_ctrl_info.mutex);

	wake_lock_init(&fc8050_ctrl_info.wake_lock,  WAKE_LOCK_SUSPEND, dev_name(&spi->dev));		
	spin_lock_init(&fc8050_ctrl_info.spin_lock);

#ifdef PM_QOS
	pm_qos_add_request(&fc8050_ctrl_info.pm_req_list, PM_QOS_CPU_DMA_LATENCY, PM_QOS_DEFAULT_VALUE);
#endif /* PM_QOS */

	printk("broadcast_fc8050_probe End\n");
	return rc;
}

static int broadcast_tdmb_fc8050_remove(struct spi_device *spi)
{
	printk("broadcast_tdmb_fc8050_remove \n");

#ifdef FEATURE_DMB_USE_WORKQUEUE
	if (fc8050_ctrl_info.spi_wq)
	{
		flush_workqueue(fc8050_ctrl_info.spi_wq);
		destroy_workqueue(fc8050_ctrl_info.spi_wq);
	}
#endif
	free_irq(spi->irq, &fc8050_ctrl_info);

	mutex_destroy(&fc8050_ctrl_info.mutex);

	wake_lock_destroy(&fc8050_ctrl_info.wake_lock);

#ifdef PM_QOS
	pm_qos_remove_request(&fc8050_ctrl_info.pm_req_list);
#endif /* PM_QOS */

	memset((unsigned char*)&fc8050_ctrl_info, 0x0, sizeof(struct tdmb_fc8050_ctrl_blk));
	return 0;
}

static int broadcast_tdmb_fc8050_suspend(struct spi_device *spi, pm_message_t mesg)
{
	printk("broadcast_tdmb_fc8050_suspend \n");
	return 0;
}

static int broadcast_tdmb_fc8050_resume(struct spi_device *spi)
{
	printk("broadcast_tdmb_fc8050_resume \n");
	return 0;
}

int __devinit broadcast_tdmb_drv_init(void)
{
	int rc;

	rc = broadcast_tdmb_drv_start();
	printk("broadcast_tdmb_fc8050_probe start %d\n", rc);

	return spi_register_driver(&broadcast_tdmb_driver);
}

static void __exit broadcast_tdmb_drv_exit(void)
{
	spi_unregister_driver(&broadcast_tdmb_driver);
}

module_init(broadcast_tdmb_drv_init);
module_exit(broadcast_tdmb_drv_exit);

/* optional part when we include driver code to build-on
it's just used when we make device driver to module(.ko)
so it doesn't work in build-on */
MODULE_DESCRIPTION("FC8050 tdmb device driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("FCI");

