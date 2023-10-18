/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_store.h"
#include "uip_auth_key.h"
#include "uip_log.h"

#define UIP_CHANNEL_KV_TABLE "uctKV"

#define DB_NAME "UIP"

#define CHANNEL_NAME "channel"
#define CHANNEL_DISC "(id int primary key auto_increment,"\
	"channel_name VARCHAR(512) NOT NULL,"\
	"auth_key VARCHAR(512) NOT NULL,"\
	"allow_method TEXT,"\
	"valid ENUM('Y', 'N') NOT NULL,"\
	"updatetime timestamp default current_timestamp,"\
	"INDEX (channel_name));"

typedef struct {
	s8 channel_name[DAVE_NORMAL_NAME_LEN];
	s8 auth_key[DAVE_AUTH_KEY_STR_LEN];
	ub veriify_counter;
	void *pAllowMethodKV;
} UIPChannelTable;

static void *pKV = NULL;

static void *
_uip_channel_kv_method_to_json(void *pAllowMethod)
{
	ub safe_counter;
	s8 key[256], value[256];
	void *pArrayJson;

	if(pAllowMethod == NULL)
		return NULL;

	pArrayJson = dave_json_array_malloc();

	for(safe_counter=0; safe_counter<102400; safe_counter++)
	{
		if(base_ramkv_index_key_value(pAllowMethod, safe_counter, key, sizeof(key), value, sizeof(value)) <= 0)
		{
			break;
		}

		dave_json_array_add_str(pArrayJson, key);
	}

	return pArrayJson;
}

static void *
_uip_channel_kv_method_add(UIPChannelTable *pTable, s8 *allow_method)
{
	void *pArrayJson;
	sb array_length, array_index;
	s8 allow_kv_name[256];
	s8 *method;

	if((allow_method == NULL) || (allow_method[0] == '\0'))
		return pTable->pAllowMethodKV;

	pArrayJson = dave_string_to_json(allow_method, dave_strlen(allow_method));
	if(pArrayJson == NULL)
	{
		UIPLOG("invalid allow_method:%s", allow_method);
		return pTable->pAllowMethodKV;
	}

	array_length = dave_json_get_array_length(pArrayJson);
	if(array_length == 0)
	{
		UIPLOG("invalid allow_method:%s", allow_method);
		return pTable->pAllowMethodKV;
	}

	if(pTable->pAllowMethodKV == NULL)
	{
		dave_snprintf(allow_kv_name, sizeof(allow_kv_name), "%s-allow-method", pTable->channel_name);
		pTable->pAllowMethodKV = kv_malloc(allow_kv_name, 0, NULL);
	}

	for(array_index=0; array_index<array_length; array_index++)
	{
		method = dave_json_c_array_get_str(pArrayJson, array_index, NULL);
		kv_add_key_ptr(pTable->pAllowMethodKV, method, pTable);
	}

	return pTable->pAllowMethodKV;
}

static dave_bool
_uip_channel_kv_method_inq(UIPChannelTable *pTable, s8 *allow_method)
{
	if(pTable->pAllowMethodKV == NULL)
		return dave_true;

	if(kv_inq_key_ptr(pTable->pAllowMethodKV, allow_method) != NULL)
		return dave_true;

	return dave_false;
}

static UIPChannelTable *
_uip_channel_kv_add(s8 *channel_name, s8 *auth_key, s8 *allow_method)
{
	UIPChannelTable *pTable;

	if((channel_name == NULL) || (auth_key == NULL))
	{
		UIPLOG("empty channel_name:%s or auth_key:%s", channel_name, auth_key);
		return NULL;
	}

	pTable = kv_inq_key_ptr(pKV, channel_name);
	if(pTable == NULL)
	{
		pTable = dave_ralloc(sizeof(UIPChannelTable));
		pTable->veriify_counter = 0;
		pTable->pAllowMethodKV = NULL;
	}

	dave_strcpy(pTable->channel_name, channel_name, DAVE_NORMAL_NAME_LEN);
	dave_strcpy(pTable->auth_key, auth_key, DAVE_AUTH_KEY_STR_LEN);
	pTable->pAllowMethodKV = _uip_channel_kv_method_add(pTable, allow_method);

	UIPTRACE("pTable:%x channel:%s key:%s",
		pTable, pTable->channel_name, pTable->auth_key);

	if(kv_add_key_ptr(pKV, pTable->channel_name, pTable) == dave_false)
	{
		dave_free(pTable);
		return NULL;
	}

	return pTable;
}

