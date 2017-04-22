/**
 ******************************************************************************
 * @Author: ThanNT
 * @Date:   13/08/2016
 ******************************************************************************
**/

#include <stdbool.h>

#include "../ak/fsm.h"
#include "../ak/port.h"
#include "../ak/message.h"

#include "../common/cmd_line.h"
#include "../common/utils.h"
#include "../common/xprintf.h"

#include "../sys/sys_dbg.h"
#include "../sys/sys_ctrl.h"
#include "../sys/sys_irq.h"

#include "app.h"
#include "app_dbg.h"
#include "task_shell.h"
#include "task_list.h"

struct shell_t {
	uint8_t index;
	uint8_t data[SHELL_BUFFER_LENGHT];
} shell;

void sys_irq_shell() {
	uint8_t c = 0;
	c = sys_ctrl_shell_get_char();
	if (shell.index < SHELL_BUFFER_LENGHT - 1) {
		if (c == '\r' || c == '\n') {
			shell.data[shell.index] = c;
			shell.data[shell.index + 1] = 0;

			{
				ak_msg_t* msg = get_common_msg();
				set_msg_sig(msg, AC_SHELL_LOGIN_CMD);
				set_data_common_msg(msg, &shell.data[0], shell.index + 2);
				task_post(AC_TASK_SHELL_ID, msg);
			}

			shell.index = 0;
		}
		else if (c == 8) {
			if (shell.index) {
				shell.index--;
			}
		}
		else {
			shell.data[shell.index++] = c;
		}
	}
	else {
		APP_PRINT("\nerror: cmd too long, cmd size: %d, try again !\n", SHELL_BUFFER_LENGHT);
		shell.index = 0;
	}
}

void task_shell(ak_msg_t* msg) {
	uint8_t fist_char = *(get_data_common_msg(msg));

	switch (msg->sig) {
	case AC_SHELL_LOGIN_CMD:
		set_dymc_output_type(DYMC_UART_SHELL_TYPE);
		break;

	case AC_SHELL_REMOTE_CMD:
		set_dymc_output_type(DYMC_RF_REMOTE_TYPE);
		break;

	default:
		FATAL("SHELL", 0x01);
		break;
	}

	switch (cmd_line_parser(lgn_cmd_table, get_data_common_msg(msg))) {
	case CMD_SUCCESS:
		break;

	case CMD_NOT_FOUND:
		if (fist_char != '\r' &&
				fist_char != '\n') {
			LOGIN_PRINT("cmd unknown\n");
		}
		break;

	case CMD_TOO_LONG:
		LOGIN_PRINT("cmd too long\n");
		break;

	case CMD_TBL_NOT_FOUND:
		LOGIN_PRINT("cmd table not found\n");
		break;

	default:
		LOGIN_PRINT("cmd error\n");
		break;
	}

	LOGIN_PRINT("#");
}
