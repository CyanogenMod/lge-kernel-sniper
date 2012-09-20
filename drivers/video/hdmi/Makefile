

############### select your chip + platform here ###################

TDA_TX := TDA19989
# TDA_TX := TDA9984
# TDA_TX := TDA9983
# TDA_TX := TDA9981

TDA_PLATFORM := ZOOMII
# TDA_PLATFORM := OTHERS

TDA_HDCP := 0
#TDA_HDCP := TMFL_HDCP_SUPPORT

####################################################################

EXTRA_CFLAGS += -DFUNC_PTR=" " -DCONST_DAT="const " -DRAM_DAT=" "
EXTRA_CFLAGS += -DTDA_NAME=$(TDA_TX)
EXTRA_CFLAGS += -DTMFL_NO_RTOS -DIRQ
# EXTRA_CFLAGS += -DTMFL_NO_RTOS -DIRQ -DFORMAT_PC

ifeq ($(TDA_PLATFORM),ZOOMII)
EXTRA_CFLAGS += -DZOOMII_PATCH
#EXTRA_CFLAGS += -DTWL4030_HACK
EXTRA_CFLAGS += -DANDROID_DSS
EXTRA_CFLAGS += -DGUI_OVER_HDMI
endif

ifeq ($(TDA_TX),TDA19989)
EXTRA_CFLAGS += -DTMFL_TDA19989 -DTMFL_TDA9989
ifeq ($(TDA_HDCP),TMFL_HDCP_SUPPORT)
EXTRA_CFLAGS += -D$(TDA_HDCP)
else
EXTRA_CFLAGS += -DNO_HDCP
endif
endif

ifeq ($(TDA_TX),TDA9984)
EXTRA_CFLAGS += -DNO_HDCP
endif

ifeq ($(TDA_TX),TDA9981)
EXTRA_CFLAGS += -DTMFL_TDA9981_SUPPORT -DTMFL_RX_SENSE_ON
endif

########## devlib ##################################################

ifeq ($(TDA_TX),TDA19989)
TXSRC := comps/tmdlHdmiTx/src
TXCFG := comps/tmdlHdmiTx/cfg/TDA9989
CECSRC := comps/tmdlHdmiCEC/src
CECCFG := comps/tmdlHdmiCEC/cfg
endif

ifeq ($(TDA_TX),TDA9984)
TXSRC := comps/tmdlHdmiTx/src
TXCFG := comps/tmdlHdmiTx/cfg/TDA9989
endif

ifeq ($(TDA_TX),TDA9983)
TXSRC := comps/tmdlTDA9983/src
TXCFG := comps/tmdlTDA9983/cfg
endif

ifeq ($(TDA_TX),TDA9981)
TXSRC := comps/tmdlTDA9983/src
TXCFG := comps/tmdlTDA9983/cfg
endif

########## board service layer #####################################

ifeq ($(TDA_TX),TDA19989)
BSL := comps/tmbslTDA9989NoHdcp/src/TDA9989n2
endif

ifeq ($(TDA_TX),TDA9984)
BSL := comps/tmbslTDA9984NoHdcp/src
endif

ifeq ($(TDA_TX),TDA9983)
BSL := comps/tmbslTDA9983/src
endif

ifeq ($(TDA_TX),TDA9981)
BSL := comps/tmbslTDA9983/src
endif

####################################################################

#obj-m += hdmitx.o #changhoony.lee@lge.com
obj-$(CONFIG_NXP_HDMI)  += hdmitx.o	#changhoony.lee@lge.com
ifeq ($(TDA_TX),TDA19989)
#obj-m += hdmicec.o	#changhoony.lee@lge.com
endif

# Linux module
hdmitx-objs := tda998x.o
#hdmicec-objs := tda998x_cec.o	#changhoony.lee@lge.com

# NXP comps
ifeq ($(TDA_TX),TDA19989)
hdmitx-objs += $(TXSRC)/tmdlHdmiTx.o
hdmitx-objs += $(TXSRC)/tmdlHdmiTx_local.o
hdmitx-objs += $(TXCFG)/tmdlHdmiTx_Linux.o
hdmitx-objs += $(BSL)/tmbslTDA9989_local.o
hdmitx-objs += $(BSL)/tmbslTDA9989_InOut.o
hdmitx-objs += $(BSL)/tmbslTDA9989_HDCP.o
hdmitx-objs += $(BSL)/tmbslTDA9989_State.o
hdmitx-objs += $(BSL)/tmbslTDA9989_Misc.o
hdmitx-objs += $(BSL)/tmbslTDA9989_Edid.o
hdmicec-objs += $(CECSRC)/tmdlHdmiCEC.o
hdmicec-objs += $(CECSRC)/tmdlHdmiCEC_local.o
hdmicec-objs += $(CECCFG)/tmdlHdmiCEC_Linux.o
endif