static UIPChannelTable *
_uip_channel_kv_inq(s8 *channel_name)
{
	return (UIPChannelTable *)kv_inq_key_ptr(pKV, channel_name);
}

static RetCode
_uip_channel_kv_del(void *kv, s8 *channel_name)
{
	UIPChannelTable *pTable;

	pTable = kv_inq_key_ptr(pKV, channel_name);
	if(pTable == NULL)
	{
		return RetCode_empty_data;
	}

	UIPTRACE("channel_name:%s", pTable->channel_name);

	kv_del_key_ptr(pKV, pTable->channel_name);

	if(pTable->pAllowMethodKV != NULL)
	{
		kv_free(pTable->pAllowMethodKV, NULL);
	}

	dave_free(pTable);

	return RetCode_OK;
}

static void
_uip_channel_load_from_db(void)
{
	ub table_id;
	ub safe_counter, channel_number;
	StoreSqlRet ret;
	s8 *channel_name, *auth_key, *allow_method;

	STORESQL("CREATE DATABASE %s", DB_NAME);
	STORESQL("CREATE TABLE %s.%s %s", DB_NAME, CHANNEL_NAME, CHANNEL_DISC);

	UIPLOG("uip channel table load ...");

	table_id = 0;
	safe_counter = channel_number = 0;
	while((safe_counter ++) < 102400)
	{
		ret = STORESQL("SELECT id, channel_name, auth_key, allow_method FROM %s.%s WHERE id >= %d and valid = \"Y\" LIMIT 1;",
			DB_NAME, CHANNEL_NAME,
			table_id);
		if(ret.ret != RetCode_OK)
		{
			UIPLOG("ret:%s", retstr(ret.ret));
			break;
		}
		if(ret.pJson == NULL)
		{
			break;
		}

		table_id = STORELOADsb(ret, 0);
		channel_name = STORELOADstr(ret, 1);
		auth_key = STORELOADstr(ret, 2);
		allow_method = STORELOADstr(ret, 3);

		UIPTRACE("table_id:%d channel_name:%s auth_key:%s allow_method:%s",
			table_id, channel_name, auth_key, allow_method);

		if(_uip_channel_kv_add(channel_name, auth_key, allow_method) == NULL)
		{
			break;
		}

		table_id ++;
		channel_number ++;
	}

	UIPLOG("uip channel table load done(%d)!", channel_number);
}

static UIPChannelTable *
_uip_channel_store_to_db(s8 *channel_name, s8 *auth_key, s8 *allow_method)
{
	StoreSqlRet select_ret, ret;

	select_ret = STORESQL("SELECT id FROM %s.%s WHERE channel_name = \"%s\";",
		DB_NAME, CHANNEL_NAME,
		channel_name);

	if((select_ret.ret == RetCode_OK) && (select_ret.pJson != NULL))
	{
		if(allow_method == NULL)
		{
			ret = STORESQL("UPDATE %s.%s SET auth_key = \"%s\", valid = \"Y\", updatetime=now() WHERE channel_name = \"%s\";",
				DB_NAME, CHANNEL_NAME,
				auth_key,
				channel_name);
		}
		else
		{
			ret = STORESQL("UPDATE %s.%s SET auth_key = \"%s\", allow_method = \"%s\", valid = \"Y\", updatetime=now() WHERE channel_name = \"%s\";",
				DB_NAME, CHANNEL_NAME,
				auth_key, allow_method,
				channel_name);
		}
	}
	else
	{
		if(allow_method == NULL)
		{
			ret = STORESQL("INSERT INTO %s.%s (channel_name, auth_key, valid) VALUES (\"%s\", \"%s\", \"Y\");",
				DB_NAME, CHANNEL_NAME,
				channel_name, auth_key);
		}
		else
		{
			ret = STORESQL("INSERT INTO %s.%s (channel_name, auth_key, allow_method, valid) VALUES (\"%s\", \"%s\", \"%s\", \"Y\");",
				DB_NAME, CHANNEL_NAME,
				channel_name, auth_key, allow_method);
		}
	}

	dave_json_free(ret.pJson);
	dave_json_free(select_ret.pJson);

	if((ret.ret != RetCode_OK) && (ret.ret != RetCode_empty_data))
	{
		UIPLOG("ret:%s channel_name:%s allow_method:%s",
			retstr(ret.ret), channel_name, allow_method);
		return NULL;
	}

	return _uip_channel_kv_add(channel_name, auth_key, NULL);
}

