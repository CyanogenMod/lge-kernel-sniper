/**
 * Copyright (C) 2006 NXP N.V., All Rights Reserved.
 * This source code and any compilation or derivative thereof is the proprietary
 * information of NXP N.V. and is confidential in nature. Under no circumstances
 * is this software to be  exposed to or placed under an Open Source License of
 * any type without the expressed written permission of NXP N.V.
 *
 * Version       Revision: 1.0
 *
 * Date          Date: 27/10/09
 *
 * Brief         API for the TDA1998x HDMI Transmitters
 *
 **/

#include <linux/types.h>


#ifndef __cec_ioctl__
#define __cec_ioctl__

//#ifdef __cec_h__

typedef struct {
   UInt8 DayOfMonth;
   UInt8 MonthOfYear;
   UInt16 StartTime;
   tmdlHdmiCECDuration_t Duration;
   UInt8 RecordingSequence;
   tmdlHdmiCECAnalogueBroadcastType_t AnalogueBroadcastType;
   UInt16 AnalogueFrequency;
   tmdlHdmiCECBroadcastSystem_t BroadcastSystem;
} cec_analogue_timer;

typedef struct {
   UInt8 DayOfMonth;
   UInt8 MonthOfYear;
   UInt16 StartTime;
   tmdlHdmiCECDuration_t Duration;
   UInt8 RecordingSequence;
   tmdlHdmiCECDigitalServiceIdentification_t ServiceIdentification;
} cec_digital_timer;

typedef struct {
   UInt8 DayOfMonth;
   UInt8 MonthOfYear;
   UInt16 StartTime;
   tmdlHdmiCECDuration_t Duration;
   UInt8 RecordingSequence;
   tmdlHdmiCECExternalPlug_t ExternalPlug;
} cec_ext_timer_with_ext_plug;

typedef struct {
   UInt8 DayOfMonth;
   UInt8 MonthOfYear;
   UInt16 StartTime;
   tmdlHdmiCECDuration_t Duration;
   UInt8 RecordingSequence;
   tmdlHdmiCECExternalPhysicalAddress_t ExternalPhysicalAddress;
} cec_ext_timer_with_phy_addr;

typedef struct {
   tmdlHdmiCECFeatureOpcode_t FeatureOpcode;
   tmdlHdmiCECAbortReason_t AbortReason;
} cec_feature_abort;

typedef struct {
   tmdlHdmiCECAnalogueBroadcastType_t AnalogueBroadcastType;
   UInt16 AnalogueFrequency;
   tmdlHdmiCECBroadcastSystem_t BroadcastSystem;
} cec_analogue_service;

typedef struct {
   UInt16 OriginalAddress;
   UInt16 NewAddress;
} cec_routing_change;

typedef struct {
   char data[15];
   unsigned char length;
} cec_string;

typedef struct {
   tmdlHdmiCECDisplayControl_t DisplayControl;
   char data[15];
   unsigned char length;
} cec_osd_string;

typedef struct {
   tmdlHdmiCECRecordingFlag_t RecordingFlag;
   tmdlHdmiCECTunerDisplayInfo_t TunerDisplayInfo;
   tmdlHdmiCECAnalogueBroadcastType_t AnalogueBroadcastType;
   UInt16 AnalogueFrequency;
   tmdlHdmiCECBroadcastSystem_t BroadcastSystem;
} cec_tuner_device_status_analogue;

typedef struct {
   tmdlHdmiCECRecordingFlag_t RecordingFlag;
   tmdlHdmiCECTunerDisplayInfo_t TunerDisplayInfo;
   tmdlHdmiCECDigitalServiceIdentification_t ServiceIdentification;
} cec_tuner_device_status_digital;

typedef struct {
   unsigned long VendorID;
   cec_string cmd;
} cec_vendor_command_with_id;

/*
  typedef struct {
  UInt8 *pData;
  UInt16 lenData;
  } cec_send_msg;
*/

typedef struct
{
   unsigned char count;
   unsigned char service;
   unsigned char addr;
   unsigned char data[15];
} cec_frame;
/* typedef tmdlHdmiCecFrameFormat_t cec_frame; */

