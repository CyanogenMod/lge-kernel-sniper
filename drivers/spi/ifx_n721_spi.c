/*
 * ifx_n721_spi.c -- Serial peheripheral interface framing layer for IFX modem.
 *
 * Copyright (C) 2009 Texas Instruments
 * Authors:	Umesh Bysani <bysani@ti.com> and
 *		Shreekanth D.H <sdh@ti.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */  

#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/smp_lock.h>		// kibum.lee compile error // ICS
#include <asm/uaccess.h>
#include <linux/irq.h>
#include <mach/gpio.h>
#include <plat/omap-pm.h>
#include <linux/tty.h>
#include <linux/tty_driver.h>
#include <linux/tty_flip.h>
#include <linux/interrupt.h>
#include <linux/completion.h>
#include <linux/spi/spi.h>
#include <linux/workqueue.h>
//LGSI_P970_WAP_24thGBXMM_BaselineAdoption_Santosh_Start
#include <plat/omap-pm.h>
//LGSI_P970_WAP_24thGBXMM_BaselineAdoption_Santosh_End
#include "ifx_n721_spi.h"
#include <linux/delay.h>	
#define MRDY_DELAY_TIME 400     //20101127-1, syblue.lee@lge.com, Change delay time for transcation : 1000us -> 400us
#define MODEM_CHK 177           //gpio_177 AP_SLEEP_CHK [LGE-SPI]

//#define CONFIG_SPI_DEBUG
#undef CONFIG_SPI_DEBUG

void dump_atcmd_simple(char *txt, char *data, int len) 
{
	int j ;
	
	if(len>20)
		len = 10;

	printk("%s ", txt);
	for(j=0;j<len;j++)
	{
		if(data[j]>=32 && data[j]<=126)
			printk("%c,",data[j]);	
		else
			printk("%x,",data[j]);	
	}
	printk("\n");
}

#ifdef CONFIG_SPI_DEBUG
#define SPI_DEBUG_PRINT(format, args...)  printk(format , ## args)
#else 
#define SPI_DEBUG_PRINT(format, args...) do{}while(0)
#endif

typedef struct {
	__u8 ea:1;
	__u8 cr:1;
	__u8 d:1;
	__u8 server_chn:5;
} __attribute__ ((packed)) address_field;

typedef struct {
	__u8 ea:1;
	__u8 len:7;
} __attribute__ ((packed)) short_length;

typedef struct {
	__u8 ea:1;
	__u8 l_len:7;
	__u8 h_len;
} __attribute__ ((packed)) long_length;

typedef struct {
	address_field addr;
	__u8 control;
	short_length length;
} __attribute__ ((packed)) short_frame_head;

typedef struct {
	short_frame_head h;
	__u8 data[0];
} __attribute__ ((packed)) short_frame;

typedef struct {
	address_field addr;
	__u8 control;
	long_length length;
	__u8 data[0];
} __attribute__ ((packed)) long_frame_head;

typedef struct {
	long_frame_head h;
	__u8 data[0];
} __attribute__ ((packed)) long_frame;

#define GET_LONG_LENGTH(a) ( ((a).h_len << 7) | ((a).l_len) )

void dump_atcmd(char *data) 
{

	short_frame *short_pkt = (short_frame *) data;
	long_frame *long_pkt;
	u8 *uih_data_start;
	u32 uih_len;
	int i;

	if ((short_pkt->h.length.ea) == 0) {
		long_pkt = (long_frame *) data;
		uih_len = GET_LONG_LENGTH(long_pkt->h.length);
		uih_data_start = long_pkt->h.data;
#ifdef CONFIG_SPI_DEBUG
		//printk("long packet length %d\n", uih_len);
#endif
	} else {
		uih_len = short_pkt->h.length.len;
		uih_data_start = short_pkt->data;
#ifdef CONFIG_SPI_DEBUG
		//printk("long packet length %d\n", uih_len);
#endif
	}
	if(uih_len>10)
		uih_len = 10;
	
	for(i=0; i<uih_len; i++)
	{
		if(uih_data_start[i]>=32 && uih_data_start[i]<=126)
			SPI_DEBUG_PRINT("%c ", uih_data_start[i]);
		else
			SPI_DEBUG_PRINT("%x ", uih_data_start[i]);
	}
	SPI_DEBUG_PRINT("\n");
}

/* ################################################################################################################ */

/* Structure used to store private data */
struct ifx_spi_data {
	dev_t			        devt;
	spinlock_t		        spi_lock;
	struct spi_device	    *spi;
	struct list_head	    device_entry;
    struct completion       ifx_read_write_completion;
    struct tty_struct       *ifx_tty;

	/* buffer is NULL unless this device is open (users > 0) */

