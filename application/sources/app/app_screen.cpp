#include "app_screen.h"
#include "../sys/sys_dbg.h"
#include "../sys/sys_ctrl.h"

#include "app.h"
#include "app_dbg.h"

#include "task_list.h"
#include "task_ui.h"
#include "task_time.h"
#include "task_sensor.h"
#include "task_setting.h"

#include "../common/utils.h"
#include "../common/xprintf.h"

static void view_scr_main();
static void view_scr_setting_busy();
static void view_scr_firmware_update();
static void view_scr_cursor_up_down();

static void view_scr_settings_system_present();
static void view_scr_settings_calib_temp_hum();
static void view_scr_settings_air_current();
static void view_scr_settings_air_current_auto_calib();

static void view_scr_settings_ir_rec_warning();
static void view_scr_settings_ir_test_auto();

static void view_scr_settings_air_time_range();
static void view_scr_settings_air_num();

static void view_scr_settings_init_data_warning();
static void view_scr_settings_init_data_start();

/**
 ******************************************************************************
 * item object
 *
 ******************************************************************************
 */

/*****************************************************************************/
/* item object of screent main
 */
/*****************************************************************************/
view_dynamic_t view_main = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_main
};

view_dynamic_t view_system_busy = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_setting_busy
};

view_dynamic_t view_firmware_update = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_firmware_update
};

/*****************************************************************************/
/* item object of screent parameter system present
 */
/*****************************************************************************/
view_dynamic_t view_system_parameter = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_system_present
};

/*****************************************************************************/
/* item object of screent calibration temp - hum
 */
/*****************************************************************************/
view_dynamic_t calib_temp_hum_tilde = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_calib_temp_hum
};

view_rectangle_t calib_temp_hum_num = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 10,
	.y		= 16,
	.width	= 110,
	.height = 20,

	.border_width = 0
};

/*****************************************************************************/
/* item object of screent settings air_cond current
 */
/*****************************************************************************/
view_dynamic_t view_air_current = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_air_current
};

view_dynamic_t current_air_auto_calib = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_air_current_auto_calib
};

view_rectangle_t current_air_num_1_2 = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 3,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 10,
	.y		= 0,
	.width	= 110,
	.height = 20,

	.border_width = 0
};

view_rectangle_t current_air_num_3_4 = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 3,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 10,
	.y		= 22,
	.width	= 110,
	.height = 20,

	.border_width = 0
};

view_rectangle_t back = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'B', 'A', 'C', 'K'},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 10,
	.y		= 44,
	.width	= 110,
	.height = 20,

	.border_width = 0
};

/*****************************************************************************/
/* item object of screent setting time
 */
/*****************************************************************************/
view_dynamic_t view_scr_cursor = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_cursor_up_down
};

view_rectangle_t setting_time = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'T','I','M','E'},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 20,
	.y		= 20,
	.width	= 90,
	.height = 25,

	.border_width = 0

};

view_rectangle_t hour = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 10,
	.y		= 0,
	.width	= 110,
	.height = 20,

	.border_width = 0
};

view_rectangle_t day = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 10,
	.y		= 22,
	.width	= 110,
	.height = 20,

	.border_width = 0
};

/*****************************************************************************/
/* item object of screent setting ir
 */
/*****************************************************************************/
view_rectangle_t setting_ir = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'I','R'},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 20,
	.y		= 20,
	.width	= 90,
	.height = 25,

	.border_width = 0

};

view_rectangle_t rec = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'R','E','C'},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 10,
	.y		= 0,
	.width	= 110,
	.height = 20,

	.border_width = 0
};

view_dynamic_t setting_ir_rec = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_ir_rec_warning
};

view_dynamic_t setting_ir_rec_on = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_ir_rec_on
};

view_dynamic_t setting_ir_rec_on_fail = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_ir_rec_on_fail
};

view_dynamic_t setting_ir_rec_on_saving = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_ir_rec_on_saving
};

