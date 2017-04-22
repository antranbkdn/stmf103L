#include "../ak/fsm.h"
#include "../ak/port.h"
#include "../ak/timer.h"
#include "../ak/message.h"

#include "../sys/sys_dbg.h"
#include "../sys/sys_ctrl.h"

#include "../common/utils.h"
#include "../common/xprintf.h"
#include "../common/view_render.h"
#include "../common/screen_manager.h"

#include "app.h"
#include "app_dbg.h"
#include "app_screen.h"

#include "task_list.h"
#include "task_ui.h"
#include "task_time.h"
#include "task_sensor.h"
#include "task_setting.h"

#define SCR_TRAN_SETTING_NONE			(0)
#define SCR_TRAN_SETTING_TIME			(1)
#define SCR_TRAN_SETTING_ENGINEER		(2)

Adafruit_ssd1306syp lcd;

fsm_t fsm_ui;
scr_mng_t screen_ui;

button_t btn_mode;
button_t btn_up;
button_t btn_down;

static uint8_t item_setting = 0;
static uint8_t item_setting_cursor = 0;

static uint8_t air_cond_active = 0;
static uint8_t key_air[2];

static uint8_t calib_stt = 0;
static uint32_t calib_air_off_buf[3][TOTAL_AIR_COND];
static uint32_t calib_air_on_buf[3][TOTAL_AIR_COND];
static uint32_t calib_air_res_buf[3][TOTAL_AIR_COND];


void btn_mode_callback(void* b) {
	button_t* me_b = (button_t*)b;
	switch (me_b->state) {
	case BUTTON_SW_STATE_RELEASE:
		break;

	case BUTTON_SW_STATE_SHORT_HOLD_PRESS: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_UI_GOTO_HOME);
		task_post(AC_TASK_UI_ID, s_msg);
	}
		break;

	case BUTTON_SW_STATE_SHORT_RELEASE_PRESS: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_UI_BUTTON_MODE_PRESS);
		task_post(AC_TASK_UI_ID, s_msg);
	}
		break;

	case BUTTON_SW_STATE_LONG_PRESS: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_UI_BUTTON_MODE_LONG_HOLD);
		task_post(AC_TASK_UI_ID, s_msg);
	}
		break;

	default:
		break;
	}
}

void btn_up_callback(void* b) {
	button_t* me_b = (button_t*)b;
	switch (me_b->state) {
	case BUTTON_SW_STATE_SHORT_RELEASE_PRESS: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_UI_BUTTON_UP_PRESS);
		task_post(AC_TASK_UI_ID, s_msg);
	}
		break;

	default:
		break;
	}
}

void btn_down_callback(void* b) {
	button_t* me_b = (button_t*)b;
	switch (me_b->state) {
	case BUTTON_SW_STATE_SHORT_RELEASE_PRESS: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_UI_BUTTON_DOWN_PRESS);
		task_post(AC_TASK_UI_ID, s_msg);
	}
		break;

	default:
		break;
	}
}

void task_ui(ak_msg_t* msg) {
	fsm_dispatch(&fsm_ui, msg);
}

void ui_state_display_on(ak_msg_t* msg) {
	if (msg->sig == AC_UI_SCREEN_HOME_UPDATE) {
		if (ctrl_scr_home == scr_mng_get_current_screen()) {
			view_render_screen(&scr_main);
		}
	}
	else if (msg->sig == AC_UI_GOTO_HOME) {
		if (ctrl_scr_home != scr_mng_get_current_screen()) {
			item_setting = 0;
			item_setting_cursor = 0;

			/*reset parameter setting real time*/
			scr_setting_time.focus_item = 0;
			((view_rectangle_t*) scr_setting_time.item[0])->border_width = 0;
			((view_rectangle_t*) scr_setting_time.item[1])->border_width = 0;

			/*reset parameter setting temperature for fuzzy setup */
			scr_setting_temp.focus_item = 1;
			((view_rectangle_t*) scr_setting_temp.item[1])->border_width = 0;

			/*reset parameter calibration temp-hum*/
			scr_setting_calib_temp_hum.focus_item = 1;
			((view_rectangle_t*) scr_setting_calib_temp_hum.item[1])->border_width = 0;

			/*reset parameter total & alternate air_cond*/
			scr_setting_total_air.focus_item = 1;
			((view_rectangle_t*) scr_setting_total_air.item[1])->border_width = 0;

			/*reset parameter calib current air_cond by hand*/
			scr_setting_air_current_set.focus_item = 0;
			scr_setting_air_current_set_hand.focus_item = 0;
			((view_rectangle_t*) scr_setting_air_current_set_hand.item[0])->border_width = 0;
			((view_rectangle_t*) scr_setting_air_current_set_hand.item[1])->border_width = 0;

			ak_msg_t* s_msg = get_pure_msg();
			set_msg_sig(s_msg, AC_SM_UI_SETTING_END);
			task_post(AC_TASK_SM_ID, s_msg);

			SCREEN_TRAN(ctrl_scr_home, &scr_main);
		}

		timer_set(AC_TASK_UI_ID, AC_UI_DISPLAY_OFF, AC_UI_TIMER_DISPLAY_OFF_INTERVAL, TIMER_ONE_SHOT);
	}
	else {
		scr_mng_dispatch(msg);

		if (scr_mng_get_current_screen() != ctrl_scr_settings_firmware_update) {
			timer_set(AC_TASK_UI_ID, AC_UI_GOTO_HOME, AC_UI_TIMER_TIMEOUT_INTERVAL, TIMER_ONE_SHOT);
		}
	}
}

void ui_state_display_off(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS: {
		FSM_TRAN(&fsm_ui, ui_state_display_on);

		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_UI_GOTO_HOME);
		task_post(AC_TASK_UI_ID, s_msg);

		view_render_display_on();
	}
		break;

	default:
		break;
	}
}

/*****************************************************************************/
/* main screen
 */
/*****************************************************************************/
void ctrl_scr_home(ak_msg_t* msg) {
	static uint8_t scr_tran_setting = SCR_TRAN_SETTING_NONE;

	switch (msg->sig) {
	case SCREEN_ENTRY: {
		timer_set(AC_TASK_UI_ID, AC_UI_DISPLAY_OFF, AC_UI_TIMER_DISPLAY_OFF_INTERVAL, TIMER_ONE_SHOT);
	}
		break;

	case AC_UI_DISPLAY_OFF: {
		FSM_TRAN(&fsm_ui, ui_state_display_off);
		view_render_display_off();
	}
		break;

	case AC_UI_INITIAL: {
		/* enable using button */
		button_enable(&btn_mode);
		button_enable(&btn_up);
		button_enable(&btn_down);

		view_render_init();

		timer_set(AC_TASK_UI_ID, AC_UI_DISPLAY_OFF, AC_UI_TIMER_DISPLAY_OFF_INTERVAL, TIMER_ONE_SHOT);
	}
		break;

	case AC_UI_BUTTON_MODE_PRESS: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_SM_UI_SETTING_START);
		task_post(AC_TASK_SM_ID, s_msg);

		scr_tran_setting = SCR_TRAN_SETTING_TIME;
	}
		break;

	case AC_UI_BUTTON_MODE_LONG_HOLD: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_SM_UI_SETTING_START);
		task_post(AC_TASK_SM_ID, s_msg);

		scr_tran_setting = SCR_TRAN_SETTING_ENGINEER;
	}
		break;

	case AC_UI_SETTING_SM_BUSY: {
		scr_tran_setting = SCR_TRAN_SETTING_NONE;
		SCREEN_TRAN(ctrl_scr_settings_system_busy, &scr_settings_system_busy);
	}
		break;

	case AC_UI_SETTING_SM_OK: {
		switch (scr_tran_setting) {
		case SCR_TRAN_SETTING_TIME:
			SCREEN_TRAN(ctrl_scr_time, &scr_time);
			break;

		case SCR_TRAN_SETTING_ENGINEER:
			SCREEN_TRAN(ctrl_scr_settings_system_present, &scr_settings_system_present);
			break;

		default:
			break;
		}
	}
		break;

	case AC_UI_FIRMWARE_UPDATE_OK: {
		SCREEN_TRAN(ctrl_scr_settings_firmware_update, &scr_settings_firmware_update);
	}
		break;

	default:
		break;
	}
}

