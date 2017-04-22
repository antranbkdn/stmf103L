#ifndef __TASK_LIST_H__
#define __TASK_LIST_H__

#include "../ak/ak.h"
#include "../ak/task.h"

extern task_t app_task_table[];

/*****************************************************************************/
/*  DECLARE: Internal Task ID
 *  Note: Task id MUST be increasing order.
 */
/*****************************************************************************/
/**
  * SYSTEM TASKS
  **************/
#define TASK_TIMER_TICK_ID				0

/**
  * APP TASKS
  **************/
#define AC_TASK_SHELL_ID				1
#define AC_TASK_LIFE_ID					2
#define AC_TASK_GW_IF_ID				3
#define AC_TASK_IF_ID					4
#define AC_TASK_TIME_ID					5
#define AC_TASK_SM_ID					6
#define AC_TASK_IR_ID					7
#define AC_TASK_SENSOR_ID				8
#define AC_TASK_UI_ID					9
#define AC_TASK_SETTING_ID				10
#define AC_TASK_FIRMWARE_ID				11

/**
  * EOT task ID
  **************/
#define AC_TASK_EOT_ID					12

/*****************************************************************************/
/*  DECLARE: Task entry point
 */
/*****************************************************************************/
extern void task_shell(ak_msg_t*);
extern void task_life(ak_msg_t*);
extern void task_gw_if(ak_msg_t*);
extern void task_if(ak_msg_t*);
extern void task_time(ak_msg_t*);
extern void task_sm(ak_msg_t*);
extern void task_ir(ak_msg_t*);
extern void task_sensor(ak_msg_t*);
extern void task_ui(ak_msg_t*);
extern void task_setting(ak_msg_t*);
extern void task_firmware(ak_msg_t*);

#endif //__TASK_LIST_H__
