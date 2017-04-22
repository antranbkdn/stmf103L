/**
 ******************************************************************************
 * @Author: ThanNT
 * @Date:   13/08/2016
 ******************************************************************************
**/

#ifndef APP_H
#define APP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "app_if.h"
#include "app_eeprom.h"

/*****************************************************************************/
/*  life task define
 */
/*****************************************************************************/
/* define timer */
#define AC_LIFE_TASK_TIMER_LED_LIFE_INTERVAL		(1000)

/* define signal */
#define AC_LIFE_SYSTEM_CHECK						(0)

/*****************************************************************************/
/*  shell task define
 */
/*****************************************************************************/
/* define timer */

/* define signal */
#define AC_SHELL_LOGIN_CMD							(0)
#define AC_SHELL_REMOTE_CMD							(1)

/*****************************************************************************/
/*  rf24 task define
 */
/*****************************************************************************/
/* define timer */
#define AC_RF24_IF_TIMER_PACKET_DELAY_INTERVAL		(100) /* 100 ms */

/* define signal */
/* interrupt signal */
#define AC_RF24_IF_IRQ_TX_FAIL						(1)
#define AC_RF24_IF_IRQ_TX_SUCCESS					(2)
#define AC_RF24_IF_IRQ_RX_READY						(3)

/* task signal */
#define AC_RF24_IF_INIT_NETWORK						(4)
#define AC_RF24_IF_PURE_MSG_OUT						(5)
#define AC_RF24_IF_COMMON_MSG_OUT					(6)
#define AC_RF24_IF_TIMER_PACKET_DELAY				(7)

/*****************************************************************************/
/*  time task define
 */
/*****************************************************************************/
/* define timer */
#define AC_RTC_UPDATE_TIME_INTERVAL					(120000)	/* 2' */
#define AC_RTC_INTINAL_INTERVAL						(100)		/* 100ms */

/* define signal */
#define AC_TIME_RTC_INTINAL							(1)
#define AC_TIME_RTC_UPDATE_STATUS					(2)
#define AC_TIME_STATUS_RES_OK						(3)
#define AC_TIME_STATUS_RES_ERR						(4)
#define AC_TIME_RTC_SETTING_REQ						(5)
#define AC_TIME_AC_SENSOR_TEMPERATURE_STATUS_REQ	(6)

/* private define */
#define AC_RTC_STICK_TIME_MIN						(AC_RTC_UPDATE_TIME_INTERVAL / 60000)

/*****************************************************************************/
/*  sm task define
 */
/*****************************************************************************/
/* define timer */
#define AC_SM_TIMEOUT_RETRY_SEND_IR_INTERVAL		(15000)		/* 15s */

/* define signal */
#define AC_SM_TIME_PROCESS_UPDATE_REQ				(1)
#define AC_SM_SENSOR_AIR_COND_STATUS_RES			(2)
#define AC_SM_SENSOR_TEMPERATURE_STATUS_RES			(3)
#define AC_SM_TIMEOUT_RETRY_SEND_IR					(4)
#define AC_SM_FIRMWARE_UPDATE_REQ					(5)
#define AC_SM_FIRMWARE_RELEASE_REQ					(6)
#define AC_SM_SAFE_MODE_REQ							(7)
#define AC_SM_UI_SETTING_START						(8)
#define AC_SM_UI_SETTING_END						(9)

/*****************************************************************************/
/*  ir task define
 */
/*****************************************************************************/
/* define timer */
#define AC_AC_IR_TIMER_AUTO_TEST_INTERVAL			(10000)
#define AC_UI_TIMER_AUTO_VIEW_INTERVAL				(2000)

/* define signal */
#define AC_IR_AIR_COND_STATUS_REQ					(1)
#define AC_IR_AIR_COND_ON_DECODE_REQ				(2)
#define AC_IR_AIR_COND_OFF_DECODE_REQ				(3)
#define AC_IR_AIR_COND_EXIT_DECODE_REQ				(4)
#define AC_IR_AIR_COND_ON_DECODE_RES				(5)
#define AC_IR_AIR_COND_OFF_DECODE_RES				(6)
#define AC_IR_AIR_COND_START_DECODE_REQ				(7)
#define AC_IR_AIR_COND_STOP_DECODE_REQ				(8)
#define AC_IR_AIR_COND_AUTO_TEST_KEY_REQ			(9)
#define AC_IR_AIR_COND_AUTO_CALIB_START_REQ			(10)
#define AC_IR_AIR_COND_AUTO_CALIB_END_REQ			(11)
#define AC_IR_AIR_COND_HAND_TEST_KEY_REQ			(12)
#define AC_IR_TIMER_AUTO_TEST						(13)

#define AIR_COND_NO_CHANGE							(0x00)
#define AIR_COND_ON									(0x01)
#define AIR_COND_OFF								(0x02)

/* error code define */
#define ERROR_CODE_CTRL_AIR_COND_ON_FAIL			(0x01)
#define ERROR_CODE_CTRL_AIR_COND_OFF_FAIL			(0x02)

/*****************************************************************************/
/*  sensor task define
 */
/*****************************************************************************/
/* define timer */
/* define signal */
#define AC_SENSOR_REPORT_REQ						(1)
#define AC_SENSOR_AIR_COND_STATUS_REQ				(2)
#define AC_SENSOR_TEMPERATURE_STATUS_REQ			(3)

