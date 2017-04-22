#ifndef __TASK_GW_IF_H__
#define __TASK_GW_IF_H__

#include "../common/cmd_line.h"

#include "../rf_protocols/RF24/RF24.h"
#include "../rf_protocols/RF24Network/RF24Network.h"
#include "../rf_protocols/RF24Network/RF24Network_config.h"

#define RF24_BUFFER_SIZE		128

extern RF24 nrf24l01;
extern RF24Network local_network;

#endif // __TASK_GW_IF_H__
