/**
 ******************************************************************************
 * @Author: ThanNT
 * @Date:   05/09/2016
 ******************************************************************************
**/
#include "sys_dbg.h"
#include "sys_ctrl.h"

#include "../ak/ak.h"

#include "../driver/eeprom/eeprom.h"
#include "../driver/flash/flash.h"

#include "../app/app_eeprom.h"
#include "../app/app_flash.h"

#include "../common/utils.h"

#if defined(STM32L_PLATFORM)
#include "../common/xprintf.h"
#include "../platform/stm32l/io_cfg.h"
#include "../platform/stm32l/sys_cfg.h"
#elif defined(STM32F10X_PLATFORM)
#include "../common/xprintf.h"
#include "../platform/stm32f10x/io_cfg.h"
#include "../platform/stm32f10x/sys_cfg.h"
#else
#error Please choose platform for kernel.
#endif

static fatal_log_t fatal_log;

void sys_dbg_fatal(const int8_t* s, uint8_t c) {
	unsigned char rev_c = 0;
	task_t*		ptemp_current_task;
	ak_msg_t*	ptemp_current_active_object;
	ak_msg_t	t_msg;
	uint32_t	flash_sys_log_address = APP_FLASH_DBG_SECTOR_1;

#if defined(TIVA_PLATFORM)
	UARTprintf("%s\t%x\n", s, c);
#endif
#if defined(STM32L_PLATFORM) || defined(STM32F10X_PLATFORM)
	xprintf("%s\t%x\n", s, c);
#endif
	/* read fatal data from epprom */
	eeprom_read(EEPROM_FATAL_LOG_ADDR, (uint8_t*)&fatal_log, sizeof(fatal_log_t));

	/* increase fatal time */
	fatal_log.fatal_times ++;

	/* set fatal string */
	mem_set(fatal_log.string, 0, 10);
	str_cpy(fatal_log.string, s);

	/* set fatal code */
	fatal_log.code = c;

	/* get task fatal */
	ptemp_current_task = get_current_task_info();

	/* get active object fatal */
	ptemp_current_active_object = get_current_active_object();

	mem_cpy(&fatal_log.current_task, ptemp_current_task, sizeof(task_t));
	mem_cpy(&fatal_log.current_active_object, ptemp_current_active_object, sizeof(ak_msg_t));

	/* rewrite fatal data to epprom */
	eeprom_write(EEPROM_FATAL_LOG_ADDR, (uint8_t*)&fatal_log, sizeof(fatal_log_t));

	/* erase sys log sector */
	flash_erase_sector(flash_sys_log_address);

	SYS_PRINT("start write system log to flash\n");

	while(log_queue_len(&log_task_dbg_object_queue)) {
		log_queue_get(&log_task_dbg_object_queue, &t_msg);
		flash_write(flash_sys_log_address, (uint8_t*)&t_msg, sizeof(ak_msg_t));
		flash_sys_log_address += sizeof(ak_msg_t);
	}

	sys_ctrl_delay_us(1000);

#if defined(RELEASE)
	sys_ctrl_reset();
#endif

	/* Disable all interrupt */
	DISABLE_INTERRUPTS();

	while(1) {
		/* reset watchdog */
		sys_ctrl_independent_watchdog_reset();
		sys_ctrl_soft_watchdog_reset();

		/* FATAL debug option */
		rev_c = sys_ctrl_shell_get_char();
		if (rev_c) {
			switch (rev_c) {
			case 'r':
				sys_ctrl_reset();
				break;

			default:
				break;
			}
		}

		/* led notify FATAL */
		led_life_on();
		sys_ctrl_delay_us(200000);
		led_life_off();
		sys_ctrl_delay_us(200000);
	}
}