void ctrl_scr_settings_system_busy(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY:
		timer_set(AC_TASK_UI_ID, AC_UI_BUSY_GOTO_HOME, AC_UI_TIMER_AUTO_VIEW_INTERVAL, TIMER_ONE_SHOT);
		break;

	case AC_UI_BUSY_GOTO_HOME: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_UI_GOTO_HOME);
		task_post(AC_TASK_UI_ID, s_msg);
	}
		break;

	default:
		break;
	}
}

void ctrl_scr_settings_firmware_update(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY :
		timer_remove_attr(AC_TASK_UI_ID, AC_UI_GOTO_HOME);
		break;

	case AC_UI_FIRMWARE_GOTO_HOME: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_UI_GOTO_HOME);
		task_post(AC_TASK_UI_ID, s_msg);
	}
		break;

	default:
		break;
	}
}

/*****************************************************************************/
/* engineer screen
 */
/*****************************************************************************/
void increase_current_air(uint8_t air_cond_number) {
	air_cond[air_cond_number].milestone_on += 5;

	if(air_cond[air_cond_number].milestone_on == 1000) {
		air_cond[air_cond_number].milestone_on = 50;
	}
}

void decrease_current_air(uint8_t air_cond_number) {
	air_cond[air_cond_number].milestone_on -= 5;

	if(air_cond[air_cond_number].milestone_on < 50 || air_cond[air_cond_number].milestone_on > 1000) {
		air_cond[air_cond_number].milestone_on = 995;
	}
}

void setting_current_air(void) {
	ak_msg_t* s_msg = get_pure_msg();
	set_msg_sig(s_msg, AC_SETTING_MILESTONE_AIR_COND_ON_UPDATE);
	task_post(AC_TASK_SETTING_ID, s_msg);
}

void invert_operations_temp(void) {
	app_setting.operations_calib_temp = ~ app_setting.operations_calib_temp;
}

void invert_operations_hum(void) {
	app_setting.operations_calib_hum = ~ app_setting.operations_calib_hum;
}

void increase_temp_calib(void) {
	app_setting.temp_calibration ++;

	if (app_setting.temp_calibration > MAX_CALIB_TEMP ) {
		app_setting.temp_calibration = 0;
	}
}

void decrease_temp_calib(void) {
	app_setting.temp_calibration --;

	if (app_setting.temp_calibration == 255 ) {
		app_setting.temp_calibration = MAX_CALIB_TEMP;
	}
}

void increase_hum_calib(void) {
	app_setting.hum_calibration ++;

	if (app_setting.hum_calibration > MAX_CALIB_HUM ) {
		app_setting.hum_calibration = 0;
	}
}

void decrease_hum_calib(void) {
	app_setting.hum_calibration --;

	if (app_setting.hum_calibration == 255 ) {
		app_setting.hum_calibration = MAX_CALIB_HUM;
	}
}

void setting_temp_hum(void) {
	ak_msg_t* s_msg = get_pure_msg();
	set_msg_sig(s_msg, AC_SETTING_RANGE_TIME_AIR_COND_UPDATE);
	task_post(AC_TASK_SETTING_ID, s_msg);
}

void ctrl_scr_settings_system_present(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS:
		SCREEN_TRAN(ctrl_scr_setting_calib_temp_hum, &scr_setting_calib_temp_hum);
		break;

	case AC_UI_BUTTON_UP_PRESS:
		SCREEN_TRAN(ctrl_scr_settings_air_current, &scr_settings_air_current);
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		SCREEN_TRAN(ctrl_scr_settings_air_current, &scr_settings_air_current);
		break;

	default:
		break;
	}
}

void ctrl_scr_setting_calib_temp_hum(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS:
		if (item_setting == 0 && scr_setting_calib_temp_hum.focus_item == 1) {
			item_setting = 1;
			((view_rectangle_t*) scr_setting_calib_temp_hum.item[1])->focus_size = 1;
			((view_rectangle_t*) scr_setting_calib_temp_hum.item[1])->focus_cursor = 0;
			((view_rectangle_t*) scr_setting_calib_temp_hum.item[1])->border_width = 1;
		}
		else if (item_setting == 1) {
			switch (item_setting_cursor) {
			case 0:
				((view_rectangle_t*) scr_setting_calib_temp_hum.item[1])->focus_size = 2;
				item_setting_cursor = item_setting_cursor + 1;
				break;
			case 1:
				((view_rectangle_t*) scr_setting_calib_temp_hum.item[1])->focus_size = 1;
				item_setting_cursor = item_setting_cursor + 4;
				break;
			case 5:
				((view_rectangle_t*) scr_setting_calib_temp_hum.item[1])->focus_size = 2;
				item_setting_cursor = item_setting_cursor + 1;
				break;

			default:
				item_setting_cursor = item_setting_cursor + 1;
				break;
			}

			if (item_setting_cursor > 6) {
				/*send msg to task setting write epprom*/
				setting_temp_hum();

				item_setting = 0;
				item_setting_cursor = 0;
				scr_setting_calib_temp_hum.focus_item = 2;

				((view_rectangle_t*) scr_setting_calib_temp_hum.item[1])->border_width = 0;
			}
		}
		else if (scr_setting_calib_temp_hum.focus_item == 2) {
			item_setting = 0;
			item_setting_cursor = 0;
			scr_setting_calib_temp_hum.focus_item = 1;

			SCREEN_TRAN(ctrl_scr_settings_system_present, &scr_settings_system_present);
		}

		((view_rectangle_t*) scr_setting_calib_temp_hum.item[1])->focus_cursor = item_setting_cursor;
		break;

	case AC_UI_BUTTON_UP_PRESS:
		if (item_setting == 0) {
			scr_setting_calib_temp_hum.focus_item --;
			if (scr_setting_calib_temp_hum.focus_item == 0) {
				scr_setting_calib_temp_hum.focus_item = 2;
			}
		}
		else {
			if (scr_setting_calib_temp_hum.focus_item == 1) {
				if (item_setting_cursor == 0) {
					invert_operations_temp();
				}
				else if (item_setting_cursor == 1) {
					increase_temp_calib();
				}
				else if (item_setting_cursor == 5) {
					invert_operations_hum();
				}
				else if (item_setting_cursor == 6) {
					increase_hum_calib();
				}
			}
		}
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		if (item_setting == 0) {
			scr_setting_calib_temp_hum.focus_item ++;
			if (scr_setting_calib_temp_hum.focus_item == NUMBER_SCREEN_ITEMS_MAX) {
				scr_setting_calib_temp_hum.focus_item = 1;
			}
		}
		else {
			if (scr_setting_calib_temp_hum.focus_item == 1){
				if (item_setting_cursor == 0) {
					invert_operations_temp();
				}
				else if (item_setting_cursor == 1){
					decrease_temp_calib();
				}
				else if (item_setting_cursor == 5) {
					invert_operations_hum();
				}
				else if (item_setting_cursor == 6) {
					decrease_hum_calib();
				}
			}
		}
		break;

	default:
		break;
	}

	xsprintf(((view_rectangle_t*) scr_setting_calib_temp_hum.item[1])->text,"%s%02d  %s%02d",(app_setting.operations_calib_temp ? ("+") : "-"),app_setting.temp_calibration,(app_setting.operations_calib_hum ? ("+") : "-"), app_setting.hum_calibration);
}

