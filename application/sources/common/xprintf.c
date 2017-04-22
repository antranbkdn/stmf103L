/**
 ******************************************************************************
 * @Author: ThanNT
 * @Date:   13/08/2016
 ******************************************************************************
**/
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include "utils.h"
#include "xprintf.h"

#define CR_CRLF								(1)

#define DYMC_BUFFER_SIZE					(256)

#define DYMC_UART_SHELL_TYPE				(0x00)
#define DYMC_RF_REMOTE_TYPE					(0x01)

static uint8_t dymc_print_type = DYMC_UART_SHELL_TYPE;
static uint8_t dymc_print_buf[DYMC_BUFFER_SIZE];

void (*xfunc_out)(uint8_t );         /* Pointer to the output stream */

void xputc(uint8_t c) {
	if (CR_CRLF && (c == (uint8_t)'\n')) {
		xfunc_out('\r');
	}
	if (xfunc_out) {
		xfunc_out(c);
	}
}

void xprintf(const char *fmt, ...) {
	va_list va_args;
	uint32_t num, digit;
	int32_t i;
	int32_t zero_padding = 0;
	int32_t format_lenght = 0;
	int32_t base;
	int32_t minus;
	int8_t num_stack[11];
	uint8_t* ps;

	va_start(va_args, fmt);

	while (*fmt) {
		switch (*fmt) {
		case '%':
			zero_padding = 0;
			if (fmt[1] == '0') {
				zero_padding = 1;
				++fmt;
			}
			format_lenght = 0;
			while (*++fmt) {
				switch (*fmt) {
				case '%':
					xputc(*fmt);
					goto next_loop;

				case 'c':
					xputc(va_arg(va_args, int32_t));
					goto next_loop;

				case 'd':
				case 'X':
				case 'x':
					minus = 0;
					num = va_arg(va_args, uint32_t);
					if (*fmt == 'd') {
						base = 10;
						if (num & (uint32_t)0x80000000) {
							num = -(int32_t)num;
							minus = 1;
						}
					} else {
						base = 16;
					}
					for (digit = 0; digit < sizeof(num_stack);) {
						num_stack[digit++] = num%base;
						num /= base;
						if (num == 0) break;
					}
					if (minus) num_stack[digit++] = 0x7F;
					if (format_lenght > digit) {
						int8_t paddingint8_t = ' ';
						format_lenght -= digit;
						if (zero_padding)
							paddingint8_t = '0';
						while (format_lenght--) {
							xputc(paddingint8_t);
						}
					}
					for (i = digit-1; i >= 0; i--) {
						if (num_stack[i] == 0x7F) {
							xputc('-');
						} else if (num_stack[i] > 9) {
							xputc(num_stack[i]-10 + 'A');
						} else {
							xputc(num_stack[i] + '0');
						}
					}
					goto next_loop;

				case 's':
					ps = va_arg(va_args, uint8_t*);
					while(*ps) {
						xputc(*ps++);
					}
					goto next_loop;

				default:
					if (*fmt >= '0' && *fmt <= '9') {
						format_lenght = format_lenght*10 + (*fmt-'0');
					} else {
						goto exit;
					}
				}

			}
			if (*fmt == 0) {
				goto exit;
			}

		default:
			xputc(*fmt);
			break;
		}
next_loop:
		fmt++;
	}
exit:
	va_end(va_args);
}

int xsprintf(char* str,const char* fmt, ...) {
	va_list va_args;
	uint32_t num, digit;
	int32_t i;
	int32_t idx = 0;
	int32_t zero_padding = 0;
	int32_t format_lenght = 0;
	int32_t base;
	int32_t minus;
	int8_t num_stack[11];
	uint8_t* ps;

	va_start(va_args, fmt);

	while (*fmt) {
		switch (*fmt) {
		case '%':
			zero_padding = 0;
			if (fmt[1] == '0') {
				zero_padding = 1;
				++fmt;
			}
			format_lenght = 0;
			while (*++fmt) {
				switch (*fmt) {
				case '%':
					*(str + idx) = *fmt;
					idx++;
					goto next_loop;

				case 'c':
					*(str + idx) = va_arg(va_args, int32_t);
					idx++;
					goto next_loop;

				case 'd':
				case 'X':
				case 'x':
					minus = 0;
					num = va_arg(va_args, uint32_t);
					if (*fmt == 'd') {
						base = 10;
						if (num & (uint32_t)0x80000000) {
							num = -(int32_t)num;
							minus = 1;
						}
					} else {
						base = 16;
					}
					for (digit = 0; digit < sizeof(num_stack);) {
						num_stack[digit++] = num%base;
						num /= base;
						if (num == 0) break;
					}
					if (minus) num_stack[digit++] = 0x7F;
					if (format_lenght > digit) {
						int8_t paddingint8_t = ' ';
						format_lenght -= digit;
						if (zero_padding)
							paddingint8_t = '0';
						while (format_lenght--) {
							*(str + idx) = paddingint8_t;
							idx++;
						}
					}
					for (i = digit-1; i >= 0; i--) {
						if (num_stack[i] == 0x7F) {
							*(str + idx) = '-';
							idx++;
						} else if (num_stack[i] > 9) {
							*(str + idx) = num_stack[i]-10 + 'A';
							idx++;
						} else {
							*(str + idx) = num_stack[i] + '0';
							idx++;
						}
					}
					goto next_loop;

				case 's':
					ps = va_arg(va_args, uint8_t*);
					while(*ps) {
						*(str + idx) = *ps++;
						idx++;
					}
					goto next_loop;

				default:
					if (*fmt >= '0' && *fmt <= '9') {
						format_lenght = format_lenght*10 + (*fmt-'0');
					} else {
						goto exit;
					}
				}

			}
			if (*fmt == 0) {
				goto exit;
			}

		default:
			*(str + idx) = *fmt;
			idx ++;
			break;
		}
next_loop:
		fmt++;
	}
exit:
	va_end(va_args);
	return idx;
}

