/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "t_rpc_ver3_enumdata.h"
#include "t_rpc_ver3_msgdata.h"
#include "t_rpc_ver3_structdata.h"
#include "t_rpc_ver3_metadata.h"
#include "tools_log.h"

static TLock _metadata_pv;
static MBUF *_mbuf_auto_free = NULL;
static dave_bool _work_local_flag = dave_true;

/*
 * This is for action t_bson_array_ins_mbuf, 
 * delayed release of memory
 */
static inline void
_t_rpc_ver3_insert_mbuf(MBUF *mbuf_data)
{
	if(mbuf_data == NULL)
		return;

	t_lock_spin(&_metadata_pv);
	_mbuf_auto_free = dave_mchain(_mbuf_auto_free, mbuf_data);
	t_unlock_spin(&_metadata_pv);
}

static inline void
_t_rpc_ver3_leave_mbuf(void)
{
	MBUF *mbuf_data;

	t_lock_spin(&_metadata_pv);
	mbuf_data = _mbuf_auto_free;
	_mbuf_auto_free = NULL;
	t_unlock_spin(&_metadata_pv);

	if(mbuf_data != NULL)
		dave_mfree(mbuf_data);
}

static inline void *
_t_rpc_ver3_zip_bin(u8 *zip_data, ub zip_len)
{
	void *pArrayBson = t_bson_malloc_array();

	t_bson_array_ins_bin(pArrayBson, (char *)zip_data, (int)zip_len);

	return pArrayBson;
}

static inline void *
_t_rpc_ver3_zip_mbuf(MBUF *mbuf_data)
{
	void *pArrayBson = t_bson_malloc_array();

	t_bson_array_ins_mbuf(pArrayBson, mbuf_data);

	_t_rpc_ver3_insert_mbuf(mbuf_data);

	return pArrayBson;
}

static inline ub
_t_rpc_ver3_unzip_bin(u8 *unzip_data, ub unzip_len, void *pArrayBson)
{
	size_t real_data_len;

	if(pArrayBson == NULL)
	{
		TOOLSLTRACE(360,1,"the pBson is NULL!");
		unzip_data[0] = '\0';
		return 0;
	}

	real_data_len = (int)unzip_len;

	if(t_bson_array_cpy_bin(pArrayBson, 0, (char *)unzip_data, &real_data_len) == dave_false)
		return 0;

	if((size_t)unzip_len > real_data_len)
	{
		dave_memset(&unzip_data[real_data_len], 0x00, (int)unzip_len-real_data_len);
	}

	return (ub)real_data_len;
}

static inline void *
_t_rpc_ver3_zip_sb(sb zip_data)
{
	void *pArrayBson = t_bson_malloc_array();

	t_bson_array_add_int64(pArrayBson, (u64)zip_data);

	return pArrayBson;
}

static inline dave_bool
_t_rpc_ver3_unzip_sb(void *pArrayBson, sb *sb_data, s8 *fun, ub line)
{
	u64 u64_data;

	if(pArrayBson == NULL)
	{
		TOOLSLTRACE(360,1,"the pArrayBson is NULL! <%s:%d>", fun, line);
		*sb_data = 0;
		/*
		 * Maybe the version change is missing this variable.
		 */
		return dave_true;
	}

	if(t_bson_array_inq_int64(pArrayBson, 0, &u64_data) == false)
	{
		TOOLSLOG("inq int64 failed! <%s:%d>", fun, line);
		*sb_data = 0;
		return dave_false;
	}

	*sb_data = (sb)u64_data;

	return dave_true;
}

static inline void *
_t_rpc_ver3_zip_double(double zip_data)
{
	void *pArrayBson = t_bson_malloc_array();

	t_bson_array_add_double(pArrayBson, zip_data);

	return pArrayBson;
}

static inline dave_bool
_t_rpc_ver3_unzip_double(void *pArrayBson, double *double_data, s8 *fun, ub line)
{
	if(pArrayBson == NULL)
	{
		TOOLSLTRACE(360,1,"the pArrayBson is NULL!");
		*double_data = 0;
		return dave_true;
	}

	if(t_bson_array_inq_double(pArrayBson, 0, double_data) == false)
	{
		TOOLSLOG("inq double failed! <%s:%d>", fun, line);
		*double_data = 0;
		return dave_false;
	}

	return dave_true;
}

static inline void *
_t_rpc_ver3_zip_void_ptr(void *zip_data)
{
	return _t_rpc_ver3_zip_sb((sb)zip_data);
}