void ctrl_scr_settings_air_current(ak_msg_t* msg) {
	static uint8_t time_out = 0;
	switch (msg->sig) {
	case SCREEN_ENTRY:
		timer_set(AC_TASK_UI_ID, AC_UI_VIEW_CURRENT_UPDATE, AC_UI_TIMER_AUTO_VIEW_INTERVAL, TIMER_ONE_SHOT);
		break;

	case AC_UI_BUTTON_MODE_PRESS:
		timer_remove_attr(AC_TASK_UI_ID,AC_UI_VIEW_CURRENT_UPDATE);
		SCREEN_TRAN(ctrl_scr_settings_air_current_set, &scr_setting_air_current_set);
		break;

	case AC_UI_BUTTON_UP_PRESS:
		timer_remove_attr(AC_TASK_UI_ID,AC_UI_VIEW_CURRENT_UPDATE);
		SCREEN_TRAN(ctrl_scr_settings_system_present, &scr_settings_system_present );
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		SCREEN_TRAN(ctrl_scr_settings_system_present, &scr_settings_system_present );
		break;

	case AC_UI_VIEW_CURRENT_UPDATE:
		time_out ++;

		if (time_out * AC_UI_TIMER_AUTO_VIEW_INTERVAL >= AC_UI_TIMER_TIMEOUT_INTERVAL) {
			SCREEN_TRAN(ctrl_scr_home, &scr_main);
		}
		else {
			timer_set(AC_TASK_UI_ID, AC_UI_VIEW_CURRENT_UPDATE, AC_UI_TIMER_AUTO_VIEW_INTERVAL, TIMER_ONE_SHOT);
		}
		break;

	default:
		break;
	}
}

void ctrl_scr_settings_air_current_set(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS:
		if (scr_setting_air_current_set.focus_item == 0) {
			xsprintf(((view_rectangle_t*) scr_setting_air_current_set_auto.item[0])->text,"Auto");
			SCREEN_TRAN(ctrl_scr_settings_air_current_set_auto, &scr_setting_air_current_set_auto);
		}
		else if (scr_setting_air_current_set.focus_item == 1) {
			SCREEN_TRAN(ctrl_scr_settings_air_current_set_hand, &scr_setting_air_current_set_hand);
		}
		else {
			SCREEN_TRAN(ctrl_scr_settings_air_current, &scr_settings_air_current);
		}

		scr_setting_air_current_set.focus_item = 0;
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		scr_setting_air_current_set.focus_item ++;
		if (scr_setting_air_current_set.focus_item == NUMBER_SCREEN_ITEMS_MAX) {
			scr_setting_air_current_set.focus_item = 0;
		}
		break;

	case AC_UI_BUTTON_UP_PRESS:
		scr_setting_air_current_set.focus_item --;
		if (scr_setting_air_current_set.focus_item == 255) {
			scr_setting_air_current_set.focus_item = 2;
		}
		break;

	default:
		break;
	}
}