	unsigned int		users;
	struct work_struct      ifx_work;
	struct workqueue_struct *ifx_wq;
	unsigned char       wq_name[20];

	unsigned int		ifx_master_initiated_transfer;
	unsigned int		ifx_spi_count;
	unsigned int		ifx_sender_buf_size;
	unsigned int		ifx_receiver_buf_size;
	unsigned int		ifx_current_frame_size;
	unsigned int		ifx_valid_frame_size;
	unsigned int		ifx_ret_count;
	const unsigned char *ifx_spi_buf;
	unsigned char		*ifx_tx_buffer;
	unsigned char       *ifx_rx_buffer;
	unsigned		    mrdy_gpio;
	unsigned		    srdy_gpio;
	unsigned int        wcount;
	int                 ifx_spi_lock;
};

union ifx_spi_frame_header{
	struct{
		unsigned int curr_data_size:12;
		unsigned int more:1;
		unsigned int res1:1;
		unsigned int res2:2;      
		unsigned int next_data_size:12;
		unsigned int ri:1;
		unsigned int dcd:1;
		unsigned int cts_rts:1;
		unsigned int dsr_dtr:1;
	}ifx_spi_header;
	unsigned char framesbytes[IFX_SPI_HEADER_SIZE];
};

/*spi_data_table is consist of 1 spi_data for each mcspi port */
struct ifx_spi_data* spi_data_table[IFX_N_SPI_MINORS];
struct tty_driver 	*ifx_spi_tty_driver;

/* ################################################################################################################ */
/* Global Declarations */
/* Function Declarations */
static void ifx_spi_set_header_info(unsigned char *header_buffer, unsigned int curr_buf_size, unsigned int next_buf_size);
static int ifx_spi_get_header_info(unsigned char *rx_buffer, unsigned int *valid_buf_size);
static void ifx_spi_set_mrdy_signal(struct ifx_spi_data *spi_data, int value);
static void ifx_spi_setup_transmission(struct ifx_spi_data *spi_data);
static void ifx_spi_send_and_receive_data(struct ifx_spi_data *spi_data);
static int ifx_spi_get_next_frame_size(int count);
static int ifx_spi_allocate_frame_memory(struct ifx_spi_data *spi_data, unsigned int memory_size);
static void ifx_spi_free_frame_memory(struct ifx_spi_data *spi_data);
static void ifx_spi_buffer_initialization(struct ifx_spi_data *spi_data);
static unsigned int ifx_spi_sync_read_write(struct ifx_spi_data *spi_data, unsigned int len);
static irqreturn_t ifx_spi_handle_srdy_irq(int irq, void *handle);
static void ifx_spi_handle_work(struct work_struct *work);

/* ################################################################################################################ */

static LIST_HEAD(device_list);
static DEFINE_MUTEX(device_list_lock);

/* ################################################################################################################ */

/* IFX SPI Operations */

/*
 * Function opens a tty device when called from user space
 */
static int 
ifx_spi_open(struct tty_struct *tty, struct file *filp)
{
	struct ifx_spi_data *spi_data;

	switch (tty->index)
	{
	case 0: /* ttyspi0 */
		spi_data = spi_data_table[1];
		break;
	case 1: /* ttyspi1 */
		spi_data = spi_data_table[0];
		break;
	default:
		spi_data = spi_data_table[0];
		break;
	}

        if (spi_data == NULL) {
            return -ENODEV;
        }
	spi_data->ifx_spi_lock = 0;

	ifx_spi_buffer_initialization(spi_data);
	if (spi_data->users++ == 0)
	{
		tty->driver_data = (void *)spi_data;
		spi_data->ifx_tty = tty;
	}

	return 0;
}

/*
 * Function closes a opened a tty device when called from user space
 */
static void 
ifx_spi_close(struct tty_struct *tty, struct file *filp)
{  
	struct ifx_spi_data *spi_data = (struct ifx_spi_data *)tty->driver_data;

	if(!spi_data) return;
	if(spi_data->users == 0) return;

	if(--spi_data->users == 0)
	{
		spi_data->ifx_tty = NULL;
		tty->driver_data = NULL;
	}
}  

/*
 * Function is called from user space to send data to MODEM, it setups the transmission, enable MRDY signal and
 * waits for SRDY signal HIGH from MDOEM. Then starts transmission and reception of data to and from MODEM.
 * Once data read from MODEM is transferred to TTY core flip buffers, then "ifx_read_write_completion" is set
 * and this function returns number of bytes sent to MODEM
 */