typedef tmSWVersion_t cec_sw_version;
typedef tmPowerState_t cec_power;
typedef tmdlHdmiCecInstanceSetup_t cec_setup;
typedef tmdlHdmiCecEvent_t cec_event;
typedef tmdlHdmiCecClockSource_t cec_clock;
typedef tmdlHdmiCECSystemAudioStatus_t cec_sys_audio_status;
typedef tmdlHdmiCECAudioRate_t cec_audio_rate;
typedef tmdlHdmiCECDigitalServiceIdentification_t cec_digital_service;
typedef tmdlHdmiCECVersion_t cec_version;
typedef tmdlHdmiCECDecControlMode_t cec_deck_ctrl;
typedef tmdlHdmiCECDecInfo_t cec_deck_status;
typedef tmdlHdmiCECStatusRequest_t cec_status_request;
typedef tmdlHdmiCECMenuRequestType_t cec_menu_request;
typedef tmdlHdmiCECMenuState_t cec_menu_status;
typedef tmdlHdmiCECPlayMode_t cec_play;
typedef tmdlHdmiCECExternalPlug_t cec_ext_plug;
typedef tmdlHdmiCECRecordStatusInfo_t cec_rec_status;
typedef tmdlHdmiCECAudioStatus_t cec_audio_status;
typedef tmdlHdmiCECPowerStatus_t cec_power_status;
typedef tmdlHdmiCECTimerClearedStatusData_t cec_timer_cleared_status;
typedef tmdlHdmiCECTimerStatusData_t cec_timer_status;
typedef tmdlHdmiCECUserRemoteControlCommand_t cec_user_ctrl;
typedef tmdlHdmiCECChannelIdentifier_t cec_user_ctrl_tune;
typedef tmdlHdmiCECDeviceType_t cec_device_type;

#define CEC_IOCTL_BASE 0x40

/* service */
enum {
   CEC_WAITING = 0x80,
   CEC_RELEASE,
   CEC_RX_DONE,
   CEC_TX_DONE
};

enum {
   /* driver specific */
   CEC_VERBOSE_ON_CMD = 0,
   CEC_VERBOSE_OFF_CMD,
   CEC_BYEBYE_CMD,