view_dynamic_t setting_ir_rec_off = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_ir_rec_off
};

view_dynamic_t setting_ir_rec_off_fail = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_ir_rec_off_fail
};

view_dynamic_t setting_ir_rec_off_saving = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_ir_rec_off_saving
};

view_rectangle_t air_cond_ordinal = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 30,
	.y		= 44,
	.width	= 60,
	.height = 20,

	.border_width = 0
};

view_rectangle_t test = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'T','E','S','T'},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 10,
	.y		= 22,
	.width	= 110,
	.height = 20,

	.border_width = 0
};

view_rectangle_t test_auto {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'A', 'U', 'T', 'O'},
			.text_color		= BLACK,
			.font_size		= 2,

			.focus_cursor	= 0,
			.focus_size		= 2,

			.type			= BACK_GND_STYLE_OUTLINE,

			.x		= 10,
			.y		= 0,
			.width	= 110,
			.height = 20,

			.border_width = 0
};

view_dynamic_t test_auto_start = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_ir_test_auto
};


view_rectangle_t test_hand = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'H', 'A', 'N', 'D'},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 10,
	.y		= 22,
	.width	= 110,
	.height = 20,

	.border_width = 0
};

view_rectangle_t test_hand_on = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 10,
	.y		= 8,
	.width	= 105,
	.height = 20,

	.border_width = 0
};

view_rectangle_t test_hand_off = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 10,
	.y		= 36,
	.width	= 105,
	.height = 20,

	.border_width = 0
};

/*****************************************************************************/
/* item object of screent setting temp
 */
/*****************************************************************************/
view_rectangle_t setting_temp = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'T','E','M','P'},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 20,
	.y		= 20,
	.width	= 90,
	.height = 25,

	.border_width = 0
};

view_rectangle_t temp_fuzzy_tidle = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'C',' ',' ','N',' ',' ','H'},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_NONE_OUTLINE,

	.x		= 10,
	.y		= 0,
	.width	= 110,
	.height = 20,

	.border_width = 0
};

view_rectangle_t temp_fuzzy_num = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 10,
	.y		= 20,
	.width	= 110,
	.height = 20,

	.border_width = 0
};

/*****************************************************************************/
/* item object of screent setting air
 */
/*****************************************************************************/
view_rectangle_t setting_air = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'A','I','R'},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 20,
	.y		= 20,
	.width	= 90,
	.height = 25,

	.border_width = 0

};

view_rectangle_t total_air = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'T','O','T','A','L','-','A','L'},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 10,
	.y		= 0,
	.width	= 110,
	.height = 20,

	.border_width = 0
};

view_rectangle_t timer_air = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'T','I','M','E','R'},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 10,
	.y		= 22,
	.width	= 110,
	.height = 20,

	.border_width = 0
};

view_dynamic_t total_air_tilde = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_air_num
};

view_rectangle_t total_air_num = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 10,
	.y		= 16,
	.width	= 110,
	.height = 20,

	.border_width = 0
};

view_dynamic_t timer_air_tilde = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_air_time_range,
};

view_rectangle_t timer_air_num = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 40,
	.y		= 16,
	.width	= 40,
	.height = 20,

	.border_width = 0
};

/*****************************************************************************/
/* item object of screent setting init data
 */
/*****************************************************************************/
view_rectangle_t setting_default = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'D','E','F','A','U','L','T'},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 20,
	.y		= 20,
	.width	= 90,
	.height = 25,

	.border_width = 0

};

view_dynamic_t init_data_warning = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_init_data_warning
};

view_dynamic_t init_data_start = {
	{
		.item_type = ITEM_TYPE_DYNAMIC,
	},
	view_scr_settings_init_data_start
};

view_rectangle_t init_data_yes = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'Y'},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 5,
	.y		= 37,
	.width	= 30,
	.height = 20,

	.border_width = 0
};

view_rectangle_t init_data_no = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'N'},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 93,
	.y		= 37,
	.width	= 30,
	.height = 20,

	.border_width = 0
};

