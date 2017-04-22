#include <stdbool.h>
#include <stdint.h>

#include "app.h"
#include "app_if.h"
#include "app_dbg.h"
#include "app_data.h"

#include "task_if.h"
#include "task_list.h"
#include "task_list_if.h"
#include "task_sensor.h"

#include "../ak/fsm.h"
#include "../ak/port.h"
#include "../ak/message.h"
#include "../ak/timer.h"

#include "../common/utils.h"

#include "../sys/sys_dbg.h"
#include "../sys/sys_irq.h"
#include "../sys/sys_io.h"


void task_if(ak_msg_t* msg) {
	if (msg->if_type == IF_TYPE_RF24) {
		switch (msg->sig) {
		case AC_IF_PURE_MSG_IN: {
			APP_DBG("AC_IF_PURE_MSG_IN\n");
			msg_inc_ref_count(msg);
			set_msg_sig(msg, msg->if_sig);
			task_post(msg->if_task_id, msg);
		}
			break;

		case AC_IF_PURE_MSG_OUT: {
			APP_DBG("AC_IF_PURE_MSG_OUT\n");
			msg_inc_ref_count(msg);
			set_msg_sig(msg, AC_RF24_IF_PURE_MSG_OUT);
			task_post(AC_TASK_GW_IF_ID, msg);
		}
			break;

		case AC_IF_COMMON_MSG_IN: {
			APP_DBG("AC_IF_COMMON_MSG_IN\n");
			msg_inc_ref_count(msg);
			set_msg_sig(msg, msg->if_sig);
			task_post(msg->if_task_id, msg);
		}
			break;

		case AC_IF_COMMON_MSG_OUT: {
			APP_DBG("AC_IF_COMMON_MSG_OUT\n");
			msg_inc_ref_count(msg);
			set_msg_sig(msg, AC_RF24_IF_COMMON_MSG_OUT);
			task_post(AC_TASK_GW_IF_ID, msg);
		}
			break;

		default:
			break;
		}
	}
}