   /* CEC */
   CEC_IOCTL_RX_ADDR_CMD, /* receiver logical address selector */
   CEC_IOCTL_PHY_ADDR_CMD, /* physical address selector */
   CEC_IOCTL_WAIT_FRAME_CMD,
   CEC_IOCTL_ABORT_MSG_CMD,
   CEC_IOCTL_ACTIVE_SRC_CMD,
   CEC_IOCTL_VERSION_CMD,
   CEC_IOCTL_CLEAR_ANALOGUE_TIMER_CMD,
   CEC_IOCTL_CLEAR_DIGITAL_TIMER_CMD,
   CEC_IOCTL_CLEAR_EXT_TIMER_WITH_EXT_PLUG_CMD,
   CEC_IOCTL_CLEAR_EXT_TIMER_WITH_PHY_ADDR_CMD,
   CEC_IOCTL_DECK_CTRL_CMD,
   CEC_IOCTL_DECK_STATUS_CMD,
   CEC_IOCTL_DEVICE_VENDOR_ID_CMD,
   CEC_IOCTL_FEATURE_ABORT_CMD,
   CEC_IOCTL_GET_CEC_VERSION_CMD,
   CEC_IOCTL_GET_MENU_LANGUAGE_CMD,
   CEC_IOCTL_GIVE_AUDIO_STATUS_CMD,
   CEC_IOCTL_GIVE_DECK_STATUS_CMD,
   CEC_IOCTL_GIVE_DEVICE_POWER_STATUS_CMD,
   CEC_IOCTL_GIVE_DEVICE_VENDOR_ID_CMD,
   CEC_IOCTL_GIVE_OSD_NAME_CMD,
   CEC_IOCTL_GIVE_PHY_ADDR_CMD,
   CEC_IOCTL_GIVE_SYS_AUDIO_MODE_STATUS_CMD,
   CEC_IOCTL_GIVE_TUNER_DEVICE_STATUS_CMD,
   CEC_IOCTL_IMAGE_VIEW_ON_CMD,
   CEC_IOCTL_INACTIVE_SRC_CMD,
   CEC_IOCTL_MENU_REQUEST_CMD,
   CEC_IOCTL_MENU_STATUS_CMD,
   CEC_IOCTL_PLAY_CMD,
   CEC_IOCTL_POLLING_MSG_CMD,
   CEC_IOCTL_REC_OFF_CMD,
   CEC_IOCTL_REC_ON_ANALOGUE_SERVICE_CMD,
   CEC_IOCTL_REC_ON_DIGITAL_SERVICE_CMD,
   CEC_IOCTL_REC_ON_EXT_PHY_ADDR_CMD,
   CEC_IOCTL_REC_ON_EXT_PLUG_CMD,
   CEC_IOCTL_REC_ON_OWN_SRC_CMD,
   CEC_IOCTL_REC_STATUS_CMD,
   CEC_IOCTL_REC_TV_SCREEN_CMD,
   CEC_IOCTL_REPORT_AUDIO_STATUS_CMD,
   CEC_IOCTL_REPORT_PHY_ADDR_CMD,
   CEC_IOCTL_REPORT_POWER_STATUS_CMD,
   CEC_IOCTL_REQUEST_ACTIVE_SRC_CMD,
   CEC_IOCTL_ROUTING_CHANGE_CMD,
   CEC_IOCTL_ROUTING_INFORMATION_CMD,
   CEC_IOCTL_SELECT_ANALOGUE_SERVICE_CMD,
   CEC_IOCTL_SELECT_DIGITAL_SERVICE_CMD,
   CEC_IOCTL_SET_ANALOGUE_TIMER_CMD,
   CEC_IOCTL_SET_AUDIO_RATE_CMD,
   CEC_IOCTL_SET_DIGITAL_TIMER_CMD,
   CEC_IOCTL_SET_EXT_TIMER_WITH_EXT_PLUG_CMD,
   CEC_IOCTL_SET_EXT_TIMER_WITH_PHY_ADDR_CMD,
   CEC_IOCTL_SET_MENU_LANGUAGE_CMD,
   CEC_IOCTL_SET_OSD_NAME_CMD,
   CEC_IOCTL_SET_OSD_STRING_CMD,
   CEC_IOCTL_SET_STREAM_PATH_CMD,
   CEC_IOCTL_SET_SYS_AUDIO_MODE_CMD,
   CEC_IOCTL_SET_TIMER_PROGRAM_TITLE_CMD,
   CEC_IOCTL_STANDBY_CMD,
   CEC_IOCTL_SYS_AUDIO_MODE_REQUEST_CMD,
   CEC_IOCTL_SYS_AUDIO_MODE_STATUS_CMD,
   CEC_IOCTL_TEXT_VIEW_ON_CMD,
   CEC_IOCTL_TIMER_CLEARED_STATUS_CMD,
   CEC_IOCTL_TIMER_STATUS_CMD,
   CEC_IOCTL_TUNER_DEVICE_STATUS_ANALOGUE_CMD,
   CEC_IOCTL_TUNER_DEVICE_STATUS_DIGITAL_CMD,
   CEC_IOCTL_TUNER_STEP_DECREMENT_CMD,
   CEC_IOCTL_TUNER_STEP_INCREMENT_CMD,
   CEC_IOCTL_USER_CTRL_CMD,
   CEC_IOCTL_USER_CTRL_PLAY_CMD,
   CEC_IOCTL_USER_CTRL_SELECT_AUDIOINPUT_CMD,
   CEC_IOCTL_USER_CTRL_SELECT_AVINPUT_CMD,
   CEC_IOCTL_USER_CTRL_SELECT_MEDIA_CMD,
   CEC_IOCTL_USER_CTRL_TUNE_CMD,
   CEC_IOCTL_USER_CTRL_RELEASED_CMD,
   CEC_IOCTL_VENDOR_COMMAND_CMD,
   CEC_IOCTL_VENDOR_COMMAND_WITH_ID_CMD,
   CEC_IOCTL_VENDOR_REMOTE_BUTTON_DOWN_CMD,
   CEC_IOCTL_VENDOR_REMOTE_BUTTON_UP_CMD,
   CEC_IOCTL_GET_SW_VERSION_CMD,
   CEC_IOCTL_SET_POWER_STATE_CMD,
   CEC_IOCTL_GET_POWER_STATE_CMD,
   CEC_IOCTL_INSTANCE_CONFIG_CMD,
   CEC_IOCTL_INSTANCE_SETUP_CMD,
   CEC_IOCTL_GET_INSTANCE_SETUP_CMD,
   CEC_IOCTL_ENABLE_EVENT_CMD,
   CEC_IOCTL_DISABLE_EVENT_CMD,
   CEC_IOCTL_ENABLE_CALIBRATION_CMD,
   CEC_IOCTL_DISABLE_CALIBRATION_CMD,
   CEC_IOCTL_SEND_MSG_CMD,
   CEC_IOCTL_SET_REGISTER_CMD
};