int set_dymc_output_type(uint8_t type) {
	int ret = -1;
	switch (type) {
	case DYMC_UART_SHELL_TYPE:
	case DYMC_RF_REMOTE_TYPE:
		dymc_print_type = type;
		ret = 0;
		break;

	default:
		break;
	}

	return ret;
}

void xdymcprintf(const char* fmt, ...) {
	va_list va_args;
	uint32_t num, digit;
	int32_t i;
	int32_t idx = 0;
	int32_t zero_padding = 0;
	int32_t format_lenght = 0;
	int32_t base;
	int32_t minus;
	int8_t num_stack[11];
	uint8_t* ps;

	va_start(va_args, fmt);

	mem_set(dymc_print_buf, 0, DYMC_BUFFER_SIZE);

	while (*fmt) {
		switch (*fmt) {
		case '%':
			zero_padding = 0;
			if (fmt[1] == '0') {
				zero_padding = 1;
				++fmt;
			}
			format_lenght = 0;
			while (*++fmt) {
				switch (*fmt) {
				case '%':
					*(dymc_print_buf + idx) = *fmt;
					idx++;
					goto next_loop;

				case 'c':
					*(dymc_print_buf + idx) = va_arg(va_args, int32_t);
					idx++;
					goto next_loop;

				case 'd':
				case 'X':
				case 'x':
					minus = 0;
					num = va_arg(va_args, uint32_t);
					if (*fmt == 'd') {
						base = 10;
						if (num & (uint32_t)0x80000000) {
							num = -(int32_t)num;
							minus = 1;
						}
					} else {
						base = 16;
					}
					for (digit = 0; digit < sizeof(num_stack);) {
						num_stack[digit++] = num%base;
						num /= base;
						if (num == 0) break;
					}
					if (minus) num_stack[digit++] = 0x7F;
					if (format_lenght > digit) {
						int8_t paddingint8_t = ' ';
						format_lenght -= digit;
						if (zero_padding)
							paddingint8_t = '0';
						while (format_lenght--) {
							*(dymc_print_buf + idx) = paddingint8_t;
							idx++;
						}
					}
					for (i = digit-1; i >= 0; i--) {
						if (num_stack[i] == 0x7F) {
							*(dymc_print_buf + idx) = '-';
							idx++;
						} else if (num_stack[i] > 9) {
							*(dymc_print_buf + idx) = num_stack[i]-10 + 'A';
							idx++;
						} else {
							*(dymc_print_buf + idx) = num_stack[i] + '0';
							idx++;
						}
					}
					goto next_loop;

				case 's':
					ps = va_arg(va_args, uint8_t*);
					while(*ps) {
						*(dymc_print_buf + idx) = *ps++;
						idx++;
					}
					goto next_loop;

				default:
					if (*fmt >= '0' && *fmt <= '9') {
						format_lenght = format_lenght*10 + (*fmt-'0');
					} else {
						goto exit;
					}
				}

			}
			if (*fmt == 0) {
				goto exit;
			}

		default:
			*(dymc_print_buf + idx) = *fmt;
			idx ++;
			break;
		}
next_loop:
		fmt++;
	}
exit:
	va_end(va_args);

	switch (dymc_print_type) {
	case DYMC_UART_SHELL_TYPE:
		xprintf("%s", dymc_print_buf);
		break;


	case DYMC_RF_REMOTE_TYPE:
		rf_printf(dymc_print_buf, idx);
		break;

	default:
		break;
	}
}