static inline dave_bool
_t_rpc_ver3_unzip_void_ptr(void **unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	sb sb_data;

	if(_t_rpc_ver3_unzip_sb(pArrayBson, &sb_data, fun, line) == dave_false)
	{
		*unzip_data = NULL;
		return dave_false;
	}

	*unzip_data = (void *)sb_data;
	return dave_true;
}

static inline void *
__t_rpc_ver3_zip_MBUF_ptr_local__(MBUF *zip_data, s8 *fun, ub line)
{
	void *pBson, *pArrayBson;
	s8 temp_buffer[128];

	pBson = t_bson_malloc_object();

	if(zip_data != NULL)
	{
		pArrayBson = _t_rpc_ver3_zip_mbuf(zip_data);
		dave_snprintf(temp_buffer, sizeof(temp_buffer), "array_len_%d", 0);
		t_bson_add_int(pBson, temp_buffer, zip_data->tot_len);
		dave_snprintf(temp_buffer, sizeof(temp_buffer), "array_%d", 0);
		t_bson_add_object(pBson, temp_buffer, pArrayBson);

		t_bson_add_int(pBson, "array_number", 1);
	}
	else
	{
		t_bson_add_int(pBson, "array_number", 0);
	}

	return pBson;
}

static dave_bool
__t_rpc_ver3_unzip_MBUF_ptr_local__(MBUF **unzip_data, void *pBson, s8 *fun, ub line)
{
	int array_index, array_number, array_len;
	void *pArrayBson;
	s8 temp_buffer[128];
	MBUF *unzip_data_loop;

	*unzip_data = NULL;

	t_bson_inq_int(pBson, "array_number", &array_number);

	array_index = 0;

	while(array_index < array_number)
	{
		dave_snprintf(temp_buffer, sizeof(temp_buffer), "array_len_%d", array_index);
		t_bson_inq_int(pBson, temp_buffer, &array_len);
		dave_snprintf(temp_buffer, sizeof(temp_buffer), "array_%d", array_index);
		pArrayBson = t_bson_inq_object(pBson, (char *)temp_buffer);

		unzip_data_loop = __base_mmalloc__(array_len + 1, fun, line);
		if(t_rpc_ver3_unzip_u8_d((u8 *)(unzip_data_loop->payload), (ub)array_len, pArrayBson) == dave_false)
		{
			TOOLSLTRACE(360,1,"can't find data on bson");
			dave_mfree(unzip_data_loop);
		}
		else
		{
			unzip_data_loop->tot_len = unzip_data_loop->len = array_len;
			((s8 *)(unzip_data_loop->payload))[unzip_data_loop->len] = '\0';

			*unzip_data = dave_mchain(*unzip_data, unzip_data_loop);
		}

		array_index ++;
	}

	return dave_true;
}

static inline void *
__t_rpc_ver3_zip_MBUF_ptr_remote__(MBUF *zip_data, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_void_ptr((void *)zip_data);
}

static inline dave_bool
__t_rpc_ver3_unzip_MBUF_ptr_remote__(MBUF **unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	return _t_rpc_ver3_unzip_void_ptr((void **)unzip_data, pArrayBson, fun, line);
}

// =====================================================================

void
t_rpc_ver3_metadata_init(void)
{
	t_lock_reset(&_metadata_pv);
	_mbuf_auto_free = NULL;
}

void
t_rpc_ver3_metadata_exit(void)
{
	_t_rpc_ver3_leave_mbuf();
}

void
t_rpc_ver3_leave_mbuf(void)
{
	_t_rpc_ver3_leave_mbuf();
}

void
t_rpc_ver3_metadata_work(dave_bool work_flag)
{
	_work_local_flag = work_flag;
}

void *
__t_rpc_ver3_zip_dave_bool__(dave_bool zip_data, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_sb((sb)zip_data);
}

dave_bool
__t_rpc_ver3_unzip_dave_bool__(dave_bool *unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	sb sb_data;

	if(_t_rpc_ver3_unzip_sb(pArrayBson, &sb_data, fun, line) == dave_false)
	{
		*unzip_data = dave_false;
		return dave_false;
	}

	*unzip_data = (dave_bool)sb_data;
	return dave_true;
}

void *
__t_rpc_ver3_zip_char_d__(char *zip_data, ub zip_h_len, ub zip_l_len, s8 *fun, ub line)
{
	void *pArrayBson = t_bson_malloc_array();
	char *str_data;
	ub zip_index, str_len;

	for(zip_index=0; zip_index<zip_h_len; zip_index++)
	{
		str_data = &zip_data[zip_index * zip_l_len];

		str_len = dave_strlen(str_data);
		if(str_len >= zip_l_len)
		{
			TOOLSABNOR("Character is out of range or does not end with 0! %d/%d/%d <%s:%d>",
				str_len, zip_l_len, zip_h_len, fun, line);

			str_len = zip_l_len - 1;
		}

		t_bson_array_ins_bin(pArrayBson, str_data, str_len);
	}

	return pArrayBson;
}