/* driver specific */
#define CEC_IOCTL_VERBOSE_ON       _IO(CEC_IOCTL_BASE, CEC_VERBOSE_ON_CMD)
#define CEC_IOCTL_VERBOSE_OFF      _IO(CEC_IOCTL_BASE, CEC_VERBOSE_OFF_CMD)
#define CEC_IOCTL_BYEBYE      _IO(CEC_IOCTL_BASE, CEC_BYEBYE_CMD)

/* CEC */
#define CEC_IOCTL_RX_ADDR      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_RX_ADDR_CMD,unsigned char)
#define CEC_IOCTL_PHY_ADDR      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_PHY_ADDR_CMD,unsigned short)
#define CEC_IOCTL_WAIT_FRAME      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_WAIT_FRAME_CMD,cec_frame)
#define CEC_IOCTL_ABORT_MSG      _IO(CEC_IOCTL_BASE,CEC_IOCTL_ABORT_MSG_CMD)
#define CEC_IOCTL_ACTIVE_SRC      _IO(CEC_IOCTL_BASE,CEC_IOCTL_ACTIVE_SRC_CMD)
#define CEC_IOCTL_VERSION      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_VERSION_CMD,cec_version)
#define CEC_IOCTL_CLEAR_ANALOGUE_TIMER      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_CLEAR_ANALOGUE_TIMER_CMD,cec_analogue_timer)
#define CEC_IOCTL_CLEAR_DIGITAL_TIMER      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_CLEAR_DIGITAL_TIMER_CMD,cec_digital_timer)
#define CEC_IOCTL_CLEAR_EXT_TIMER_WITH_EXT_PLUG      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_CLEAR_EXT_TIMER_WITH_EXT_PLUG_CMD,cec_ext_timer_with_ext_plug)
#define CEC_IOCTL_CLEAR_EXT_TIMER_WITH_PHY_ADDR      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_CLEAR_EXT_TIMER_WITH_PHY_ADDR_CMD,cec_ext_timer_with_phy_addr)
#define CEC_IOCTL_DECK_CTRL      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_DECK_CTRL_CMD,cec_deck_ctrl)
#define CEC_IOCTL_DECK_STATUS      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_DECK_STATUS_CMD,cec_deck_status)
#define CEC_IOCTL_DEVICE_VENDOR_ID      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_DEVICE_VENDOR_ID_CMD,unsigned long)
#define CEC_IOCTL_FEATURE_ABORT      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_FEATURE_ABORT_CMD,cec_feature_abort)
#define CEC_IOCTL_GET_CEC_VERSION      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_GET_CEC_VERSION_CMD,unsigned char)
#define CEC_IOCTL_GET_MENU_LANGUAGE      _IO(CEC_IOCTL_BASE,CEC_IOCTL_GET_MENU_LANGUAGE_CMD)
#define CEC_IOCTL_GIVE_AUDIO_STATUS      _IO(CEC_IOCTL_BASE,CEC_IOCTL_GIVE_AUDIO_STATUS_CMD)
#define CEC_IOCTL_GIVE_DECK_STATUS      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_GIVE_DECK_STATUS_CMD,cec_status_request)
#define CEC_IOCTL_GIVE_DEVICE_POWER_STATUS      _IO(CEC_IOCTL_BASE,CEC_IOCTL_GIVE_DEVICE_POWER_STATUS_CMD)
#define CEC_IOCTL_GIVE_DEVICE_VENDOR_ID      _IO(CEC_IOCTL_BASE,CEC_IOCTL_GIVE_DEVICE_VENDOR_ID_CMD)
#define CEC_IOCTL_GIVE_OSD_NAME      _IO(CEC_IOCTL_BASE,CEC_IOCTL_GIVE_OSD_NAME_CMD)
#define CEC_IOCTL_GIVE_PHY_ADDR      _IO(CEC_IOCTL_BASE,CEC_IOCTL_GIVE_PHY_ADDR_CMD)
#define CEC_IOCTL_GIVE_SYS_AUDIO_MODE_STATUS      _IO(CEC_IOCTL_BASE,CEC_IOCTL_GIVE_SYS_AUDIO_MODE_STATUS_CMD)
#define CEC_IOCTL_GIVE_TUNER_DEVICE_STATUS      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_GIVE_TUNER_DEVICE_STATUS_CMD,cec_status_request)
#define CEC_IOCTL_IMAGE_VIEW_ON      _IO(CEC_IOCTL_BASE,CEC_IOCTL_IMAGE_VIEW_ON_CMD)
#define CEC_IOCTL_INACTIVE_SRC      _IO(CEC_IOCTL_BASE,CEC_IOCTL_INACTIVE_SRC_CMD)
#define CEC_IOCTL_MENU_REQUEST      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_MENU_REQUEST_CMD,cec_menu_request)
#define CEC_IOCTL_MENU_STATUS      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_MENU_STATUS_CMD,cec_menu_status)
#define CEC_IOCTL_PLAY      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_PLAY_CMD,cec_play)
#define CEC_IOCTL_POLLING_MSG      _IO(CEC_IOCTL_BASE,CEC_IOCTL_POLLING_MSG_CMD)
#define CEC_IOCTL_REC_OFF      _IO(CEC_IOCTL_BASE,CEC_IOCTL_REC_OFF_CMD)
#define CEC_IOCTL_REC_ON_ANALOGUE_SERVICE      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_REC_ON_ANALOGUE_SERVICE_CMD,cec_analogue_service)
#define CEC_IOCTL_REC_ON_DIGITAL_SERVICE      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_REC_ON_DIGITAL_SERVICE_CMD,cec_digital_service)
#define CEC_IOCTL_REC_ON_EXT_PHY_ADDR      _IO(CEC_IOCTL_BASE,CEC_IOCTL_REC_ON_EXT_PHY_ADDR_CMD)
#define CEC_IOCTL_REC_ON_EXT_PLUG      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_REC_ON_EXT_PLUG_CMD,cec_ext_plug)
#define CEC_IOCTL_REC_ON_OWN_SRC      _IO(CEC_IOCTL_BASE,CEC_IOCTL_REC_ON_OWN_SRC_CMD)
#define CEC_IOCTL_REC_STATUS      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_REC_STATUS_CMD,cec_rec_status)
#define CEC_IOCTL_REC_TV_SCREEN      _IO(CEC_IOCTL_BASE,CEC_IOCTL_REC_TV_SCREEN_CMD)
#define CEC_IOCTL_REPORT_AUDIO_STATUS      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_REPORT_AUDIO_STATUS_CMD,cec_audio_status)
#define CEC_IOCTL_REPORT_PHY_ADDR      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_REPORT_PHY_ADDR_CMD,cec_device_type)
#define CEC_IOCTL_REPORT_POWER_STATUS      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_REPORT_POWER_STATUS_CMD,cec_power_status)
#define CEC_IOCTL_REQUEST_ACTIVE_SRC      _IO(CEC_IOCTL_BASE,CEC_IOCTL_REQUEST_ACTIVE_SRC_CMD)
#define CEC_IOCTL_ROUTING_CHANGE      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_ROUTING_CHANGE_CMD,cec_routing_change)
#define CEC_IOCTL_ROUTING_INFORMATION      _IO(CEC_IOCTL_BASE,CEC_IOCTL_ROUTING_INFORMATION_CMD)
#define CEC_IOCTL_SELECT_ANALOGUE_SERVICE      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_SELECT_ANALOGUE_SERVICE_CMD,cec_analogue_service)
#define CEC_IOCTL_SELECT_DIGITAL_SERVICE      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_SELECT_DIGITAL_SERVICE_CMD,cec_digital_service)
#define CEC_IOCTL_SET_ANALOGUE_TIMER      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_SET_ANALOGUE_TIMER_CMD,cec_analogue_timer)
#define CEC_IOCTL_SET_AUDIO_RATE      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_SET_AUDIO_RATE_CMD,cec_audio_rate)
#define CEC_IOCTL_SET_DIGITAL_TIMER      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_SET_DIGITAL_TIMER_CMD,cec_digital_timer)
#define CEC_IOCTL_SET_EXT_TIMER_WITH_EXT_PLUG      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_SET_EXT_TIMER_WITH_EXT_PLUG_CMD,cec_ext_timer_with_ext_plug)
#define CEC_IOCTL_SET_EXT_TIMER_WITH_PHY_ADDR      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_SET_EXT_TIMER_WITH_PHY_ADDR_CMD,cec_ext_timer_with_phy_addr)
#define CEC_IOCTL_SET_MENU_LANGUAGE      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_SET_MENU_LANGUAGE_CMD,cec_string)
#define CEC_IOCTL_SET_OSD_NAME      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_SET_OSD_NAME_CMD,cec_string)
#define CEC_IOCTL_SET_OSD_STRING      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_SET_OSD_STRING_CMD,cec_osd_string)
#define CEC_IOCTL_SET_STREAM_PATH      _IO(CEC_IOCTL_BASE,CEC_IOCTL_SET_STREAM_PATH_CMD)
#define CEC_IOCTL_SET_SYS_AUDIO_MODE      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_SET_SYS_AUDIO_MODE_CMD,cec_sys_audio_status)
#define CEC_IOCTL_SET_TIMER_PROGRAM_TITLE      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_SET_TIMER_PROGRAM_TITLE_CMD,cec_string)
#define CEC_IOCTL_STANDBY      _IO(CEC_IOCTL_BASE,CEC_IOCTL_STANDBY_CMD)
#define CEC_IOCTL_SYS_AUDIO_MODE_REQUEST      _IO(CEC_IOCTL_BASE,CEC_IOCTL_SYS_AUDIO_MODE_REQUEST_CMD)
#define CEC_IOCTL_SYS_AUDIO_MODE_STATUS      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_SYS_AUDIO_MODE_STATUS_CMD,cec_sys_audio_status)
#define CEC_IOCTL_TEXT_VIEW_ON      _IO(CEC_IOCTL_BASE,CEC_IOCTL_TEXT_VIEW_ON_CMD)
#define CEC_IOCTL_TIMER_CLEARED_STATUS      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_TIMER_CLEARED_STATUS_CMD,cec_timer_cleared_status)
#define CEC_IOCTL_TIMER_STATUS      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_TIMER_STATUS_CMD,cec_timer_status)
#define CEC_IOCTL_TUNER_DEVICE_STATUS_ANALOGUE      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_TUNER_DEVICE_STATUS_ANALOGUE_CMD,cec_tuner_device_status_analogue)
#define CEC_IOCTL_TUNER_DEVICE_STATUS_DIGITAL      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_TUNER_DEVICE_STATUS_DIGITAL_CMD,cec_tuner_device_status_digital)
#define CEC_IOCTL_TUNER_STEP_DECREMENT      _IO(CEC_IOCTL_BASE,CEC_IOCTL_TUNER_STEP_DECREMENT_CMD)
#define CEC_IOCTL_TUNER_STEP_INCREMENT      _IO(CEC_IOCTL_BASE,CEC_IOCTL_TUNER_STEP_INCREMENT_CMD)
#define CEC_IOCTL_USER_CTRL_PRESSED      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_USER_CTRL_CMD,cec_user_ctrl)
#define CEC_IOCTL_USER_CTRL_PLAY      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_USER_CTRL_PLAY_CMD,cec_play)
#define CEC_IOCTL_USER_CTRL_SELECT_AUDIOINPUT      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_USER_CTRL_SELECT_AUDIOINPUT_CMD,unsigned char)
#define CEC_IOCTL_USER_CTRL_SELECT_AVINPUT      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_USER_CTRL_SELECT_AVINPUT_CMD,unsigned char)
#define CEC_IOCTL_USER_CTRL_SELECT_MEDIA      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_USER_CTRL_SELECT_MEDIA_CMD,unsigned char)
#define CEC_IOCTL_USER_CTRL_TUNE      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_USER_CTRL_TUNE_CMD,cec_user_ctrl_tune)
#define CEC_IOCTL_USER_CTRL_RELEASED      _IO(CEC_IOCTL_BASE,CEC_IOCTL_USER_CTRL_RELEASED_CMD)
#define CEC_IOCTL_VENDOR_COMMAND      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_VENDOR_COMMAND_CMD,cec_string)
#define CEC_IOCTL_VENDOR_COMMAND_WITH_ID      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_VENDOR_COMMAND_WITH_ID_CMD,cec_vendor_command_with_id)
#define CEC_IOCTL_VENDOR_REMOTE_BUTTON_DOWN      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_VENDOR_REMOTE_BUTTON_DOWN_CMD,cec_string)
#define CEC_IOCTL_VENDOR_REMOTE_BUTTON_UP      _IO(CEC_IOCTL_BASE,CEC_IOCTL_VENDOR_REMOTE_BUTTON_UP_CMD)
#define CEC_IOCTL_GET_SW_VERSION      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_GET_SW_VERSION_CMD,cec_sw_version)
#define CEC_IOCTL_SET_POWER_STATE      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_SET_POWER_STATE_CMD,cec_power)
#define CEC_IOCTL_GET_POWER_STATE      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_GET_POWER_STATE_CMD,cec_power)
#define CEC_IOCTL_INSTANCE_CONFIG      _IO(CEC_IOCTL_BASE,CEC_IOCTL_INSTANCE_CONFIG_CMD)
#define CEC_IOCTL_INSTANCE_SETUP      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_INSTANCE_SETUP_CMD,cec_setup)
#define CEC_IOCTL_GET_INSTANCE_SETUP      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_GET_INSTANCE_SETUP_CMD,cec_setup)
#define CEC_IOCTL_ENABLE_EVENT      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_ENABLE_EVENT_CMD,cec_event)
#define CEC_IOCTL_DISABLE_EVENT      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_DISABLE_EVENT_CMD,cec_event)
#define CEC_IOCTL_ENABLE_CALIBRATION      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_ENABLE_CALIBRATION_CMD,cec_clock)
#define CEC_IOCTL_DISABLE_CALIBRATION      _IO(CEC_IOCTL_BASE,CEC_IOCTL_DISABLE_CALIBRATION_CMD)
//#define CEC_IOCTL_SEND_MSG      _IOWR(CEC_IOCTL_BASE,CEC_IOCTL_SEND_MSG_CMD,cec_send_msg)