/*****************************************************************************/
/* ui task define
 */
/*****************************************************************************/
/* define timer */
#define AC_UI_TIMER_DISPLAY_OFF_INTERVAL			(120000)	/* 2' */
#define AC_UI_TIMER_TIME_UPDATE_INTERVAL			(10000)		/* 10s */
#define AC_UI_TIMER_TIMEOUT_INTERVAL				(30000)		/* 30s */

/* define signal */
#define AC_UI_INITIAL								(1)
#define AC_UI_SCREEN_HOME_UPDATE					(2)
#define AC_UI_BUTTON_MODE_PRESS						(3)
#define AC_UI_BUTTON_MODE_SHORT_HOLD				(4)
#define AC_UI_BUTTON_MODE_LONG_HOLD					(5)
#define AC_UI_BUTTON_UP_PRESS						(6)
#define AC_UI_BUTTON_DOWN_PRESS						(7)
#define AC_UI_KEY_ON_DECODE_OK						(8)
#define AC_UI_KEY_ON_DECODE_FAIL					(9)
#define AC_UI_KEY_OFF_DECODE_OK						(10)
#define AC_UI_KEY_OFF_DECODE_FAIL					(11)
#define AC_UI_KEY_ON_SAVING							(12)
#define AC_UI_KEY_OFF_SAVING						(13)
#define AC_UI_TEST_KEY_AUTO_END						(14)
#define AC_UI_VIEW_CURRENT_UPDATE					(15)
#define AC_UI_CALIB_CURRENT_AIR_ON					(16)
#define AC_UI_CALIB_CURRENT_AIR_OFF					(17)
#define AC_UI_CALIB_CURRENT_AIR_STOP				(18)
#define AC_UI_CALIB_CURRENT_AIR_RETURN				(19)
#define AC_UI_TIMEOUT								(20)
#define AC_UI_GOTO_HOME								(21)
#define AC_UI_TIMER_TIMEOUT							(22)
#define AC_UI_SETTING_SM_BUSY						(23)
#define AC_UI_SETTING_SM_OK							(24)
#define AC_UI_FIRMWARE_UPDATE_OK					(25)
#define AC_UI_BUSY_GOTO_HOME						(26)
#define AC_UI_FIRMWARE_GOTO_HOME					(27)
#define AC_UI_DISPLAY_OFF							(28)

/*****************************************************************************/
/* setting task define
 */
/*****************************************************************************/
/* define timer */
/* define signal */
#define AC_SETTING_APP_UPDATE						(1)
#define AC_SETTING_TOTAL_AIR_COND_UPDATE			(2)
#define AC_SETTING_TEMP_AIR_COND_UPDATE				(3)
#define AC_SETTING_TIME_RTC_UPDATE					(4)
#define AC_SETTING_RANGE_TIME_AIR_COND_UPDATE		(5)
#define AC_SETTING_MILESTONE_AIR_COND_ON_UPDATE		(6)
#define AC_SETTING_CALIBRATION_TEMP_HUM_UPDATE		(7)
#define AC_SETTING_EEPROM_INIT_UPDATE				(8)

/*****************************************************************************/
/* if task define
 */
/*****************************************************************************/
/* define timer */
#define AC_IF_TIMER_PACKET_TIMEOUT_INTERVAL			(500)

/* define signal */
#define AC_IF_PURE_MSG_IN							(1)
#define AC_IF_PURE_MSG_OUT							(2)
#define AC_IF_PURE_MSG_OUT_RES_OK					(3)
#define AC_IF_PURE_MSG_OUT_RES_NG					(4)
#define AC_IF_COMMON_MSG_IN							(5)
#define AC_IF_COMMON_MSG_OUT						(6)
#define AC_IF_COMMON_MSG_OUT_RES_OK					(7)
#define AC_IF_COMMON_MSG_OUT_RES_NG					(8)
#define AC_IF_PACKET_TIMEOUT						(9)

/*****************************************************************************/
/* firmware task define
 */
/*****************************************************************************/
/* define timer */
#define AC_TIMER_FIRMWARE_PACKED_TIMEOUT_INTERVAL	(5000)
#define AC_TIMER_FIRMWARE_CHECKING_INTERVAL			(1000)

/* define signal */
#define AC_FIRMWARE_CURRENT_INFO_REQ				(1)
#define AC_FIRMWARE_UPDATE_REQ						(2)
#define AC_FIRMWARE_UPDATE_SM_OK					(3)
#define AC_FIRMWARE_TRANSFER_REQ					(4)
#define AC_FIRMWARE_INTERNAL_UPDATE_RES_OK			(5)
#define AC_FIRMWARE_SAFE_MODE_RES_OK				(6)
#define AC_FIRMWARE_UPDATE_SM_BUSY					(7)
#define AC_FIRMWARE_PACKED_TIMEOUT					(8)
#define AC_FIRMWARE_CHECKING_REQ					(9)

/*****************************************************************************/
/*  global define variable
 */
/*****************************************************************************/
#define APP_OK									(0x00)
#define APP_NG									(0x01)

#define APP_FLAG_OFF							(0x00)
#define APP_FLAG_ON								(0x01)

/*****************************************************************************/
/*  app function declare
 */
/*****************************************************************************/
#define AC_NUMBER_SAMPLE_CT_SENSOR				3000

extern int  main_app();

#ifdef __cplusplus
}
#endif

#endif //APP_H
