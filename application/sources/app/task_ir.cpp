#include "../ak/fsm.h"
#include "../ak/port.h"
#include "../ak/timer.h"
#include "../ak/message.h"

#include "../sys/sys_dbg.h"
#include "../sys/sys_io.h"

#include "../driver/flash/flash.h"

#include "../common/utils.h"

#include "app.h"
#include "app_dbg.h"
#include "app_eeprom.h"
#include "app_flash.h"

#include "task_list.h"
#include "task_ir.h"
#include "task_time.h"
#include "task_setting.h"

#define IR_SEND_DUMP_EN			0
#define IR_DECODE_DUMP_EN		0

ir_t  ir;
app_airconds_ir_cmd_info_t app_airconds_ir_cmd_info;

static uint16_t ir_cmd_buf[IR_BUF_MAX_SIZE];

static uint8_t air_cond_decode_active = 0;
static uint8_t air_cond_decoding_flag = APP_FLAG_OFF;
static uint8_t auto_ir_test_flag = APP_FLAG_OFF;
static uint8_t auto_air_calib_flag = APP_FLAG_OFF;

static void callback_ir_off(void*);
static void callback_ir_on(void*);

void task_ir(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_IR_AIR_COND_STATUS_REQ: {
		if (air_cond_decoding_flag == APP_FLAG_OFF && auto_air_calib_flag == APP_FLAG_OFF ) {
			for (uint8_t i = 0; i < app_setting.total_air_cond; i++) {
				/* select IR */
				ir_select_direction(i);

				if (air_cond_current_status[i] == AIR_COND_ON) {
					eeprom_read(EEPROM_APP_IR_HEADER_BASE_ADDR, (uint8_t*)&app_airconds_ir_cmd_info, sizeof(app_airconds_ir_cmd_info_t));

					if (app_airconds_ir_cmd_info.air_cond_ir_cmd[i].on.len >= IR_BUF_MAX_SIZE) {
						FATAL("IRSEND", 0x03);
					}

					flash_read(app_airconds_ir_cmd_info.air_cond_ir_cmd[i].on.address, (uint8_t*)ir_cmd_buf, app_airconds_ir_cmd_info.air_cond_ir_cmd[i].on.len * sizeof(uint16_t));
#if IR_SEND_DUMP_EN
					/* dump IR decode info for debug */
					APP_DBG("len = %d\n", app_airconds_ir_cmd_info.air_cond_ir_cmd[i].on.len);

					for (uint32_t j = 0; j < app_airconds_ir_cmd_info.air_cond_ir_cmd[i].on.len; j++) {
						APP_DBG("%d\t",ir_cmd_buf[j]);
						if (j % 20 == 0 && j > 2) {
							APP_DBG("\n");
						}
					}
					APP_DBG("\n");
#endif
					if (ir_send_rawdata(&ir, ir_cmd_buf, app_airconds_ir_cmd_info.air_cond_ir_cmd[i].on.len) == IR_DRIVER_NG) {
						FATAL("IRSEND", 0x02);
					}
				}
				else if (air_cond_current_status[i] == AIR_COND_OFF) {
					eeprom_read(EEPROM_APP_IR_HEADER_BASE_ADDR, (uint8_t*)&app_airconds_ir_cmd_info, sizeof(app_airconds_ir_cmd_info_t));

					if (app_airconds_ir_cmd_info.air_cond_ir_cmd[i].off.len >= IR_BUF_MAX_SIZE) {
						FATAL("IRSEND", 0x03);
					}

					flash_read(app_airconds_ir_cmd_info.air_cond_ir_cmd[i].off.address, (uint8_t*)ir_cmd_buf, app_airconds_ir_cmd_info.air_cond_ir_cmd[i].off.len * sizeof(uint16_t));
#if IR_SEND_DUMP_EN
					/* dump IR decode info for debug */
					APP_DBG("len = %d\n", app_airconds_ir_cmd_info.air_cond_ir_cmd[i].off.len);

					for (uint32_t j = 0; j < app_airconds_ir_cmd_info.air_cond_ir_cmd[i].off.len; j++) {
						APP_DBG("%d\t",ir_cmd_buf[j]);
						if (j % 20 == 0 && j > 2) {
							APP_DBG("\n");
						}
					}
					APP_DBG("\n");
#endif
					if (ir_send_rawdata(&ir, ir_cmd_buf, app_airconds_ir_cmd_info.air_cond_ir_cmd[i].off.len) == IR_DRIVER_NG) {
						FATAL("IRSEND", 0x02);
					}
				}
			}
		}

		if (auto_ir_test_flag == APP_FLAG_ON) {
			auto_ir_test_flag = APP_FLAG_OFF;
			ak_msg_t* s_msg = get_common_msg();
			set_msg_sig(s_msg, AC_UI_TEST_KEY_AUTO_END);
			task_post(AC_TASK_UI_ID, s_msg);
		}
	}
		break;

	case AC_IR_AIR_COND_START_DECODE_REQ: {
		/* clear IR header in epprom */
		app_airconds_ir_cmd_info_t bss_app_airconds_ir_cmd_info;
		mem_set(&bss_app_airconds_ir_cmd_info, 0, sizeof(app_airconds_ir_cmd_info_t));
		eeprom_write(EEPROM_APP_IR_HEADER_BASE_ADDR, (uint8_t*)&bss_app_airconds_ir_cmd_info, sizeof(app_airconds_ir_cmd_info_t));

		/* clear flash for decode */
		flash_erase_sector(APP_FLASH_IR_SECTOR_1);
		flash_erase_sector(APP_FLASH_IR_SECTOR_2);

		/* turn ON start decode flag */
		air_cond_decoding_flag = APP_FLAG_ON;
	}
		break;

	case AC_IR_AIR_COND_STOP_DECODE_REQ:
		/* turn ON start decode flag */
		ir_decode_exit();
		air_cond_decoding_flag = APP_FLAG_OFF;
		break;

	case AC_IR_AIR_COND_ON_DECODE_REQ:
		air_cond_decode_active = (get_data_common_msg(msg))[0];
		APP_DBG("air_cond_decode_active: %d\n", air_cond_decode_active);

		ir_decode_callback_register(&ir, callback_ir_on);

		if (ir_decode_start(&ir) == IR_DRIVER_OK) {
			APP_DBG("start decode ON key OK!\n");
		}
		else {
			APP_DBG("start decode ON key FAILED!\n");
		}
		break;

	case AC_IR_AIR_COND_OFF_DECODE_REQ:
		air_cond_decode_active = (get_data_common_msg(msg))[0];
		APP_DBG("air_cond_decode_active: %d\n", air_cond_decode_active);

		ir_decode_callback_register(&ir, callback_ir_off);

		if (ir_decode_start(&ir) == IR_DRIVER_OK) {
			APP_DBG("start decode OFF key OK!\n");
		}
		else {
			APP_DBG("start decode OFF key FAILED!\n");
		}

		break;

	case AC_IR_AIR_COND_ON_DECODE_RES: {
		uint32_t me_ir_addr = *((uint32_t*)get_data_common_msg(msg));
		ir_t* me = (ir_t*)me_ir_addr;
#if IR_DECODE_DUMP_EN
		/* dump IR decode info for debug */
		APP_DBG("len = %d\n", me->bit_len);

		for (uint32_t i = 0; i < me->bit_len; i++) {
			APP_DBG("%d\t", me->buf[i]);
			if (i % 20 == 0 && i > 2) {
				APP_DBG("\n");
			}
		}
		APP_DBG("\n");
#endif
		eeprom_read(EEPROM_APP_IR_HEADER_BASE_ADDR, (uint8_t*)&app_airconds_ir_cmd_info, sizeof(app_airconds_ir_cmd_info_t));

		APP_DBG("writing data key ON . . .\n");

		if (air_cond_decode_active > 0) {
			app_airconds_ir_cmd_info.air_cond_ir_cmd[air_cond_decode_active].on.address = app_airconds_ir_cmd_info.air_cond_ir_cmd[air_cond_decode_active - 1].off.address + app_airconds_ir_cmd_info.air_cond_ir_cmd[air_cond_decode_active - 1].off.len * sizeof(uint16_t);
			app_airconds_ir_cmd_info.air_cond_ir_cmd[air_cond_decode_active].on.len = me->bit_len;

			/* update header to epprom */
			eeprom_write(EEPROM_APP_IR_HEADER_BASE_ADDR, (uint8_t*)&app_airconds_ir_cmd_info, sizeof(app_airconds_ir_cmd_info_t));

			/* write data decode to flash */
			flash_write(app_airconds_ir_cmd_info.air_cond_ir_cmd[air_cond_decode_active].on.address, (uint8_t*)me->buf, (me->bit_len) * sizeof(uint16_t));
		}
		else {
			app_airconds_ir_cmd_info.air_cond_ir_cmd[0].on.address = APP_FLASH_IR_SECTOR_1;
			app_airconds_ir_cmd_info.air_cond_ir_cmd[0].on.len = me->bit_len;

			/* update header to epprom */
			eeprom_write(EEPROM_APP_IR_HEADER_BASE_ADDR, (uint8_t*)&app_airconds_ir_cmd_info, sizeof(app_airconds_ir_cmd_info_t));

			/* write data decode to flash */
			flash_write(app_airconds_ir_cmd_info.air_cond_ir_cmd[0].on.address, (uint8_t*)me->buf, (me->bit_len) * sizeof(uint16_t));
		}

		APP_DBG("writing data key ON completed\n");

		/* send msg to TASK_UI when decode and write data key ON complete*/
		{
			ak_msg_t* s_msg = get_common_msg();
			set_msg_sig(s_msg, AC_UI_KEY_ON_DECODE_OK);
			task_post(AC_TASK_UI_ID, s_msg);
		}
	}
		break;

	case AC_IR_AIR_COND_OFF_DECODE_RES: {
		uint32_t me_ir_addr = *((uint32_t*)get_data_common_msg(msg));
		ir_t* me = (ir_t*)me_ir_addr;

#if IR_DECODE_DUMP_EN
		/* dump IR decode info for debug */
		APP_DBG("len = %d\n", me->bit_len);

		for (uint32_t i = 0; i < me->bit_len; i++) {
			APP_DBG("%d\t", me->buf[i]);
			if (i % 20 == 0 && i > 2) {
				APP_DBG("\n");
			}
		}

		APP_DBG("\n");
#endif
		eeprom_read(EEPROM_APP_IR_HEADER_BASE_ADDR, (uint8_t*)&app_airconds_ir_cmd_info, sizeof(app_airconds_ir_cmd_info_t));

		APP_DBG("writing data key OFF . . .\n");

		app_airconds_ir_cmd_info.air_cond_ir_cmd[air_cond_decode_active].off.address = app_airconds_ir_cmd_info.air_cond_ir_cmd[air_cond_decode_active].on.address + app_airconds_ir_cmd_info.air_cond_ir_cmd[air_cond_decode_active].on.len * sizeof(uint16_t);
		app_airconds_ir_cmd_info.air_cond_ir_cmd[air_cond_decode_active].off.len = me->bit_len;

		/* update header to epprom */
		eeprom_write(EEPROM_APP_IR_HEADER_BASE_ADDR, (uint8_t*)&app_airconds_ir_cmd_info, sizeof(app_airconds_ir_cmd_info_t));

		/* write data decode to flash */
		flash_write(app_airconds_ir_cmd_info.air_cond_ir_cmd[air_cond_decode_active].off.address, (uint8_t*)me->buf, (me->bit_len) * sizeof(uint16_t));


		APP_DBG("writing data key OFF completed\n");

		/* send msg to TASK_UI when decode and write data key OFF complete*/
		{
			ak_msg_t* s_msg = get_common_msg();
			set_msg_sig(s_msg, AC_UI_KEY_OFF_DECODE_OK);
			task_post(AC_TASK_UI_ID, s_msg);
		}
	}
		break;

	case AC_IR_AIR_COND_HAND_TEST_KEY_REQ: {
		uint8_t* hand_test = ((uint8_t*)get_data_common_msg(msg));

		ir_select_direction(hand_test[0]);

		if (hand_test[1] == AIR_COND_ON) {
			eeprom_read(EEPROM_APP_IR_HEADER_BASE_ADDR, (uint8_t*)&app_airconds_ir_cmd_info, sizeof(app_airconds_ir_cmd_info_t));

			if (app_airconds_ir_cmd_info.air_cond_ir_cmd[hand_test[0]].on.len >= IR_BUF_MAX_SIZE) {
				FATAL("IRSEND", 0x03);
			}

			flash_read(app_airconds_ir_cmd_info.air_cond_ir_cmd[hand_test[0]].on.address, (uint8_t*)ir_cmd_buf, app_airconds_ir_cmd_info.air_cond_ir_cmd[hand_test[0]].on.len * sizeof(uint16_t));

			if (ir_send_rawdata(&ir, ir_cmd_buf, app_airconds_ir_cmd_info.air_cond_ir_cmd[hand_test[0]].on.len) == IR_DRIVER_NG) {
				FATAL("IRSEND", 0x02);
			}
		}
		else if (hand_test[1] == AIR_COND_OFF) {
			eeprom_read(EEPROM_APP_IR_HEADER_BASE_ADDR, (uint8_t*)&app_airconds_ir_cmd_info, sizeof(app_airconds_ir_cmd_info_t));

			if (app_airconds_ir_cmd_info.air_cond_ir_cmd[hand_test[0]].off.len >= IR_BUF_MAX_SIZE) {
				FATAL("IRSEND", 0x03);
			}

			flash_read(app_airconds_ir_cmd_info.air_cond_ir_cmd[hand_test[0]].off.address, (uint8_t*)ir_cmd_buf, app_airconds_ir_cmd_info.air_cond_ir_cmd[hand_test[0]].off.len * sizeof(uint16_t));

			if (ir_send_rawdata(&ir, ir_cmd_buf, app_airconds_ir_cmd_info.air_cond_ir_cmd[hand_test[0]].off.len) == IR_DRIVER_NG) {
				FATAL("IRSEND", 0x02);
			}
		}
	}
		break;

	case AC_IR_AIR_COND_AUTO_TEST_KEY_REQ: {
		mem_set(air_cond_current_status, AIR_COND_ON, app_setting.total_air_cond);
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_IR_AIR_COND_STATUS_REQ);
		task_post(AC_TASK_IR_ID, s_msg);

		timer_set(AC_TASK_IR_ID, AC_IR_TIMER_AUTO_TEST, AC_AC_IR_TIMER_AUTO_TEST_INTERVAL, TIMER_ONE_SHOT);
	}
		break;

	case AC_IR_TIMER_AUTO_TEST: {
		auto_ir_test_flag = APP_FLAG_ON;
		mem_set(air_cond_current_status, AIR_COND_OFF, app_setting.total_air_cond);
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_IR_AIR_COND_STATUS_REQ);
		task_post(AC_TASK_IR_ID, s_msg);
	}
		break;

	case AC_IR_AIR_COND_AUTO_CALIB_START_REQ : {
		auto_air_calib_flag = APP_FLAG_ON;

		if (air_cond_decoding_flag == APP_FLAG_OFF && auto_air_calib_flag == APP_FLAG_ON ) {
			for (uint8_t i = 0; i < app_setting.total_air_cond; i++) {
				/* select IR */
				ir_select_direction(i);

				if (air_cond_current_status[i] == AIR_COND_ON) {
					eeprom_read(EEPROM_APP_IR_HEADER_BASE_ADDR, (uint8_t*)&app_airconds_ir_cmd_info, sizeof(app_airconds_ir_cmd_info_t));

					if (app_airconds_ir_cmd_info.air_cond_ir_cmd[i].on.len >= IR_BUF_MAX_SIZE) {
						FATAL("IRSEND", 0x03);
					}

					flash_read(app_airconds_ir_cmd_info.air_cond_ir_cmd[i].on.address, (uint8_t*)ir_cmd_buf, app_airconds_ir_cmd_info.air_cond_ir_cmd[i].on.len * sizeof(uint16_t));

					if (ir_send_rawdata(&ir, ir_cmd_buf, app_airconds_ir_cmd_info.air_cond_ir_cmd[i].on.len) == IR_DRIVER_NG) {
						FATAL("IRSEND", 0x02);
					}
				}
				else if (air_cond_current_status[i] == AIR_COND_OFF) {
					eeprom_read(EEPROM_APP_IR_HEADER_BASE_ADDR, (uint8_t*)&app_airconds_ir_cmd_info, sizeof(app_airconds_ir_cmd_info_t));

					if (app_airconds_ir_cmd_info.air_cond_ir_cmd[i].off.len >= IR_BUF_MAX_SIZE) {
						FATAL("IRSEND", 0x03);
					}

					flash_read(app_airconds_ir_cmd_info.air_cond_ir_cmd[i].off.address, (uint8_t*)ir_cmd_buf, app_airconds_ir_cmd_info.air_cond_ir_cmd[i].off.len * sizeof(uint16_t));

					if (ir_send_rawdata(&ir, ir_cmd_buf, app_airconds_ir_cmd_info.air_cond_ir_cmd[i].off.len) == IR_DRIVER_NG) {
						FATAL("IRSEND", 0x02);
					}
				}
			}
		}

	}
		break;
	case AC_IR_AIR_COND_AUTO_CALIB_END_REQ :
		auto_air_calib_flag = APP_FLAG_OFF;
		break;

	default:
		FATAL("TKIR", 0x01);
		break;
	}
}