//#define LGE_DUMP_SPI_BUFFER   // woojun.ye: disable SPI log printk.
#ifdef LGE_DUMP_SPI_BUFFER
#define COL_SIZE 20	
static void dump_spi_buffer(const unsigned char *txt, const unsigned char *buf, int count)
{
    char dump_buf_str[COL_SIZE+1];

    if (buf != NULL) 
    {
        int j = 0;
        char *cur_str = dump_buf_str;
        unsigned char ch;
        while((j < COL_SIZE) && (j  < COL_SIZE/* count */))	
        {
            ch = buf[j];
            if ((ch < 32) || (ch > 126))
            {
                *cur_str = '.';
            } else
            {
                *cur_str = ch;
            }
            cur_str++;
            j++;
        }
        *cur_str = 0;
	
        printk("%s:count:%d [%s]\n", txt, count, dump_buf_str);                        

    }
    else
    {

        printk("%s: buffer is NULL\n", txt);                 

    }
}
#else
#define dump_spi_buffer(...)
#endif

static int 
ifx_spi_write(struct tty_struct *tty, const unsigned char *buf, int count)
{	
	struct ifx_spi_data *spi_data = (struct ifx_spi_data *)tty->driver_data;

	if(!spi_data) return 0;

	spi_data->ifx_ret_count = 0;

#ifdef LGE_DUMP_SPI_BUFFER
    dump_spi_buffer("ifx_spi_write()", buf, count);
#endif

// hgahn
	if(spi_data->ifx_spi_lock)
		return spi_data->ifx_ret_count;

	spi_data->ifx_tty = tty;
	spi_data->ifx_tty->low_latency = 1;
	if( !buf ){
		printk("File: ifx_n721_spi.c\tFunction: int ifx_spi_write()\t Buffer NULL\n");
		return 0;
	}
	if(!count){
		printk("File: ifx_n721_spi.c\tFunction: int ifx_spi_write()\t Count is ZERO\n");
		return 0;
	}
#ifdef CONFIG_SPI_DEBUG
	printk("[AP]----------------------[S] \n");
#endif
	spi_data->ifx_master_initiated_transfer  = 1;
	spi_data->ifx_spi_buf  = buf;
	spi_data->ifx_spi_count  = count;

#ifdef CONFIG_LGE_SPI_MODE_SLAVE
	queue_work(spi_data->ifx_wq, &spi_data->ifx_work);    
#else
	ifx_spi_set_mrdy_signal(spi_data, 1);  
#endif
//	dump_atcmd(buf+2) ; 	

// LGE_UPDATE_S eungbo.shim@lge.com 20110111 -- SPI RETRY 

//LGSI_P970_WAP_24thGBXMM_BaselineAdoption_Santosh_Start
	wait_for_completion_timeout(&spi_data->ifx_read_write_completion, 3*HZ);	
//LGSI_P970_WAP_24thGBXMM_BaselineAdoption_Santosh_End

	if(spi_data->ifx_ret_count==0)
	{	

        int pin_val;
        pin_val = gpio_get_value(spi_data->srdy_gpio);
		printk("SRDY SIGNAL = %d\n", pin_val);

		ifx_spi_set_mrdy_signal(spi_data, 0);
 
        printk("%s - timeout!! Can't get SRDY from CP for 10sec. Set MRDY high to low\n", __FUNCTION__); // 20120213 taeju.park@lge.com To delete compile warning, too many arguments for format
        dump_spi_buffer("timeout - ifx_spi_write()", buf, count);

		init_completion(&spi_data->ifx_read_write_completion);

        return -3;

	}
// LGE_UPDATE_E eungbo.shim@lge.com 20110111 -- SPI RETRY 
	init_completion(&spi_data->ifx_read_write_completion);
#ifdef CONFIG_SPI_DEBUG
	printk("[AP] ---------------------[END] \n");
#endif
	return spi_data->ifx_ret_count; /* Number of bytes sent to the device */
}

/* This function should return number of free bytes left in the write buffer in this case always return 2048 */

static int 
ifx_spi_write_room(struct tty_struct *tty)
{
    return IFX_SPI_MAX_BUF_SIZE;	
}

/* End of IFX SPI Operations */


static void 
ifx_spi_ap_ready(struct spi_device *spi, int enable)
{
	struct ifx_spi_data *spi_data = spi_data_table[spi->master->bus_num - 1];
	ifx_spi_set_mrdy_signal(spi_data, enable);

#ifdef CONFIG_SPI_DEBUG
	printk("[LGE-SPI] Send ap_ready Signal, %d\n", enable);
#endif
}

/* ################################################################################################################ */

/* TTY - SPI driver Operations */