void ctrl_scr_settings_air_current_set_auto(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		mem_set(air_cond_current_status, AIR_COND_OFF, app_setting.total_air_cond);

		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_IR_AIR_COND_AUTO_CALIB_START_REQ);
		task_post(AC_TASK_IR_ID, s_msg);

		timer_set(AC_TASK_UI_ID, AC_UI_CALIB_CURRENT_AIR_OFF, 3000, TIMER_ONE_SHOT);
	}
		break;

	case AC_UI_CALIB_CURRENT_AIR_OFF: {
		calib_air_off_buf[calib_stt][0] = air_cond[0].available ? (1000 * ct_sensor1.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
		calib_air_off_buf[calib_stt][1] = air_cond[1].available ? (1000 * ct_sensor2.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
		calib_air_off_buf[calib_stt][2] = air_cond[2].available ? (1000 * ct_sensor3.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
		calib_air_off_buf[calib_stt][3] = air_cond[3].available ? (1000 * ct_sensor4.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;

		mem_set(air_cond_current_status, AIR_COND_ON, app_setting.total_air_cond);

		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_IR_AIR_COND_AUTO_CALIB_START_REQ);
		task_post(AC_TASK_IR_ID, s_msg);

		timer_set(AC_TASK_UI_ID, AC_UI_CALIB_CURRENT_AIR_ON, 15000, TIMER_ONE_SHOT);
	}
		break;

	case AC_UI_CALIB_CURRENT_AIR_ON:
		calib_air_on_buf[calib_stt][0] = air_cond[0].available ? (1000 * ct_sensor1.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
		calib_air_on_buf[calib_stt][1] = air_cond[1].available ? (1000 * ct_sensor2.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
		calib_air_on_buf[calib_stt][2] = air_cond[2].available ? (1000 * ct_sensor3.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
		calib_air_on_buf[calib_stt][3] = air_cond[3].available ? (1000 * ct_sensor4.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;

		calib_stt ++;

		if (calib_stt >= 3) {
			{
				mem_set(air_cond_current_status, AIR_COND_OFF, app_setting.total_air_cond);
				ak_msg_t* s_msg = get_pure_msg();
				set_msg_sig(s_msg, AC_IR_AIR_COND_AUTO_CALIB_START_REQ);
				task_post(AC_TASK_IR_ID, s_msg);
			}

			{
				ak_msg_t* s_msg = get_pure_msg();
				set_msg_sig(s_msg, AC_IR_AIR_COND_AUTO_CALIB_END_REQ);
				task_post(AC_TASK_IR_ID, s_msg);
			}

			calib_stt = 0;
			/*caculator threshold current air_cond*/
			for (uint8_t i = 0; i < app_setting.total_air_cond; i ++) {
				for (uint8_t j = 0; j < 3; j ++) {
					if (calib_air_on_buf[j][i] > calib_air_off_buf[j][i]) {
						calib_air_res_buf[j][i] = calib_air_off_buf[j][i] + (calib_air_on_buf[j][i] - calib_air_off_buf[j][i]) / 2 ;
						APP_DBG("calib_air_res_buf[%d][%d] = %d\n",j,i,calib_air_res_buf[j][i]);
					}
					else {
						ak_msg_t* s_msg = get_pure_msg();
						set_msg_sig(s_msg, AC_IR_AIR_COND_AUTO_CALIB_END_REQ);
						task_post(AC_TASK_IR_ID, s_msg);

						xsprintf(((view_rectangle_t*) scr_setting_air_current_set_auto.item[0])->text,"Fail");

						SCREEN_TRAN(ctrl_scr_settings_air_current_set_auto_fail, &scr_setting_air_current_set_auto);
					}
				}
			}

			for (uint8_t i = 0; i < app_setting.total_air_cond; i++) {
				air_cond[i].milestone_on = (calib_air_res_buf[0][i] + calib_air_res_buf[1][i] + calib_air_res_buf[2][i]) / 3;
			}

			/*send msg to task setting write epprom*/
			setting_current_air();

			xsprintf(((view_rectangle_t*) scr_setting_air_current_set_auto.item[0])->text,"BACK");

			SCREEN_TRAN(ctrl_scr_settings_air_current_set_hand, &scr_setting_air_current_set_hand);

		}
		else {
			SCREEN_TRAN(ctrl_scr_settings_air_current_set_auto, &scr_setting_air_current_set_auto);
		}
		break;

	default:
		break;
	}
}

void ctrl_scr_settings_air_current_set_auto_fail(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY:
		timer_set(AC_TASK_UI_ID, AC_UI_CALIB_CURRENT_AIR_RETURN, 1000, TIMER_ONE_SHOT);
		break;

	case AC_UI_CALIB_CURRENT_AIR_RETURN:
		xsprintf(((view_rectangle_t*) scr_setting_air_current_set_auto.item[0])->text,"BACK");
		SCREEN_TRAN(ctrl_scr_settings_air_current_set, &scr_setting_air_current_set);
		break;

	default :
		break;
	}
}

void ctrl_scr_settings_air_current_set_hand(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS: {
		if (item_setting == 0 && scr_setting_air_current_set_hand.focus_item != 2) {
			((view_rectangle_t*) scr_setting_air_current_set_hand.item[scr_setting_air_current_set_hand.focus_item])->focus_cursor = 0;
			((view_rectangle_t*) scr_setting_air_current_set_hand.item[scr_setting_air_current_set_hand.focus_item])->border_width = 1;
			item_setting = 1;
		}
		else if (item_setting == 1 ) {
			item_setting_cursor = item_setting_cursor + 6;
			if (item_setting_cursor > 6) {
				/*send msg to task setting write epprom*/
				setting_current_air();

				/*reset patameter setting*/
				item_setting = 0;
				item_setting_cursor = 0;
				((view_rectangle_t*) scr_setting_air_current_set_hand.item[scr_setting_air_current_set_hand.focus_item])->border_width = 0;
			}
			else {
				((view_rectangle_t*) scr_setting_air_current_set_hand.item[scr_setting_air_current_set_hand.focus_item])->focus_cursor = item_setting_cursor;
				((view_rectangle_t*) scr_setting_air_current_set_hand.item[scr_setting_air_current_set_hand.focus_item])->border_width = 1;
			}
		}
		else if (scr_setting_air_current_set_hand.focus_item == 2) {
			/*reset patameter setting*/
			item_setting = 0;
			item_setting_cursor = 0;
			scr_setting_air_current_set_hand.focus_item = 0;

			SCREEN_TRAN(ctrl_scr_settings_air_current, &scr_settings_air_current);
		}
	}
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		if (item_setting == 0) {
			scr_setting_air_current_set_hand.focus_item ++;
			if (scr_setting_air_current_set_hand.focus_item == NUMBER_SCREEN_ITEMS_MAX) {
				scr_setting_air_current_set_hand.focus_item = 0;
			}
		}
		else {
			if (scr_setting_air_current_set_hand.focus_item == 0) {
				if (item_setting_cursor == 0) {
					decrease_current_air(0);
				}
				else {
					decrease_current_air(1);
				}
			}
			else if (scr_setting_air_current_set_hand.focus_item == 1){
				if (item_setting_cursor == 0) {
					decrease_current_air(2);
				}
				else {
					decrease_current_air(3);
				}
			}
		}
		break;

	case AC_UI_BUTTON_UP_PRESS:
		if (item_setting == 0) {
			scr_setting_air_current_set_hand.focus_item --;
			if (scr_setting_air_current_set_hand.focus_item == 255) {
				scr_setting_air_current_set_hand.focus_item = 2;
			}
		}
		else {
			if (scr_setting_air_current_set_hand.focus_item == 0) {
				if (item_setting_cursor == 0) {
					increase_current_air(0);
				}
				else {
					increase_current_air(1);
				}
			}
			else if (scr_setting_air_current_set_hand.focus_item == 1) {
				if (item_setting_cursor == 0) {
					increase_current_air(2);
				}
				else {
					increase_current_air(3);
				}
			}
		}
		break;

	default:
		break;
	}

	xsprintf(((view_rectangle_t*) scr_setting_air_current_set_hand.item[0])->text,"%03d - %03d",air_cond[0].milestone_on, air_cond[1].milestone_on);
	xsprintf(((view_rectangle_t*) scr_setting_air_current_set_hand.item[1])->text,"%03d - %03d",air_cond[2].milestone_on, air_cond[3].milestone_on);
}

/*****************************************************************************/
/* seeting time screen
 */
/*****************************************************************************/
void read_ds_1302(void) {
	Time t(2016, 1, 1, 1, 0, 0, Time::Day::kMonday);

	t = rtc_ds1302.time();

	if (t.yr >= 2000 && t.yr < 2100) {
		setting_rtc_time.yr   = t.yr - 2000;
	}
	else {
		setting_rtc_time.yr   = 0;
	}

	setting_rtc_time.mon  = t.mon;
	setting_rtc_time.date = t.date;
	setting_rtc_time.hr   = t.hr;
	setting_rtc_time.min  = t.min;
}

void increase_real_time_hour(void) {
	setting_rtc_time.hr ++;

	if (setting_rtc_time.hr > 23) {
		setting_rtc_time.hr = 0;
	}
}

void decrease_real_time_hour(void) {
	setting_rtc_time.hr --;

	if (setting_rtc_time.hr == 255) {
		setting_rtc_time.hr = 23;
	}
}

void increase_real_time_min(void) {
	setting_rtc_time.min ++;

	if (setting_rtc_time.min > 59) {
		setting_rtc_time.min = 0;
	}
}

void decrease_real_time_min(void) {
	setting_rtc_time.min --;

	if (setting_rtc_time.min == 255) {
		setting_rtc_time.min = 59;
	}
}

void increase_real_time_date(void) {
	setting_rtc_time.date ++;

	if (setting_rtc_time.date > 31) {
		setting_rtc_time.date = 1;
	}
}

void decrease_real_time_date(void) {
	setting_rtc_time.date --;

	if (setting_rtc_time.date == 0) {
		setting_rtc_time.date = 31;
	}
}

void increase_real_time_mon(void) {
	setting_rtc_time.mon ++;

	if (setting_rtc_time.mon > 12) {
		setting_rtc_time.mon = 1;
	}
}

void decrease_real_time_mon(void) {
	setting_rtc_time.mon --;

	if (setting_rtc_time.mon == 0) {
		setting_rtc_time.mon = 12;
	}
}

void increase_real_time_year(void) {
	setting_rtc_time.yr ++;

	if (setting_rtc_time.yr == 100) {
		setting_rtc_time.yr = 0;
	}
}

void decrease_real_time_year(void) {
	setting_rtc_time.yr --;

	if (setting_rtc_time.yr == 255) {
		setting_rtc_time.yr = 99;
	}
}

void setting_real_time_dmy(void) {
	if (setting_rtc_time.mon == 4 ||setting_rtc_time.mon == 6 || setting_rtc_time.mon == 9 ||setting_rtc_time.mon == 11) {
		if(setting_rtc_time.date == 31) {
			setting_rtc_time.date = 30;
		}
	}
	else if (setting_rtc_time.mon == 2 && setting_rtc_time.yr % 4 == 0) {
		if(setting_rtc_time.date > 29) {
			setting_rtc_time.date = 29;
		}
	}
	else if (setting_rtc_time.mon == 2 && setting_rtc_time.yr % 4 != 0) {
		if(setting_rtc_time.date > 28) {
			setting_rtc_time.date = 28;
		}
	}

	/*send msg to task setting*/
	ak_msg_t* s_msg = get_pure_msg();
	set_msg_sig(s_msg, AC_SETTING_TIME_RTC_UPDATE);
	task_post(AC_TASK_SETTING_ID, s_msg);
}

void ctrl_scr_time(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS:
		read_ds_1302();

		SCREEN_TRAN(ctrl_scr_setting_time, &scr_setting_time);
		break;

	case AC_UI_BUTTON_UP_PRESS:
		SCREEN_TRAN(ctrl_scr_exit, &scr_exit);
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		SCREEN_TRAN(ctrl_scr_ir, &scr_ir);
		break;

	default:
		break;
	}
}

void ctrl_scr_setting_time(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS: {
		if (item_setting == 0 && scr_setting_time.focus_item != 2) {
			item_setting = 1;

			((view_rectangle_t*) scr_setting_time.item[scr_setting_time.focus_item])->focus_cursor = 0;
			((view_rectangle_t*) scr_setting_time.item[scr_setting_time.focus_item])->border_width = 1;
		}
		else if (item_setting == 1 ) {
			item_setting_cursor = item_setting_cursor + 3;

			if ((scr_setting_time.focus_item == 0 && item_setting_cursor > 3)||(scr_setting_time.focus_item == 1 && item_setting_cursor > 6)) {
				setting_real_time_dmy();
				item_setting = 0;
				item_setting_cursor = 0;

				((view_rectangle_t*) scr_setting_time.item[scr_setting_time.focus_item])->border_width = 0;
			}
			else {
				((view_rectangle_t*) scr_setting_time.item[scr_setting_time.focus_item])->focus_cursor = item_setting_cursor;
				((view_rectangle_t*) scr_setting_time.item[scr_setting_time.focus_item])->border_width = 1;
			}
		}
		else if (scr_setting_time.focus_item == 2) {
			item_setting = 0;
			item_setting_cursor = 0;
			scr_setting_time.focus_item = 0;

			SCREEN_TRAN(ctrl_scr_time, &scr_time);
		}
	}
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		if (item_setting == 0) {
			scr_setting_time.focus_item ++;

			if (scr_setting_time.focus_item == NUMBER_SCREEN_ITEMS_MAX) {
				scr_setting_time.focus_item = 0;
			}
		}
		else {
			if (scr_setting_time.focus_item == 0) {
				if (item_setting_cursor == 0) {
					decrease_real_time_hour();
				}
				else {
					decrease_real_time_min();
				}
			}
			else if (scr_setting_time.focus_item == 1){
				if (item_setting_cursor == 0) {
					decrease_real_time_date();
				}
				else if (item_setting_cursor == 3){
					decrease_real_time_mon();
				}
				else {
					decrease_real_time_year();
				}
			}
		}
		break;

	case AC_UI_BUTTON_UP_PRESS:
		if (item_setting == 0) {
			scr_setting_time.focus_item --;
			if (scr_setting_time.focus_item == 255) {
				scr_setting_time.focus_item = 2;
			}
		}
		else {
			if (scr_setting_time.focus_item == 0) {
				if (item_setting_cursor == 0) {
					increase_real_time_hour();
				}
				else {
					increase_real_time_min();
				}
			}
			else if (scr_setting_time.focus_item == 1) {
				if (item_setting_cursor == 0) {
					increase_real_time_date();
				}
				else if (item_setting_cursor == 3) {
					increase_real_time_mon();
				}
				else {
					increase_real_time_year();
				}
			}
		}
		break;

	default:
		break;
	}

	xsprintf(((view_rectangle_t*) scr_setting_time.item[0])->text,"%02d:%02d",setting_rtc_time.hr, setting_rtc_time.min);
	xsprintf(((view_rectangle_t*) scr_setting_time.item[1])->text,"%02d-%02d-%02d",setting_rtc_time.date, setting_rtc_time.mon, setting_rtc_time.yr);
}

/*****************************************************************************/
/* seeting ir screen
 */
/*****************************************************************************/
void ctrl_scr_ir(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS:
		SCREEN_TRAN(ctrl_scr_setting_ir, &scr_setting_ir);
		break;

	case AC_UI_BUTTON_UP_PRESS:
		SCREEN_TRAN(ctrl_scr_time, &scr_time);
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		SCREEN_TRAN(ctrl_scr_temp, &scr_temp);
		break;

	default:
		break;
	}
}

