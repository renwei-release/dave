/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "t_bson_define.h"
#include "t_bson_json.h"
#include "json_object_private.h"
#include "tools_log.h"

static inline void
_t_bson_json_boolean(json_object *pJson, tBsonData *pData)
{
	if(pJson->o_type == json_type_object)
		json_object_object_add(pJson,
			(const char *)pData->key_ptr,
			json_object_new_boolean((json_bool)(pData->value_ptr.bool_value)));
	else
		json_object_array_add(pJson,
			json_object_new_boolean((json_bool)(pData->value_ptr.bool_value)));	
}

static inline void
_t_json_bson_boolean(tBsonObject *pBson, char *key, json_object *pJson)
{
	if(pBson->type == tBsonType_object)
		t_bson_add_boolean(pBson, key, json_object_get_boolean(pJson));
	else
		t_bson_array_add_boolean(pBson, json_object_get_boolean(pJson));
}

static inline void
_t_bson_json_int(json_object *pJson, tBsonData *pData)
{
	if(pJson->o_type == json_type_object)
		json_object_object_add(pJson,
			(const char *)pData->key_ptr,
			json_object_new_int((int32_t)(pData->value_ptr.int_value)));
	else
		json_object_array_add(pJson,
			json_object_new_int((int32_t)(pData->value_ptr.int_value)));		
}

static inline void
_t_json_bson_int(tBsonObject *pBson, char *key, json_object *pJson)
{
	if(pBson->type == tBsonType_object)
		t_bson_add_int(pBson, key, json_object_get_int(pJson));
	else
		t_bson_array_add_int(pBson, json_object_get_int(pJson));
}

static inline void
_t_bson_json_int64(json_object *pJson, tBsonData *pData)
{
	if(pJson->o_type == json_type_object)
		json_object_object_add(pJson,
			(const char *)pData->key_ptr,
			json_object_new_int64((int64_t)(pData->value_ptr.int64_value)));
	else
		json_object_array_add(pJson,
			json_object_new_int64((int64_t)(pData->value_ptr.int64_value)));		
}

static inline void
_t_bson_json_double(json_object *pJson, tBsonData *pData)
{
	if(pJson->o_type == json_type_object)
		json_object_object_add(pJson,
			(const char *)pData->key_ptr,
			json_object_new_double(pData->value_ptr.double_value));
	else
		json_object_array_add(pJson,
			json_object_new_double(pData->value_ptr.double_value));		
}

static inline void
_t_json_bson_double(tBsonObject *pBson, char *key, json_object *pJson)
{
	if(pBson->type == tBsonType_object)
		t_bson_add_double(pBson, key, json_object_get_double(pJson));
	else
		t_bson_array_add_double(pBson, json_object_get_double(pJson));		
}

static inline void
_t_bson_json_string(json_object *pJson, tBsonData *pData)
{
	if(pJson->o_type == json_type_object)
		json_object_object_add(pJson,
			(const char *)pData->key_ptr,
			json_object_new_string_len(pData->value_ptr.mem_value, pData->value_len));
	else
		json_object_array_add(pJson,
			json_object_new_string_len(pData->value_ptr.mem_value, pData->value_len));		
}

static inline void
_t_json_bson_string(tBsonObject *pBson, char *key, json_object *pJson)
{
	if(pBson->type == tBsonType_object)
		t_bson_add_string(pBson, key, (char *)json_object_get_string(pJson));
	else
		t_bson_array_add_string(pBson, (char *)json_object_get_string(pJson));		
}

static inline void
_t_bson_json_bin(json_object *pJson, tBsonData *pData)
{
	int base64_str_len;
	int base64_str_index;
	char *base64_str_ptr;

	if(t_is_all_show_char_or_rn((u8 *)(pData->value_ptr.mem_value), pData->value_len) == dave_true)
	{
		_t_bson_json_string(pJson, pData);
	}
	else
	{
		base64_str_len = 256 + pData->value_len * 2;
		base64_str_ptr = dave_malloc(base64_str_len);

		base64_str_index = dave_snprintf((s8 *)base64_str_ptr, base64_str_len, "__BSON_BIN__:%d:", pData->value_len);

		t_crypto_base64_encode(
			(u8 *)(pData->value_ptr.mem_value), pData->value_len,
			(s8 *)(&base64_str_ptr[base64_str_index]), base64_str_len-base64_str_index);

		if(pJson->o_type == json_type_object)
			json_object_object_add(pJson,
				(const char *)pData->key_ptr,
				json_object_new_string_len(base64_str_ptr, base64_str_index));
		else
			json_object_array_add(pJson,
				json_object_new_string_len(base64_str_ptr, base64_str_index));		

		dave_free(base64_str_ptr);
	}
}

static inline void
_t_bson_json_array(json_object *pJson, tBsonData *pData)
{
	json_object *pSubObject = t_bson_json((tBsonObject *)(pData->value_ptr.object_value));

	if(pJson->o_type == json_type_object)
		json_object_object_add(pJson, (const char *)pData->key_ptr, pSubObject);
	else
		json_object_array_add(pJson, pSubObject);
}

