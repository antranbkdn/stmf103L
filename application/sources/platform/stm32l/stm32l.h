/**
 ******************************************************************************
 * @Author: ThanNT
 * @Date:   05/09/2016
 ******************************************************************************
**/
#ifndef __STM32L_H__
#define __STM32L_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if defined(STM32L_PLATFORM)

extern void enable_interrupts();
extern void disable_interrupts();
extern void entry_critical();
extern void exit_critical();

#define ENTRY_CRITICAL()            entry_critical()
#define EXIT_CRITICAL()             exit_critical()
#define ENABLE_INTERRUPTS()         enable_interrupts()
#define DISABLE_INTERRUPTS()        disable_interrupts()

#endif

#ifdef __cplusplus
}
#endif

#endif //__STM32L_H__
