#
# Makefile for misc devices that really don't fit anywhere else.
#

obj-$(CONFIG_IBM_ASM)		+= ibmasm/
obj-$(CONFIG_AD525X_DPOT)	+= ad525x_dpot.o
obj-$(CONFIG_AD525X_DPOT_I2C)	+= ad525x_dpot-i2c.o
obj-$(CONFIG_AD525X_DPOT_SPI)	+= ad525x_dpot-spi.o
obj-$(CONFIG_ATMEL_PWM)		+= atmel_pwm.o
obj-$(CONFIG_ATMEL_SSC)		+= atmel-ssc.o
obj-$(CONFIG_ATMEL_TCLIB)	+= atmel_tclib.o
obj-$(CONFIG_BMP085)		+= bmp085.o
obj-$(CONFIG_ICS932S401)	+= ics932s401.o
obj-$(CONFIG_LKDTM)		+= lkdtm.o
obj-$(CONFIG_TIFM_CORE)       	+= tifm_core.o
obj-$(CONFIG_TIFM_7XX1)       	+= tifm_7xx1.o
obj-$(CONFIG_PHANTOM)		+= phantom.o
obj-$(CONFIG_ANDROID_PMEM)	+= pmem.o
obj-$(CONFIG_SGI_IOC4)		+= ioc4.o
obj-$(CONFIG_ENCLOSURE_SERVICES) += enclosure.o
obj-$(CONFIG_KERNEL_DEBUGGER_CORE)	+= kernel_debugger.o
obj-$(CONFIG_KGDB_TESTS)	+= kgdbts.o
obj-$(CONFIG_SGI_XP)		+= sgi-xp/
obj-$(CONFIG_SGI_GRU)		+= sgi-gru/
obj-$(CONFIG_CS5535_MFGPT)	+= cs5535-mfgpt.o
obj-$(CONFIG_HP_ILO)		+= hpilo.o
obj-$(CONFIG_ISL29003)		+= isl29003.o
obj-$(CONFIG_SENSORS_TSL2550)	+= tsl2550.o
obj-$(CONFIG_EP93XX_PWM)	+= ep93xx_pwm.o
obj-$(CONFIG_DS1682)		+= ds1682.o
obj-$(CONFIG_TI_DAC7512)	+= ti_dac7512.o
obj-$(CONFIG_UID_STAT)		+= uid_stat.o
obj-$(CONFIG_C2PORT)		+= c2port/
obj-$(CONFIG_IWMC3200TOP)      += iwmc3200top/
obj-y				+= eeprom/
obj-y				+= cb710/
obj-$(CONFIG_VMWARE_BALLOON)	+= vmware_balloon.o
obj-$(CONFIG_SENSORS_BH1780)	+= bh1780gli.o
obj-$(CONFIG_WL127X_RFKILL)	+= wl127x-rfkill.o
obj-$(CONFIG_LBEE9QMB_RFKILL)	+= lbee9qmb-rfkill.o
obj-$(CONFIG_APANIC)		+= apanic.o
obj-$(CONFIG_SENSORS_AK8975)	+= akm8975.o
obj-$(CONFIG_TWL6040_VIB)	+= twl6040-vib.o
obj-$(CONFIG_CDC_TCXO)		+= cdc_tcxo.o
obj-y				+= ti-st/
obj-$(CONFIG_SENSORS_LIS331DLH)	+= lis331dlh.o
# LGE_CHANGE_S, hyun.seungjin@lge.com, 2011-02-23, Move directory from driver/hub to misc
obj-$(CONFIG_GP2AP_PROXIMITY) += gp2ap_proximity.o
# LGE_CHANGE_E, hyun.seungjin@lge.com, 2011-02-23, Move directory from driver/hub to misc
# LGE_CHANGE_S, ryu.seeyeol@lge.com, 2011-01-28, Porting for Sensor Driver 
#LGE_TELECA_RIL_RECOVERY_CP_RESET_6  -START
#20110505, ravi.holal@teleca.com, RIL RECOVERY
#Description: Enable modem watcher
# LGE_CHANGE [jugwan.eom@lge.com] 2011-04-25, Modem Watcher
obj-$(CONFIG_LGE_MODEM_WATCHER)	+= modem_watcher.o
#LGE_TELECA_RIL_RECOVERY_CP_RESET_6  -END


obj-$(CONFIG_MPU_SENSORS_MPU3050)   += mpu3050/
# LGE_CHANGE_E, ryu.seeyeol@lge.com, 2011-01-28, Porting for Sensor Driver 
# prime@sdcmicro.com Merge from 2.6.32 [START]
obj-$(CONFIG_TSPDRV)		+= tspdrv/
# prime@sdcmicro.com Merge from 2.6.32 [END]
