# MUIC Class Driver
obj-$(CONFIG_MUIC)		+= muic_class.o muic_client_class.o

obj-$(CONFIG_DP3T)		+= dp3t.o
obj-$(CONFIG_USIF)		+= usif.o


ifeq ($(CONFIG_PRODUCT_LGE_LU6800),y)
obj-$(CONFIG_MUIC_TS5USBA33402) += lu6800_muic.o
endif
ifeq ($(CONFIG_PRODUCT_LGE_KU5900),y)
obj-$(CONFIG_MUIC_MAX14526DEEWP)          += ku5900_muic.o
endif
ifeq ($(CONFIG_PRODUCT_LGE_P970),y)
obj-$(CONFIG_MUIC_MAX14526DEEWP) += p970_muic.o
endif


