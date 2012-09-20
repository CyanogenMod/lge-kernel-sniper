obj-$(CONFIG_LGE_MTC_ETA)	+= mtc_eta/
obj-$(CONFIG_HUB_CHARGING_IC)   += hub_charging_ic.o
obj-$(CONFIG_HUB_MODEM_CONTROL) += hub_modem_ctrl.o

ifeq ($(CONFIG_PRODUCT_LGE_P970),y)
obj-$(CONFIG_HUB_MUIC)          += hub_muic.o
endif
ifeq ($(CONFIG_PRODUCT_LGE_KU5900),y)
obj-$(CONFIG_HUB_MUIC)          += ku5900_muic.o
endif
ifeq ($(CONFIG_PRODUCT_LGE_LU6800),y)
obj-$(CONFIG_HUB_MUIC)          += lu6800_muic.o
endif

obj-$(CONFIG_LGE_USB_SWITCH)    += usb_switch.o
obj-$(CONFIG_HUB_HEADSET_DET)  	+= headset_det.o
obj-$(CONFIG_HUB_AMP_WM9093)  += wm9093.o
obj-$(CONFIG_SPI_MDM6600) += ts0710mux_mdm/
obj-$(CONFIG_KERNEL_GSM0710MUX) += ts0710mux/
obj-$(CONFIG_RINETWORK_DRIVER)  += rindrv/
obj-$(CONFIG_HUB_GPS_GPIO)	+= hub_gps.o
obj-$(CONFIG_BD_ADDRESS)	+= bd_address.o
obj-$(CONFIG_BACKLIGHT_LM3528)	+=	lm3528.o		# sghyun_justin_lcd

