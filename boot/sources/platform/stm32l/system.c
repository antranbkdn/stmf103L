/**
 ******************************************************************************
 * @Author: ThanNT
 * @Date:   05/09/2016
 ******************************************************************************
**/
#include <stdint.h>
#include <stdbool.h>

#include "sys_cfg.h"
#include "system.h"
#include "stm32l.h"

#include "system_stm32l1xx.h"
#include "stm32l1xx.h"
#include "stm32l1xx_conf.h"
#include "core_cm3.h"

#include "../../sys/sys_dbg.h"
#include "../../sys/sys_ctrl.h"
#include "../../sys/sys_irq.h"

#include "../../app/app.h"

/*****************************************************************************/
/* linker variable                                                           */
/*****************************************************************************/
extern uint32_t _ldata;
extern uint32_t _data;
extern uint32_t _edata;
extern uint32_t _bss;
extern uint32_t _ebss;
extern uint32_t _estack;

/*****************************************************************************/
/* static function prototype                                                 */
/*****************************************************************************/
/*****************************/
/* system interrupt function */
/*****************************/
void default_handler();
void reset_handler();

/* cortex-M processor fault exceptions */
void nmi_handler()          __attribute__ ((weak));
void hard_fault_handler()   __attribute__ ((weak));
void mem_manage_handler()   __attribute__ ((weak));
void bus_fault_handler()    __attribute__ ((weak));
void usage_fault_handler()  __attribute__ ((weak));

/* cortex-M processor non-fault exceptions */
void svc_handler();
void dg_monitor_handler()   __attribute__ ((weak, alias("default_handler")));
void pendsv_handler()       __attribute__ ((weak, alias("default_handler")));
void systick_handler();

/*****************************************************************************/
/* interrupt vector table                                                    */
/*****************************************************************************/
__attribute__((section(".isr_vector")))
void (* const isr_vector[])() = {
		((void (*)())(uint32_t)&_estack),		//	The initial stack pointer
		reset_handler,							//	The reset handler
		nmi_handler,							//	The NMI handler
		hard_fault_handler,						//	The hard fault handler
		mem_manage_handler,						//	The MPU fault handler
		bus_fault_handler,						//	The bus fault handler
		usage_fault_handler,					//	The usage fault handler
		0,										//	Reserved
		0,										//	Reserved
		0,										//	Reserved
		0,										//	Reserved
		svc_handler,							//	SVCall handler
		dg_monitor_handler,						//	Debug monitor handler
		0,										//	Reserved
		pendsv_handler,							//	The PendSV handler
		systick_handler,						//	The SysTick handler
		};

void sys_delay_us(volatile uint32_t count) {
	volatile uint32_t delay_value = 0;
	delay_value = count * 4;
	while(delay_value--);
}

void sys_delay_ms(volatile uint32_t count) {
	volatile uint32_t delay = 1000 * count;
	sys_delay_us(delay);
}

/*****************************************************************************/
/* static function defination                                                */
/*****************************************************************************/
void default_handler() {
}

void reset_handler() {
	uint32_t *pSrc	= &_ldata;
	uint32_t *pDest	= &_data;

	/* init system */
	SystemInit();

	/* copy init data from FLASH to SRAM */
	while(pDest < &_edata) {
		*pDest++ = *pSrc++;
	}

	/* zero bss */
	for (pDest = &_bss; pDest < &_ebss; pDest++) {
		*pDest = 0UL;
	}

	sys_cfg_tick();
	sys_cfg_console();

	/* entry app function */
	boot_main();
}

/***************************************/
/* cortex-M processor fault exceptions */
/***************************************/
void nmi_handler() {
}

void hard_fault_handler() {
}

void mem_manage_handler() {
}

void bus_fault_handler() {
}

void usage_fault_handler() {
}

void svc_handler () {
}


/*******************************************/
/* cortex-M processor non-fault exceptions */
/*******************************************/
void systick_handler() {
	sys_irq_timer_10ms();
}