/* --- Full list --- */

/* legend: */
/* ------- */
/* [ ] : not supported */
/* [x] : IOCTL */
/* [i] : open, init... */

/* [ ] tmdlHdmiCecAbortMessage */
/* [ ] tmdlHdmiCecActiveSource */
/* [ ] tmdlHdmiCecVersion */
/* [ ] tmdlHdmiCecClearAnalogueTimer */
/* [ ] tmdlHdmiCecClearDigitalTimer */
/* [ ] tmdlHdmiCecClearExternalTimerWithExternalPlug */
/* [ ] tmdlHdmiCecClearExternalTimerWithPhysicalAddress */
/* [ ] tmdlHdmiCecDeckControl */
/* [ ] tmdlHdmiCecDeckStatus */
/* [ ] tmdlHdmiCecDeviceVendorID */
/* [ ] tmdlHdmiCecFeatureAbort */
/* [ ] tmdlHdmiCecGetCecVersion */
/* [ ] tmdlHdmiCecGetMenuLanguage */
/* [ ] tmdlHdmiCecGiveAudioStatus */
/* [ ] tmdlHdmiCecGiveDeckStatus */
/* [ ] tmdlHdmiCecGiveDevicePowerStatus */
/* [ ] tmdlHdmiCecGiveDeviceVendorID */
/* [ ] tmdlHdmiCecGiveOsdName */
/* [ ] tmdlHdmiCecGivePhysicalAddress */
/* [ ] tmdlHdmiCecGiveSystemAudioModeStatus */
/* [ ] tmdlHdmiCecGiveTunerDeviceStatus */
/* [ ] tmdlHdmiCecImageViewOn */
/* [ ] tmdlHdmiCecInactiveSource */
/* [ ] tmdlHdmiCecMenuRequest */
/* [ ] tmdlHdmiCecMenuStatus */
/* [ ] tmdlHdmiCecPlay */
/* [ ] tmdlHdmiCecPollingMessage */
/* [ ] tmdlHdmiCecRecordOff */
/* [ ] tmdlHdmiCecRecordOnAnalogueService */
/* [ ] tmdlHdmiCecRecordOnDigitalService */
/* [ ] tmdlHdmiCecRecordOnExternalPhysicalAddress */
/* [ ] tmdlHdmiCecRecordOnExternalPlug */
/* [ ] tmdlHdmiCecRecordOnOwnSource */
/* [ ] tmdlHdmiCecRecordStatus */
/* [ ] tmdlHdmiCecRecordTvScreen */
/* [ ] tmdlHdmiCecReportAudioStatus */
/* [ ] tmdlHdmiCecReportPhysicalAddress */
/* [ ] tmdlHdmiCecReportPowerStatus */
/* [ ] tmdlHdmiCecRequestActiveSource */
/* [ ] tmdlHdmiCecRoutingChange */
/* [ ] tmdlHdmiCecRoutingInformation */
/* [ ] tmdlHdmiCecSelectAnalogueService */
/* [ ] tmdlHdmiCecSelectDigitalService */
/* [ ] tmdlHdmiCecSetAnalogueTimer */
/* [ ] tmdlHdmiCecSetAudioRate */
/* [ ] tmdlHdmiCecSetDigitalTimer */
/* [ ] tmdlHdmiCecSetExternalTimerWithExternalPlug */
/* [ ] tmdlHdmiCecSetExternalTimerWithPhysicalAddress */
/* [ ] tmdlHdmiCecSetMenuLanguage */
/* [ ] tmdlHdmiCecSetOsdName */
/* [ ] tmdlHdmiCecSetOsdString */
/* [ ] tmdlHdmiCecSetStreamPath */
/* [ ] tmdlHdmiCecSetSystemAudioMode */
/* [ ] tmdlHdmiCecSetTimerProgramTitle */
/* [ ] tmdlHdmiCecStandby */
/* [ ] tmdlHdmiCecSystemAudioModeRequest */
/* [ ] tmdlHdmiCecSystemAudioModeStatus */
/* [ ] tmdlHdmiCecTextViewOn */
/* [ ] tmdlHdmiCecTimerClearedStatus */
/* [ ] tmdlHdmiCecTimerStatus */
/* [ ] tmdlHdmiCecTunerDeviceStatusAnalogue */
/* [ ] tmdlHdmiCecTunerDeviceStatusDigital */
/* [ ] tmdlHdmiCecTunerStepDecrement */
/* [ ] tmdlHdmiCecTunerStepIncrement */
/* [ ] tmdlHdmiCecUserControlPressed */
/* [ ] tmdlHdmiCecUserControlPressedPlay */
/* [ ] tmdlHdmiCecUserControlPressedSelectAudioInput */
/* [ ] tmdlHdmiCecUserControlPressedSelectAVInput */
/* [ ] tmdlHdmiCecUserControlPressedSelectMedia */
/* [ ] tmdlHdmiCecUserControlPressedTune */
/* [ ] tmdlHdmiCecUserControlReleased */
/* [ ] tmdlHdmiCecVendorCommand */
/* [ ] tmdlHdmiCecVendorCommandWithID */
/* [ ] tmdlHdmiCecVendorRemoteButtonDown */
/* [ ] tmdlHdmiCecVendorRemoteButtonUp */
/* [ ] tmdlHdmiCecGetSWVersion */
/* [ ] tmdlHdmiCecGetNumberOfUnits */
/* [ ] tmdlHdmiCecGetCapabilities */
/* [ ] tmdlHdmiCecGetCapabilitiesM */
/* [ ] tmdlHdmiCecOpen */
/* [ ] tmdlHdmiCecOpenM */
/* [ ] tmdlHdmiCecClose */
/* [ ] tmdlHdmiCecSetPowerState */
/* [ ] tmdlHdmiCecGetPowerState */
/* [ ] tmdlHdmiCecInstanceConfig */
/* [ ] tmdlHdmiCecInstanceSetup */
/* [ ] tmdlHdmiCecGetInstanceSetup */
/* [ ] tmdlHdmiCecHandleInterrupt */
/* [ ] tmdlHdmiCecRegisterCallbacks */
/* [ ] tmdlHdmiCecSetAutoAnswer */
/* [ ] tmdlHdmiCecSetLogicalAddress */
/* [ ] tmdlHdmiCecEnableEvent */
/* [ ] tmdlHdmiCecDisableEvent */
/* [ ] tmdlHdmiCecEnableCalibration */
/* [ ] tmdlHdmiCecDisableCalibration */
/* [ ] tmdlHdmiCecSendMessage */
/* [ ] tmdlHdmiCecSetRegister */


//#endif /* __cec_h__ */
#endif /* __cec_ioctl__ */