static int 
ifx_spi_probe(struct spi_device *spi)
{
	int status;
	struct ifx_spi_data *spi_data;
	struct ifx_spi_platform_data *spi_pd = (struct ifx_spi_platform_data *)spi->dev.platform_data;

	/* Allocate SPI driver data */
	spi_data = (struct ifx_spi_data*)kzalloc(sizeof(struct ifx_spi_data), GFP_KERNEL);
	if (!spi_data){
		return -ENOMEM;
        }

	status = ifx_spi_allocate_frame_memory(spi_data, IFX_SPI_MAX_BUF_SIZE + IFX_SPI_HEADER_SIZE);
        if(status != 0){
		printk("File: ifx_n721_spi.c\tFunction: int ifx_spi_probe\tFailed to allocate memory for buffers\n");
		kfree(spi_data);
		return -ENOMEM;
        }
	
        dev_set_drvdata(&spi->dev,spi_data);
        spin_lock_init(&spi_data->spi_lock);
        INIT_WORK(&spi_data->ifx_work,ifx_spi_handle_work);

	sprintf(spi_data->wq_name, "spiwq%d", spi->master->bus_num);
	//rt_workqueue -> singlethread_workqueue
	spi_data->ifx_wq = create_singlethread_workqueue(spi_data->wq_name); //create_rt_workqueue

	if(!spi_data->ifx_wq){
		printk("Failed to setup workqueue - ifx_wq \n");          
        }

	init_completion(&spi_data->ifx_read_write_completion);

	spi_data->mrdy_gpio = spi_pd->mrdy_gpio;
	spi_data->srdy_gpio = spi_pd->srdy_gpio;
	/* Configure SPI */
	spi_data->spi = spi;
	spi->mode = SPI_MODE_1;
	spi->bits_per_word = 8;//32; For A-Project
	spi->chip_select = 0 ;
	spi->max_speed_hz = 24000000; //48000000; //to 24Mhz	
	status = spi_setup(spi);

	spi_data->ifx_spi_lock = 1;

	if(status < 0){
		printk("Failed to setup SPI \n");
	}             

	if (gpio_request(spi_data->srdy_gpio, "ifx srdy") < 0) {
		printk(KERN_ERR "Can't get SRDY GPIO\n");
	}
	gpio_direction_input(spi_data->srdy_gpio);

	if (gpio_request(spi_data->mrdy_gpio, "ifx mrdy") < 0) {
		printk(KERN_ERR "Can't get MRDY GPIO\n");
	}
	gpio_direction_output(spi_data->mrdy_gpio, 0);

	spi->irq = OMAP_GPIO_IRQ(spi_data->srdy_gpio);

	/* Enable SRDY Interrupt request - If the SRDY signal is high then ifx_spi_handle_srdy_irq() is called */
	status = request_irq(spi->irq, ifx_spi_handle_srdy_irq,  IRQF_TRIGGER_RISING, spi->dev.driver->name, spi_data);
	if (status != 0){
		printk(KERN_ERR "Failed to request IRQ for SRDY\n");
		printk(KERN_ERR "IFX SPI Probe Failed\n");
		ifx_spi_free_frame_memory(spi_data);
		if(spi_data){
			kfree(spi_data);
		}          
	}
	
	enable_irq_wake(spi->irq);
	ifx_spi_buffer_initialization(spi_data);
	spi_data_table[spi->master->bus_num - 1] = spi_data;

	gpio_request(MODEM_CHK, "MODEM_CHK");
	// 20100929 yoolje.cho@lge.com  it should be done [START_LGE]
    gpio_direction_output(MODEM_CHK, 1);
	// 20100929 yoolje.cho@lge.com  [END_LGE]
	
	return status;
}

static int 
ifx_spi_remove(struct spi_device *spi)
{	
	struct ifx_spi_data *spi_data;
	spi_data = spi_get_drvdata(spi);
	spin_lock_irq(&spi_data->spi_lock);
	spi_data->spi = NULL;
	spi_set_drvdata(spi, NULL);
	spin_unlock_irq(&spi_data->spi_lock);

	ifx_spi_free_frame_memory(spi_data);
	if(spi_data){
		kfree(spi_data);
	}          
	spi_data_table[spi->master->bus_num - 1] = NULL;
	return 0;
}

static int
ifx_spi_suspend(struct spi_device *spi, pm_message_t mesg)
{
    gpio_direction_output(MODEM_CHK,0);
    
    return 0;
}

static int
ifx_spi_resume(struct spi_device *spi)
{
    printk("modem_chk = %d \n",gpio_get_value(177));
    gpio_direction_output(MODEM_CHK,1);
    
    return 0;
}
/* End of TTY - SPI driver Operations */

/* ################################################################################################################ */

static struct spi_driver ifx_spi_driver = {
	.driver = {
		.name = "ifxn721",
        .bus = &spi_bus_type,
		.owner = THIS_MODULE,
	},
	.probe = ifx_spi_probe,
	.remove = __devexit_p(ifx_spi_remove),
    .suspend = ifx_spi_suspend,
    .resume = ifx_spi_resume,
};

