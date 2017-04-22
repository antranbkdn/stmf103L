/**
 ******************************************************************************
 * @Author: ThanNT
 * @Date:   13/08/2016
 ******************************************************************************
**/
#ifndef __XPRINTF_H__
#define __XPRINTF_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdarg.h>
#include <stdint.h>

#define DYMC_UART_SHELL_TYPE				(0x00)
#define DYMC_RF_REMOTE_TYPE					(0x01)

#define DYMC_DEFAULD_TYPE					DYMC_UART_SHELL_TYPE

extern void (*xfunc_out)(uint8_t);
extern void xprintf(const char* fmt, ...);
extern int xsprintf(char* str, const char* fmt, ...);

extern int set_dymc_output_type(uint8_t);
extern void rf_printf(uint8_t* buf, uint32_t len);
extern void xdymcprintf(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif //__XPRINTF_H__
