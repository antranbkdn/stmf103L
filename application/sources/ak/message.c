/**
 ******************************************************************************
 * @author: ThanNT
 * @date:   13/08/2016
 ******************************************************************************
**/

#include "ak.h"
#include "ak_dbg.h"
#include "message.h"

#include "../common/utils.h"
#include "../sys/sys_dbg.h"

/* common pool memory */
static ak_msg_common_t    msg_common_pool[AK_COMMON_MSG_POOL_SIZE];
static ak_msg_t*        free_list_common_msg_pool;

/* pure pool memory */
static ak_msg_pure_t    msg_pure_pool[AK_PURE_MSG_POOL_SIZE];
static ak_msg_t*        free_list_pure_msg_pool;

static void common_msg_pool_init();
static void pure_msg_pool_init();

static void free_common_msg(ak_msg_t* msg);
static void free_pure_msg(ak_msg_t* msg);

static uint32_t	get_pool_msg_free(ak_msg_t* pool_msg);

void msg_init() {
	/* init common message pool */
	common_msg_pool_init();

	/* init pure message pool */
	pure_msg_pool_init();
}

void msg_free(ak_msg_t* msg) {
	uint8_t pool_type = 0;

	/* decrease reference counter of message */
	msg_dec_ref_count(msg);

	if (get_msg_ref_count(msg) == 0) {

		/* get type of message */
		pool_type = get_msg_type(msg);

		switch (pool_type) {
		case COMMON_MSG_TYPE:
			free_common_msg(msg);
			break;

		case PURE_MSG_TYPE:
			free_pure_msg(msg);
			break;

		case USER_MSG_TYPE:
			break;

		default:
			FATAL("MF", 0x20);
			break;

		}
	}
}

void msg_inc_ref_count(ak_msg_t* msg) {
	if (get_msg_ref_count(msg) < AK_MSG_REF_COUNT_MAX) {
		msg->ref_count++;
	}
	else {
		FATAL("MF", 0x21);
	}
}

void msg_dec_ref_count(ak_msg_t* msg) {
	if (get_msg_ref_count(msg) > 0) {
		msg->ref_count--;
	}
	else {
		FATAL("MF", 0x22);
	}
}

uint32_t	get_pool_msg_free(ak_msg_t* pool_msg) {
	uint32_t used = 0;
	ak_msg_t* head_pool = pool_msg;
	while(head_pool != AK_MSG_NULL) {
		used++;
		head_pool = head_pool->next;
	}
	return used;
}

/*****************************************************************************
 * common message function define.
 *****************************************************************************/
void common_msg_pool_init() {
	uint32_t index = 0;

	free_list_common_msg_pool = (ak_msg_t*)msg_common_pool;

	for (index = 0; index < AK_COMMON_MSG_POOL_SIZE; index++) {
		msg_common_pool[index].msg_header.ref_count |= COMMON_MSG_TYPE;
		if (index == (AK_COMMON_MSG_POOL_SIZE - 1)) {
			msg_common_pool[index].msg_header.next = AK_MSG_NULL;
		}
		else {
			msg_common_pool[index].msg_header.next = (ak_msg_t*)&msg_common_pool[index + 1];
		}
	}
}

uint32_t get_common_msg_pool_used() {
	return (AK_COMMON_MSG_POOL_SIZE - get_pool_msg_free((ak_msg_t*)free_list_common_msg_pool));
}

ak_msg_t* get_common_msg() {
	ak_msg_t* allocate_massage = AK_MSG_NULL;

	ENTRY_CRITICAL();
	allocate_massage = free_list_common_msg_pool;

	if (allocate_massage == AK_MSG_NULL) {
		FATAL("MF", 0x21);
		return (AK_MSG_NULL);
	}
	else {
		free_list_common_msg_pool = allocate_massage->next;
	}

	if (get_msg_type(allocate_massage) != COMMON_MSG_TYPE) {
		FATAL("MF", 0x27);
		return (AK_MSG_NULL);
	}

	msg_inc_ref_count(allocate_massage);
	EXIT_CRITICAL();

	return allocate_massage;
}

