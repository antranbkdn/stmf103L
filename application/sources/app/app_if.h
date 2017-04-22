/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   30/1/2017
 * @brief:  app external signal define
 ******************************************************************************
**/
#ifndef __APP_IF_H__
#define __APP_IF_H__
#include <stdint.h>

/*****************************************************************************/
/*  getway if_rf24 define
 */
/*****************************************************************************/
/* define timer */

/* define signal */
#define GW_IF_RF24_SEND_REQ									(0x01)

/*****************************************************************************/
/*  getway task rf24 define
 */
/*****************************************************************************/
/* define timer */
#define GW_RF24_IF_TIMER_PACKET_DELAY_INTERVAL		(100)

/* define signal */
#define GW_RF24_IF_PURE_MSG_OUT						(1)
#define GW_RF24_IF_COMMON_MSG_OUT					(2)
#define GW_RF24_IF_TIMER_PACKET_DELAY				(3)

/*****************************************************************************/
/*  getway task console define
 */
/*****************************************************************************/
/* define timer */

/* define signal */
#define GW_CONSOLE_INTERNAL_LOGIN_CMD						(0x01)
#define GW_CONSOLE_AC_LOGIN_CMD								(0x02)

/*****************************************************************************/
/*  getway task rf433 define
 */
/*****************************************************************************/
/* define timer */
#define GW_RF433_TIMER_SMOKE_WARNNING_CHECK_INTERVAL		(20 * 60000)	/* 20' */
#define GW_RF433_TIMER_WATER_WARNNING_CHECK_INTERVAL		(20 * 60000)	/* 20' */

/* define signal */
#define GW_RF433_ENTRY_RECORD_CODE_REQ						(0x01)
#define GW_RF433_EXIT_RECORD_CODE_REQ						(0x02)

#define GW_RF433_RECORD_DOOR_OPENED_CODE_REQ				(0x03)
#define GW_RF433_RECORD_DOOR_CLOSED_CODE_REQ				(0x04)
#define GW_RF433_RECORD_SMOKE_WARNNING_CODE_REQ				(0x05)
#define GW_RF433_RECORD_WATER_WARNNING_CODE_REQ				(0x06)

#define GW_RF433_RECV_NOTIFY								(0x07)
#define GW_RF433_SMOKE_TIMER_WARNNING						(0x08)
#define GW_RF433_WATER_TIMER_WARNNING						(0x09)

/*****************************************************************************/
/*  getway task mqtt define
 */
/*****************************************************************************/
/* define timer */

/* define signal */
#define GW_MQTT_SENSOR_RES								(0x01)
#define GW_MQTT_SETTING_REQ								(0x02)
#define GW_MQTT_SETTING_RES								(0x03)
#define GW_MQTT_CONTROL_REQ								(0x04)
#define GW_MQTT_CONTROL_RES								(0x05)

/*****************************************************************************/
/*  getway task firmware define
 */
/*****************************************************************************/
/* define timer */
#define GW_FIRMWARE_TRANFER_PACKET_PENDING_INTERVAL		(150)
#define GW_TIMER_FIRMWARE_PACKED_TIMEOUT_INTERVAL		(5000)

/* define signal */
#define GW_FIRMWARE_OTA_REQ								(1)
#define GW_FIRMWARE_CURRENT_INFO_RES					(2)
#define GW_FIRMWARE_UPDATE_RES_OK						(3)
#define GW_FIRMWARE_TRANFER_REQ							(4)
#define GW_FIRMWARE_TRANSFER_RES_OK						(5)
#define GW_FIRMWARE_TRANSFER_RES_CHECKSUM_ERR			(6)
#define GW_FIRMWARE_TRANSFER_CHECKSUM_ERR				(7)
#define GW_FIRMWARE_AC_INTERNAL_UPDATE_REQ				(8)
#define GW_FIRMWARE_UPDATE_BUSY							(9)
#define GW_FIRMWARE_PACKED_TIMEOUT						(10)
#define GW_FIRMWARE_UPDATE_COMPLETED					(11)

/*****************************************************************************/
/* getway if task define
 */
/*****************************************************************************/
/* define timer */
#define GW_IF_TIMER_PACKET_TIMEOUT_INTERVAL			(500)

/* define signal */
#define GW_IF_PURE_MSG_IN							(1)
#define GW_IF_PURE_MSG_OUT							(2)
#define GW_IF_PURE_MSG_OUT_RES_OK					(3)
#define GW_IF_PURE_MSG_OUT_RES_NG					(4)
#define GW_IF_COMMON_MSG_IN							(5)
#define GW_IF_COMMON_MSG_OUT						(6)
#define GW_IF_COMMON_MSG_OUT_RES_OK					(7)
#define GW_IF_COMMON_MSG_OUT_RES_NG					(8)
#define GW_IF_PACKET_TIMEOUT						(9)

/*****************************************************************************/
/* getway sensor task define
 */
/*****************************************************************************/
/* define timer */
#define GW_SENSOR_AC_TIMER_INTERVAL					(30000)		/* 30s */

/* define signal */
#define GW_AC_SENSOR_REQ							(1)
#define GW_AC_SENSOR_RES							(2)

/*****************************************************************************/
/* snmp task define
 */
/*****************************************************************************/
/* define timer */
/* define signal */
#define GW_SNMP_AC_SENSOR_REP						(1)
#define GW_SNMP_AC_SETTINGS_REP						(2)

#endif //__APP_IF_H__