/* callback when decode suscess key on*/
void callback_ir_on(void* _me) {

	ir_t* me = (ir_t*)_me;

	switch (me->decode_result) {
	case IR_DECODE_SUCCESS: {
		uint32_t me_addr = (uint32_t)me;
		{
			ak_msg_t* s_msg = get_common_msg();
			set_msg_sig(s_msg, AC_UI_KEY_ON_SAVING);
			task_post(AC_TASK_UI_ID, s_msg);
		}

		{
			ak_msg_t* s_msg = get_common_msg();
			set_msg_sig(s_msg, AC_IR_AIR_COND_ON_DECODE_RES);
			set_data_common_msg(s_msg, (uint8_t*)&me_addr, sizeof(uint32_t));
			task_post(AC_TASK_IR_ID, s_msg);
		}
	}
		break;

	case IR_DECODE_ERR_REV: {
		/* send msg to TASK_UI when decode and write data key ON fail*/
		ak_msg_t* s_msg = get_common_msg();
		set_msg_sig(s_msg, AC_UI_KEY_ON_DECODE_FAIL);
		task_post(AC_TASK_UI_ID, s_msg);
	}
		break;

	case IR_DECODE_ERR_LEN: {
		/* send msg to TASK_UI when decode and write data key ON fail*/
		ak_msg_t* s_msg = get_common_msg();
		set_msg_sig(s_msg, AC_UI_KEY_ON_DECODE_FAIL);
		task_post(AC_TASK_UI_ID, s_msg);
	}
		break;

	default:
		break;
	}
}

