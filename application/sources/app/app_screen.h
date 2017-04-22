#ifndef __APP_SCREEN_H__
#define __APP_SCREEN_H__

#include <stdint.h>
#include "../common/view_item.h"


/* parameter time to check day/night */
#define DAY_TIME						(6)
#define NIGHT_TIME						(18)

#define RANGE_CURSOR					(25)
#define RANGE_NUMBER					(7)

/* cursor for temp*/
#define X_TEMP_CURSOR					(25)
#define Y_TEMP_CURSOR					(8)
#define X_DIRTY_CURSOR					(X_TEMP_CURSOR + 65)
#define Y_DIRTY_CURSOR					(Y_TEMP_CURSOR - 5)
#define X_CENCIUS_CURSOR				(X_DIRTY_CURSOR + 5)

/* cursor for hum*/
#define X_HUM_CURSOR					(45)
#define Y_HUM_CURSOR					(28)
#define X_PERCENT_CURSOR				(X_HUM_CURSOR + 30)
#define Y_PERCENT_CURSOR				(Y_HUM_CURSOR)


/* cursor for clock*/
#define X_HOUR_CURSOR					(38)
#define X_MIN_CURSOR					(X_HOUR_CURSOR + RANGE_CURSOR)
#define X_SEC_CURSOR					(X_MIN_CURSOR + RANGE_CURSOR)
#define Y_CLOCK_CURSOR					(50)

#define X_HOUR_DOT_CURSOR				(X_HOUR_CURSOR + (RANGE_CURSOR + 1 * 5) / 2)
#define X_MIN_DOT_CURSOR				(X_MIN_CURSOR + (RANGE_CURSOR + 1 * 5) / 2)



/*parameter cursor for setting time*/
#define X_SETTING_CURSOR				(20)
#define Y_SETTING_CURSOR				(20)
#define SETTING_RECTANGE_WIDTH			(90)
#define SETTING_RECTANGE_HEIGHT			(25)

/*parameter cursor up button*/
#define X0_UP							(X_SETTING_CURSOR + 30)
#define Y0_UP							(Y_SETTING_CURSOR - 7)
#define X1_UP							(X_SETTING_CURSOR + SETTING_RECTANGE_WIDTH - 30)
#define Y1_UP							(Y0_UP)
#define X2_UP							((X0_UP + X1_UP) / 2)
#define Y2_UP							(Y0_UP - 7)

/*parameter cursor down button*/
#define X0_DOWN							(X0_UP)
#define Y0_DOWN							(Y_SETTING_CURSOR + SETTING_RECTANGE_HEIGHT + 7)
#define X1_DOWN							(X1_UP)
#define Y1_DOWN							(Y0_DOWN)
#define X2_DOWN							(X2_UP)
#define Y2_DOWN							(Y0_DOWN + 7)

extern void view_scr_settings_ir_rec_on();
extern void view_scr_settings_ir_rec_on_fail();
extern void view_scr_settings_ir_rec_on_saving();
extern void view_scr_settings_ir_rec_off();
extern void view_scr_settings_ir_rec_off_fail();
extern void view_scr_settings_ir_rec_off_saving();

/*****************************************************************************/
/* object main
 */
/*****************************************************************************/
extern view_screen_t scr_main;
extern view_screen_t scr_settings_system_busy;
extern view_screen_t scr_settings_firmware_update;

/*****************************************************************************/
/* object engineer - calib temp-hum
 */
/*****************************************************************************/
extern view_screen_t scr_settings_system_present;
extern view_screen_t scr_setting_calib_temp_hum;

/*****************************************************************************/
/* object current air - calib threshold current air
 */
/*****************************************************************************/
extern view_screen_t scr_settings_air_current;
extern view_screen_t scr_setting_air_current_set;
extern view_screen_t scr_setting_air_current_set_hand;
extern view_screen_t scr_setting_air_current_set_auto;
extern view_screen_t scr_setting_air_current_set_auto_fail;

/*****************************************************************************/
/* object time - setting time
 */
/*****************************************************************************/
extern view_screen_t scr_time;
extern view_screen_t scr_setting_time;

extern view_rectangle_t hour;
extern view_rectangle_t day;
extern view_rectangle_t time_back;

/*****************************************************************************/
/* object ir - setting ir
 */
/*****************************************************************************/
extern view_screen_t scr_ir;
extern view_screen_t scr_setting_ir;

extern view_screen_t scr_setting_ir_rec;
extern view_screen_t scr_setting_ir_rec_start;

extern view_screen_t scr_setting_ir_test;
extern view_screen_t scr_setting_ir_test_auto;
extern view_screen_t scr_setting_ir_test_hand;

/*****************************************************************************/
/* object temp - setting temp
 */
/*****************************************************************************/
extern view_screen_t scr_temp;
extern view_screen_t scr_setting_temp;

/*****************************************************************************/
/* object air - setting air
 */
/*****************************************************************************/
extern view_screen_t scr_air;
extern view_screen_t scr_setting_air;
extern view_screen_t scr_setting_total_air;
extern view_screen_t scr_setting_timer_air;

/*****************************************************************************/
/* object init data
 */
/*****************************************************************************/
extern view_screen_t scr_default;
extern view_screen_t scr_setting_default;
extern view_screen_t scr_init_data;

/*****************************************************************************/
/* object exit setting
 */
/*****************************************************************************/
extern view_screen_t scr_exit;

#endif // __APP_SCREEN_H__
