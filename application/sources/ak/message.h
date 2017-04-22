/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   13/08/2016
 ******************************************************************************
**/

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "ak.h"

#define AK_MSG_NULL ((ak_msg_t*)0)

#define AK_MSG_NG					(0)
#define AK_MSG_OK					(1)

#define AK_COMMON_MSG_POOL_SIZE		(32)
#define AK_COMMON_MSG_DATA_SIZE		(64)

#define AK_PURE_MSG_POOL_SIZE		(64)

#define AK_MSG_TYPE_MASK			(0xC0)
#define AK_MSG_REF_COUNT_MASK		(0x3F)

#define AK_MSG_REF_COUNT_MAX		(7)

#define get_msg_ref_count(x)		((((ak_msg_t*)x)->ref_count) & AK_MSG_REF_COUNT_MASK)
#define get_msg_type(x)				((((ak_msg_t*)x)->ref_count) & AK_MSG_TYPE_MASK)

typedef struct {
	/* time of message handler */
	uint32_t	start_post;
	uint32_t	start_exe;
	uint32_t	stop_exe;
} dbg_handler_t;

typedef struct ak_msg_t {
	/*******************************
	 * private for kernel.
	 ******************************/
	/* message management */
	struct ak_msg_t*	next;

	/*******************************
	 * kernel debug.
	 ******************************/
	/* task debug */
#if (AK_TASK_DEBUG == AK_ENABLE)
	dbg_handler_t		dbg_handler;
#endif

	/* task header */
	uint8_t				task_id;
	uint8_t				ref_count;
	uint8_t				sig;
	uint8_t				reserved;

	/*******************************
	 * public for user application.
	 ******************************/
	/* external task header */
	uint8_t				if_task_id;
	uint8_t				if_type;
	uint8_t				if_sig;
	uint8_t				if_reserved;
} ak_msg_t;

typedef struct {
	ak_msg_t    msg_header;
	uint8_t     len;
	uint8_t     data[AK_COMMON_MSG_DATA_SIZE];
} ak_msg_common_t;

typedef struct {
	ak_msg_t    msg_header;
} ak_msg_pure_t;

typedef struct {
	uint8_t task_id;
	uint8_t sig;
} ak_msg_pure_if_t;

typedef struct {
	uint8_t task_id;
	uint8_t sig;

	uint8_t len;
	uint8_t data[AK_COMMON_MSG_DATA_SIZE];
} ak_msg_common_if_t;

#define set_msg_sig(m, s)		(((ak_msg_t*)m)->sig = s)
#define set_msg_task_id(m, t)		(((ak_msg_t*)m)->task_id = t)

/* external if interface */
#define set_if_task_id(m, t)	(((ak_msg_t*)m)->if_task_id = t)
#define set_if_type(m, t)		(((ak_msg_t*)m)->if_type = t)
#define set_if_sig(m, s)		(((ak_msg_t*)m)->if_sig = s)
#define set_if_data_common_message(m, d, s) \
	set_data_common_msg(m, d, s)

extern void msg_init();
extern void msg_free(ak_msg_t* msg);
extern void msg_inc_ref_count(ak_msg_t* msg);
extern void msg_dec_ref_count(ak_msg_t* msg);

/*****************************************************************************
 * DEFINITION: common message
 *
 *****************************************************************************/
#define COMMON_MSG_TYPE					(0xC0)
#define PURE_MSG_TYPE					(0x80)
#define USER_MSG_TYPE					(0x40)

extern ak_msg_t* get_common_msg();
extern uint32_t  get_common_msg_pool_used();
extern uint8_t   set_data_common_msg(ak_msg_t* msg, uint8_t* data, uint8_t size);
extern uint8_t*  get_data_common_msg(ak_msg_t* msg);
extern uint8_t   get_data_len_common_msg(ak_msg_t* msg);

extern ak_msg_t* get_pure_msg();
extern uint32_t  get_pure_msg_pool_used();

#ifdef __cplusplus
}
#endif

#endif //__MESSAGE_H__