/*****************************************************************************/
/* item object of screent exit setting
 */
/*****************************************************************************/
view_rectangle_t setting_exit = {
	{
		.item_type = ITEM_TYPE_RECTANGLE,
	},
	.text			= {'E','X','I','T'},
	.text_color		= BLACK,
	.font_size		= 2,

	.focus_cursor	= 0,
	.focus_size		= 2,

	.type			= BACK_GND_STYLE_OUTLINE,

	.x		= 20,
	.y		= 20,
	.width	= 90,
	.height = 25,

	.border_width = 0
};

/**
 ******************************************************************************
 * view object
 *
 ******************************************************************************
 */

/*****************************************************************************/
/* object main
 */
/*****************************************************************************/
view_screen_t scr_main = {
	&view_main,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0
};

view_screen_t scr_settings_system_busy = {
	&view_system_busy,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0
};

view_screen_t scr_settings_firmware_update = {
	&view_firmware_update,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0
};

/*****************************************************************************/
/* object engineer - calib temp-hum
 */
/*****************************************************************************/
view_screen_t scr_settings_system_present = {
	&view_system_parameter,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0
};

view_screen_t scr_setting_calib_temp_hum = {
	&calib_temp_hum_tilde,
	&calib_temp_hum_num,
	&back,
	.focus_item = 1
};

/*****************************************************************************/
/* object current air - calib threshold current air
 */
/*****************************************************************************/
view_screen_t scr_settings_air_current = {
	&view_air_current,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0
};

view_screen_t scr_setting_air_current_set = {
	&test_auto,
	&test_hand,
	&back,
	.focus_item = 0
};

view_screen_t scr_setting_air_current_set_hand = {
	&current_air_num_1_2,
	&current_air_num_3_4,
	&back,
	.focus_item = 0
};

view_screen_t scr_setting_air_current_set_auto = {
	&back,
	&current_air_auto_calib,
	ITEM_NULL,
	.focus_item = 0
};

/*****************************************************************************/
/* object time - setting time
 */
/*****************************************************************************/
view_screen_t scr_time = {
	&setting_time,
	&view_scr_cursor,
	ITEM_NULL,
	.focus_item = 0
};

view_screen_t scr_setting_time = {
	&hour,
	&day,
	&back,
	.focus_item = 0
};

/*****************************************************************************/
/* object ir - setting ir
 */
/*****************************************************************************/
view_screen_t scr_ir = {
	&setting_ir,
	&view_scr_cursor,
	ITEM_NULL,
	.focus_item = 0
};

view_screen_t scr_setting_ir = {
	&rec,
	&test,
	&back,
	.focus_item = 0
};

view_screen_t scr_setting_ir_rec = {
	&setting_ir_rec,
	&init_data_no,
	&init_data_yes,
	.focus_item = 1
};

view_screen_t scr_setting_ir_rec_start = {
	&setting_ir_rec_on,
	&air_cond_ordinal,
	ITEM_NULL,
	.focus_item = 1
};

view_screen_t scr_setting_ir_test = {
	&test_auto,
	&test_hand,
	&back,
	.focus_item = 0
};

view_screen_t scr_setting_ir_test_auto = {
	&test_auto_start,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0
};

view_screen_t scr_setting_ir_test_hand = {
	&test_hand_on,
	&test_hand_off,
	ITEM_NULL,
	.focus_item = 0
};

/*****************************************************************************/
/* object temp - setting temp
 */
/*****************************************************************************/
view_screen_t scr_temp = {
	&setting_temp,
	&view_scr_cursor,
	ITEM_NULL,
	.focus_item = 0
};

view_screen_t scr_setting_temp = {
	&temp_fuzzy_tidle,
	&temp_fuzzy_num,
	&back,
	.focus_item = 1
};



/*****************************************************************************/
/* object air - setting air
 */
/*****************************************************************************/
view_screen_t scr_air = {
	&setting_air,
	&view_scr_cursor,
	ITEM_NULL,
	.focus_item = 0
};

