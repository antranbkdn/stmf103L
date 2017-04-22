#ifndef __TASK_IR_H__
#define __TASK_IR_H__
#include <stdint.h>

#include "../driver/ir/ir.h"
#include "../driver/eeprom/eeprom.h"

#include "app_eeprom.h"

#define IR_BUF_MAX_SIZE						(450)

extern ir_t  ir;
extern app_airconds_ir_cmd_info_t app_airconds_ir_cmd_info;

#endif //__TASK_IR_H__