static inline tBsonObject *
_t_json_array_to_bson_array(json_object *pJson)
{
	tBsonObject *pArrayBson;
	size_t array_len = json_object_array_length(pJson);
	size_t array_index;
	json_object *sub_data;

	pArrayBson = t_bson_malloc_array();

	for(array_index=0; array_index<array_len; array_index++)
	{
		sub_data = json_object_array_get_idx(pJson, array_index);
		switch(sub_data->o_type)
		{
			case json_type_boolean:
					t_bson_array_add_boolean(pArrayBson, json_object_get_boolean(sub_data));
				break;
			case json_type_double:
					t_bson_array_add_double(pArrayBson, json_object_get_double(sub_data));
				break;
			case json_type_int:
					t_bson_array_add_int(pArrayBson, json_object_get_int(sub_data));
				break;
			case json_type_string:
					t_bson_array_add_string(pArrayBson, (char *)json_object_get_string(sub_data));
				break;
			case json_type_array:
					t_bson_array_add_object(pArrayBson, _t_json_array_to_bson_array(sub_data));
				break;
			case json_type_object:
					t_bson_array_add_object(pArrayBson, t_json_to_bson(sub_data));
				break;
			default:
				break;
		}
	}

	return pArrayBson;
}

static inline void
_t_json_bson_array(tBsonObject *pBson, char *key, json_object *pJson)
{
	tBsonObject *pArrayBson = _t_json_array_to_bson_array(pJson);

	if(pBson->type == tBsonType_object)
		t_bson_add_object(pBson, key, pArrayBson);
	else
		t_bson_array_add_object(pBson, pArrayBson);
}

static inline void
_t_bson_json_object(json_object *pJson, tBsonData *pData)
{
	json_object *pSubObject = t_bson_json((tBsonObject *)(pData->value_ptr.object_value));

	if(pJson->o_type == json_type_object)
		json_object_object_add(pJson, (const char *)pData->key_ptr, pSubObject);
	else
		json_object_array_add(pJson, pSubObject);
}

static inline void
_t_json_bson_object(tBsonObject *pBson, char *key, json_object *pJson)
{
	tBsonObject *pSubObject = t_json_bson(pJson);

	if(pBson->type == tBsonType_object)
		t_bson_add_object(pBson, key, pSubObject);
	else
		t_bson_array_add_object(pBson, pSubObject);
}

// =====================================================================

json_object *
t_bson_json(tBsonObject *pBson)
{
	json_object *pJson;
	tBsonData *pData;

	if(pBson == NULL)
	{
		return NULL;
	}

	if(pBson->type == tBsonType_object)
	{
		pJson = json_object_new_object();
	}
	else if(pBson->type == tBsonType_array)
	{
		pJson = json_object_new_array();
	}
	else
	{
		TOOLSLOG("invalid type:%d", pBson->type);
		return NULL;
	}

	pData = pBson->pDataHead;
	while(pData != NULL)
	{
		switch(pData->type)
		{
			case tBsonType_boolean:
					_t_bson_json_boolean(pJson, pData);
				break;
			case tBsonType_int:
					_t_bson_json_int(pJson, pData);
				break;
			case tBsonType_int64:
					_t_bson_json_int64(pJson, pData);
				break;
			case tBsonType_double:
					_t_bson_json_double(pJson, pData);
				break;
			case tBsonType_string:
					_t_bson_json_string(pJson, pData);
				break;
			case tBsonType_bin:
			case tBsonType_bin_insertion:
					_t_bson_json_bin(pJson, pData);
				break;
			case tBsonType_array:
					_t_bson_json_array(pJson, pData);
				break;
			case tBsonType_object:
					_t_bson_json_object(pJson, pData);
				break;
			default:
				break;
		}

		pData = (tBsonData *)(pData->next);
	}

	return pJson;
}

tBsonObject *
t_json_bson(json_object *pJson)
{
	tBsonObject *pBson;
	struct lh_entry *entry = NULL;
	char *key = NULL;
	struct json_object* val = NULL;

	if(pJson == NULL)
	{
		return NULL;
	}

	if(pJson->o_type == json_type_object)
	{
		pBson = (tBsonObject *)t_bson_malloc_object();
	}
	else
	{
		TOOLSLOG("invalid type:%d", pJson->o_type);
		return NULL;
	}

	entry = json_object_get_object(pJson)->head;

	while(entry)
	{
		key = (char *)entry->k;
		val = (struct json_object *)entry->v;

		switch(json_object_get_type(val))
		{
			case json_type_boolean:
					_t_json_bson_boolean(pBson, key, val);
				break;
			case json_type_int:
					_t_json_bson_int(pBson, key, val);
				break;
			case json_type_double:
					_t_json_bson_double(pBson, key, val);
				break;
			case json_type_string:
					_t_json_bson_string(pBson, key, val);
				break;
			case json_type_array:
					_t_json_bson_array(pBson, key, val);
				break;
			case json_type_object:
					_t_json_bson_object(pBson, key, val);
				break;
			default:
				break;
		}

		entry = entry->next;
	}

	return pBson;
}

