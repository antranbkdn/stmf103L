#include "../ak/ak.h"
#include "../ak/port.h"
#include "../ak/message.h"
#include "../ak/timer.h"

#include "../sys/sys_dbg.h"
#include "../sys/sys_ctrl.h"

#include "../driver/flash/flash.h"

#include "../common/utils.h"

#include "app.h"
#include "app_if.h"
#include "app_dbg.h"
#include "app_data.h"
#include "app_flash.h"

#include "task_if.h"
#include "task_list.h"
#include "task_list_if.h"
#include "task_firmware.h"

/* transfer firmware info */
static firmware_header_t firmware_header_file;

/* position of transfer firmware info */
static uint32_t bin_file_cursor;

static void write_firmware_info_flash(firmware_header_t* firmware_info);

void task_firmware(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_FIRMWARE_CHECKING_REQ: {
		APP_DBG("AC_FIRMWARE_CHECKING_REQ\n");

		firmware_header_t firmware_info;
		flash_read(APP_FLASH_FIRMWARE_INFO_SECTOR_1, (uint8_t*)&firmware_info, sizeof(firmware_header_t));

		if (firmware_info.psk == FIRMWARE_LOK) {
			firmware_info.psk = 0;
			firmware_info.checksum = 0;
			firmware_info.bin_len = 0;

			flash_erase_sector(APP_FLASH_FIRMWARE_INFO_SECTOR_1);
			flash_write(APP_FLASH_FIRMWARE_INFO_SECTOR_1, (uint8_t*)&firmware_info, sizeof(firmware_header_t));

			ak_msg_t* s_msg = get_pure_msg();

			set_if_type(s_msg, IF_TYPE_RF24);
			set_if_task_id(s_msg, GW_TASK_FIRMWARE_ID);
			set_if_sig(s_msg, GW_FIRMWARE_UPDATE_COMPLETED);

			set_msg_sig(s_msg, AC_IF_PURE_MSG_OUT);
			task_post(AC_TASK_IF_ID, s_msg);
		}
	}
		break;

	case AC_FIRMWARE_CURRENT_INFO_REQ: {
		APP_DBG("AC_FIRMWARE_CURRENT_INFO_REQ\n");

		firmware_header_t firmware_header_req;
		sys_ctrl_get_firmware_info(&firmware_header_req);

		ak_msg_t* s_msg = get_common_msg();

		set_if_type(s_msg, IF_TYPE_RF24);
		set_if_task_id(s_msg, GW_TASK_FIRMWARE_ID);
		set_if_sig(s_msg, GW_FIRMWARE_CURRENT_INFO_RES);
		set_if_data_common_message(s_msg, (uint8_t*)&firmware_header_req, sizeof(firmware_header_t));

		set_msg_sig(s_msg, AC_IF_COMMON_MSG_OUT);
		task_post(AC_TASK_IF_ID, s_msg);
	}
		break;

	case AC_FIRMWARE_UPDATE_REQ: {
		APP_DBG("AC_FIRMWARE_UPDATE_REQ\n");

		mem_cpy(&firmware_header_file, get_data_common_msg(msg), sizeof(firmware_header_t));
		APP_DBG("firmware_header_file.checksum:%04X\n", firmware_header_file.checksum);
		APP_DBG("firmware_header_file.bin_len:%d\n", firmware_header_file.bin_len);

		/* reset bin_file_cursor */
		bin_file_cursor = 0;

		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_SM_FIRMWARE_UPDATE_REQ);
		task_post(AC_TASK_SM_ID, s_msg);
	}
		break;

	case AC_FIRMWARE_UPDATE_SM_OK: {

		/* clear flash loader */
		for (int i = 0; i < APP_FLASH_FIRMWARE_BLOCK_64K_SIZE; i++) {
			flash_erase_block_64k(APP_FLASH_FIRMWARE_START_ADDR + (FLASH_BLOCK_64K_SIZE * i));
		}
		APP_DBG("erase temp OK\n");

		ak_msg_t* s_msg = get_pure_msg();

		set_if_type(s_msg, IF_TYPE_RF24);
		set_if_task_id(s_msg, GW_TASK_FIRMWARE_ID);
		set_if_sig(s_msg, GW_FIRMWARE_UPDATE_RES_OK);

		set_msg_sig(s_msg, AC_IF_PURE_MSG_OUT);
		task_post(AC_TASK_IF_ID, s_msg);
	}
		break;

	case AC_FIRMWARE_UPDATE_SM_BUSY: {
		ak_msg_t* s_msg = get_pure_msg();

		set_if_type(s_msg, IF_TYPE_RF24);
		set_if_task_id(s_msg, GW_TASK_FIRMWARE_ID);
		set_if_sig(s_msg, GW_FIRMWARE_UPDATE_BUSY);

		set_msg_sig(s_msg, AC_IF_PURE_MSG_OUT);
		task_post(AC_TASK_IF_ID, s_msg);
	}
		break;

	case AC_FIRMWARE_TRANSFER_REQ: {
		sys_ctrl_independent_watchdog_reset();
		sys_ctrl_soft_watchdog_reset();

		timer_set(AC_TASK_FIRMWARE_ID, AC_FIRMWARE_PACKED_TIMEOUT, AC_TIMER_FIRMWARE_PACKED_TIMEOUT_INTERVAL, TIMER_ONE_SHOT);

		/* write firmware packet to external flash */
		uint8_t* firmware_packet = get_data_common_msg(msg);
		uint8_t firmware_packet_len = get_data_len_common_msg(msg);
		flash_write(APP_FLASH_FIRMWARE_START_ADDR + bin_file_cursor, firmware_packet, firmware_packet_len);

		/* increase transfer binary file cursor */
		bin_file_cursor += firmware_packet_len;

		/* send response message to getway */
		ak_msg_t* s_msg = get_pure_msg();

		/* transfer completed */
		if (bin_file_cursor >= firmware_header_file.bin_len) {
			timer_remove_attr(AC_TASK_FIRMWARE_ID, AC_FIRMWARE_PACKED_TIMEOUT);

			/* start calculate chechsum */
			uint32_t checksum_buffer = 0;
			uint32_t word = 0;

			APP_DBG("start calculate checksum\n");
			for (uint32_t index = 0; index < firmware_header_file.bin_len; index += sizeof(uint32_t)) {
				sys_ctrl_independent_watchdog_reset();
				sys_ctrl_soft_watchdog_reset();

				word = 0;
				flash_read(APP_FLASH_FIRMWARE_START_ADDR + index, (uint8_t*)&word, sizeof(uint32_t));
				checksum_buffer += word;
			}

			uint16_t checksum_calculated = (uint16_t)(checksum_buffer & 0xFFFF);
			APP_DBG("checksum_calculated:%04X\n", checksum_calculated);
			APP_DBG("checksum_transfer:%04X\n", firmware_header_file.checksum);

			/* checksum correctly */
			if (checksum_calculated == firmware_header_file.checksum) {
				APP_DBG("checksum correctly\n");
				write_firmware_info_flash(&firmware_header_file);

				set_if_sig(s_msg, GW_FIRMWARE_AC_INTERNAL_UPDATE_REQ);
			}
			/* checksum incorrect*/
			else {
				APP_DBG("checksum incorrect\n");
				set_if_sig(s_msg, GW_FIRMWARE_TRANSFER_RES_CHECKSUM_ERR);
			}
		}

		/* transferring countimue */
		else {
			set_if_sig(s_msg, GW_FIRMWARE_TRANSFER_RES_OK);
		}

		set_if_type(s_msg, IF_TYPE_RF24);
		set_if_task_id(s_msg, GW_TASK_FIRMWARE_ID);

		set_msg_sig(s_msg, AC_IF_PURE_MSG_OUT);
		task_post(AC_TASK_IF_ID, s_msg);
	}
		break;

	case AC_FIRMWARE_PACKED_TIMEOUT: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_SM_FIRMWARE_RELEASE_REQ);
		task_post(AC_TASK_SM_ID, s_msg);
	}
		break;

	case AC_FIRMWARE_INTERNAL_UPDATE_RES_OK: {
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_SM_SAFE_MODE_REQ);
		task_post(AC_TASK_SM_ID, s_msg);
	}
		break;

	case AC_FIRMWARE_SAFE_MODE_RES_OK: {
		sys_ctrl_reset();
	}
		break;

	default:
		break;
	}
}

void write_firmware_info_flash(firmware_header_t* firmware_info) {
	firmware_info->psk = FIRMWARE_PSK;

	flash_erase_sector(APP_FLASH_FIRMWARE_INFO_SECTOR_1);
	if (FLASH_DRIVER_OK == flash_write(APP_FLASH_FIRMWARE_INFO_SECTOR_1, (uint8_t*)firmware_info, sizeof(firmware_header_t))) {
		APP_DBG("write_firmware_info_flash OK\n");
	}
	else {
		APP_DBG("write_firmware_info_flash ERR\n");
	}
}
