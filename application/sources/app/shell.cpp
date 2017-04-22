/**
 ******************************************************************************
 * @Author: ThanNT
 * @Date:   13/08/2016
 ******************************************************************************
**/

#include <stdint.h>
#include <stdlib.h>

#include "../ak/ak.h"
#include "../ak/task.h"
#include "../ak/timer.h"
#include "../ak/message.h"

#include "../common/cmd_line.h"
#include "../common/utils.h"
#include "../common/xprintf.h"

#include "../sys/sys_ctrl.h"
#include "../sys/sys_io.h"
#include "../sys/sys_dbg.h"

#include "app.h"
#include "app_if.h"
#include "app_dbg.h"
#include "app_data.h"
#include "app_flash.h"
#include "app_eeprom.h"

#include "task_shell.h"
#include "task_list.h"
#include "task_list_if.h"
#include "task_if.h"
#include "task_time.h"
#include "task_life.h"
#include "task_sensor.h"
#include "task_setting.h"

#include "../driver/rtc/rtc.h"
#include "../driver/led/led.h"
#include "../driver/eeprom/eeprom.h"
#include "../driver/Adafruit_ssd1306syp/Adafruit_ssd1306syp.h"
#include "../driver/EmonLib/EmonLib.h"
#include "../driver/ds1302/DS1302.h"
#include "../driver/flash/flash.h"
#include "../driver/hs1101/hs1101.h"

/*****************************************************************************/
/*  command function declare
 */
/*****************************************************************************/
static int32_t shell_reset(uint8_t* argv);
static int32_t shell_ver(uint8_t* argv);
static int32_t shell_help(uint8_t* argv);
static int32_t shell_reboot(uint8_t* argv);
static int32_t shell_fatal(uint8_t* argv);
static int32_t shell_status(uint8_t* argv);
static int32_t shell_epi(uint8_t* argv);
static int32_t shell_eps(uint8_t* argv);
static int32_t shell_flash(uint8_t* argv);

/*****************************************************************************/
/*  command table
 */
/*****************************************************************************/
cmd_line_t lgn_cmd_table[] = {

	/*************************************************************************/
	/* system command */
	/*************************************************************************/
	{(const int8_t*)"reset",	shell_reset,		(const int8_t*)"reset terminal"},
	{(const int8_t*)"ver",		shell_ver,			(const int8_t*)"version info"},
	{(const int8_t*)"help",		shell_help,			(const int8_t*)"help command info"},
	{(const int8_t*)"reboot",	shell_reboot,		(const int8_t*)"reboot system"},
	{(const int8_t*)"epi",		shell_epi,			(const int8_t*)"epprom init"},

	{(const int8_t*)"fatal",	shell_fatal,		(const int8_t*)"fatal info"},
	{(const int8_t*)"status",	shell_status,		(const int8_t*)"system status"},
	{(const int8_t*)"eps",		shell_eps,			(const int8_t*)"epprom"},
	{(const int8_t*)"flash",	shell_flash,		(const int8_t*)"flash"},

	/*************************************************************************/
	/* debug command */
	/*************************************************************************/

	/* End Of Table */
	{(const int8_t*)0,(pf_cmd_func)0,(const int8_t*)0}
};

/*****************************************************************************/
/*  command function definaion
 */
/*****************************************************************************/
int32_t shell_reset(uint8_t* argv) {
	(void)argv;
	xprintf("\033[2J\r");
	return 0;
}

int32_t shell_ver(uint8_t* argv) {
	(void)argv;
	firmware_header_t firmware_header;
	sys_ctrl_get_firmware_info(&firmware_header);

	LOGIN_PRINT("kernel version: %s\n", AK_VERSION);
	LOGIN_PRINT("firmware checksum: %04x\n", firmware_header.checksum);
	LOGIN_PRINT("firmware length: %d\n", firmware_header.bin_len);
	return 0;
}

int32_t shell_help(uint8_t* argv) {
	uint32_t idx = 0;
	switch (*(argv + 4)) {
	default:
		LOGIN_PRINT("\nCOMMANDS INFORMATION:\n\n");
		while(lgn_cmd_table[idx].cmd != (const int8_t*)0) {
			LOGIN_PRINT("%s\n\t%s\n\n", lgn_cmd_table[idx].cmd, lgn_cmd_table[idx].info);
			idx++;
		}
		break;
	}
	return 0;
}

int32_t shell_reboot(uint8_t* argv) {
	(void)argv;
	sys_ctrl_reset();
	return 0;
}

