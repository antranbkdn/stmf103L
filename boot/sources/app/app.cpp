/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   05/02/2017
 ******************************************************************************
**/
/* driver include */
#include "../driver/led/led.h"
#include "../driver/flash/flash.h"

/* app include */
#include "app.h"
#include "app_dbg.h"
#include "app_data.h"
#include "app_flash.h"

/* sys include */
#include "../sys/sys_irq.h"
#include "../sys/sys_io.h"
#include "../sys/sys_ctrl.h"
#include "../sys/sys_dbg.h"
#include "../sys/sys_arduino.h"

led_t led_life;

static void erase_internal_flash(uint32_t address, uint32_t len);
static void jump_to_application();

int boot_main() {
	uint32_t temp;
	uint32_t index;
	uint32_t flash_status;
	firmware_header_t external_flash_firmware;

	APP_PRINT("[BOOT] started\n");

	/**************************************************************************
	* hardware configure
	***************************************************************************/
	SPI.begin();
	flash_io_ctrl_init();
	sys_ctrl_independent_watchdog_init();	/* 32s */


	/**************************************************************************
	* software instansce configure
	***************************************************************************/
	led_init(&led_life, led_life_init, led_life_on, led_life_off);


	/**************************************************************************
	* boot
	***************************************************************************/
	/**
	 * read firmware info on external flash
	 */
	flash_read(APP_FLASH_FIRMWARE_INFO_SECTOR_1, (uint8_t*)&external_flash_firmware, sizeof(firmware_header_t));

	/**
	 * if new flash, the (psk != FIRMWARE_PSK) is true,
	 * if checksum = 0 && binary file size = 0, firmware is updated.
	 */
	if (external_flash_firmware.psk != FIRMWARE_PSK) {

		APP_PRINT("[BOOT] start application\n");
		jump_to_application();
	}

	/* update firmware */
	else if (external_flash_firmware.checksum != 0
			 &&  external_flash_firmware.bin_len != 0){
		/**
		 * unlock flash and clear all pendings flash's status
		 */
		FLASH_Unlock();
		FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
						FLASH_FLAG_SIZERR | FLASH_FLAG_OPTVERR | FLASH_FLAG_OPTVERRUSR);

		/**
		 * erase application internal flash, prepare for new firmware
		 */
		APP_PRINT("[BOOT] erase internal flash\n");
		erase_internal_flash(NORMAL_START_ADDRESS, external_flash_firmware.bin_len);

		/**
		 * copy firmware data from external flash to internal flash
		 */
		APP_PRINT("[BOOT] copy firmware from external flash\n");

		/* update led status */
		led_blink_set(&led_life, 1000, 980);		/* led flash with duty 1s*/

		index = 0;
		while (index < external_flash_firmware.bin_len) {
			temp = 0;
			sys_ctrl_independent_watchdog_reset();

			flash_read(APP_FLASH_FIRMWARE_START_ADDR + index, (uint8_t*)&temp, sizeof(uint32_t));

			flash_status = FLASH_FastProgramWord(NORMAL_START_ADDRESS + index, temp);

			if(flash_status == FLASH_COMPLETE) {
				index += sizeof(uint32_t);
			}
			else {
				FLASH_ClearFlag(FLASH_FLAG_EOP|FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
								FLASH_FLAG_SIZERR | FLASH_FLAG_OPTVERR | FLASH_FLAG_OPTVERRUSR);
			}
		}

		/**
		 * calculate checksum, if its incorrectly, restart system and update again
		 */
		uint32_t internal_flash_checksum_cal = 0;
		for (index = 0; index < external_flash_firmware.bin_len; index += sizeof(uint32_t)) {
			internal_flash_checksum_cal += *((uint32_t*)(NORMAL_START_ADDRESS + index));
		}

		uint16_t internal_flash_checksum = (uint16_t)(internal_flash_checksum_cal & 0xFFFF);
		if (internal_flash_checksum != external_flash_firmware.checksum) {
			APP_PRINT("[BOOT] internal checksum incorrect\n");
			APP_PRINT("[BOOT] system restart\n");
			sys_ctrl_delay_ms(1000);
			sys_ctrl_reset();
		}
		else {
			APP_PRINT("[BOOT] internal checksum correctly\n");

			/**
			 * update firmware is completed
			 * reset temporary firmware information in external flash
			 */
			external_flash_firmware.psk = FIRMWARE_LOK;

			flash_erase_sector(APP_FLASH_FIRMWARE_INFO_SECTOR_1);
			flash_write(APP_FLASH_FIRMWARE_INFO_SECTOR_1, (uint8_t*)&external_flash_firmware, sizeof(firmware_header_t));

			led_blink_reset(&led_life);
			led_off(&led_life);

			APP_PRINT("[BOOT] start application\n");
			jump_to_application();
		}
	}
	else {
		/**
		 * unexpected status
		 */
		led_blink_set(&led_life, 500, 480);		/* led flash with duty 0.5s*/
	}

	return 0;
}

/**
 * @brief erase_internal_flash
 * @param address
 * @param len
 */
void erase_internal_flash(uint32_t address, uint32_t len) {
	uint32_t page_number;
	uint32_t index;

	page_number = len / 256;

	if ((page_number * 256) < len) {
		page_number++;
	}

	for (index = 0; index < page_number; index++) {
		FLASH_ErasePage(address + (index * 256));
	}
}

/**
 * @brief jump_to_application
 */
void jump_to_application() {
	volatile uint32_t normal_stack_pointer	=	(uint32_t) *(volatile uint32_t*)(NORMAL_START_ADDRESS);
	volatile uint32_t normal_jump_address	=	(uint32_t) *(volatile uint32_t*)(NORMAL_START_ADDRESS + 4);

	p_jump_func jump_to_normal = (p_jump_func)normal_jump_address;

	SCB->VTOR = NORMAL_START_ADDRESS;

	__asm volatile ("MSR msp, %0\n" : : "r" (normal_stack_pointer) : "sp");
	jump_to_normal();
	while(1);
}

/**
 * @brief sys_irq_timer_10ms
 */
void sys_irq_timer_10ms() {
	led_blink_polling(&led_life);
}
