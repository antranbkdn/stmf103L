#include "../ak/fsm.h"
#include "../ak/port.h"
#include "../ak/message.h"

#include "../sys/sys_dbg.h"

#include "../common/utils.h"

#include "app.h"
#include "app_dbg.h"
#include "app_data.h"

#include "task_list.h"
#include "task_time.h"
#include "task_ir.h"
#include "task_sensor.h"
#include "task_setting.h"

#define NO_CHANGE_AIR_COND				(0)
#define CHANGE_AIR_COND					(1)

rtc_t rtc;
DS1302 rtc_ds1302;
air_cond_t air_cond[TOTAL_AIR_COND];
uint8_t air_cond_expect_status[TOTAL_AIR_COND];
uint8_t air_cond_current_status[TOTAL_AIR_COND];

static uint8_t time_switch_air();
static void process_priority_air_cond();
static void process_status_air_cond(uint8_t temp);
static void process_error_air_cond();

void task_time(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_TIME_RTC_UPDATE_STATUS: {
		/* send request to TASK_SENSOR for get temperature */
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_SENSOR_TEMPERATURE_STATUS_REQ);
		task_post(AC_TASK_SENSOR_ID, s_msg);
	}
		break;

	case AC_TIME_STATUS_RES_OK:
		break;

	case AC_TIME_STATUS_RES_ERR: {
		process_error_air_cond();
	}
		break;

	case AC_TIME_AC_SENSOR_TEMPERATURE_STATUS_REQ: {
		sensor_packet_t* sensor = ((sensor_packet_t*)get_data_common_msg(msg));
		APP_DBG("remote temp:%d\n",sensor->remote_temperature);

		/*function process air_cond status */
		if(time_switch_air() == CHANGE_AIR_COND) {
			process_priority_air_cond();
		}

		process_status_air_cond(sensor->remote_temperature);

		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_SM_TIME_PROCESS_UPDATE_REQ);
		task_post(AC_TASK_SM_ID, s_msg);
	}
		break;

	default:
		FATAL("TKTE", 0x01);
		break;
	}
}

uint8_t time_switch_air() {
	uint8_t ret;
	app_setting.time_air_counter = app_setting.time_air_counter + AC_RTC_STICK_TIME_MIN;
	APP_DBG("time_air_counter = %d\n",app_setting.time_air_counter);

	if (app_setting.time_air_counter >= app_setting.time_air_range * 60) {
		app_setting.time_air_counter = 0;
		ret =  CHANGE_AIR_COND;
	}
	else {
		ret = NO_CHANGE_AIR_COND;
	}

	/* write eeprom with  period  T = 5*AC_RTC_STICK_TIME_MIN = 10*2 = 20 Min*/
	if (app_setting.time_air_counter % (AC_RTC_STICK_TIME_MIN * 10) == 0) {
		eeprom_write(EEPROM_APP_SETTING_ADDR, (uint8_t*)&app_setting.time_air_counter, sizeof(app_setting.time_air_counter));
	}

	return ret;
}

void process_priority_air_cond() {
	uint8_t current_active_index;

	for (uint8_t i = 0; i < app_setting.total_air_cond; i++) {
		/* set next air-cond active */
		if (air_cond[i].active == 1) {
			/* clean current active */
			air_cond[i].active = 0;
			current_active_index = i;

			/* set new current active */
			while (1) {
				/* determine next air-cond */
				i = i + app_setting.total_air_cond_alternate;
				i = i % app_setting.total_air_cond;

				if (air_cond[i].available) {
					air_cond[i].active = 1;
					break;
				}

				if (i == current_active_index) {
					/* TODO: handle all air-condition die */
					FATAL("TKTE", 0x03);
				}
			}
		}
	}

	/* update air-cond data to eeprom */
	eeprom_write(EEPROM_AIR_COND_START_BASE_ADDR, (uint8_t*)air_cond, TOTAL_AIR_COND * sizeof(air_cond_t));

	if (app_setting.total_air_cond <= 0 || app_setting.total_air_cond > TOTAL_AIR_COND) {
		FATAL("TKTE", 0x02);
	}
}

void process_status_air_cond(uint8_t temp) {
	uint8_t i, level, percent, priority = 0, total_air_on = 0;
	uint8_t total_air_available = 0;
	static const uint8_t air_on_stt[5][5]= {0, 0, 0, 0, 0,
											0, 0, 1, 1, 1,
											0, 1, 1, 2, 2,
											0, 1, 2, 3, 3,
											0, 1, 2, 3, 4};

	if (app_setting.total_air_cond <= 0 || app_setting.total_air_cond > TOTAL_AIR_COND) {
		FATAL("TKTE", 0x02);
	}

	/* caculator total air cond available and air_cond priority */
	for (i = 0; i < app_setting.total_air_cond; i++) {
		if (air_cond[i].available == 1) {
			total_air_available ++;
		}
		if (air_cond[i].active == 1) {
			priority = i;
		}
	}

	/* caculator power of air_cond available by fuzzy logic */
	if (temp_fuzzy_logic_set(app_setting.milestone_temp_cool, app_setting.milestone_temp_normal, app_setting.milestone_temp_hot) == FUZZY_LOGIC_NG) {
		FATAL("TKTE", 0x04);
	}

	if (temp_fuzzy_logic_cal(&level, &percent, temp) == FUZZY_LOGIC_NG) {
		FATAL("TKTE", 0x05);
	}

	APP_DBG("level:%d\n", level);
	APP_DBG("percent:%d\n", percent);

	/* caculator total air condtional ON by table 'air_on_stt' */
	if (level == FUZZY_LOGIC_COOL && percent == 100) {
		/* power = 0% */
		total_air_on = air_on_stt[total_air_available][0];
	}
	else if (level == FUZZY_LOGIC_COOL && percent > 50) {
		/* power = 25% */
		total_air_on = air_on_stt[total_air_available][1];
	}
	else if (level == FUZZY_LOGIC_NORMAL && percent > 50) {
		/* power = 50% */
		total_air_on = air_on_stt[total_air_available][2];
	}
	else if (level == FUZZY_LOGIC_HOT && percent > 50 && percent < 100) {
		/* power = 75% */
		total_air_on = air_on_stt[total_air_available][3];
	}
	else if (level == FUZZY_LOGIC_HOT && percent == 100) {
		/* power = 100% */
		total_air_on = air_on_stt[total_air_available][4];
	}

	total_air_on = total_air_on * app_setting.total_air_cond_alternate;

	if (total_air_on > app_setting.total_air_cond) {
		total_air_on = app_setting.total_air_cond;
	}

	/* create expect status of all air conditional*/
	mem_set(air_cond_expect_status, AIR_COND_OFF, app_setting.total_air_cond);

	if (total_air_on > 0) {
		air_cond_expect_status[priority] = AIR_COND_ON;
		total_air_on --;
		i = priority;

		while (total_air_on) {
			/* determine next air-cond */
			i++;
			i = i % app_setting.total_air_cond;

			if (air_cond[i].available == 1) {
				air_cond_expect_status[i] = AIR_COND_ON;
				total_air_on --;
			}
		}
	}
}

void process_error_air_cond() {
	for (uint8_t i = 0;i < app_setting.total_air_cond; i++) {
		if (air_cond_current_status[i] != AIR_COND_NO_CHANGE && air_cond[i].available != 0) {
			air_cond[i].failed_counter ++;
		}
	}

	eeprom_write(EEPROM_AIR_COND_START_BASE_ADDR, (uint8_t*)air_cond, TOTAL_AIR_COND * sizeof(air_cond_t));
}