dave_bool
__t_rpc_ver3_unzip_char_d__(char *unzip_data, ub unzip_h_len, ub unzip_l_len, void *pArrayBson, s8 *fun, ub line)
{
	size_t array_len = t_bson_array_number(pArrayBson);
	char *str_data;
	size_t unzip_index, str_len;

	for(unzip_index=0; unzip_index<array_len; unzip_index++)
	{
		if(t_bson_array_inq_bin(pArrayBson, unzip_index, &str_data, &str_len) == dave_true)
		{
			if(str_len >= unzip_l_len)
			{
				TOOLSABNOR("Character is out of range or does not end with 0! %d/%d <%s:%d>",
						str_len, unzip_l_len, fun, line);
				str_len = unzip_l_len - 1;
			}

			dave_memcpy(&unzip_data[unzip_index * unzip_l_len], str_data, str_len);
			// for UTF-8 decode.
			while(str_len < unzip_l_len)
			{
				(&unzip_data[unzip_index * unzip_l_len])[str_len ++] = '\0';
			}
		}
		else
		{
			dave_memset((s8 *)(&unzip_data[unzip_index * unzip_l_len]), 0x00, unzip_l_len);
		}
	}

	while(unzip_index < unzip_h_len)
	{
		dave_memset(&unzip_data[unzip_index * unzip_l_len], 0x00, unzip_l_len);
		unzip_index ++;
	}

	return dave_true;
}

void *
__t_rpc_ver3_zip_int__(int zip_data, s8 *fun, ub line)
{
	return t_rpc_ver3_zip_sb((sb)zip_data);
}

dave_bool
__t_rpc_ver3_unzip_int__(int *unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	sb sb_data;

	if(_t_rpc_ver3_unzip_sb(pArrayBson, &sb_data, fun, line) == dave_false)
	{
		*unzip_data = 0;
		return dave_false;
	}

	*unzip_data = (int)sb_data;
	return dave_true;
}

void *
__t_rpc_ver3_zip_s8__(s8 zip_data, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_sb((sb)zip_data);
}

dave_bool
__t_rpc_ver3_unzip_s8__(s8 *unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	sb sb_data;

	if(_t_rpc_ver3_unzip_sb(pArrayBson, &sb_data, fun, line) == false)
	{
		*unzip_data = 0x00;
		return dave_false;
	}

	*unzip_data = (s8)sb_data;
	return dave_true;
}

void *
__t_rpc_ver3_zip_s8_d__(s8 *zip_data, ub zip_h_len, ub zip_l_len, s8 *fun, ub line)
{
	return __t_rpc_ver3_zip_char_d__((char *)zip_data, zip_h_len, zip_l_len, fun, line);
}

dave_bool
__t_rpc_ver3_unzip_s8_d__(s8 *unzip_data, ub unzip_h_len, ub unzip_l_len, void *pArrayBson, s8 *fun, ub line)
{
	return __t_rpc_ver3_unzip_char_d__((char *)unzip_data, unzip_h_len, unzip_l_len, pArrayBson, fun, line);
}

void *
__t_rpc_ver3_zip_u8__(u8 zip_data, s8 *fun, ub line)
{
	return __t_rpc_ver3_zip_s8__((s8)zip_data, fun, line);
}

dave_bool
__t_rpc_ver3_unzip_u8__(u8 *unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	return __t_rpc_ver3_unzip_s8__((s8 *)unzip_data, pArrayBson, fun, line);
}

void *
__t_rpc_ver3_zip_u8_d__(u8 *zip_data, ub zip_len, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_bin(zip_data, zip_len);
}

dave_bool
__t_rpc_ver3_unzip_u8_d__(u8 *unzip_data, ub unzip_len, void *pArrayBson, s8 *fun, ub line)
{
	unzip_len = _t_rpc_ver3_unzip_bin(unzip_data, unzip_len, pArrayBson);

	if(unzip_len == 0)
		return dave_false;
	return dave_true;
}

void *
__t_rpc_ver3_zip_s16__(s16 zip_data, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_sb((sb)zip_data);
}

