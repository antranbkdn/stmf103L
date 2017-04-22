#ifndef __APP_DATA_H__
#define __APP_DATA_H__
#include <stdint.h>
#include "app.h"

/******************************************************************************
* Data type of RF24Network
*******************************************************************************/
#define RF24_DATA_COMMON_MSG_TYPE			(1)
#define RF24_DATA_PURE_MSG_TYPE				(2)
#define RF24_DATA_REMOTE_CMD_TYPE			(3)

/******************************************************************************
* Commom data structure for transceiver data
*******************************************************************************/
#define FIRMWARE_PSK		0x1A2B3C4D
#define FIRMWARE_LOK		0x1234ABCD

typedef struct {
	uint32_t psk;
	uint32_t bin_len;
	uint16_t checksum;
} firmware_header_t;

#endif //__APP_DATA_H__