int32_t shell_fatal(uint8_t* argv) {
	fatal_log_t login_fatal_log;
	ak_msg_t t_msg;

	switch (*(argv + 6)) {
	case 'r':
		mem_set((uint8_t*)&login_fatal_log, 0, sizeof(fatal_log_t));
		eeprom_write(EEPROM_FATAL_LOG_ADDR, (uint8_t*)&login_fatal_log, sizeof(fatal_log_t));

		LOGIN_PRINT("reset fatal log OK\n");
		break;

	case 'l': {
		eeprom_read(EEPROM_FATAL_LOG_ADDR, (uint8_t*)&login_fatal_log, sizeof(fatal_log_t));

		LOGIN_PRINT("fatal times:\t%d\n", login_fatal_log.fatal_times);
		LOGIN_PRINT("restart times:\t%d\n", login_fatal_log.restart_times);

		LOGIN_PRINT("fatal type:\t%s\n", login_fatal_log.string);
		LOGIN_PRINT("fatal code:\t%d\n", login_fatal_log.code);

		LOGIN_PRINT("task id:\t%d\n", login_fatal_log.current_task.id);
		LOGIN_PRINT("task pri:\t%d\n", login_fatal_log.current_task.pri);
		LOGIN_PRINT("task entry:\t0x%x\n", login_fatal_log.current_task.task);

		LOGIN_PRINT("obj sig:\t%d\n", login_fatal_log.current_active_object.sig);
		LOGIN_PRINT("obj type:\t0x%x\n", get_msg_type(&login_fatal_log.current_active_object));
		LOGIN_PRINT("obj ref count:\t%d\n", get_msg_ref_count(&login_fatal_log.current_active_object));
		LOGIN_PRINT("obj wait time:\t%d\n", login_fatal_log.current_active_object.dbg_handler.start_exe - login_fatal_log.current_active_object.dbg_handler.start_post);
	}
		break;

	case 'm': {
		uint32_t	flash_sys_log_address = APP_FLASH_DBG_SECTOR_1;
		for (uint32_t index = 0; index < 32; index++) {
			/* reset watchdog */
			sys_ctrl_independent_watchdog_reset();
			sys_ctrl_soft_watchdog_reset();

			flash_read(flash_sys_log_address, (uint8_t*)&t_msg, sizeof(ak_msg_t));
			flash_sys_log_address += sizeof(ak_msg_t);

			uint32_t wait_time;
			if (t_msg.dbg_handler.start_exe >= t_msg.dbg_handler.start_post) {
				wait_time = t_msg.dbg_handler.start_exe - t_msg.dbg_handler.start_post;
			}
			else {
				wait_time = t_msg.dbg_handler.start_exe + (0xFFFFFFFF - t_msg.dbg_handler.start_post);
			}

			uint32_t exe_time;
			if (t_msg.dbg_handler.stop_exe >= t_msg.dbg_handler.start_exe) {
				exe_time = t_msg.dbg_handler.stop_exe - t_msg.dbg_handler.start_exe;
			}
			else {
				exe_time = t_msg.dbg_handler.stop_exe + (0xFFFFFFFF - t_msg.dbg_handler.start_exe);
			}

			LOGIN_PRINT("index: %d\ttask_id: %d\tmsg_type:0x%x\tref_count:%d\tsig: %d\t\twait_time: %d\texe_time: %d\n"\
						, index										\
						, t_msg.task_id								\
						, (t_msg.ref_count & AK_MSG_TYPE_MASK)		\
						, (t_msg.ref_count & AK_MSG_REF_COUNT_MASK)	\
						, t_msg.sig									\
						, (wait_time)								\
						, (exe_time));
		}
	}
		break;

	default:
		break;
	}

	return 0;
}

