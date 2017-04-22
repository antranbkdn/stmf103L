#ifndef __TASK_TIME_H__
#define __TASK_TIME_H__
#include <stdint.h>

/* driver include */
#include "app_eeprom.h"

#include "../driver/rtc/rtc.h"
#include "../driver/ds1302/DS1302.h"
#include "../driver/fuzzy_logic/fuzzy_logic.h"

#define TOTAL_AIR_COND					(4)
#define DATE_NO_CHANGE					(4)

typedef struct {
	uint8_t available;
	uint8_t failed_counter;
	uint8_t active;
	uint8_t resersed;
	uint32_t milestone_on;
} air_cond_t;

extern rtc_t rtc;
extern DS1302 rtc_ds1302;
extern air_cond_t air_cond[TOTAL_AIR_COND];
extern uint8_t air_cond_expect_status[TOTAL_AIR_COND];
extern uint8_t air_cond_current_status[TOTAL_AIR_COND];

#endif //__TASK_TIME_H__