dave_bool
__t_rpc_ver3_unzip_s16__(s16 *unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	sb sb_data;

	if(_t_rpc_ver3_unzip_sb(pArrayBson, &sb_data, fun, line) == dave_false)
	{
		*unzip_data = 0;
		return dave_false;
	}

	*unzip_data = (s16)sb_data;
	return dave_true;
}

void *
__t_rpc_ver3_zip_u16__(u16 zip_data, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_sb((sb)zip_data);
}

dave_bool
__t_rpc_ver3_unzip_u16__(u16 *unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	sb sb_data;

	if(_t_rpc_ver3_unzip_sb(pArrayBson, &sb_data, fun, line) == dave_false)
	{
		*unzip_data = 0;
		return dave_false;
	}

	*unzip_data = (u16)sb_data;
	return dave_true;
}

void *
__t_rpc_ver3_zip_s32__(s32 zip_data, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_sb((sb)zip_data);
}

dave_bool
__t_rpc_ver3_unzip_s32__(s32 *unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	sb sb_data;

	if(_t_rpc_ver3_unzip_sb(pArrayBson, &sb_data, fun, line) == dave_false)
	{
		*unzip_data = 0;
		return dave_false;
	}

	*unzip_data = (s32)sb_data;
	return dave_true;
}

void *
__t_rpc_ver3_zip_u32__(u32 zip_data, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_sb((sb)zip_data);
}

dave_bool
__t_rpc_ver3_unzip_u32__(u32 *unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	sb sb_data;

	if(_t_rpc_ver3_unzip_sb(pArrayBson, &sb_data, fun, line) == dave_false)
	{
		*unzip_data = 0;
		return dave_false;
	}

	*unzip_data = (u32)sb_data;
	return dave_true;
}

void *
__t_rpc_ver3_zip_s64__(s64 zip_data, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_sb((sb)zip_data);
}

dave_bool
__t_rpc_ver3_unzip_s64__(s64 *unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	sb sb_data;

	if(_t_rpc_ver3_unzip_sb(pArrayBson, &sb_data, fun, line) == dave_false)
	{
		*unzip_data = 0;
		return dave_false;
	}

	*unzip_data = (s64)sb_data;
	return dave_true;
}

void *
__t_rpc_ver3_zip_u64__(u64 zip_data, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_sb((sb)zip_data);
}

dave_bool
__t_rpc_ver3_unzip_u64__(u64 *unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	sb sb_data;

	if(_t_rpc_ver3_unzip_sb(pArrayBson, &sb_data, fun, line) == dave_false)
	{
		*unzip_data = 0;
		return dave_false;
	}

	*unzip_data = (u64)sb_data;
	return dave_true;
}

void *
__t_rpc_ver3_zip_sb__(sb zip_data, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_sb(zip_data);
}

dave_bool
__t_rpc_ver3_unzip_sb__(sb *unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	return _t_rpc_ver3_unzip_sb(pArrayBson, unzip_data, fun, line);
}

void *
__t_rpc_ver3_zip_ub__(ub zip_data, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_sb((ub)zip_data);
}

dave_bool
__t_rpc_ver3_unzip_ub__(ub *unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	return _t_rpc_ver3_unzip_sb(pArrayBson, (sb *)unzip_data, fun, line);
}

void *
__t_rpc_ver3_zip_ub_d__(ub *zip_data, ub zip_len, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_bin((u8 *)zip_data, sizeof(ub) * zip_len);
}

dave_bool
__t_rpc_ver3_unzip_ub_d__(ub *unzip_data, ub unzip_len, void *pArrayBson, s8 *fun, ub line)
{
	_t_rpc_ver3_unzip_bin((u8 *)unzip_data, sizeof(ub) * unzip_len, pArrayBson);
	return dave_true;
}

void *
__t_rpc_ver3_zip_float__(float zip_data, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_double((double)zip_data);
}

dave_bool
__t_rpc_ver3_unzip_float__(float *unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	double double_data;

	if(_t_rpc_ver3_unzip_double(pArrayBson, &double_data, fun, line) == dave_false)
	{
		*unzip_data = 0;
		return dave_false;
	}

	*unzip_data  = (float)double_data;
	return dave_true;
}

void *
__t_rpc_ver3_zip_float_d__(float *zip_data, ub zip_len, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_bin((u8 *)zip_data, sizeof(float) * zip_len);
}

dave_bool
__t_rpc_ver3_unzip_float_d__(float *unzip_data, ub unzip_len, void *pArrayBson, s8 *fun, ub line)
{
	_t_rpc_ver3_unzip_bin((u8 *)unzip_data, sizeof(float) * unzip_len, pArrayBson);
	return dave_true;
}