static dave_bool
_uip_channel_delete_to_db(s8 *channel_name)
{
	StoreSqlRet ret;

	ret = STORESQL("UPDATE %s.%s SET valid = \"N\", updatetime=now() WHERE channel_name = \"%s\";",
		DB_NAME, CHANNEL_NAME,
		channel_name);

	dave_json_free(ret.pJson);

	if(ret.ret == RetCode_OK)
		return dave_true;
	else
		return dave_false;
}

static RetCode
_uip_channel_verify(s8 *channel_name, s8 *auth_key, s8 *allow_method)
{
	UIPChannelTable *pTable;
	RetCode ret = RetCode_OK;

	if((NULL == channel_name) || ('\0' == channel_name[0]))
	{
		UIPLOG("Invalid channel!");
		return RetCode_Invalid_channel;
	}

	if((NULL == auth_key) || ('\0' == auth_key[0]))
	{
		UIPLOG("Invalid auth_key_str!");
		return RetCode_channel_not_exist;
	}

	pTable = _uip_channel_kv_inq(channel_name);
	if(pTable != NULL)
	{
		t_lock;
		pTable->veriify_counter ++;
		t_unlock;

		if(dave_strcmp(channel_name, pTable->channel_name) == dave_false)
		{
			UIPLOG("channel:%s/%s mismatch!",
				channel_name, pTable->channel_name);
			ret = RetCode_Invalid_channel;
		}

		if(dave_strcmp(auth_key, pTable->auth_key) == dave_false)
		{
			UIPLOG("the channel:%s auth_key:%s/%s mismatch!",
				channel_name, auth_key, pTable->auth_key);
			ret = RetCode_Invalid_channel;
		}

		if(_uip_channel_kv_method_inq(pTable, allow_method) == dave_false)
		{
			UIPLOG("Unauthorized access on channel:%s allow_method:%s!",
				channel_name, allow_method);
			ret = RetCode_Unauthorized_access;
		}
	}
	else
	{
		UIPLOG("channel:%s key:%s not find!", channel_name, auth_key);
		ret = RetCode_channel_not_exist;
	}

	return ret;
}

static s8 *
_uip_channel_inq(s8 *channel_name)
{
	UIPChannelTable *pTable;

	pTable = _uip_channel_kv_inq(channel_name);
	if(pTable == NULL)
	{
		return NULL;
	}

	return pTable->auth_key;
}

static UIPChannelTable *
_uip_channel_new(s8 *channel_name, s8 *user_input_auth_key)
{
	s8 new_auth_key[DAVE_AUTH_KEY_STR_LEN];

	if(user_input_auth_key != NULL)
	{
		dave_strcpy(new_auth_key, user_input_auth_key, sizeof(new_auth_key));
	}
	else
	{
		if(uip_auth_key_build(new_auth_key, DB_NAME, channel_name) == NULL)
		{
			UIPLOG("channel:%s uip auth key build failed!", channel_name);
			return NULL;
		}
	}

	return _uip_channel_store_to_db(channel_name, new_auth_key, NULL);
}

static s8 *
_uip_channel_add(s8 *channel_name, s8 *user_input_auth_key)
{
	UIPChannelTable *pTable;

	pTable = _uip_channel_kv_inq(channel_name);
	if(pTable != NULL)
	{
		return pTable->auth_key;
	}

	if((user_input_auth_key != NULL) && (uip_auth_key_check(user_input_auth_key, channel_name) == dave_false))
	{
		user_input_auth_key = NULL;
	}

	pTable = _uip_channel_new(channel_name, user_input_auth_key);
	if(pTable == NULL)
	{
		return NULL;
	}

	return pTable->auth_key;
}

static dave_bool
_uip_channel_del(s8 *channel_name)
{
	_uip_channel_kv_del(NULL, channel_name);

	return _uip_channel_delete_to_db(channel_name);
}

