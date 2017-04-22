#ifndef __APP_DBG_H__
#define __APP_DBG_H__

#if defined(GCC_PLATFORM)
#include <stdio.h>

#if defined(APP_DBG_EN)
#define APP_DBG(fmt, ...)       printf(fmt, ##__VA_ARGS__)
#else
#define APP_DBG(fmt, ...)
#endif

#if defined(APP_PRINT_EN)
#define APP_PRINT(fmt, ...)       printf(fmt, ##__VA_ARGS__)
#else
#define APP_PRINT(fmt, ...)
#endif
#endif

#if defined(TIVA_PLATFORM)
#include <stdbool.h>
#include <stdint.h>
#include "../platform/tiva/utils/uartstdio.h"

#if defined(APP_DBG_EN)
#define APP_DBG(fmt, ...)       UARTprintf((const char*)fmt, ##__VA_ARGS__)
#else
#define APP_DBG(fmt, ...)
#endif

#if defined(APP_PRINT_EN)
#define APP_PRINT(fmt, ...)       UARTprintf((const char*)fmt, ##__VA_ARGS__)
#else
#define APP_PRINT(fmt, ...)
#endif

#endif

#if defined(STM32L_PLATFORM)
#include "../common/xprintf.h"

#if defined(APP_DBG_EN)
#define APP_DBG(fmt, ...)       xprintf((const char*)fmt, ##__VA_ARGS__)
#else
#define APP_DBG(fmt, ...)
#endif

#if defined(APP_PRINT_EN)
#define APP_PRINT(fmt, ...)       xprintf((const char*)fmt, ##__VA_ARGS__)
#else
#define APP_PRINT(fmt, ...)
#endif

#if defined(LOGIN_PRINT_EN)
#define LOGIN_PRINT(fmt, ...)       xdymcprintf((const char*)fmt, ##__VA_ARGS__)
#else
#define LOGIN_PRINT(fmt, ...)
#endif
#endif

#endif //__APP_DBG_H__
