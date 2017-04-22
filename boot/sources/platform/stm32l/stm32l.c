/**
 ******************************************************************************
 * @Author: ThanNT
 * @Date:   05/09/2016
 ******************************************************************************
**/
#include <stdint.h>

#include "stm32l.h"

#include "stm32l1xx.h"
#include "core_cm3.h"
#include "core_cmFunc.h"

void enable_interrupts() {
	__enable_irq();
}

void disable_interrupts() {
	__disable_irq();
}

static uint8_t nest_entry_critical_counter = 0;

void entry_critical() {
	if (nest_entry_critical_counter == 0) {
		disable_interrupts();
	}
	nest_entry_critical_counter++;
}

void exit_critical() {
	if (nest_entry_critical_counter <= 1) {
		enable_interrupts();
	}
	nest_entry_critical_counter--;
}