view_screen_t scr_setting_air = {
	&total_air,
	&timer_air,
	&back,
	.focus_item = 0
};

view_screen_t scr_setting_timer_air = {
	&timer_air_tilde,
	&timer_air_num,
	&back,
	.focus_item = 1
};

view_screen_t scr_setting_total_air = {
	&total_air_tilde,
	&total_air_num,
	&back,
	.focus_item = 1
};

/*****************************************************************************/
/* object init data
 */
/*****************************************************************************/
view_screen_t scr_default = {
	&setting_default,
	&view_scr_cursor,
	ITEM_NULL,
	.focus_item = 0
};

view_screen_t scr_init_data = {
	&init_data_start,
	ITEM_NULL,
	ITEM_NULL,
	.focus_item = 0
};

view_screen_t scr_setting_default = {
	&init_data_warning,
	&init_data_yes,
	&init_data_no,
	.focus_item = 2
};

/*****************************************************************************/
/* object exit setting
 */
/*****************************************************************************/
view_screen_t scr_exit = {
	&setting_exit,
	&view_scr_cursor,
	ITEM_NULL,
	.focus_item = 0
};

/**
 ******************************************************************************
 * render function for dynamic object
 *
 ******************************************************************************
 */

void view_scr_main() {
	float temp;
	uint8_t hum;
	Time t(2099, 1, 1, 0, 0, 0, Time::Day::kMonday);

	temp = app_setting.operations_calib_temp ? ((float)thermistor.read_f() / 10 + app_setting.temp_calibration) : ((float)thermistor.read_f() / 10 - app_setting.temp_calibration);
	hum = app_setting.operations_calib_hum ? (hs1101_read(&hs1101) + app_setting.hum_calibration) : (hs1101_read(&hs1101) - app_setting.hum_calibration);

	if(hum > 200) {
		hum = 0;
	}
	else if (hum > 100) {
		hum = 99;
	}

	t = rtc_ds1302.time();

	view_render.setTextColor(WHITE);

	for(uint8_t i = 0; i < 2; i++) {
		view_render.drawRect(i,i,SSD1306_WIDTH - i, SSD1306_HEIGHT - i, WHITE);
	}

	view_render.setTextSize(2);
	view_render.setCursor(X_TEMP_CURSOR, Y_TEMP_CURSOR);
	view_render.print(temp);
	view_render.setTextSize(1);
	view_render.setCursor(X_DIRTY_CURSOR, Y_DIRTY_CURSOR);
	view_render.print("o");
	view_render.setCursor(X_CENCIUS_CURSOR, Y_TEMP_CURSOR);
	view_render.setTextSize(2);
	view_render.print("C");

	view_render.setTextSize(2);
	view_render.setCursor(X_HUM_CURSOR, Y_HUM_CURSOR);
	view_render.print(hum);

	view_render.setCursor(X_PERCENT_CURSOR, Y_PERCENT_CURSOR);
	view_render.print("%");

	view_render.setCursor(X_PERCENT_CURSOR + 15, Y_PERCENT_CURSOR);

	view_render.setTextSize(1);
	view_render.print("RH");

	if (t.hr < 10) {
		view_render.setCursor(X_HOUR_CURSOR + RANGE_NUMBER, Y_CLOCK_CURSOR);
		view_render.print(t.hr);
	}
	else {
		view_render.setCursor(X_HOUR_CURSOR,Y_CLOCK_CURSOR);
		view_render.print(t.hr);
	}

	view_render.setCursor(X_HOUR_DOT_CURSOR,Y_CLOCK_CURSOR);
	view_render.print(":");

	if (t.min < 10) {
		view_render.setCursor(X_MIN_CURSOR,Y_CLOCK_CURSOR);
		view_render.print(0);
		view_render.setCursor(X_MIN_CURSOR + RANGE_NUMBER, Y_CLOCK_CURSOR);
		view_render.print(t.min);
	}
	else {
		view_render.setCursor(X_MIN_CURSOR,Y_CLOCK_CURSOR);
		view_render.print(t.min);
	}

	if(t.hr >= DAY_TIME && t.hr < NIGHT_TIME) {
		view_render.drawSun(90,Y_CLOCK_CURSOR - 5 , 14, WHITE);
	}
	else {
		view_render.drawMoon(90,Y_CLOCK_CURSOR - 5, 14, WHITE);
	}
}