/* callback when decode suscess key off*/
void callback_ir_off(void* _me) {

	ir_t* me = (ir_t*)_me;

	switch (me->decode_result) {
	case IR_DECODE_SUCCESS: {
		uint32_t me_addr = (uint32_t)me;
		{
			ak_msg_t* s_msg = get_common_msg();
			set_msg_sig(s_msg, AC_UI_KEY_OFF_SAVING);
			task_post(AC_TASK_UI_ID, s_msg);
		}
		ak_msg_t* s_msg = get_common_msg();
		set_msg_sig(s_msg, AC_IR_AIR_COND_OFF_DECODE_RES);
		set_data_common_msg(s_msg, (uint8_t*)&me_addr, sizeof(uint32_t));
		task_post(AC_TASK_IR_ID, s_msg);
	}
		break;

	case IR_DECODE_ERR_REV: {
		/* send msg to TASK_UI when decode and write data key OFF fail*/
		ak_msg_t* s_msg = get_common_msg();
		set_msg_sig(s_msg, AC_UI_KEY_OFF_DECODE_FAIL);
		task_post(AC_TASK_UI_ID, s_msg);
	}
		break;

	case IR_DECODE_ERR_LEN: {
		/* send msg to TASK_UI when decode and write data key OFF fail*/
		ak_msg_t* s_msg = get_common_msg();
		set_msg_sig(s_msg, AC_UI_KEY_OFF_DECODE_FAIL);
		task_post(AC_TASK_UI_ID, s_msg);
	}
		break;

	default:
		break;
	}
}
