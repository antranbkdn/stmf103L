#ifndef __TASK_SM_H__
#define __TASK_SM_H__
#include <stdint.h>

#include "../ak/ak.h"
#include "../ak/fsm.h"

extern fsm_t fsm_sm;

extern void sm_state_idle(ak_msg_t* msg);
extern void sm_state_requesting(ak_msg_t* msg);
extern void sm_state_setting(ak_msg_t* msg);
extern void sm_state_firmware_update(ak_msg_t* msg);

#endif //__TASK_SM_H__