ifeq ($(TDA_TX),TDA9984)
hdmitx-objs += $(TXSRC)/tmdlHdmiTx.o
hdmitx-objs += $(TXSRC)/tmdlHdmiTx_local.o
hdmitx-objs += $(TXCFG)/tmdlHdmiTx_Linux.o
hdmitx-objs += $(BSL)/tmbslTDA9984_local.o 
hdmitx-objs += $(BSL)/tmbslTDA9984_InOut.o
hdmitx-objs += $(BSL)/tmbslTDA9984_HDCP.o
hdmitx-objs += $(BSL)/tmbslTDA9984_State.o
hdmitx-objs += $(BSL)/tmbslTDA9984_Misc.o
hdmitx-objs += $(BSL)/tmbslTDA9984_Edid.o
endif

ifeq ($(TDA_TX),TDA9983)
hdmitx-objs += $(BSL)/tmbslHdmiTx_2.o
hdmitx-objs += $(BSL)/tmbslHdmiTx_1.o
hdmitx-objs += $(BSL)/tmbslHdmiTx_local.o
hdmitx-objs += $(TXSRC)/tmdlHdmiTx.o
hdmitx-objs += $(TXCFG)/tmdlHdmiTx_Linux.o
endif

ifeq ($(TDA_TX),TDA9981)
hdmitx-objs += $(BSL)/tmbslHdmiTx_2.o
hdmitx-objs += $(BSL)/tmbslHdmiTx_1.o
hdmitx-objs += $(BSL)/tmbslHdmiTx_local.o
hdmitx-objs += $(TXSRC)/tmdlHdmiTx.o
hdmitx-objs += $(TXCFG)/tmdlHdmiTx_Linux.o
endif

####################################################################

EXTRA_CFLAGS += -I$(obj)
EXTRA_CFLAGS += -I$(obj)/inc

ifeq ($(TDA_TX),TDA19989)
EXTRA_CFLAGS += -I$(obj)/comps/tmbslHdmiTx/inc
EXTRA_CFLAGS += -I$(obj)/comps/tmdlHdmiTx/src
EXTRA_CFLAGS += -I$(obj)/comps/tmdlHdmiTx/cfg
EXTRA_CFLAGS += -I$(obj)/comps/tmdlHdmiTx/inc
EXTRA_CFLAGS += -I$(obj)/comps/tmbslTDA9989NoHdcp/inc
EXTRA_CFLAGS += -I$(obj)/comps/tmbslTDA9989NoHdcp/src/TDA9989n2
EXTRA_CFLAGS += -I$(obj)/comps/tmdlHdmiCEC/inc
EXTRA_CFLAGS += -I$(obj)/comps/tmdlHdmiCEC/src
EXTRA_CFLAGS += -I$(obj)/comps/tmdlHdmiCEC/cfg
endif

ifeq ($(TDA_TX),TDA9984)
EXTRA_CFLAGS += -I$(obj)/comps/tmbslHdmiTx/inc
EXTRA_CFLAGS += -I$(obj)/comps/tmdlHdmiTx/src
EXTRA_CFLAGS += -I$(obj)/comps/tmdlHdmiTx/cfg
EXTRA_CFLAGS += -I$(obj)/comps/tmdlHdmiTx/inc
EXTRA_CFLAGS += -I$(obj)/comps/tmbslTDA9984NoHdcp/inc
EXTRA_CFLAGS += -I$(obj)/comps/tmbslTDA9984NoHdcp/src
endif

ifeq ($(TDA_TX),TDA9983)
EXTRA_CFLAGS += -I$(obj)/comps/tmbslTDA9983/inc
EXTRA_CFLAGS += -I$(obj)/comps/tmdlTDA9983/inc
EXTRA_CFLAGS += -I$(obj)/comps/tmdlTDA9983/src
EXTRA_CFLAGS += -I$(obj)/comps/tmdlTDA9983/cfg
endif

ifeq ($(TDA_TX),TDA9981)
EXTRA_CFLAGS += -I$(obj)/comps/tmbslTDA9983/inc
EXTRA_CFLAGS += -I$(obj)/comps/tmdlTDA9983/inc
EXTRA_CFLAGS += -I$(obj)/comps/tmdlTDA9983/src
EXTRA_CFLAGS += -I$(obj)/comps/tmdlTDA9983/cfg
endif