/*
 * Structure to specify tty core about tty driver operations supported in TTY SPI driver.
 */
static const struct tty_operations ifx_spi_ops = {
    .open = ifx_spi_open,
    .close = ifx_spi_close,
    .write = ifx_spi_write,
    .write_room = ifx_spi_write_room,
};

/* ################################################################################################################ */

/*
 * Intialize frame sizes as "IFX_SPI_DEFAULT_BUF_SIZE"(128) bytes for first SPI frame transfer
 */
static void 
ifx_spi_buffer_initialization(struct ifx_spi_data *spi_data)
{
	spi_data->ifx_sender_buf_size = IFX_SPI_DEFAULT_BUF_SIZE;
	spi_data->ifx_receiver_buf_size = IFX_SPI_DEFAULT_BUF_SIZE;
}

/*
 * Allocate memeory for TX_BUFFER and RX_BUFFER
 */
static int 
ifx_spi_allocate_frame_memory(struct ifx_spi_data *spi_data, unsigned int memory_size)
{
	int status = 0;
	spi_data->ifx_rx_buffer = kmalloc(memory_size+IFX_SPI_HEADER_SIZE, GFP_KERNEL);
	if (!spi_data->ifx_rx_buffer){
		printk("Open Failed ENOMEM\n");
		status = -ENOMEM;
	}
	spi_data->ifx_tx_buffer = kmalloc(memory_size+IFX_SPI_HEADER_SIZE, GFP_KERNEL);
	if (!spi_data->ifx_tx_buffer){		
		printk("Open Failed ENOMEM\n");
		status = -ENOMEM;
	}
	if(status == -ENOMEM){
		if(spi_data->ifx_tx_buffer){    
			kfree(spi_data->ifx_tx_buffer);
		}
		if(spi_data->ifx_rx_buffer){
			kfree(spi_data->ifx_rx_buffer);            
		}
	}
	return status;
}
static void 
ifx_spi_free_frame_memory(struct ifx_spi_data *spi_data)
{
	if(spi_data->ifx_tx_buffer){
		kfree(spi_data->ifx_tx_buffer);
	}
	if(spi_data->ifx_rx_buffer){
		kfree(spi_data->ifx_rx_buffer);            
	}
}

/*
 * Function to set header information according to IFX SPI framing protocol specification
 */
static void 
ifx_spi_set_header_info(unsigned char *header_buffer, unsigned int curr_buf_size, unsigned int next_buf_size)
{
	int i;
	union ifx_spi_frame_header header;
	for(i=0; i<4; i++){
		header.framesbytes[i] = 0;
	}

	header.ifx_spi_header.curr_data_size = curr_buf_size;
	if(next_buf_size){
		header.ifx_spi_header.more=1;
		header.ifx_spi_header.next_data_size = next_buf_size;
	}
	else{
		header.ifx_spi_header.more=0;
		header.ifx_spi_header.next_data_size = 128;
	}

	for(i=3; i>=0; i--){
	header_buffer[i] = header.framesbytes[/*3-*/i];
	}
}

/*
 * Function to get header information according to IFX SPI framing protocol specification
 */
static int 
ifx_spi_get_header_info(unsigned char *rx_buffer, unsigned int *valid_buf_size)
{
	int i;
	union ifx_spi_frame_header header;

	for(i=0; i<4; i++){
		header.framesbytes[i] = 0;
	}

	for(i=3; i>=0; i--){
		header.framesbytes[i] = rx_buffer[/*3-*/i];
	}

 //20101127-2, syblue.lee@lge.com, Discard if mux size is bigger than MAX SIZE [START]
       if(header.ifx_spi_header.curr_data_size > IFX_SPI_MAX_BUF_SIZE)   //20101201-1, syblue.lee@lge.com, bug fix : >= -> >
       {
           printk("%s - invalid header : 0x%x 0x%x 0x%x 0x%x!!!\n", __FUNCTION__, header.framesbytes[0], header.framesbytes[1], header.framesbytes[2], header.framesbytes[3]);
           *valid_buf_size = 0;
        }
       else
	*valid_buf_size = header.ifx_spi_header.curr_data_size;
 //20101127-2, syblue.lee@lge.com, Discard if mux size is bigger than MAX SIZE [END]
	if(header.ifx_spi_header.more)
	{
//		printk(KERN_ERR "ifx_spi_get_header_info, There is more packet = %d\n", header.ifx_spi_header.next_data_size);
		return header.ifx_spi_header.next_data_size;
	}
	return 0;
}

/*
 * Function to set/reset MRDY signal
 */
static void 
ifx_spi_set_mrdy_signal(struct ifx_spi_data *spi_data, int value)
{
	gpio_set_value(spi_data->mrdy_gpio, value);

#ifdef CONFIG_SPI_DEBUG
	printk("SEND MRDY SIGNAL = %d\n", value);
#endif
}