void ctrl_scr_setting_ir(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS:
		if (scr_setting_ir.focus_item == 0) {
			SCREEN_TRAN(ctrl_scr_setting_ir_rec, &scr_setting_ir_rec);
		}
		else if (scr_setting_ir.focus_item == 1) {
			SCREEN_TRAN(ctrl_scr_setting_ir_test, &scr_setting_ir_test);
		}
		else {
			scr_setting_ir.focus_item = 0;

			SCREEN_TRAN(ctrl_scr_ir, &scr_ir);
		}
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		scr_setting_ir.focus_item ++;

		if (scr_setting_ir.focus_item == NUMBER_SCREEN_ITEMS_MAX) {
			scr_setting_ir.focus_item = 0;
		}
		break;

	case AC_UI_BUTTON_UP_PRESS:
		scr_setting_ir.focus_item --;

		if (scr_setting_ir.focus_item == 255) {
			scr_setting_ir.focus_item = 2;
		}
		break;

	default:
		break;
	}
}

void ctrl_scr_setting_ir_rec(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS:
		if (scr_setting_ir_rec.focus_item == 1) {
			SCREEN_TRAN(ctrl_scr_setting_ir, &scr_setting_ir);
		}
		else {
			scr_setting_ir_rec.focus_item = 1;
			air_cond_active = 0;

			ak_msg_t* s_msg = get_pure_msg();
			set_msg_sig(s_msg, AC_IR_AIR_COND_START_DECODE_REQ);
			task_post(AC_TASK_IR_ID, s_msg);

			SCREEN_TRAN(ctrl_scr_setting_ir_rec_on, &scr_setting_ir_rec_start);
		}
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		scr_setting_ir_rec.focus_item ++;

		if (scr_setting_ir_rec.focus_item == NUMBER_SCREEN_ITEMS_MAX) {
			scr_setting_ir_rec.focus_item = 1;
		}
		break;

	case AC_UI_BUTTON_UP_PRESS:
		scr_setting_ir_rec.focus_item --;

		if (scr_setting_ir_rec.focus_item == 0) {
			scr_setting_ir_rec.focus_item = 2;
		}
		break;

	default:
		break;
	}
}

