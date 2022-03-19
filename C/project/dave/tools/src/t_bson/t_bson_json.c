/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "t_bson_define.h"
#include "t_bson_json.h"
#include "json_object_private.h"
#include "tools_log.h"

static void
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

static void
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

static void
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

static void
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

static void
_t_bson_json_string(json_object *pJson, tBsonData *pData)
{
	if(pJson->o_type == json_type_object)
		json_object_object_add(pJson,
			(const char *)pData->key_ptr,
			json_object_new_string(pData->value_ptr.mem_value));
	else
		json_object_array_add(pJson,
			json_object_new_string(pData->value_ptr.mem_value));		
}

static void
_t_bson_json_bin(json_object *pJson, tBsonData *pData)
{
	int base64_str_len = 256 + pData->value_len * 2;
	int base64_str_index = 0;
	char *base64_str_ptr;

	base64_str_ptr = dave_malloc(base64_str_len);

	base64_str_index = dave_snprintf((s8 *)base64_str_ptr, base64_str_len, "__BSON_BIN__:%d:", pData->value_len);

	t_base64_encode(
		(u8 *)(pData->value_ptr.mem_value), pData->value_len,
		(s8 *)(&base64_str_ptr[base64_str_index]), base64_str_len-base64_str_index);

	if(pJson->o_type == json_type_object)
		json_object_object_add(pJson,
			(const char *)pData->key_ptr,
			json_object_new_string(base64_str_ptr));
	else
		json_object_array_add(pJson,
			json_object_new_string(base64_str_ptr));		

	dave_free(base64_str_ptr);
}

static void
_t_bson_json_array(json_object *pJson, tBsonData *pData)
{
	json_object *pSubObject = t_bson_json((tBsonObject *)(pData->value_ptr.object_value));

	if(pJson->o_type == json_type_object)
		json_object_object_add(pJson, (const char *)pData->key_ptr, pSubObject);
	else
		json_object_array_add(pJson, pSubObject);
}

static void
_t_bson_json_object(json_object *pJson, tBsonData *pData)
{
	json_object *pSubObject = t_bson_json((tBsonObject *)(pData->value_ptr.object_value));

	if(pJson->o_type == json_type_object)
		json_object_object_add(pJson, (const char *)pData->key_ptr, pSubObject);
	else
		json_object_array_add(pJson, pSubObject);
}

// =====================================================================

json_object *
t_bson_json(tBsonObject *pBson)
{
	json_object *pJson;
	tBsonData *pData;

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
	return NULL;
}

