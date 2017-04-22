#ifndef __TASK_UI_H__
#define __TASK_UI_H__

#include <stdint.h>

#include "../ak/ak.h"
#include "../ak/fsm.h"

#include "../common/screen_manager.h"

#include "../driver/button/button.h"

#define BUTTON_MODE_ID					(0x01)
#define BUTTON_UP_ID					(0x02)
#define BUTTON_DOWN_ID					(0x03)

#define TEMP_AIR_COOL_MAX				(30)
#define TEMP_AIR_WARM_MAX				(45)
#define TEMP_AIR_HOT_MAX				(60)

#define MAX_CALIB_TEMP					(20)
#define MAX_CALIB_HUM					(30)

#define MAX_SCREEN						(80)
#define MAX_EVENT_CONVERT_SCR			(4)
#define NONE_CHANGE						(0xFF)

extern fsm_t fsm_ui;

extern void ui_state_display_on(ak_msg_t* msg);
extern void ui_state_display_off(ak_msg_t* msg);

extern scr_mng_t screen_ui;

extern button_t btn_mode;
extern button_t btn_up;
extern button_t btn_down;

extern void btn_mode_callback(void*);
extern void btn_up_callback(void*);
extern void btn_down_callback(void*);

/*****************************************************************************/
/* main screen
 */
/*****************************************************************************/
extern void ctrl_scr_home(ak_msg_t* msg);
extern void ctrl_scr_settings_system_busy(ak_msg_t* msg);
extern void ctrl_scr_settings_firmware_update(ak_msg_t* msg);

/*****************************************************************************/
/* engineer screen
 */
/*****************************************************************************/
extern void ctrl_scr_settings_system_present(ak_msg_t* msg);
extern void ctrl_scr_setting_calib_temp_hum(ak_msg_t* msg);
extern void ctrl_scr_settings_air_current(ak_msg_t* msg);
extern void ctrl_scr_settings_air_current_set(ak_msg_t* msg);
extern void ctrl_scr_settings_air_current_set_hand(ak_msg_t* msg);
extern void ctrl_scr_settings_air_current_set_auto(ak_msg_t* msg);
extern void ctrl_scr_settings_air_current_set_auto_fail(ak_msg_t* msg);

/*****************************************************************************/
/* seeting time screen
 */
/*****************************************************************************/
extern void ctrl_scr_time(ak_msg_t* msg);
extern void ctrl_scr_setting_time(ak_msg_t* msg);

/*****************************************************************************/
/* seeting ir screen
 */
/*****************************************************************************/
extern void ctrl_scr_ir(ak_msg_t* msg);
extern void ctrl_scr_setting_ir(ak_msg_t* msg);

extern void ctrl_scr_setting_ir_rec(ak_msg_t* msg);
extern void ctrl_scr_setting_ir_rec_on(ak_msg_t* msg);
extern void ctrl_scr_setting_ir_rec_off(ak_msg_t* msg);

extern void ctrl_scr_setting_ir_test(ak_msg_t* msg);
extern void ctrl_scr_setting_ir_test_auto(ak_msg_t* msg);
extern void ctrl_scr_setting_ir_test_hand(ak_msg_t* msg);

/*****************************************************************************/
/* seeting temperature screen
 */
/*****************************************************************************/
extern void ctrl_scr_temp(ak_msg_t* msg);
extern void ctrl_scr_setting_temp(ak_msg_t* msg);

/*****************************************************************************/
/* seeting air conditional screen
 */
/*****************************************************************************/
extern void ctrl_scr_air(ak_msg_t* msg) ;
extern void ctrl_scr_setting_air(ak_msg_t* msg);
extern void ctrl_scr_setting_total_air(ak_msg_t* msg);
extern void ctrl_scr_setting_timer_air(ak_msg_t* msg);

/*****************************************************************************/
/* seeting default parameter system screen
 */
/*****************************************************************************/
extern void ctrl_scr_default(ak_msg_t* msg);
extern void ctrl_scr_setting_default(ak_msg_t* msg);
extern void ctrl_scr_init_data(ak_msg_t* msg);


/*****************************************************************************/
/* exit setting screen
 */
/*****************************************************************************/
extern void ctrl_scr_exit(ak_msg_t* msg);

#endif //__TASK_UI_H__
