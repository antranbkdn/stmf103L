#include "tsm.h"
#include "ak_dbg.h"

#include "../sys/sys_dbg.h"

void tsm_init(tsm_tbl_t* tsm_tbl, tsm_state_t state) {
	if (tsm_tbl != (tsm_tbl_t*)0) {
		FATAL("TSM", 0x01);
	}

	tsm_tbl->state = state;
}

void tsm_dispatch(tsm_tbl_t* tsm_tbl, ak_msg_t* msg) {
	tsm_t* respective_table = tsm_tbl->table[tsm_tbl->state];

	/* search tsm state respective */
	while (respective_table->sig != msg->sig) {
		respective_table++;
	}

	/* update next state */
	tsm_tbl->state = respective_table->next_state;

	if (respective_table->tsm_func != TSM_FUNCTION_NULL) {
		respective_table->tsm_func(msg);
	}
}
