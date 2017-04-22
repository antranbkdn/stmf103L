#include "../ak/fsm.h"
#include "../ak/port.h"
#include "../ak/message.h"

#include "../sys/sys_dbg.h"
#include "../common/utils.h"

#include "app.h"
#include "app_if.h"
#include "app_dbg.h"
#include "app_data.h"

#include "task_list.h"
#include "task_list_if.h"

#include "task_sensor.h"
#include "task_time.h"
#include "task_ir.h"
#include "task_setting.h"
#include "task_if.h"

#define SENSOR_RET_OK					(0x00)
#define SENSOR_RET_NG					(0x01)

#define SENSOR_TYPE_REMOTE_TEMP			(0x01)
#define SENSOR_TYPE_HUMIDITY			(0x02)
#define SENSOR_TYPE_AIR_COND			(0x03)

static sensor_packet_t sensor_packet;

THERMISTOR thermistor(1,10000,3590,10000);
hs1101_t hs1101;

EnergyMonitor ct_sensor1;
EnergyMonitor ct_sensor2;
EnergyMonitor ct_sensor3;
EnergyMonitor ct_sensor4;

static uint8_t get_sensor_data_req(uint8_t type);

void task_sensor(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_SENSOR_REPORT_REQ:
		/* get all sensor data */
		get_sensor_data_req(SENSOR_TYPE_REMOTE_TEMP);
		get_sensor_data_req(SENSOR_TYPE_AIR_COND);
		get_sensor_data_req(SENSOR_TYPE_HUMIDITY);

	{
		ak_msg_t* s_msg = get_common_msg();
		set_msg_sig(s_msg, AC_IF_COMMON_MSG_OUT);

		set_if_type(s_msg, IF_TYPE_RF24);
		set_if_sig(s_msg, GW_AC_SENSOR_RES);
		set_if_task_id(s_msg, GW_TASK_SENSOR_ID);

		set_data_common_msg(s_msg, (uint8_t*)&sensor_packet, sizeof(sensor_packet_t));

		task_post(AC_TASK_IF_ID, s_msg);
	}
		break;

	case AC_SENSOR_AIR_COND_STATUS_REQ: {
		/*read sensor ct*/
		get_sensor_data_req(SENSOR_TYPE_AIR_COND);

		for (uint8_t i = 0; i < app_setting.total_air_cond; i++) {
			if (sensor_packet.air_cond_current[i] > air_cond[i].milestone_on) {
				air_cond_current_status[i] = AIR_COND_ON;
			}
			else {
				air_cond_current_status[i] = AIR_COND_OFF;
			}
		}

		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_SM_SENSOR_AIR_COND_STATUS_RES);
		task_post(AC_TASK_SM_ID, s_msg);
	}
		break;

	case AC_SENSOR_TEMPERATURE_STATUS_REQ: {
		sensor_packet.remote_temperature = app_setting.operations_calib_temp ? ((float)thermistor.read_kalman() / 10 + app_setting.temp_calibration) : ((float)thermistor.read_kalman() / 10 - app_setting.temp_calibration);

		ak_msg_t* s_msg = get_common_msg();
		set_msg_sig(s_msg, AC_TIME_AC_SENSOR_TEMPERATURE_STATUS_REQ);
		set_data_common_msg(s_msg, (uint8_t*)&sensor_packet, sizeof(sensor_packet_t));
		task_post(AC_TASK_TIME_ID, s_msg);
	}
		break;

	default:
		break;
	}
}

uint8_t get_sensor_data_req(uint8_t type) {
	switch(type) {
	case SENSOR_TYPE_REMOTE_TEMP:
		sensor_packet.remote_temperature = app_setting.operations_calib_temp ? ((float)thermistor.read_kalman() / 10 + app_setting.temp_calibration) : ((float)thermistor.read_kalman() / 10 - app_setting.temp_calibration);
		break;

	case SENSOR_TYPE_HUMIDITY: {
		uint8_t hum = 0;

		hum = app_setting.operations_calib_hum ? (hs1101_read(&hs1101) + app_setting.hum_calibration) : (hs1101_read(&hs1101) - app_setting.hum_calibration);

		if(hum > 200) {
			hum = 0;
		}
		else if (hum > 100) {
			hum = 99;
		}
		sensor_packet.humindity = hum;
	}
		break;

	case SENSOR_TYPE_AIR_COND:
		sensor_packet.air_cond_current[0] = air_cond[0].available ? (1000 * ct_sensor1.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
		sensor_packet.air_cond_current[1] = air_cond[1].available ? (1000 * ct_sensor2.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
		sensor_packet.air_cond_current[2] = air_cond[2].available ? (1000 * ct_sensor3.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
		sensor_packet.air_cond_current[3] = air_cond[3].available ? (1000 * ct_sensor4.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
		break;

	default:
		break;
	}

	return SENSOR_RET_OK;
}