/*
 * Function to calculate next_frame_size required for filling in SPI frame Header
 */
static int 
ifx_spi_get_next_frame_size(int count)
{
	if(count > IFX_SPI_MAX_BUF_SIZE){
		return IFX_SPI_MAX_BUF_SIZE;    
	}
	else{   
		return count;
	}
}

/*
 * Function to setup transmission and reception. It implements a logic to find out the ifx_current_frame_size,
 * valid_frame_size and sender_next_frame_size to set in SPI header frame. Copys the data to be transferred from 
 * user space to TX buffer and set MRDY signal to HIGH to indicate Master is ready to transfer data.
 */
static void 
ifx_spi_setup_transmission(struct ifx_spi_data *spi_data)
{
	if( (spi_data->ifx_sender_buf_size != 0) || (spi_data->ifx_receiver_buf_size != 0) ){
		if(spi_data->ifx_sender_buf_size > spi_data->ifx_receiver_buf_size){
			spi_data->ifx_current_frame_size = spi_data->ifx_sender_buf_size;
		}
		else{ 
			spi_data->ifx_current_frame_size = spi_data->ifx_receiver_buf_size;    
		}
		if(spi_data->ifx_spi_count > 0){
			if(spi_data->ifx_spi_count > spi_data->ifx_current_frame_size){
				spi_data->ifx_valid_frame_size = spi_data->ifx_current_frame_size;
				spi_data->ifx_spi_count -= spi_data->ifx_current_frame_size;
			}
			else{
				spi_data->ifx_valid_frame_size = spi_data->ifx_spi_count;
				spi_data->ifx_spi_count = 0;
			}
                }
		else{
			spi_data->ifx_valid_frame_size = 0;
			spi_data->ifx_sender_buf_size = 0;
		}
		spi_data->ifx_sender_buf_size = ifx_spi_get_next_frame_size(spi_data->ifx_spi_count);

		/* memset buffers to 0 */
		memset(spi_data->ifx_tx_buffer,0,IFX_SPI_MAX_BUF_SIZE+IFX_SPI_HEADER_SIZE);
		memset(spi_data->ifx_rx_buffer,0,IFX_SPI_MAX_BUF_SIZE+IFX_SPI_HEADER_SIZE);

		/* Set header information */
		ifx_spi_set_header_info(spi_data->ifx_tx_buffer, spi_data->ifx_valid_frame_size, spi_data->ifx_sender_buf_size);
		if( spi_data->ifx_valid_frame_size > 0 ){      
			memcpy(spi_data->ifx_tx_buffer+IFX_SPI_HEADER_SIZE, spi_data->ifx_spi_buf, spi_data->ifx_valid_frame_size);
			spi_data->ifx_spi_buf += spi_data->ifx_valid_frame_size;
		}
	}
}


/*
 * Function starts Read and write operation and transfers received data to TTY core. It pulls down MRDY signal
 * in case of single frame transfer then sets "ifx_read_write_completion" to indicate transfer complete.
 */
static void 
ifx_spi_send_and_receive_data(struct ifx_spi_data *spi_data)
{
	unsigned int rx_valid_buf_size;
	int status = 0; 

#ifdef CONFIG_SPI_DEBUG
	printk("SPI READ & WRITE [S] \n");
#endif
	status = ifx_spi_sync_read_write(spi_data, spi_data->ifx_current_frame_size+IFX_SPI_HEADER_SIZE); /* 4 bytes for header */                       
#ifdef CONFIG_SPI_DEBUG
	printk("SPI READ & WRITE [END] \n");
	SPI_DEBUG_PRINT("SPI TX : ");
//	dump_atcmd(spi_data->ifx_tx_buffer+IFX_SPI_HEADER_SIZE+2) ; 	
#endif

	if(status > 0)
	{
		memset(spi_data->ifx_tx_buffer,0,IFX_SPI_MAX_BUF_SIZE+IFX_SPI_HEADER_SIZE);
		spi_data->ifx_ret_count += spi_data->ifx_valid_frame_size;
	}
    
	if(*((int*)spi_data->ifx_rx_buffer) == 0xFFFFFFFF)
	{
		spi_data->ifx_receiver_buf_size = 0;
#ifdef CONFIG_SPI_DEBUG
		printk("Recv is Nothing~~!! By Shim\n");
#endif
		return;
	}

	/* Handling Received data */
	spi_data->ifx_receiver_buf_size = ifx_spi_get_header_info(spi_data->ifx_rx_buffer, &rx_valid_buf_size);

	if((spi_data->users > 0) && (rx_valid_buf_size != 0))
	{
#ifdef LGE_DUMP_SPI_BUFFER
    dump_spi_buffer("ifx_spi_send_and_receive_data()[Recev]", &(spi_data->ifx_rx_buffer[4]), rx_valid_buf_size);
#endif

//		dump_atcmd(spi_data->ifx_rx_buffer+IFX_SPI_HEADER_SIZE+2) ;		
		tty_insert_flip_string(spi_data->ifx_tty, spi_data->ifx_rx_buffer+IFX_SPI_HEADER_SIZE, rx_valid_buf_size);

		tty_flip_buffer_push(spi_data->ifx_tty);
	}  
	/*else
  	{ 
	handle RTS and CTS in SPI flow control
	Reject the packet as of now 
	}*/
}

