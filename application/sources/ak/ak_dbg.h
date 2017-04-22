#ifndef __AK_DBG_H__
#define __AK_DBG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>
#include "ak.h"

#define		MSG_DBG_EN				AK_DISABLE
#define		TIM_DBG_EN				AK_DISABLE
#define		KERNEL_DBG_EN			AK_DISABLE

#if defined TIVA_DBG
#include "../common/xprintf.h"

#if MSG_DBG_EN==AK_ENABLE
#define MSG_DBG(fmt, ...)       xprintf(fmt, ##__VA_ARGS__)
#else
#define MSG_DBG(fmt, ...)
#endif

#if TIM_DBG_EN==AK_ENABLE
#define TIM_DBG(fmt, ...)       xprintf(fmt, ##__VA_ARGS__)
#else
#define TIM_DBG(fmt, ...)
#endif

#if KERNEL_DBG_EN==AK_ENABLE
#define KERNEL_DBG(fmt, ...)    xprintf(fmt, ##__VA_ARGS__)
#else
#define KERNEL_DBG(fmt, ...)
#endif

#elif defined STM8_DBG
#include "../common/xprintf.h"

#if MSG_DBG_EN==AK_ENABLE
#define MSG_DBG(fmt, ...)       xprintf(fmt, ##__VA_ARGS__)
#else
#define MSG_DBG(fmt, ...)
#endif

#if TIM_DBG_EN==AK_ENABLE
#define TIM_DBG(fmt, ...)       xprintf(fmt, ##__VA_ARGS__)
#else
#define TIM_DBG(fmt, ...)
#endif

#if KERNEL_DBG_EN==AK_ENABLE
#define KERNEL_DBG(fmt, ...)    xprintf(fmt, ##__VA_ARGS__)
#else
#define KERNEL_DBG(fmt, ...)
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif //__AK_DBG_H__
