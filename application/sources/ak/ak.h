/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   13/08/2016
 * @brief:  Main defination of active kernel
 ******************************************************************************
**/

#ifndef __AK_H__
#define __AK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "fsm.h"
#include "task.h"
#include "port.h"

/*****************************************************************************
 * DEFINITION: active kernel
 *
 *****************************************************************************/
#define AK_VERSION						"1.1"
#define AK_ENABLE						(0x01)
#define AK_DISABLE						(0x00)

#define AK_FLAG_ON						(0x01)
#define AK_FLAG_OFF						(0x00)

/* debug option */
#define AK_TASK_DEBUG					AK_ENABLE

/* preemptive option */
#define AK_PREEMPTIVE					AK_DISABLE

/*****************************************************************************
 * DEFINITION: tasking
 *
 *****************************************************************************/
#define TASK_PRI_MAX_SIZE				(8)

#define TASK_PRI_LEVEL_0				(0)
#define TASK_PRI_LEVEL_1				(1)
#define TASK_PRI_LEVEL_2				(2)
#define TASK_PRI_LEVEL_3				(3)
#define TASK_PRI_LEVEL_4				(4)
#define TASK_PRI_LEVEL_5				(5)
#define TASK_PRI_LEVEL_6				(6)
#define TASK_PRI_LEVEL_7				(7)
#define TASK_PRI_LEVEL_8				(8)

/*****************************************************************************
 * DEFINITION: timer
 *
 *****************************************************************************/
#define TIMER_POOL_SIZE					(16)
#define TIMER_TICK_VALUE				(10)

#ifdef __cplusplus
}
#endif

#endif // __AK_H__
