#include <stdbool.h>
#include <stdint.h>

#include "app.h"
#include "app_dbg.h"
#include "app_data.h"

#include "task_gw_if.h"
#include "task_if.h"
#include "task_list.h"
#include "task_sensor.h"

#include "../ak/fsm.h"
#include "../ak/port.h"
#include "../ak/message.h"
#include "../ak/timer.h"

#include "../sys/sys_dbg.h"
#include "../sys/sys_irq.h"
#include "../sys/sys_io.h"

#include "../common/utils.h"
#include "../common/fifo.h"

#include "../driver/eeprom/eeprom.h"

#define SEND_FIFO_BUFFER_SIZE			2

RF24 nrf24l01(1, 2);
RF24Network local_network(nrf24l01);

const uint16_t me_node_channel = 90;			/* node channel */
const uint16_t me_node_address = 01;			/* node address */
const uint16_t server_node_address = 00;		/* server node address */

static uint8_t rf24_buffer[RF24_BUFFER_SIZE];

static ak_msg_common_if_t send_common_fifo_buffer[SEND_FIFO_BUFFER_SIZE];
static fifo_t send_common_fifo;

static ak_msg_pure_if_t send_pure_fifo_buffer[SEND_FIFO_BUFFER_SIZE];
static fifo_t send_pure_fifo;

static uint8_t rf_pending_flag = APP_FLAG_OFF;

void sys_irq_nrf24l01() {
	bool tx_ok, tx_fail, rx_ready;
	nrf24l01.whatHappened(tx_ok, tx_fail, rx_ready);

	if (tx_ok) {
		ak_msg_t* msg = get_pure_msg();
		set_msg_sig(msg, AC_RF24_IF_IRQ_TX_SUCCESS);
		task_post(AC_TASK_GW_IF_ID, msg);
	}

	if (tx_fail) {
		ak_msg_t* msg = get_pure_msg();
		set_msg_sig(msg, AC_RF24_IF_IRQ_TX_FAIL);
		task_post(AC_TASK_GW_IF_ID, msg);
	}

	if (rx_ready) {
		ak_msg_t* msg = get_pure_msg();
		set_msg_sig(msg, AC_RF24_IF_IRQ_RX_READY);
		task_post(AC_TASK_GW_IF_ID, msg);
	}
}