void free_common_msg(ak_msg_t* msg) {
	ENTRY_CRITICAL();
	msg->next = free_list_common_msg_pool;
	free_list_common_msg_pool = msg;
	EXIT_CRITICAL();
}

uint8_t set_data_common_msg(ak_msg_t* msg, uint8_t* data, uint8_t size) {
	ak_msg_common_t* msg_common = (ak_msg_common_t*)msg;

	/* check messge null */
	if ((ak_msg_t*)msg_common == AK_MSG_NULL) {
		FATAL("MF", 0x22);
		return AK_MSG_NG;
	}

	/* check message type */
	if ((msg_common->msg_header.ref_count & AK_MSG_TYPE_MASK) != COMMON_MSG_TYPE) {
		FATAL("MF", 0x23);
		return AK_MSG_NG;
	}

	/* check data lenght */
	if (size > AK_COMMON_MSG_DATA_SIZE) {
		FATAL("MF", 0x24);
		return AK_MSG_NG;
	}

	/* set data message */
	msg_common->len = size;
	mem_cpy(msg_common->data, data, size);
	return AK_MSG_OK;
}

uint8_t* get_data_common_msg(ak_msg_t* msg) {
	ak_msg_common_t* msg_common = (ak_msg_common_t*)msg;

	/* check messge null */
	if ((ak_msg_t*)msg_common == AK_MSG_NULL) {
		FATAL("MF", 0x25);
		return (uint8_t*)0;
	}

	/* check message type */
	if ((msg_common->msg_header.ref_count & AK_MSG_TYPE_MASK) != COMMON_MSG_TYPE) {
		FATAL("MF", 0x26);
		return (uint8_t*)0;
	}

	return (uint8_t*)msg_common->data;
}

uint8_t get_data_len_common_msg(ak_msg_t* msg) {
	ak_msg_common_t* msg_common = (ak_msg_common_t*)msg;

	/* check messge null */
	if ((ak_msg_t*)msg_common == AK_MSG_NULL) {
		FATAL("MF", 0x38);
		return ((uint8_t)0);
	}

	return ((uint8_t)msg_common->len);
}

/*****************************************************************************
 * pure message function define.
 *****************************************************************************/
void pure_msg_pool_init() {
	uint32_t index = 0;

	free_list_pure_msg_pool = (ak_msg_t*)msg_pure_pool;

	for (index = 0; index < AK_PURE_MSG_POOL_SIZE; index++) {
		msg_pure_pool[index].msg_header.ref_count |= PURE_MSG_TYPE;
		if (index == (AK_PURE_MSG_POOL_SIZE - 1)) {
			msg_pure_pool[index].msg_header.next = AK_MSG_NULL;
		}
		else {
			msg_pure_pool[index].msg_header.next = (ak_msg_t*)&msg_pure_pool[index + 1];
		}
	}
}

uint32_t get_pure_msg_pool_used() {
	return (AK_PURE_MSG_POOL_SIZE - get_pool_msg_free((ak_msg_t*)free_list_common_msg_pool));
}

ak_msg_t* get_pure_msg() {
	ak_msg_t* allocate_massage = AK_MSG_NULL;

	ENTRY_CRITICAL();
	allocate_massage = free_list_pure_msg_pool;

	if (allocate_massage == AK_MSG_NULL) {
		FATAL("MF", 0x31);
		return (AK_MSG_NULL);
	}
	else {
		free_list_pure_msg_pool = allocate_massage->next;
	}

	if (get_msg_type(allocate_massage) != PURE_MSG_TYPE) {
		FATAL("MF", 0x37);
		return (AK_MSG_NULL);
	}

	msg_inc_ref_count(allocate_massage);
	EXIT_CRITICAL();

	return allocate_massage;
}

void free_pure_msg(ak_msg_t* msg) {
	ENTRY_CRITICAL();
	msg->next = free_list_pure_msg_pool;
	free_list_pure_msg_pool = msg;
	EXIT_CRITICAL();
}
