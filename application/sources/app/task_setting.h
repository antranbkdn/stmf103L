#ifndef __TASK_SETTING_H__
#define __TASK_SETTING_H__

#include <stdint.h>
#include "app_eeprom.h"
#include "app_data.h"

typedef struct {
	uint8_t		yr;					/* date saved in epprom */
	uint8_t		mon;				/* total of air conditional*/
	uint8_t		date;				/* hour */
	uint8_t		hr;					/* date saved in epprom */
	uint8_t		min;				/* total of air conditional*/
} setting_rtc_time_t;

extern setting_rtc_time_t setting_rtc_time;
extern app_setting_t app_setting;

#endif //__TASK_SETTING_H__