/*
 * Function copies the TX_BUFFER and RX_BUFFER pointer to a spi_transfer structure and add it to SPI tasks.
 * And calls SPI Driver function "spi_sync" to start data transmission and reception to from MODEM
 */
static unsigned int 
ifx_spi_sync_read_write(struct ifx_spi_data *spi_data, unsigned int len)
{
	int status;
	struct spi_message	m;
	struct spi_transfer	t = {
						.tx_buf		= spi_data->ifx_tx_buffer,
                        .rx_buf		= spi_data->ifx_rx_buffer,
						.len		= len,
					};
	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	
	if (spi_data->spi == NULL)
		status = -ESHUTDOWN;
	else
		status = spi_sync(spi_data->spi, &m);
	
	if (status == 0){          
		status = m.status;
		if (status == 0)
			status = m.actual_length;
	}
        else{
		printk("File: ifx_n721_spi.c\nFunction: unsigned int ifx_spi_sync\nTransmission UNsuccessful\n");
        }
	return status;
}

/*
 * Function is a Interrupt service routine, is called when SRDY signal goes HIGH. It set up transmission and
 * reception if it is a Slave initiated data transfer. For both the cases Master intiated/Slave intiated
 * transfer it starts data transfer. 
 */
static irqreturn_t 
ifx_spi_handle_srdy_irq(int irq, void *handle)
{
	struct ifx_spi_data *spi_data = (struct ifx_spi_data *)handle;
#ifdef CONFIG_SPI_DEBUG		
	printk("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< CP ISR\n");
#endif
	{
		int pin_val;
		pin_val = gpio_get_value(spi_data->srdy_gpio);
		if(pin_val == 0){
			return IRQ_HANDLED;
		}
	}
	queue_work(spi_data->ifx_wq, &spi_data->ifx_work);    

	return IRQ_HANDLED; 
}
// <EBS> [LGE_UPDATE_S eungbo.shim@lge.com  2009-04-08 ]
// TODO: 1) ifx_master_initiated_transfer = 1; -->WHEN THE SPI WRITE function calls
// TODO: 2) ifx_master_initiated_transfer = 0; -->Default 
// [LGE_UPDATE_E eungbo.shim@lge.com  2009-04-08 ] <EBS>
static void 
ifx_spi_handle_work(struct work_struct *work)
{
	struct ifx_spi_data *spi_data = container_of(work, struct ifx_spi_data, ifx_work);

	if (!spi_data->ifx_master_initiated_transfer)
	{
//		printk("<<<<<<<<<<<<<<<< CP [S] \n");
#ifdef CONFIG_SPI_DEBUG
		printk("<<<<<<<<<<<<<<<< CP [S] \n");
#endif
//LGSI_P970_WAP_24thGBXMM_BaselineAdoption_Santosh_Start

		ifx_spi_setup_transmission(spi_data);
#ifdef CONFIG_LGE_SPI_MODE_SLAVE
#else
		ifx_spi_set_mrdy_signal(spi_data, 1);
#endif
		ifx_spi_send_and_receive_data(spi_data);
		/* Once data transmission is completed, the MRDY signal is lowered */
		if((spi_data->ifx_sender_buf_size == 0)  && (spi_data->ifx_receiver_buf_size == 0))
		{
			ifx_spi_set_mrdy_signal(spi_data, 0);
			ifx_spi_buffer_initialization(spi_data);
		}

		/* We are processing the slave initiated transfer in the mean time Mux has requested master initiated data transfer */
		/* Once Slave initiated transfer is complete then start Master initiated transfer */
		if(spi_data->ifx_master_initiated_transfer == 1)
		{
		/* It is a condition where Slave has initiated data transfer and both SRDY and MRDY are high and at the end of data transfer		
	 	* MUX has some data to transfer. MUX initiates Master initiated transfer rising MRDY high, which will not be detected at Slave-MODEM.
	 	* So it was required to rise MRDY high again */

//20100701-1, syblue.lee@lge.com, delay time until CP can be ready again [START]
                 udelay(MRDY_DELAY_TIME);
 //20100701-1, syblue.lee@lge.com, delay time until CP can be ready again [END]
#ifdef CONFIG_LGE_SPI_MODE_SLAVE
#else
                ifx_spi_set_mrdy_signal(spi_data, 1);    		
#endif
		}
#ifdef CONFIG_SPI_DEBUG
		printk("<<<<<<<<<<<<<<<< CP [END] \n");
#endif
	}
	else
    {
#ifdef CONFIG_SPI_DEBUG
		printk("[LGE-SPI] INTERRUPT BY OMAP\n");
#endif
		ifx_spi_setup_transmission(spi_data);     
		ifx_spi_send_and_receive_data(spi_data);
		/* Once data transmission is completed, the MRDY signal is lowered */
		if(spi_data->ifx_sender_buf_size == 0)
		{
			if(spi_data->ifx_receiver_buf_size == 0)
			{		
				ifx_spi_set_mrdy_signal(spi_data, 0);

//20100701-1, syblue.lee@lge.com, delay time until CP can be ready again [START]
                udelay(MRDY_DELAY_TIME);   // Data TCP up link throughput - 0us->100us Changed for Sleep current issue 20110514
//20100701-1, syblue.lee@lge.com, delay time until CP can be ready again [END]
				ifx_spi_buffer_initialization(spi_data);
			}
			spi_data->ifx_master_initiated_transfer = 0;
			complete(&spi_data->ifx_read_write_completion);
		}
	}
}
//LGSI_P970_WAP_24thGBXMM_BaselineAdoption_Santosh_End