void ctrl_scr_setting_ir_rec_on(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		xsprintf(((view_rectangle_t*) scr_setting_ir_rec_start.item[1])->text,"AIR%d",air_cond_active + 1);

		ak_msg_t* s_msg = get_common_msg();
		set_msg_sig(s_msg, AC_IR_AIR_COND_ON_DECODE_REQ);
		set_data_common_msg(s_msg, (uint8_t*)&air_cond_active, sizeof(uint8_t));
		task_post(AC_TASK_IR_ID, s_msg);
	}
		break;

	case AC_UI_KEY_ON_DECODE_OK:
		((view_dynamic_t*) scr_setting_ir_rec_start.item[0])->render = view_scr_settings_ir_rec_off;

		SCREEN_TRAN(ctrl_scr_setting_ir_rec_off, &scr_setting_ir_rec_start);
		break;

	case AC_UI_KEY_ON_DECODE_FAIL:
		((view_dynamic_t*) scr_setting_ir_rec_start.item[0])->render = view_scr_settings_ir_rec_on_fail;

		sys_ctrl_delay_ms(500);
		air_cond_active = 0;

		SCREEN_TRAN(ctrl_scr_setting_ir_rec_on, &scr_setting_ir_rec_start);
		break;

	case AC_UI_KEY_ON_SAVING:
		((view_dynamic_t*) scr_setting_ir_rec_start.item[0])->render = view_scr_settings_ir_rec_on_saving;
		break;

	default:
		break;
	}
}

void ctrl_scr_setting_ir_rec_off(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		xsprintf(((view_rectangle_t*) scr_setting_ir_rec_start.item[1])->text,"AIR%d",air_cond_active + 1);

		ak_msg_t* s_msg = get_common_msg();
		set_msg_sig(s_msg, AC_IR_AIR_COND_OFF_DECODE_REQ);
		set_data_common_msg(s_msg, (uint8_t*)&air_cond_active, sizeof(uint8_t));
		task_post(AC_TASK_IR_ID, s_msg);
	}
		break;

	case AC_UI_KEY_OFF_DECODE_OK: {
		air_cond_active ++;
		((view_dynamic_t*) scr_setting_ir_rec_start.item[0])->render = view_scr_settings_ir_rec_on;

		if(air_cond_active >= app_setting.total_air_cond) {
			ak_msg_t* s_msg = get_pure_msg();
			set_msg_sig(s_msg, AC_IR_AIR_COND_STOP_DECODE_REQ);
			task_post(AC_TASK_IR_ID, s_msg);

			SCREEN_TRAN(ctrl_scr_setting_ir, &scr_setting_ir);
		}
		else {
			ak_msg_t* s_msg = get_common_msg();
			set_msg_sig(s_msg, AC_IR_AIR_COND_ON_DECODE_REQ);
			set_data_common_msg(s_msg, (uint8_t*)&air_cond_active, sizeof(uint8_t));
			task_post(AC_TASK_IR_ID, s_msg);

			SCREEN_TRAN(ctrl_scr_setting_ir_rec_on, &scr_setting_ir_rec_start);
		}
	}
		break;

	case AC_UI_KEY_OFF_DECODE_FAIL:
		((view_dynamic_t*) scr_setting_ir_rec_start.item[0])->render = view_scr_settings_ir_rec_off_fail;

		sys_ctrl_delay_ms(500);
		air_cond_active = 0;

		SCREEN_TRAN(ctrl_scr_setting_ir_rec_on, &scr_setting_ir_rec_start);
		break;

	case AC_UI_KEY_OFF_SAVING:
		((view_dynamic_t*) scr_setting_ir_rec_start.item[0])->render = view_scr_settings_ir_rec_off_saving;

		break;

	default:
		break;
	}
}

void ctrl_scr_setting_ir_test(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS: {
		if (scr_setting_ir_test.focus_item == 0) {
			SCREEN_TRAN(ctrl_scr_setting_ir_test_auto, &scr_setting_ir_test_auto);
		}
		else if (scr_setting_ir_test.focus_item == 1) {
			SCREEN_TRAN(ctrl_scr_setting_ir_test_hand, &scr_setting_ir_test_hand);
		}
		else {
			scr_setting_ir_test.focus_item = 0;
			SCREEN_TRAN(ctrl_scr_setting_ir, &scr_setting_ir);
		}
	}
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		scr_setting_ir_test.focus_item ++;

		if (scr_setting_ir_test.focus_item == NUMBER_SCREEN_ITEMS_MAX) {
			scr_setting_ir_test.focus_item = 0;
		}
		break;

	case AC_UI_BUTTON_UP_PRESS:
		scr_setting_ir_test.focus_item --;

		if (scr_setting_ir_test.focus_item == 255) {
			scr_setting_ir_test.focus_item = 2;
		}
		break;

	default:
		break;
	}
}

void ctrl_scr_setting_ir_test_auto(ak_msg_t* msg) {
	switch (msg->sig) {
	case SCREEN_ENTRY: {
		/*send msg to task setting*/
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_IR_AIR_COND_AUTO_TEST_KEY_REQ);
		task_post(AC_TASK_IR_ID, s_msg);
	}
		break;

	case AC_UI_TEST_KEY_AUTO_END:
		SCREEN_TRAN(ctrl_scr_setting_ir_test, &scr_setting_ir_test);
		break;

	default:
		break;
	}
}

void ctrl_scr_setting_ir_test_hand(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS: {
		/*send msg to task ir*/
		if (scr_setting_ir_test_hand.focus_item == 0) {
			key_air[1] = AIR_COND_ON;
		}
		else {
			key_air[1] = AIR_COND_OFF;
		}

		ak_msg_t* s_msg = get_common_msg();
		set_msg_sig(s_msg, AC_IR_AIR_COND_HAND_TEST_KEY_REQ);
		set_data_common_msg(s_msg, (uint8_t*)&key_air, sizeof(key_air));
		task_post(AC_TASK_IR_ID, s_msg);
	}
		break;

	case AC_UI_BUTTON_UP_PRESS:
		scr_setting_ir_test_hand.focus_item --;

		if (scr_setting_ir_test_hand.focus_item == 255) {
			if (key_air[0] > 0) {
				scr_setting_ir_test_hand.focus_item = 1;
				key_air[0] --;
			}
			else {
				scr_setting_ir_test_hand.focus_item = 0;
			}
		}
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		scr_setting_ir_test_hand.focus_item ++;

		if (scr_setting_ir_test_hand.focus_item == 2) {
			scr_setting_ir_test_hand.focus_item = 0;
			key_air[0] ++;

			if (key_air[0] >= app_setting.total_air_cond) {
				key_air[0] = 0;
				SCREEN_TRAN(ctrl_scr_setting_ir_test, &scr_setting_ir_test);
			}
		}
		break;

	default:
		break;
	}

	xsprintf(((view_rectangle_t*) scr_setting_ir_test_hand.item[0])->text,"AIR%d_O",key_air[0] + 1);
	xsprintf(((view_rectangle_t*) scr_setting_ir_test_hand.item[1])->text,"AIR%d_F",key_air[0] + 1);
}

/*****************************************************************************/
/* seeting temperature screen
 */
/*****************************************************************************/
void increase_temp_air_cool(void) {
	app_setting.milestone_temp_cool ++;

	if (app_setting.milestone_temp_cool > TEMP_AIR_COOL_MAX) {
		app_setting.milestone_temp_cool = 0;
	}
}

void decrease_temp_air_cool(void) {
	app_setting.milestone_temp_cool --;

	if (app_setting.milestone_temp_cool == 255) {
		app_setting.milestone_temp_cool = TEMP_AIR_COOL_MAX;
	}
}

void tran_temp_cool_normal() {
	if (app_setting.milestone_temp_normal <= app_setting.milestone_temp_cool) {
		app_setting.milestone_temp_normal = app_setting.milestone_temp_cool + 2;
	}
}