void task_gw_if(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_RF24_IF_IRQ_RX_READY: {
		local_network.update();

		mem_set(rf24_buffer, 0, RF24_BUFFER_SIZE);
		RF24NetworkHeader rf24_rev_header;
		local_network.read(rf24_rev_header, rf24_buffer, RF24_BUFFER_SIZE);

		switch (rf24_rev_header.type) {

		case RF24_DATA_COMMON_MSG_TYPE: {
			APP_DBG("RF24_DATA_COMMON_MSG_TYPE\n");

			ak_msg_common_if_t* if_msg = (ak_msg_common_if_t*)rf24_buffer;

			ak_msg_t* s_msg = get_common_msg();

			set_if_type(s_msg, IF_TYPE_RF24);
			set_if_sig(s_msg, if_msg->sig);
			set_if_task_id(s_msg, if_msg->task_id);
			set_data_common_msg(s_msg, if_msg->data, if_msg->len);

			set_msg_sig(s_msg, AC_IF_COMMON_MSG_IN);
			task_post(AC_TASK_IF_ID, s_msg);

			rf_pending_flag = APP_FLAG_ON;
			timer_set(AC_TASK_GW_IF_ID, AC_RF24_IF_TIMER_PACKET_DELAY, AC_RF24_IF_TIMER_PACKET_DELAY_INTERVAL, TIMER_ONE_SHOT);
		}
			break;

		case RF24_DATA_PURE_MSG_TYPE: {
			APP_DBG("RF24_DATA_PURE_MSG_TYPE\n");

			ak_msg_pure_if_t* if_msg = (ak_msg_pure_if_t*)rf24_buffer;

			ak_msg_t* s_msg = get_pure_msg();

			set_if_type(s_msg, IF_TYPE_RF24);
			set_if_sig(s_msg, if_msg->sig);
			set_if_task_id(s_msg, if_msg->task_id);

			set_msg_sig(s_msg, AC_IF_PURE_MSG_IN);
			task_post(AC_TASK_IF_ID, s_msg);

			rf_pending_flag = APP_FLAG_ON;
			timer_set(AC_TASK_GW_IF_ID, AC_RF24_IF_TIMER_PACKET_DELAY, AC_RF24_IF_TIMER_PACKET_DELAY_INTERVAL, TIMER_ONE_SHOT);
		}
			break;

		default:
			break;
		}
	}
		break;

	case AC_RF24_IF_INIT_NETWORK: {
		fifo_init(&send_common_fifo, mem_cpy, send_common_fifo_buffer, SEND_FIFO_BUFFER_SIZE, sizeof(ak_msg_common_if_t));
		fifo_init(&send_pure_fifo, mem_cpy, send_pure_fifo_buffer, SEND_FIFO_BUFFER_SIZE, sizeof(ak_msg_pure_if_t));

		nrf24l01.begin();
		nrf24l01.maskIRQ(0, 0, 1);  /* enable rx interrupt */
		local_network.begin(me_node_channel, me_node_address);

#if 0
		nrf24l01.printDetails();
#endif
	}
		break;

	case AC_RF24_IF_PURE_MSG_OUT: {
		APP_DBG("AC_RF24_IF_PURE_MSG_OUT\n");

		ak_msg_pure_if_t if_msg;
		mem_set(&if_msg, 0, sizeof(ak_msg_pure_if_t));

		if_msg.task_id = msg->if_task_id;
		if_msg.sig = msg->if_sig;

		if (rf_pending_flag == APP_FLAG_OFF) {
			RF24NetworkHeader server_header(server_node_address, RF24_DATA_PURE_MSG_TYPE);
			local_network.write(server_header, &if_msg, sizeof(ak_msg_pure_if_t));
		}
		else {
			fifo_put(&send_pure_fifo, &if_msg);
		}
	}
		break;

	case AC_RF24_IF_COMMON_MSG_OUT: {
		APP_DBG("AC_RF24_IF_COMMON_MSG_OUT\n");

		ak_msg_common_if_t if_msg;
		mem_set(&if_msg, 0, sizeof(ak_msg_common_if_t));

		if_msg.task_id = msg->if_task_id;
		if_msg.sig = msg->if_sig;

		if_msg.len = get_data_len_common_msg(msg);
		mem_cpy(if_msg.data, get_data_common_msg(msg), if_msg.len);

		if (rf_pending_flag == APP_FLAG_OFF) {
			RF24NetworkHeader server_header(server_node_address, RF24_DATA_COMMON_MSG_TYPE);
			local_network.write(server_header, &if_msg, sizeof(ak_msg_common_if_t));
		}
		else {
			fifo_put(&send_common_fifo, &if_msg);
		}
	}
		break;

	case AC_RF24_IF_TIMER_PACKET_DELAY: {
		rf_pending_flag = APP_FLAG_OFF;
		/* TODO: check data queue */
		if (!fifo_is_empty(&send_pure_fifo)) {
			ak_msg_pure_if_t if_msg;

			fifo_get(&send_pure_fifo, &if_msg);

			RF24NetworkHeader server_header(server_node_address, RF24_DATA_PURE_MSG_TYPE);
			local_network.write(server_header, &if_msg, sizeof(ak_msg_pure_if_t));

			rf_pending_flag = APP_FLAG_ON;
			timer_set(AC_TASK_GW_IF_ID, AC_RF24_IF_TIMER_PACKET_DELAY, AC_RF24_IF_TIMER_PACKET_DELAY_INTERVAL, TIMER_ONE_SHOT);
		}
		else if (!fifo_is_empty(&send_common_fifo)) {
			ak_msg_common_if_t if_msg;

			fifo_get(&send_common_fifo, &if_msg);

			RF24NetworkHeader server_header(server_node_address, RF24_DATA_COMMON_MSG_TYPE);
			local_network.write(server_header, &if_msg, sizeof(ak_msg_common_if_t));

			rf_pending_flag = APP_FLAG_ON;
			timer_set(AC_TASK_GW_IF_ID, AC_RF24_IF_TIMER_PACKET_DELAY, AC_RF24_IF_TIMER_PACKET_DELAY_INTERVAL, TIMER_ONE_SHOT);
		}
	}
		break;

	default:
		break;
	}
}
