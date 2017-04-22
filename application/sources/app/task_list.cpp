#include "task_list.h"
#include "../ak/timer.h"

task_t app_task_table[] = {
	/*************************************************************************/
	/* SYSTEM TASK */
	/*************************************************************************/
	{TASK_TIMER_TICK_ID,	TASK_PRI_LEVEL_7,		task_timer_tick			},

	/*************************************************************************/
	/* APP TASK */
	/*************************************************************************/
	{AC_TASK_SHELL_ID		,	TASK_PRI_LEVEL_2	,	task_shell			},
	{AC_TASK_LIFE_ID		,	TASK_PRI_LEVEL_6	,	task_life			},
	{AC_TASK_GW_IF_ID		,	TASK_PRI_LEVEL_4	,	task_gw_if			},
	{AC_TASK_IF_ID			,	TASK_PRI_LEVEL_4	,	task_if				},
	{AC_TASK_TIME_ID		,	TASK_PRI_LEVEL_2	,	task_time			},
	{AC_TASK_SM_ID			,	TASK_PRI_LEVEL_2	,	task_sm				},
	{AC_TASK_IR_ID			,	TASK_PRI_LEVEL_2	,	task_ir				},
	{AC_TASK_SENSOR_ID		,	TASK_PRI_LEVEL_2	,	task_sensor			},
	{AC_TASK_UI_ID			,	TASK_PRI_LEVEL_2	,	task_ui				},
	{AC_TASK_SETTING_ID		,	TASK_PRI_LEVEL_2	,	task_setting		},
	{AC_TASK_FIRMWARE_ID	,	TASK_PRI_LEVEL_1	,	task_firmware		},

	/*************************************************************************/
	/* END OF TABLE */
	/*************************************************************************/
	{AC_TASK_EOT_ID,			TASK_PRI_LEVEL_0,		(pf_task)0			}
};
