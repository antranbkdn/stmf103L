/**
 ******************************************************************************
 * @Author: ThanNT
 * @Date:   13/08/2016
 ******************************************************************************
**/

/* kernel include */
#include "../ak/ak.h"
#include "../ak/message.h"
#include "../ak/timer.h"
#include "../ak/fsm.h"

/* driver include */
#include "../driver/led/led.h"
#include "../driver/button/button.h"
#include "../driver/flash/flash.h"
#include "../driver/hs1101/hs1101.h"

/* app include */
#include "app.h"
#include "app_dbg.h"
#include "app_screen.h"

#include "task_list.h"
#include "task_shell.h"
#include "task_sensor.h"
#include "task_sm.h"
#include "task_ui.h"
#include "task_life.h"
#include "task_ir.h"
#include "task_time.h"
#include "task_setting.h"
#include "task_if.h"

/* sys include */
#include "../sys/sys_irq.h"
#include "../sys/sys_io.h"
#include "../sys/sys_ctrl.h"
#include "../sys/sys_dbg.h"
#include "../sys/sys_arduino.h"

/* common include */
#include "../common/utils.h"
#include "../common/screen_manager.h"

static void app_start_timer();
static void app_init_state_machine();
static void app_task_init();

/*****************************************************************************/
/* app main function.
 */
/*****************************************************************************/
int main_app() {
	APP_PRINT("main_app() entry OK\n");

	/******************************************************************************
	* init active kernel
	*******************************************************************************/
	ENTRY_CRITICAL();
	task_init();
	task_create(app_task_table);
	EXIT_CRITICAL();

	/******************************************************************************
	* init applications
	*******************************************************************************/
	/*********************
	* hardware configure *
	**********************/
	/* init watch dog timer */
	sys_ctrl_independent_watchdog_init();	/* 32s */
	sys_ctrl_soft_watchdog_init(200);		/* 20s */

	SPI.begin();

	/* adc peripheral configure */
	io_cfg_adc1();			/* configure adc for thermistor and CT sensor */

	/* ir io & timer configure */
	timer_50us_init();		/* timer polling IR 50us */
	ir_rev_io_init();		/* io interrupt for IR */
	ir_dir_io_config();		/* config direction ir pin */
	ir_carrier_freq_init();	/* carrier frequency configure 38khz */

	/* adc configure for thermistor */
	thermistor.begin();

	/* adc configure for ct sensor */
	adc_ct_io_cfg();

	/* init humidity sensor */
	io_cfg_dac_hs1101();
	io_cfg_comp_hs1101();
	io_cfg_timer3_hs1101();
	io_cfg_timer4_hs1101();

	hs1101_init(&hs1101,100);

	/* flash io init */
	flash_io_ctrl_init();

	/*********************
	* software configure *
	**********************/
	/* life led init */
	led_init(&led_life, led_life_init, led_life_on, led_life_off);

	/* ir init */
	ir_init(&ir);

	ct_sensor1.current(CT1_ADC_PIN, (double)2000/(double)220); /* 2000/R */
	ct_sensor2.current(CT2_ADC_PIN, (double)2000/(double)220); /* 2000/R */
	ct_sensor3.current(CT3_ADC_PIN, (double)2000/(double)220); /* 2000/R */
	ct_sensor4.current(CT4_ADC_PIN, (double)2000/(double)220); /* 2000/R */

	/* button init */
	button_init(&btn_mode,	10,	BUTTON_MODE_ID,	io_button_mode_init,	io_button_mode_read,	btn_mode_callback);
	button_init(&btn_up,	10,	BUTTON_UP_ID,	io_button_up_init,		io_button_up_read,		btn_up_callback);
	button_init(&btn_down,	10,	BUTTON_DOWN_ID,	io_button_down_init,	io_button_down_read,	btn_down_callback);

	/* increase start time */
	fatal_log_t app_fatal_log;
	eeprom_read(EEPROM_FATAL_LOG_ADDR, (uint8_t*)&app_fatal_log, sizeof(fatal_log_t));
	app_fatal_log.restart_times ++;
	eeprom_write(EEPROM_FATAL_LOG_ADDR, (uint8_t*)&app_fatal_log, sizeof(fatal_log_t));

	/* get settings value */
	eeprom_read(EEPROM_APP_SETTING_ADDR, (uint8_t*)&app_setting, sizeof(app_setting_t));

	/* get IR header */
	eeprom_read(EEPROM_APP_IR_HEADER_BASE_ADDR, (uint8_t*)&app_airconds_ir_cmd_info, sizeof(app_airconds_ir_cmd_info_t));

	/* get air conditions status */
	eeprom_read(EEPROM_AIR_COND_START_BASE_ADDR, (uint8_t*)air_cond, TOTAL_AIR_COND * sizeof(air_cond_t));

	/* start timer for application */
	app_init_state_machine();
	app_start_timer();

	/******************************************************************************
	* app task initial
	*******************************************************************************/
	app_task_init();

	/******************************************************************************
	* run applications
	*******************************************************************************/
	return task_run();
}