void *
__t_rpc_ver3_zip_double__(double zip_data, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_double(zip_data);
}

dave_bool
__t_rpc_ver3_unzip_double__(double *unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	return _t_rpc_ver3_unzip_double(pArrayBson, unzip_data, fun, line);
}

void *
__t_rpc_ver3_zip_double_d__(double *zip_data, ub zip_len, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_bin((u8 *)zip_data, sizeof(double) * zip_len);
}

dave_bool
__t_rpc_ver3_unzip_double_d__(double *unzip_data, ub unzip_len, void *pArrayBson, s8 *fun, ub line)
{
	_t_rpc_ver3_unzip_bin((u8 *)unzip_data, sizeof(double) * unzip_len, pArrayBson);
	return dave_true;
}

void *
__t_rpc_ver3_zip_ThreadId__(ThreadId zip_data, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_sb((sb)zip_data);
}

dave_bool
__t_rpc_ver3_unzip_ThreadId__(ThreadId *unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	sb sb_data;

	if(_t_rpc_ver3_unzip_sb(pArrayBson, &sb_data, fun, line) == dave_false)
	{
		*unzip_data = INVALID_THREAD_ID;
		return dave_false;
	}

	*unzip_data = (ThreadId)sb_data;
	return dave_true;
}

void *
__t_rpc_ver3_zip_MBUF_ptr__(MBUF *zip_data, s8 *fun, ub line)
{
	if(_work_local_flag == dave_true)
	{
		return __t_rpc_ver3_zip_MBUF_ptr_local__(zip_data, fun, line);
	}
	else
	{
		return __t_rpc_ver3_zip_MBUF_ptr_remote__(zip_data, fun, line);
	}
}

dave_bool
__t_rpc_ver3_unzip_MBUF_ptr__(MBUF **unzip_data, void *pBson, s8 *fun, ub line)
{
	if(_work_local_flag == dave_true)
	{
		return __t_rpc_ver3_unzip_MBUF_ptr_local__(unzip_data, pBson, fun, line);
	}
	else
	{
		return __t_rpc_ver3_unzip_MBUF_ptr_remote__(unzip_data, pBson, fun, line);
	}
}

void *
__t_rpc_ver3_zip_s8_ptr__(s8 *zip_data, s8 *fun, ub line)
{
	TOOLSLTRACE(360,1,"Please note that RPC does not allow passing the pointer value! <%s:%d>", fun, line);

	return _t_rpc_ver3_zip_void_ptr(zip_data);
}

dave_bool
__t_rpc_ver3_unzip_s8_ptr__(s8 **unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	TOOLSLTRACE(360,1,"Please note that RPC does not allow passing the pointer value! <%s:%d>", fun, line);

	return _t_rpc_ver3_unzip_void_ptr((void **)unzip_data, pArrayBson, fun, line);
}

void *
__t_rpc_ver3_zip_u8_ptr__(u8 *zip_data, s8 *fun, ub line)
{
	TOOLSLTRACE(360,1,"Please note that RPC does not allow passing the pointer value! <%s:%d>", fun, line);

	return _t_rpc_ver3_zip_void_ptr(zip_data);
}

dave_bool
__t_rpc_ver3_unzip_u8_ptr__(u8 **unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	TOOLSLTRACE(360,1,"Please note that RPC does not allow passing the pointer value! <%s:%d>", fun, line);

	return _t_rpc_ver3_unzip_void_ptr((void **)unzip_data, pArrayBson, fun, line);
}

void *
__t_rpc_ver3_zip_float_ptr__(float *zip_data, s8 *fun, ub line)
{
	TOOLSLTRACE(360,1,"Please note that RPC does not allow passing the pointer value! <%s:%d>", fun, line);

	return _t_rpc_ver3_zip_void_ptr(zip_data);
}

dave_bool
__t_rpc_ver3_unzip_float_ptr__(float **unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	TOOLSLTRACE(360,1,"Please note that RPC does not allow passing the pointer value! <%s:%d>", fun, line);

	return _t_rpc_ver3_unzip_void_ptr((void **)unzip_data, pArrayBson, fun, line);
}

void *
__t_rpc_ver3_zip_void_ptr__(void *zip_data, s8 *fun, ub line)
{
	return _t_rpc_ver3_zip_void_ptr(zip_data);
}

dave_bool
__t_rpc_ver3_unzip_void_ptr__(void **unzip_data, void *pArrayBson, s8 *fun, ub line)
{
	return _t_rpc_ver3_unzip_void_ptr(unzip_data, pArrayBson, fun, line);
}