void view_scr_settings_system_present() {
	char str[20];
	view_render.setTextColor(WHITE);
	view_render.setTextSize(1);


	xsprintf(str,"total air :%02d/%02d(al) ",app_setting.total_air_cond, app_setting.total_air_cond_alternate);
	view_render.setCursor(0,0);
	view_render.print(str);

	xsprintf(str,"time_acive:%02d         ",app_setting.time_air_range);
	view_render.setCursor(0,12);
	view_render.print(str);


	xsprintf(str,"temp air  :%02d-%02d-%02d  ",app_setting.milestone_temp_cool,app_setting.milestone_temp_normal,app_setting.milestone_temp_hot);
	view_render.setCursor(0,24);
	view_render.print(str);

	xsprintf(str,"C_t/C_RH  :%02d/%02d   ",app_setting.temp_calibration,app_setting.hum_calibration);
	view_render.setCursor(0,36);
	view_render.print(str);

	xsprintf(str,"TCA:%03d:%03d:%03d:%03d",air_cond[0].milestone_on,air_cond[1].milestone_on,air_cond[2].milestone_on,air_cond[3].milestone_on);
	view_render.setCursor(0,48);
	view_render.print(str);

}

void view_scr_settings_air_current() {
	uint16_t ui_current_air_cond[4];
	ui_current_air_cond[0] = air_cond[0].available ? (1000 * ct_sensor1.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
	ui_current_air_cond[1] = air_cond[1].available ? (1000 * ct_sensor2.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
	ui_current_air_cond[2] = air_cond[2].available ? (1000 * ct_sensor3.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;
	ui_current_air_cond[3] = air_cond[3].available ? (1000 * ct_sensor4.calcIrms(AC_NUMBER_SAMPLE_CT_SENSOR)) : 0;

	view_render.setTextColor(WHITE);

	view_render.setCursor(0, 0);
	view_render.setTextSize(1);
	view_render.print(" Current air present");

	for (uint8_t i = 0; i < app_setting.total_air_cond; i++) {
		view_render.setCursor(20, 12*(i+1));
		view_render.print("AIR");
		view_render.print(i+1);
		view_render.print(" : ");
		view_render.print(ui_current_air_cond[i]);
		view_render.setCursor(95, 12*(i+1));

		if (ui_current_air_cond[i] >= air_cond[i].milestone_on) {
			view_render.print("ON");
		}
		else {
			view_render.print("OFF");
		}
	}
}

void view_scr_cursor_up_down(){
	view_render.fillTriangle(X0_UP, Y0_UP, X1_UP, Y1_UP, X2_UP, Y2_UP, WHITE);
	view_render.fillTriangle(X0_DOWN, Y0_DOWN, X1_DOWN, Y1_DOWN, X2_DOWN, Y2_DOWN,WHITE);
}

void view_scr_settings_ir_rec_warning() {
	view_render.setTextColor(WHITE);
	view_render.setTextSize(1);

	view_render.setCursor(20,0);
	view_render.print("Do you want to");
	view_render.setCursor(20,10);
	view_render.print("record all key");
	view_render.setCursor(15,20);
	view_render.print("air conditional");
}

void view_scr_settings_ir_rec_on() {
	view_render.setTextColor(WHITE);

	view_render.setTextSize(1);
	view_render.setCursor(5,8);
	view_render.print("press ON key to rec!");
}

void view_scr_settings_ir_rec_on_fail() {
	view_render.setTextColor(WHITE);

	view_render.setTextSize(1);
	view_render.setCursor(5,5);
	view_render.print("receive ON key fail!");
	view_render.setCursor(5,20);
	view_render.print("press ON key AGAIN!");
}

void view_scr_settings_ir_rec_on_saving() {
	view_render.setTextColor(WHITE);

	view_render.setTextSize(1);
	view_render.setCursor(5,5);
	view_render.print("receive ON key OK!");
	view_render.setCursor(0,20);
	view_render.print("syterm saving data...");
}


void view_scr_settings_ir_rec_off() {
	view_render.setTextColor(WHITE);

	view_render.setTextSize(1);
	view_render.setCursor(0,8);
	view_render.print("press OFF key to rec!");
}

void view_scr_settings_ir_rec_off_fail() {
	view_render.setTextColor(WHITE);

	view_render.setTextSize(1);
	view_render.setCursor(5,5);
	view_render.print("receive OFF key fail!");
	view_render.setCursor(5,20);
	view_render.print("press ON key AGAIN!");
}

void view_scr_settings_ir_rec_off_saving() {
	view_render.setTextColor(WHITE);

	view_render.setTextSize(1);
	view_render.setCursor(5,5);
	view_render.print("receive OFF key OK!");
	view_render.setCursor(0,20);
	view_render.print("system saving data...");
}

void view_scr_settings_ir_test_auto() {
	view_render.setTextColor(WHITE);

	view_render.fillRect(10, 36, 105, 20, WHITE);

	view_render.setTextSize(1);
	view_render.setCursor(5,5);
	view_render.print("  testing all key!");

	view_render.setTextColor(BLACK);
	view_render.setTextSize(2);
	view_render.setCursor(40, 39);
	view_render.print("AUTO");

}

void view_scr_settings_air_time_range() {
	view_render.setTextColor(WHITE);

	view_render.setCursor(7 , 0);
	view_render.setTextSize(1);
	view_render.print("Set time range air");
}

void view_scr_settings_air_num() {
	view_render.setTextColor(WHITE);

	view_render.setCursor(7, 0);
	view_render.setTextSize(1);
	view_render.print("total-alternate air");
}

void view_scr_settings_init_data_warning() {
	view_render.setTextColor(WHITE);

	view_render.setTextSize(1);
	view_render.setCursor(15, 0);
	view_render.print("Do you want to");
	view_render.setCursor(15, 10);
	view_render.print("setting default");
	view_render.setCursor(10, 20);
	view_render.print("system parameter?");
}

void view_scr_settings_init_data_start() {
	view_render.fillRect(10, 36, 105, 20, WHITE);

	view_render.setTextSize(1);
	view_render.setTextColor(BLACK);

	view_render.setCursor(30, 42);
	view_render.print("waitting...");
}

void view_scr_settings_calib_temp_hum() {
	view_render.setTextColor(WHITE);

	view_render.setCursor(5, 0);
	view_render.setTextSize(1);
	view_render.print("calibration temp-hum");
}

void view_scr_settings_air_current_auto_calib() {
	view_render.setTextColor(WHITE);

	view_render.setTextSize(1);
	view_render.setCursor(17, 0);
	view_render.print("Auto calibration");
	view_render.setCursor(15, 12);
	view_render.print("threshold current");
	view_render.setCursor(22, 24);
	view_render.print("air conditonal");
}

void view_scr_setting_busy() {
	view_render.setTextColor(WHITE);

	view_render.setTextSize(1);
	view_render.setCursor(17, 10);
	view_render.print("System is busy");
	view_render.setCursor(0, 30);
	view_render.print("please setting late");

}

void view_scr_firmware_update() {
	view_render.setTextColor(WHITE);

	view_render.setTextSize(2);
	view_render.setCursor(17, 10);
	view_render.print("FIRMWARE");
	view_render.setCursor(17, 35);
	view_render.print("UPDATING");
	view_render.setCursor(50, 50);
	view_render.print("...");
}