int32_t shell_status(uint8_t* argv) {
	(void)argv;
	uint8_t		h;
	uint8_t		m;
	uint8_t		s;
	uint8_t		D;
	uint8_t		M;
	uint32_t	Y;

	uint8_t		temp;

	uint32_t	current_air_cond_1;
	uint32_t	current_air_cond_2;
	uint32_t	current_air_cond_3;
	uint32_t	current_air_cond_4;

	current_air_cond_1 = air_cond[0].available ? (1000 * ct_sensor1.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
	current_air_cond_2 = air_cond[1].available ? (1000 * ct_sensor2.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
	current_air_cond_3 = air_cond[2].available ? (1000 * ct_sensor3.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
	current_air_cond_4 = air_cond[3].available ? (1000 * ct_sensor4.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;

	Time t(2099, 1, 1, 0, 0, 0, Time::Day::kMonday);
	t = rtc_ds1302.time();

	h = t.hr;
	m = t.min;
	s = t.sec;
	D = t.day;
	M = t.mon;
	Y = t.yr;

	temp = app_setting.operations_calib_temp ? ((float)thermistor.read_kalman() / 10 + app_setting.temp_calibration) : ((float)thermistor.read_kalman() / 10 - app_setting.temp_calibration);

	LOGIN_PRINT("SETTINGS:\n\n");
	LOGIN_PRINT("milestone_temp_cool:\t\t%d\n", app_setting.milestone_temp_cool);
	LOGIN_PRINT("milestone_temp_normal:\t\t%d\n", app_setting.milestone_temp_normal);
	LOGIN_PRINT("milestone_temp_hot:\t\t%d\n", app_setting.milestone_temp_hot);

	LOGIN_PRINT("total air-cond:\t\t\t%d\n", app_setting.total_air_cond);

	LOGIN_PRINT("time_air_range:\t\t\t%d\n", app_setting.time_air_range);
	LOGIN_PRINT("time_air_counter:\t\t%d\n", app_setting.time_air_counter);

	LOGIN_PRINT("temp_calibration:\t\t%d\n", app_setting.temp_calibration);
	LOGIN_PRINT("hum_calibration:\t\t%d\n", app_setting.hum_calibration);
	LOGIN_PRINT("operations_calib_temp:\t\t%d\n", app_setting.operations_calib_temp);
	LOGIN_PRINT("operations_calib_hum:\t\t%d\n\n", app_setting.operations_calib_hum);

	LOGIN_PRINT("STATUS:\n\n");
	air_cond_t log_air_cond[TOTAL_AIR_COND];
	eeprom_read(EEPROM_AIR_COND_START_BASE_ADDR, (uint8_t*)log_air_cond, TOTAL_AIR_COND * sizeof(air_cond_t));
	for (uint8_t index = 0; index < TOTAL_AIR_COND; index++) {
		LOGIN_PRINT("%d\t%d\t%d\t%d\t%d\n", index, log_air_cond[index].available, log_air_cond[index].active, log_air_cond[index].failed_counter, log_air_cond[index].milestone_on);
	}

	LOGIN_PRINT("\n\n%d:%d:%d\t%d:%d:%d  \t\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n\n"	\
				, h	\
				, m	\
				, s	\
				, D	\
				, M	\
				, Y	\
				, temp	\
				, current_air_cond_1	\
				, current_air_cond_2	\
				, current_air_cond_3	\
				, current_air_cond_4);

	return 0;
}

int32_t shell_epi(uint8_t* argv) {
	(void)argv;
	ak_msg_t* s_msg = get_common_msg();
	set_msg_sig(s_msg, AC_SETTING_EEPROM_INIT_UPDATE);
	task_post(AC_TASK_SETTING_ID, s_msg);
	return 0;
}

int32_t shell_eps(uint8_t* argv) {
	uint8_t val = 0;

	switch (*(argv + 4)) {
	case 'd': {					/* data DEC format */
		LOGIN_PRINT("\n");
		for(uint32_t i = 0; i < EEPROM_END_ADDR; i++) {
			if (!(i%16)) {
				/* reset watchdog */
				sys_ctrl_independent_watchdog_reset();
				sys_ctrl_soft_watchdog_reset();

				LOGIN_PRINT("\n0x%x\t" ,i);
			}
			eeprom_read(i, &val, sizeof(uint8_t));
			LOGIN_PRINT("%d\t", val);
		}
		LOGIN_PRINT("\n");
	}
		break;

	case 'h': {					/* data HEX format */
		LOGIN_PRINT("\n");
		for(uint32_t i = 0; i < EEPROM_END_ADDR; i++) {
			if (!(i%16)) {
				/* reset watchdog */
				sys_ctrl_independent_watchdog_reset();
				sys_ctrl_soft_watchdog_reset();

				LOGIN_PRINT("\n0x%x\t" ,i);
			}
			eeprom_read(i, &val, sizeof(uint8_t));
			LOGIN_PRINT("0x%x\t", val);
		}
		LOGIN_PRINT("\n");
	}
		break;

	case 'r': {
		LOGIN_PRINT("erasing...\n");
		eeprom_erase(EEPROM_START_ADDR, EEPROM_END_ADDR - EEPROM_START_ADDR);
		LOGIN_PRINT("completed\n");
	}
		break;

	default:
		LOGIN_PRINT("unkown option !\n");
		break;
	}

	return 0;
}

int32_t shell_flash(uint8_t* argv) {
	switch (*(argv + 6)) {

	case 'r':
		LOGIN_PRINT("flash erasing...\n");
		flash_erase_full();
		LOGIN_PRINT("completed\n");
		break;

	default:
		LOGIN_PRINT("unknow option\n");
		break;
	}

	return 0;
}
