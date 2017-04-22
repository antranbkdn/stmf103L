#include "../ak/fsm.h"
#include "../ak/port.h"
#include "../ak/message.h"
#include "../ak/timer.h"

#include "../sys/sys_dbg.h"

#include "app.h"
#include "app_dbg.h"

#include "task_list.h"
#include "task_sm.h"
#include "task_ir.h"
#include "task_time.h"
#include "task_setting.h"

#define AIR_COND_CTRL_RETRY_MAX			(3)

fsm_t fsm_sm;

static uint8_t air_cond_ctrl_retry_time = 0;

void task_sm(ak_msg_t* msg) {
	fsm_dispatch(&fsm_sm, msg);
}

void sm_state_idle(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_SM_TIME_PROCESS_UPDATE_REQ: {
		/* request all air conditional status */
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_SENSOR_AIR_COND_STATUS_REQ);
		task_post(AC_TASK_SENSOR_ID, s_msg);

		/* reset retry counter */
		air_cond_ctrl_retry_time = 0;

		/* change state to PROCESS_REQ */
		FSM_TRAN(&fsm_sm, sm_state_requesting);
	}
		break;

	case AC_SM_UI_SETTING_START: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_UI_SETTING_SM_OK);
		task_post(AC_TASK_UI_ID, s_msg);

		FSM_TRAN(&fsm_sm, sm_state_setting);
	}
		break;

	case AC_SM_FIRMWARE_UPDATE_REQ: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_FIRMWARE_UPDATE_SM_OK);
		task_post(AC_TASK_FIRMWARE_ID, s_msg);

		s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_UI_FIRMWARE_UPDATE_OK);
		task_post(AC_TASK_UI_ID, s_msg);

		FSM_TRAN(&fsm_sm, sm_state_firmware_update);
	}

		break;

	default:
		break;
	}
}

void sm_state_requesting(ak_msg_t* msg) {
	uint8_t total_air_available = 0, number_air_cond_nochange = 0;

	switch (msg->sig) {
	case AC_SM_SENSOR_AIR_COND_STATUS_RES: {
		/*dump mask_expecrt & mask_current in sequence*/
		APP_DBG("expect\t");
		APP_DBG("current\n");
		for ( uint8_t i =0; i < app_setting.total_air_cond; i++) {
			APP_DBG("%d\t",air_cond_expect_status[i]);
			APP_DBG("%d\n",air_cond_current_status[i]);
		}

		/*process mask current */
		for (uint8_t i = 0; i < app_setting.total_air_cond; i++) {
			if (air_cond[i].available == 1) {
				total_air_available ++;
				if (air_cond_current_status[i] == air_cond_expect_status[i]) {
					air_cond_current_status[i] = AIR_COND_NO_CHANGE;
					number_air_cond_nochange ++;
				}
				else {
					air_cond_current_status[i] = air_cond_expect_status[i];
				}
			}
		}

		/*dump mask_current after process*/
		APP_DBG("current state air after process:\n");
		for (uint8_t i = 0; i < app_setting.total_air_cond; i++) {
			if (air_cond[i].available == 1) {
				APP_DBG("%d\n",air_cond_current_status[i]);
			}
		}

		/*number_air_cond_nochange mask current no change */
		if (number_air_cond_nochange >= total_air_available) {
			ak_msg_t* s_msg = get_pure_msg();
			set_msg_sig(s_msg, AC_TIME_STATUS_RES_OK);
			task_post(AC_TASK_TIME_ID, s_msg);

			FSM_TRAN(&fsm_sm, sm_state_idle);
		}
		else {
			if (air_cond_ctrl_retry_time ++ < AIR_COND_CTRL_RETRY_MAX) {

				ak_msg_t* s_msg = get_pure_msg();
				set_msg_sig(s_msg,AC_IR_AIR_COND_STATUS_REQ);
				task_post(AC_TASK_IR_ID, s_msg);

				timer_set(AC_TASK_SM_ID, AC_SM_TIMEOUT_RETRY_SEND_IR, AC_SM_TIMEOUT_RETRY_SEND_IR_INTERVAL, TIMER_ONE_SHOT);
			}
			else {
				ak_msg_t* s_msg = get_common_msg();
				set_msg_sig(s_msg, AC_TIME_STATUS_RES_ERR);
				task_post(AC_TASK_TIME_ID, s_msg);

				FSM_TRAN(&fsm_sm, sm_state_idle);
			}
		}
	}
		break;

	case AC_SM_TIMEOUT_RETRY_SEND_IR: {
		/* request air condition status */
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_SENSOR_AIR_COND_STATUS_REQ);
		task_post(AC_TASK_SENSOR_ID, s_msg);
	}
		break;

	case AC_SM_UI_SETTING_START: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_UI_SETTING_SM_BUSY);
		task_post(AC_TASK_UI_ID, s_msg);
	}
		break;

	case AC_SM_FIRMWARE_UPDATE_REQ: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_FIRMWARE_UPDATE_SM_BUSY);
		task_post(AC_TASK_FIRMWARE_ID, s_msg);
	}
		break;

	default:
		break;
	}
}

void sm_state_setting(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_SM_FIRMWARE_UPDATE_REQ: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_FIRMWARE_UPDATE_SM_BUSY);
		task_post(AC_TASK_FIRMWARE_ID, s_msg);
	}
		break;

	case AC_SM_UI_SETTING_END: {
		FSM_TRAN(&fsm_sm, sm_state_idle);
	}
		break;

	default:
		break;
	}
}

void sm_state_firmware_update(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_SM_SAFE_MODE_REQ: {
		APP_DBG("AC_SM_SAFE_MODE_REQ\n");
		/**
		 * TODO: force system to safe mode, prepare for update firmware
		 */
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_FIRMWARE_SAFE_MODE_RES_OK);
		task_post(AC_TASK_FIRMWARE_ID, s_msg);
	}
		break;

	case AC_SM_FIRMWARE_RELEASE_REQ: {
		APP_DBG("AC_SM_FIRMWARE_RELEASE_REQ\n");
		FSM_TRAN(&fsm_sm, sm_state_idle);

		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_UI_FIRMWARE_GOTO_HOME);
		task_post(AC_TASK_UI_ID, s_msg);
	}
		break;

	default:
		break;
	}
}


