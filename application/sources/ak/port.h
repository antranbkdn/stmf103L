/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   13/08/2016
 ******************************************************************************
**/

#ifndef __PORT_H__
#define __PORT_H__

#if defined(TIVA_PLATFORM)
#include "../platform/tiva/tiva.h"
#elif defined(STM32L_PLATFORM)
#include "../platform/stm32l/stm32l.h"
#elif defined(STM32F10X_PLATFORM)
#include "../platform/stm32f10x/stm32.h"
#else
#error Please choose platform for kernel.
#endif

#endif //__PORT_H__
