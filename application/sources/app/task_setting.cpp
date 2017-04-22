#include "../ak/fsm.h"
#include "../ak/port.h"
#include "../ak/message.h"

#include "../sys/sys_ctrl.h"
#include "../sys/sys_dbg.h"

#include "../common/utils.h"

#include "../driver/eeprom/eeprom.h"
#include "../driver/fuzzy_logic/fuzzy_logic.h"
#include "../driver/ds1302/DS1302.h"

#include "app.h"
#include "app_if.h"
#include "app_dbg.h"
#include "app_eeprom.h"

#include "task_list.h"
#include "task_list_if.h"
#include "task_setting.h"
#include "task_time.h"
#include "task_if.h"

setting_rtc_time_t setting_rtc_time;
app_setting_t app_setting;

static uint8_t is_setting_app_valib(app_setting_t* setting);

void task_setting(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_SETTING_APP_UPDATE: {
		app_setting_t* setting = (app_setting_t*)get_data_common_msg(msg);
		if (is_setting_app_valib(setting) == APP_OK) {
			mem_cpy(&app_setting, setting, sizeof(app_setting_t));
			eeprom_write(EEPROM_APP_SETTING_ADDR, (uint8_t*)&app_setting, sizeof(app_setting_t));
		}
		else {
			APP_PRINT("setting vaule invalib\n");
		}
	}
		break;

	case AC_SETTING_TOTAL_AIR_COND_UPDATE: {
		if (is_setting_app_valib(&app_setting) == APP_OK) {
			eeprom_write(EEPROM_APP_SETTING_ADDR, (uint8_t*)&app_setting, sizeof(app_setting_t));
		}

		mem_set(air_cond, 0, TOTAL_AIR_COND * sizeof(air_cond_t));

		/* init air_cond */
		for (uint8_t i = 0; i < app_setting.total_air_cond; i++) {
			air_cond[i].available = 1;
		}
		air_cond[0].active = 1;

		eeprom_write(EEPROM_AIR_COND_START_BASE_ADDR, (uint8_t*)air_cond, TOTAL_AIR_COND * sizeof(air_cond_t));
	}
		break;

	case AC_SETTING_TEMP_AIR_COND_UPDATE: {
		if (is_setting_app_valib(&app_setting) == APP_OK) {
			if (temp_fuzzy_logic_set(app_setting.milestone_temp_cool, app_setting.milestone_temp_normal, app_setting.milestone_temp_hot ) == FUZZY_LOGIC_NG) {
				FATAL("TKTE", 0x04);
			}

			eeprom_write(EEPROM_APP_SETTING_ADDR, (uint8_t*)&app_setting, sizeof(app_setting_t));
		}
		else {
			APP_PRINT("setting vaule invalib\n");
		}

	}
		break;

	case AC_SETTING_TIME_RTC_UPDATE: {
		Time t (2016, 1, 1, 1, 0, 0, Time::Day::kMonday);

		t.yr   = setting_rtc_time.yr + 2000;
		t.mon  = setting_rtc_time.mon;
		t.date = setting_rtc_time.date;
		t.hr   = setting_rtc_time.hr;
		t.min  = setting_rtc_time.min;

		rtc_ds1302.writeProtect(false);
		rtc_ds1302.halt(false);
		rtc_ds1302.time(t);
	}
		break;

	case AC_SETTING_RANGE_TIME_AIR_COND_UPDATE: {
		if (is_setting_app_valib(&app_setting) == APP_OK) {
			eeprom_write(EEPROM_APP_SETTING_ADDR, (uint8_t*)&app_setting, sizeof(app_setting_t));
		}
		else {
			APP_PRINT("setting vaule invalib\n");
		}
	}
		break;

	case AC_SETTING_MILESTONE_AIR_COND_ON_UPDATE: {
		if (is_setting_app_valib(&app_setting) == APP_OK) {
			eeprom_write(EEPROM_APP_SETTING_ADDR, (uint8_t*)&app_setting, sizeof(app_setting_t));
		}
		else {
			APP_PRINT("setting vaule invalib\n");
		}
	}
		break;

	case AC_SETTING_EEPROM_INIT_UPDATE: {
		app_setting_t m_app_setting;

		m_app_setting.milestone_temp_cool		=	APP_SETTING_DEFAUL_MILESTONE_TEMP_COOL;
		m_app_setting.milestone_temp_normal		=	APP_SETTING_DEFAUL_MILESTONE_TEMP_NORMAL;
		m_app_setting.milestone_temp_hot		=	APP_SETTING_DEFAUL_MILESTONE_TEMP_HOT;

		m_app_setting.total_air_cond			=	APP_SETTING_DEFAUL_TOTAL_AIR_COND;
		m_app_setting.total_air_cond_alternate	=	APP_SETTING_DEFAUL_TOTAL_AIR_COND_ALTERNATE;

		m_app_setting.time_air_range			=	APP_SETTING_NUMBER_TIME_AIR_RANGE;
		m_app_setting.time_air_counter			=	APP_SETTING_DEFAUL_TIME_AIR_COUNTER;

		m_app_setting.temp_calibration			=	APP_SETTING_DEFAUL_TEMP_CALIBRATION;
		m_app_setting.hum_calibration			=	APP_SETTING_DEFAUL_TEMP_CALIBRATION;

		m_app_setting.operations_calib_temp		=	APP_SETTING_DEFAUL_OPTS_TEMP_CALIBRATION;
		m_app_setting.operations_calib_hum		=	APP_SETTING_DEFAUL_OPTS_HUM_CALIBRATION;

		eeprom_write(EEPROM_APP_SETTING_ADDR, (uint8_t*)&m_app_setting, sizeof(m_app_setting));

		mem_set(air_cond, 0, TOTAL_AIR_COND * sizeof(air_cond_t));

		/* init air_cond */
		for (uint8_t i = 0; i < m_app_setting.total_air_cond; i++) {
			air_cond[i].available = 1;
			air_cond[i].active = 0;
			air_cond[i].milestone_on = APP_SETTING_DEFAUL_MILESTONE_AIR_COND_ON;
		}

		air_cond[0].active = 1;

		mem_cpy(&app_setting, &m_app_setting, sizeof(app_setting_t));
		eeprom_write(EEPROM_AIR_COND_START_BASE_ADDR, (uint8_t*)air_cond, TOTAL_AIR_COND * sizeof(air_cond_t));

		LOGIN_PRINT("\nOK\n");

		sys_ctrl_delay_ms(300);
	}
		break;

	default:
		break;
	}

	/* post sync setting message */
	{
		ak_msg_t* s_msg = get_common_msg();
		set_msg_sig(s_msg, AC_IF_COMMON_MSG_OUT);

		set_if_type(s_msg, IF_TYPE_RF24);
		set_if_sig(s_msg, GW_SNMP_AC_SETTINGS_REP);
		set_if_task_id(s_msg, GW_TASK_SNMP_ID);

		app_setting_t setting_data;
		eeprom_read(EEPROM_APP_SETTING_ADDR, (uint8_t*)&setting_data, sizeof(app_setting_t));
		set_data_common_msg(s_msg, (uint8_t*)&setting_data, sizeof(app_setting_t));

		task_post(AC_TASK_IF_ID, s_msg);
	}
}

uint8_t is_setting_app_valib(app_setting_t* setting) {
	if (setting->milestone_temp_cool < 0 ||
			setting->milestone_temp_cool > 100) {
		return APP_NG;
	}

	if (setting->milestone_temp_normal < 0 ||
			setting->milestone_temp_normal > 100) {
		return APP_NG;
	}

	if (setting->milestone_temp_hot < 0 ||
			setting->milestone_temp_hot > 100) {
		return APP_NG;
	}

	return APP_OK;
}