/*****************************************************************************/
/* app initial function.
 */
/*****************************************************************************/

/* start software timer for application
 * used for app tasks
 */
void app_start_timer() {
	/* start timer to toggle life led */
	timer_set(AC_TASK_LIFE_ID, AC_LIFE_SYSTEM_CHECK, AC_LIFE_TASK_TIMER_LED_LIFE_INTERVAL, TIMER_PERIODIC);

	/* start timer to update LCD time */
	timer_set(AC_TASK_UI_ID, AC_UI_SCREEN_HOME_UPDATE, AC_UI_TIMER_TIME_UPDATE_INTERVAL, TIMER_PERIODIC);

	/* start timer to update air condition status */
	timer_set(AC_TASK_TIME_ID, AC_TIME_RTC_UPDATE_STATUS, AC_RTC_UPDATE_TIME_INTERVAL, TIMER_PERIODIC);

	/* checking firmware update */
	timer_set(AC_TASK_FIRMWARE_ID, AC_FIRMWARE_CHECKING_REQ, AC_TIMER_FIRMWARE_CHECKING_INTERVAL, TIMER_ONE_SHOT);
}

/* init state machine for tasks
 * used for app tasks
 */
void app_init_state_machine() {
	FSM(&fsm_sm, sm_state_idle);
	FSM(&fsm_ui, ui_state_display_on);
	SCREEN_CTOR(&screen_ui, ctrl_scr_home, &scr_main);
}

/* send first message to trigger start tasks
 * used for app tasks
 */
void app_task_init() {
	/* initial for RF24 task */
	{
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_RF24_IF_INIT_NETWORK);
		task_post(AC_TASK_GW_IF_ID, s_msg);
	}

	/* initial for UI task */
	{
		ak_msg_t* s_msg = get_pure_msg();
		set_msg_sig(s_msg, AC_UI_INITIAL);
		task_post(AC_TASK_UI_ID, s_msg);
	}
}

/*****************************************************************************/
/* app common function
 */
/*****************************************************************************/

/* hardware timer interrupt 10ms
 * used for led, button polling
 */
void sys_irq_timer_10ms() {
	button_timer_polling(&btn_mode);
	button_timer_polling(&btn_up);
	button_timer_polling(&btn_down);
}

/* hardware timer interrupt 50ms
 * used for encode and decode ir
 */
void sys_irq_timer_50us() {
	ir_decode_irq_timer_polling(&ir);
}

/* hardware rtc interrupt alarm
 * used for internal rtc alarm
 */
void sys_irq_timer_hs1101() {
	hs1101_irq_timer_polling(&hs1101);
}

/* hardware io interrupt at rev ir pin
 * used for decode ir
 */
void sys_irq_ir_io_rev() {
	ir_decode_irq_rev_io_polling(&ir);
}