void increase_temp_air_normal(void) {
	app_setting.milestone_temp_normal ++;

	if (app_setting.milestone_temp_normal > TEMP_AIR_WARM_MAX) {
		app_setting.milestone_temp_normal = app_setting.milestone_temp_cool + 1;
	}
}

void decrease_temp_air_normal(void) {
	app_setting.milestone_temp_normal --;

	if (app_setting.milestone_temp_normal <= app_setting.milestone_temp_cool || app_setting.milestone_temp_normal == 255) {
		app_setting.milestone_temp_normal = TEMP_AIR_WARM_MAX;
	}
}

void tran_temp_normal_hot(void) {
	if (app_setting.milestone_temp_hot <= app_setting.milestone_temp_normal) {
		app_setting.milestone_temp_hot = app_setting.milestone_temp_normal + 2;
	}
}

void increase_temp_air_hot(void) {
	app_setting.milestone_temp_hot ++;

	if (app_setting.milestone_temp_hot > TEMP_AIR_HOT_MAX) {
		app_setting.milestone_temp_hot = app_setting.milestone_temp_normal + 1;
	}
}

void decrease_temp_air_hot(void) {
	app_setting.milestone_temp_hot --;

	if (app_setting.milestone_temp_hot <= app_setting.milestone_temp_normal || app_setting.milestone_temp_hot == 255) {
		app_setting.milestone_temp_hot = TEMP_AIR_HOT_MAX;
	}
}

void tran_temp_hot_back(void) {
	if (app_setting.milestone_temp_normal <= app_setting.milestone_temp_cool) {
		app_setting.milestone_temp_normal = app_setting.milestone_temp_cool + 2;
	}

	if (app_setting.milestone_temp_hot <= app_setting.milestone_temp_normal) {
		app_setting.milestone_temp_hot = app_setting.milestone_temp_normal + 2;
	}
}

void setting_temp_air(void) {
	ak_msg_t* s_msg = get_pure_msg();
	set_msg_sig(s_msg, AC_SETTING_TEMP_AIR_COND_UPDATE);
	task_post(AC_TASK_SETTING_ID, s_msg);
}

void ctrl_scr_temp(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS:
		SCREEN_TRAN(ctrl_scr_setting_temp, &scr_setting_temp);
		break;

	case AC_UI_BUTTON_UP_PRESS:
		SCREEN_TRAN(ctrl_scr_ir, &scr_ir);
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		SCREEN_TRAN(ctrl_scr_air, &scr_air);
		break;

	default:
		break;
	}
}

void ctrl_scr_setting_temp(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS:
		if (item_setting == 0 && scr_setting_temp.focus_item == 1) {
			item_setting = 1;

			((view_rectangle_t*) scr_setting_temp.item[1])->focus_cursor = 0;
			((view_rectangle_t*) scr_setting_temp.item[1])->border_width = 1;
		}
		else if (item_setting == 1) {
			item_setting_cursor = item_setting_cursor + 3;

			if (item_setting_cursor > 6) {
				/*send msg to task setting write epprom*/
				setting_temp_air();

				item_setting = 0;
				item_setting_cursor = 0;
				scr_setting_temp.focus_item = 2;

				((view_rectangle_t*) scr_setting_temp.item[1])->border_width = 0;
			}
			else {
				if(item_setting_cursor == 0) {
					tran_temp_cool_normal();
				}
				else if (item_setting_cursor == 3) {
					tran_temp_normal_hot();
				}
				else {
					tran_temp_hot_back();
				}

				((view_rectangle_t*) scr_setting_temp.item[1])->focus_cursor = item_setting_cursor;
				((view_rectangle_t*) scr_setting_temp.item[1])->border_width = 1;
			}
		}
		else if (scr_setting_temp.focus_item == 2) {
			item_setting = 0;
			item_setting_cursor = 0;
			scr_setting_temp.focus_item = 1;

			SCREEN_TRAN(ctrl_scr_temp, &scr_temp);
		}
		break;

	case AC_UI_BUTTON_UP_PRESS:
		if (item_setting == 0) {
			scr_setting_temp.focus_item ++;

			if (scr_setting_temp.focus_item == NUMBER_SCREEN_ITEMS_MAX) {
				scr_setting_temp.focus_item = 1;
			}
		}
		else {
			if (scr_setting_temp.focus_item == 1){
				if (item_setting_cursor == 0) {
					increase_temp_air_cool();
				}
				else if (item_setting_cursor == 3){
					increase_temp_air_normal();
				}
				else {
					increase_temp_air_hot();
				}
			}
		}
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		if (item_setting == 0) {
			scr_setting_temp.focus_item --;

			if (scr_setting_temp.focus_item == 0) {
				scr_setting_temp.focus_item = 2;
			}
		}
		else {
			if (scr_setting_temp.focus_item == 1){
				if (item_setting_cursor == 0) {
					decrease_temp_air_cool();
				}
				else if (item_setting_cursor == 3){
					decrease_temp_air_normal();
				}
				else {
					decrease_temp_air_hot();
				}
			}
		}
		break;

	default:
		break;
	}

	xsprintf(((view_rectangle_t*) scr_setting_temp.item[1])->text,"%02d %02d %02d",app_setting.milestone_temp_cool, app_setting.milestone_temp_normal, app_setting.milestone_temp_hot);
}

/*****************************************************************************/
/* seeting air conditional screen
 */
/*****************************************************************************/
void increase_total_air_cond(void) {
	app_setting.total_air_cond ++;

	if (app_setting.total_air_cond > TOTAL_AIR_COND) {
		app_setting.total_air_cond = 1;
	}
}

void decrease_total_air_cond(void) {
	app_setting.total_air_cond --;

	if (app_setting.total_air_cond == 0 || app_setting.total_air_cond == 255) {
		app_setting.total_air_cond = TOTAL_AIR_COND;
	}
}

void increase_alternate_air(void) {
	app_setting.total_air_cond_alternate ++;

	if (app_setting.total_air_cond_alternate > app_setting.total_air_cond) {
		app_setting.total_air_cond_alternate = 1;
	}
}

void decrease_alternate_air(void) {
	app_setting.total_air_cond_alternate --;

	if (app_setting.total_air_cond_alternate == 0) {
		app_setting.total_air_cond_alternate = app_setting.total_air_cond ;
	}
}

void setting_total_air(void) {
	/*send msg to task seting*/
	ak_msg_t* s_msg = get_pure_msg();
	set_msg_sig(s_msg, AC_SETTING_TOTAL_AIR_COND_UPDATE);
	task_post(AC_TASK_SETTING_ID, s_msg);

}

void increase_timer_air(void) {
	app_setting.time_air_range ++;

	if (app_setting.time_air_range == 25) {
		app_setting.time_air_range = 1;
	}
}

void decrease_timer_air(void) {
	app_setting.time_air_range --;

	if (app_setting.time_air_range == 0 || app_setting.time_air_range == 255 ) {
		app_setting.time_air_range = 24;
	}
}

void setting_timer_air(void) {
	ak_msg_t* s_msg = get_pure_msg();
	set_msg_sig(s_msg, AC_SETTING_RANGE_TIME_AIR_COND_UPDATE);
	task_post(AC_TASK_SETTING_ID, s_msg);
}

void ctrl_scr_air(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS:
		SCREEN_TRAN(ctrl_scr_setting_air, &scr_setting_air);
		break;

	case AC_UI_BUTTON_UP_PRESS:
		SCREEN_TRAN(ctrl_scr_temp, &scr_temp);
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		SCREEN_TRAN(ctrl_scr_default, &scr_default);
		break;

	default:
		break;
	}
}

