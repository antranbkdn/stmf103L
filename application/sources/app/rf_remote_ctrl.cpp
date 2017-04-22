/**
 ******************************************************************************
 * @Author: ThanNT
 * @Date:   23/11/2016
 ******************************************************************************
**/

#include <stdint.h>

#include "../ak/ak.h"
#include "../ak/task.h"
#include "../ak/timer.h"
#include "../ak/message.h"

#include "../common/cmd_line.h"
#include "../common/utils.h"
#include "../common/xprintf.h"

#include "../sys/sys_ctrl.h"
#include "../sys/sys_io.h"
#include "../sys/sys_dbg.h"

#include "app.h"
#include "app_data.h"

#include "task_gw_if.h"

void rf_printf(uint8_t* buf, uint32_t len) {
	uint32_t remain = len;
	uint8_t* write_buf = buf;
	RF24NetworkHeader server_header(00, RF24_DATA_REMOTE_CMD_TYPE);
	while(remain) {
		if (remain > 64) {
			local_network.write(server_header, write_buf, 64);
			remain -= 64;
			write_buf += 64;
		}
		else {
			local_network.write(server_header, write_buf, remain);
			remain -= remain;
		}
	}
}