static dave_bool
_uip_channel_add_method(UIPChannelTable *pTable, s8 *allow_method)
{
	s8 method_json_str[1024];
	void *pArrayJson;
	s8 *json_str;
	s8 *escape_ptr;
	ub escape_len;

	if(pTable->pAllowMethodKV != NULL)
	{
		if(_uip_channel_kv_method_inq(pTable, allow_method) == dave_true)
		{
			return dave_true;
		}
	}

	dave_snprintf(method_json_str, sizeof(method_json_str), "[ \"%s\" ]", allow_method);

	_uip_channel_kv_method_add(pTable, method_json_str);

	pArrayJson = _uip_channel_kv_method_to_json(pTable->pAllowMethodKV);
	json_str = (s8 *)json_object_to_json_string((struct json_object *)(pArrayJson));
	if(json_str == NULL)
	{
		UIPLOG("invalid json data:%s!", method_json_str);
		dave_json_free(pArrayJson);
		return dave_false;
	}

	escape_len = dave_strlen(json_str) * 8;
	escape_ptr = dave_malloc(escape_len);
	stringescape(escape_ptr, escape_len, json_str);
	_uip_channel_store_to_db(pTable->channel_name, pTable->auth_key, escape_ptr);
	dave_free(escape_ptr);
	dave_json_free(pArrayJson);

	return dave_true;
}

static ub
_uip_channel_allow_method_info(s8 *info_ptr, ub info_len, s8 *channel_name, void *pAllowMethod)
{
	ub info_index;
	void *pArrayJson;

	if(pAllowMethod == NULL)
	{
		return 0;
	}

	info_index = 0;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "  %s ALLOW METHOD INFO:\n", channel_name);

	pArrayJson = _uip_channel_kv_method_to_json(pAllowMethod);
	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index,
		"   %s\n",
		json_object_to_json_string_length((struct json_object *)(pArrayJson), JSON_C_TO_STRING_PLAIN, NULL));
	dave_json_free(pArrayJson);

	return info_index;
}

static ub
_uip_channel_info(s8 *info_ptr, ub info_len)
{
	ub info_index, safe_counter;
	UIPChannelTable *pTable;

	info_index = 0;

	info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, "CHANNEL INFO:\n");

	for(safe_counter=0; safe_counter<102400; safe_counter++)
	{
		pTable = base_ramkv_inq_index_ptr(pKV, safe_counter);
		if(pTable == NULL)
		{
			break;
		}

		info_index += dave_snprintf(&info_ptr[info_index], info_len-info_index, " %08d | %s -> %s\n",
			pTable->veriify_counter,
			pTable->channel_name,
			pTable->auth_key);

		if(pTable->pAllowMethodKV != NULL)
		{
			info_index += _uip_channel_allow_method_info(
				&info_ptr[info_index], info_len-info_index,
				pTable->channel_name, pTable->pAllowMethodKV);
		}
	}

	return info_index;
}

// =====================================================================

void
uip_channel_init(void)
{
	pKV = kv_malloc(UIP_CHANNEL_KV_TABLE, 0, NULL);
}

void
uip_channel_exit(void)
{
	kv_free(pKV, _uip_channel_kv_del);
}

void 
uip_channel_reset(void)
{
	_uip_channel_load_from_db();
}

RetCode
uip_channel_verify(s8 *channel_name, s8 *auth_key, s8 *allow_method)
{
	return _uip_channel_verify(channel_name, auth_key, allow_method);
}

s8 *
uip_channel_inq(s8 *channel_name)
{
	if((channel_name == NULL) || (channel_name[0] == '\0'))
	{
		UIPLOG("empty channel_name");
		return NULL;
	}

	return _uip_channel_inq(channel_name);
}

s8 *
uip_channel_add(s8 *channel_name, s8 *user_input_auth_key)
{
	if((channel_name == NULL) || (channel_name[0] == '\0'))
	{
		UIPLOG("empty channel_name");
		return NULL;
	}

	if(dave_strlen(user_input_auth_key) < DAVE_AUTH_KEY_LEN)
	{
		user_input_auth_key = NULL;
	}

	return _uip_channel_add(channel_name, user_input_auth_key);
}

dave_bool
uip_channel_del(s8 *channel_name)
{
	if((channel_name == NULL) || (channel_name[0] == '\0'))
	{
		UIPLOG("empty channel_name");
		return dave_false;
	}

	return _uip_channel_del(channel_name);
}

dave_bool
uip_channel_add_method(s8 *channel_name, s8 *allow_method)
{
	UIPChannelTable *pTable;

	pTable = _uip_channel_kv_inq(channel_name);
	if(pTable == NULL)
	{
		UIPLOG("can't find the channel:%s", channel_name);
		return dave_false;
	}

	if((allow_method == NULL) || (allow_method[0] == '\0'))
	{
		UIPLOG("empty method on channel:%s", channel_name);
		return dave_false;
	}

	return _uip_channel_add_method(pTable, allow_method);
}

ub
uip_channel_info(s8 *info_ptr, ub info_len)
{
	return _uip_channel_info(info_ptr, info_len);
}

