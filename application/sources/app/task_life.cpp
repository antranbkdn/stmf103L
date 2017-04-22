#include "../ak/fsm.h"
#include "../ak/port.h"
#include "../ak/message.h"


#include "../sys/sys_ctrl.h"

#include "app.h"
#include "app_dbg.h"

#include "task_list.h"
#include "task_life.h"

led_t led_life;

void task_life(ak_msg_t* msg) {
	switch (msg->sig) {
	case AC_LIFE_SYSTEM_CHECK:
		/* reset watchdog */
		sys_ctrl_independent_watchdog_reset();
		sys_ctrl_soft_watchdog_reset();

		/* toggle led indicator */
		led_toggle(&led_life);
		break;

	default:
		break;
	}
}
