/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   13/08/2016
 ******************************************************************************
**/

#ifndef __TASK_H__
#define __TASK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#include "ak.h"
#include "port.h"
#include "message.h"

#include "../common/log_queue.h"


#define LOG_QUEUE_OBJECT_SIZE			(1024)

#define ENTRY_INTERRUPT(var_temp_pri, isr_pri) \
	do {\
	ENTRY_CRITICAL();\
	var_temp_pri = task_current;\
	task_current = isr_pri;\
	EXIT_CRITICAL();\
} while(0);

#define EXIT_INTERRUPT(var_temp_pri, cmd) \
	do {\
	ENTRY_CRITICAL();\
	task_current = var_temp_pri;\
	EXIT_CRITICAL();\
	cmd;\
} while(0);

typedef uint8_t	task_pri_t;
typedef uint8_t	task_id_t;
typedef void	(*pf_task)(ak_msg_t*);

typedef struct {
	task_id_t	id;
	task_pri_t	pri;
	pf_task		task;
} task_t;

extern log_queue_t log_task_dbg_object_queue;

extern void		task_create(task_t* task_tbl);
extern void		task_post(task_id_t task_id, ak_msg_t* msg);
extern int		task_init();
extern int		task_run();

extern task_t*	get_current_task_info();
extern ak_msg_t* get_current_active_object();

extern uint8_t	task_mutex_lock(task_pri_t);
extern void		task_mutex_unlock(task_pri_t);

#ifdef __cplusplus
}
#endif

#endif //__TASK_H__