/* ################################################################################################################ */


/* ################################################################################################################ */

/* Initialization Functions */

/*
 * Initialization function which allocates and set different parameters for TTY SPI driver. Registers the tty driver 
 * with TTY core and register SPI driver with the Kernel. It validates the GPIO pins for MRDY and then request an IRQ
 * on SRDY GPIO pin for SRDY signal going HIGH. In case of failure of SPI driver register cases it unregister tty driver
 * from tty core.
 */
static int 
__init ifx_spi_init(void)
{
int status = 0;

	/* Allocate and Register a TTY device */
	ifx_spi_tty_driver = alloc_tty_driver(IFX_N_SPI_MINORS);
	if (!ifx_spi_tty_driver){
		printk(KERN_ERR "Fail to allocate TTY Driver\n");
		return -ENOMEM;
	}

	/* initialize the tty driver */
	ifx_spi_tty_driver->owner = THIS_MODULE;
	ifx_spi_tty_driver->driver_name = "ifxn721";
	ifx_spi_tty_driver->name = "ttyspi";
	ifx_spi_tty_driver->major = IFX_SPI_MAJOR;
	ifx_spi_tty_driver->minor_start = 0;
	ifx_spi_tty_driver->type = TTY_DRIVER_TYPE_SERIAL;
	ifx_spi_tty_driver->subtype = SERIAL_TYPE_NORMAL;
	ifx_spi_tty_driver->flags = TTY_DRIVER_REAL_RAW;
	ifx_spi_tty_driver->init_termios = tty_std_termios;

	ifx_spi_tty_driver->init_termios.c_cflag = /*B115200*/B38400 | CS8 | CREAD | HUPCL | CLOCAL; //B9600 | CS8 | CREAD | HUPCL | CLOCAL; B38400 | CS8 | CREAD | HUPCL | CLOCAL;
	tty_set_operations(ifx_spi_tty_driver, &ifx_spi_ops);

	status = tty_register_driver(ifx_spi_tty_driver);
	if (status){
		printk(KERN_ERR "Failed to register IFX SPI tty driver");
		put_tty_driver(ifx_spi_tty_driver);
		return status;
	}

	/* Register SPI Driver */
	status = spi_register_driver(&ifx_spi_driver);
	if (status < 0){ 
		printk("Failed to register SPI device");
		tty_unregister_driver(ifx_spi_tty_driver);
		put_tty_driver(ifx_spi_tty_driver);
		return status;
	}
	return status;
}

module_init(ifx_spi_init);


/*
 * Exit function to unregister SPI driver and tty SPI driver
 */
static void 
__exit ifx_spi_exit(void)
{  
	spi_unregister_driver(&ifx_spi_driver);
	tty_unregister_driver(ifx_spi_tty_driver);
    put_tty_driver(ifx_spi_tty_driver);
}

module_exit(ifx_spi_exit);

/* End of Initialization Functions */

/* ################################################################################################################ */

MODULE_AUTHOR("Umesh Bysani and Shreekanth D.H, <bysani@ti.com> <sdh@ti.com>");
MODULE_DESCRIPTION("IFX SPI Framing Layer");
MODULE_LICENSE("GPL");