void ctrl_scr_setting_air(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS:
		if (scr_setting_air.focus_item == 0) {
			SCREEN_TRAN(ctrl_scr_setting_total_air, &scr_setting_total_air);
		}
		else if (scr_setting_air.focus_item == 1) {
			SCREEN_TRAN(ctrl_scr_setting_timer_air, &scr_setting_timer_air);
		}
		else {
			scr_setting_air.focus_item = 0;
			SCREEN_TRAN(ctrl_scr_air, &scr_air);
		}
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		scr_setting_air.focus_item ++;

		if (scr_setting_air.focus_item == NUMBER_SCREEN_ITEMS_MAX) {
			scr_setting_air.focus_item = 0;
		}
		break;

	case AC_UI_BUTTON_UP_PRESS:
		scr_setting_air.focus_item --;

		if (scr_setting_air.focus_item == 255) {
			scr_setting_air.focus_item = 2;
		}
		break;

	default:
		break;
	}
}

void ctrl_scr_setting_total_air(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS:
		if (item_setting == 0 && scr_setting_total_air.focus_item == 1) {
			item_setting = 1;

			((view_rectangle_t*) scr_setting_total_air.item[1])->focus_cursor = 0;
			((view_rectangle_t*) scr_setting_total_air.item[1])->border_width = 1;
		}
		else if (item_setting == 1 ) {
			item_setting_cursor = item_setting_cursor + 5;
			if (item_setting_cursor > 5) {
				/*send msg to task setting write epprom*/
				setting_total_air();

				item_setting = 0;
				item_setting_cursor = 0;
				scr_setting_total_air.focus_item = 2;

				((view_rectangle_t*) scr_setting_total_air.item[1])->border_width = 0;
			}
			else {
				((view_rectangle_t*) scr_setting_total_air.item[1])->focus_cursor = item_setting_cursor;
				((view_rectangle_t*) scr_setting_total_air.item[1])->border_width = 1;
			}
		}
		else if (scr_setting_total_air.focus_item == 2) {
			item_setting = 0;
			item_setting_cursor = 0;
			scr_setting_total_air.focus_item = 1;

			SCREEN_TRAN(ctrl_scr_setting_air, &scr_setting_air);
		}
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		if (item_setting == 0) {
			scr_setting_total_air.focus_item ++;
			if (scr_setting_total_air.focus_item == NUMBER_SCREEN_ITEMS_MAX) {
				scr_setting_total_air.focus_item = 1;
			}
		}
		else {
			if (item_setting_cursor == 0) {
				decrease_total_air_cond();
			}
			else {
				decrease_alternate_air();
			}
		}
		break;

	case AC_UI_BUTTON_UP_PRESS:
		if (item_setting == 0) {
			scr_setting_total_air.focus_item --;

			if (scr_setting_total_air.focus_item == 255) {
				scr_setting_total_air.focus_item = 2;
			}
		}
		else {
			if (item_setting_cursor == 0) {
				increase_total_air_cond();
			}
			else {
				increase_alternate_air();
			}
		}
		break;

	default:
		break;
	}

	xsprintf(((view_rectangle_t*) scr_setting_total_air.item[1])->text,"%02d - %02d",app_setting.total_air_cond, app_setting.total_air_cond_alternate);
}

void ctrl_scr_setting_timer_air(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS:
		if (item_setting == 0 && scr_setting_timer_air.focus_item == 1) {
			item_setting = 1;
			((view_rectangle_t*) scr_setting_timer_air.item[1])->focus_cursor = 0;
			((view_rectangle_t*) scr_setting_timer_air.item[1])->border_width = 1;
		}
		else if (item_setting == 1) {
			/*send msg to task setting write epprom*/
			setting_timer_air();

			item_setting = 0;
			scr_setting_timer_air.focus_item = 2;
			((view_rectangle_t*) scr_setting_timer_air.item[1])->border_width = 0;

		}
		else if (scr_setting_timer_air.focus_item == 2) {
			scr_setting_timer_air.focus_item = 1;
			SCREEN_TRAN(ctrl_scr_setting_air, &scr_setting_air);
		}
		break;

	case AC_UI_BUTTON_UP_PRESS:
		if (item_setting == 0) {
			scr_setting_timer_air.focus_item ++;

			if (scr_setting_timer_air.focus_item == NUMBER_SCREEN_ITEMS_MAX) {
				scr_setting_timer_air.focus_item = 1;
			}
		}
		else {
			increase_timer_air();
		}
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		if (item_setting == 0) {
			scr_setting_timer_air.focus_item --;

			if (scr_setting_timer_air.focus_item == 0) {
				scr_setting_timer_air.focus_item = 2;
			}
		}
		else {
			decrease_timer_air();
		}
		break;

	default:
		break;
	}

	xsprintf(((view_rectangle_t*) scr_setting_timer_air.item[1])->text,"%02d",app_setting.time_air_range);
}

/*****************************************************************************/
/* seeting default parameter system screen
 */
/*****************************************************************************/
void setting_init_data(void) {
	ak_msg_t* s_msg = get_pure_msg();
	set_msg_sig(s_msg, AC_SETTING_EEPROM_INIT_UPDATE);
	task_post(AC_TASK_SETTING_ID, s_msg);

	timer_set(AC_TASK_UI_ID, AC_UI_TIMER_TIMEOUT, 2000, TIMER_ONE_SHOT);
}

void ctrl_scr_default(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS:
		SCREEN_TRAN(ctrl_scr_setting_default, &scr_setting_default);
		break;

	case AC_UI_BUTTON_UP_PRESS:
		SCREEN_TRAN(ctrl_scr_air, &scr_air);
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		SCREEN_TRAN(ctrl_scr_exit, &scr_exit);
		break;

	default:
		break;
	}
}

void ctrl_scr_setting_default(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS:
		if (scr_setting_default.focus_item == 1) {
			setting_init_data();
			SCREEN_TRAN(ctrl_scr_init_data, &scr_init_data);
		}
		else {
			scr_setting_default.focus_item = 2;
			SCREEN_TRAN(ctrl_scr_default, &scr_default);
		}
		break;

	case AC_UI_BUTTON_UP_PRESS:
		scr_setting_default.focus_item ++;
		if (scr_setting_default.focus_item == NUMBER_SCREEN_ITEMS_MAX) {
			scr_setting_default.focus_item = 1;
		}
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		scr_setting_default.focus_item --;
		if (scr_setting_default.focus_item == 0) {
			scr_setting_default.focus_item = 2;
		}
		break;

	default:
		break;
	}
}

void ctrl_scr_init_data(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_TIMER_TIMEOUT :
		scr_setting_default.focus_item = 2;
		SCREEN_TRAN(ctrl_scr_default, &scr_default);
		break;

	default:
		break;
	}
}

void ctrl_scr_exit(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_UI_BUTTON_MODE_PRESS: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_UI_GOTO_HOME);
		task_post(AC_TASK_UI_ID, s_msg);
	}
		break;

	case AC_UI_BUTTON_UP_PRESS:
		SCREEN_TRAN(ctrl_scr_default, &scr_default);
		break;

	case AC_UI_BUTTON_DOWN_PRESS:
		SCREEN_TRAN(ctrl_scr_time, &scr_time);
		break;

	default:
		break;
	}
}
