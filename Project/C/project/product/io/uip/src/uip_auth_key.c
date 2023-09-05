/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_store.h"
#include "uip_log.h"

#define AUTH_KEY_VER (1)
#define AUTH_DB_NAME_LEN (8)
#define AUTH_CHANNEL_NAME_LEN (16)
#define AUTHENTICATION_DATA (0x1234abcd)

typedef enum {
	AuthKeyType_normal = 0,
	AuthKeyType_max = 0xff,
} AuthKeyType;

typedef struct {
	u8 version;
	u8 key_type;
	s8 db_name[AUTH_DB_NAME_LEN];
	u32 db_table_id;
	s8 channel_name[AUTH_CHANNEL_NAME_LEN];
	DateStruct build_date;
	DateStruct validity_date;
	u32 auth_data;
	u8 reserved[4];
} AuthKeyData;

typedef struct {
	AuthKeyData data;
	u8 reserved[DAVE_AUTH_KEY_LEN - sizeof(AuthKeyData)];
} AuthKey;

static void
_auth_key_des_key(u8 *des_key)
{
	des_key[0] = 0x90;
	des_key[1] = 0x19;
	des_key[2] = 0x87;
	des_key[3] = 0xbc;
	des_key[4] = 0x34;
	des_key[5] = 0xae;
	des_key[6] = 0xe7;
	des_key[7] = 0xab;
}

static RetCode
_auth_key_des_encode(u8 auth_key[DAVE_AUTH_KEY_LEN], AuthKey *pKey)
{
	u8 des_key[DAVE_DES_KEY_LEN];

	_auth_key_des_key(des_key);

	dave_memcpy(auth_key, pKey, DAVE_AUTH_KEY_LEN);

	if(t_crypto_des_encode(des_key, DAVE_DES_KEY_LEN, auth_key, DAVE_AUTH_KEY_LEN, dave_false) == 0)
	{
		return RetCode_encode_failed;
	}

	return RetCode_OK;
}

static RetCode
_auth_key_des_decode(AuthKey *pKey, u8 auth_key[DAVE_AUTH_KEY_LEN])
{
	u8 des_key[DAVE_DES_KEY_LEN];

	_auth_key_des_key(des_key);

	if(t_crypto_des_decode(des_key, DAVE_DES_KEY_LEN, auth_key, DAVE_AUTH_KEY_LEN, dave_false) == 0)
	{
		return ERRCODE_decode_failed;
	}

	dave_memcpy(pKey, auth_key, DAVE_AUTH_KEY_LEN);

	return RetCode_OK;
}

static RetCode
_auth_key_encode(u8 auth_key[DAVE_AUTH_KEY_LEN], s8 *db_name, ub db_table_id, s8 *channel_name, DateStruct validity_date)
{
	DateStruct build_date;
	AuthKey key;

	t_time_get_date(&build_date);

	if(t_time_struct_second(&validity_date) < t_time_struct_second(&build_date))
	{
		UIPABNOR("The key has expired %s/%s.", datestr(&validity_date), datestr2(&validity_date));
		return RetCode_Invalid_parameter;
	}

	dave_memset(&key, 0x00, sizeof(AuthKey));

	key.data.version = AUTH_KEY_VER;
	key.data.key_type = (u8)AuthKeyType_normal;
	dave_strcpy(key.data.db_name, db_name, sizeof(key.data.db_name));
	key.data.db_table_id = (u32)db_table_id;
	dave_strcpy(key.data.channel_name, channel_name, sizeof(key.data.channel_name));
	key.data.build_date = build_date;
	key.data.validity_date = validity_date;
	key.data.auth_data = AUTHENTICATION_DATA;

	return _auth_key_des_encode(auth_key, &key);
}

static RetCode
_auth_key_decode(s8 *db_name, ub *db_table_id, s8 *channel_name, DateStruct *validity_date, u8 auth_key[DAVE_AUTH_KEY_LEN])
{
	AuthKey key;
	RetCode ret;

	ret = _auth_key_des_decode(&key, auth_key);
	if(ret != RetCode_OK)
	{
		UIPABNOR("invalid auth_key");
		return ret;
	}

	if(db_name != NULL)
		dave_strcpy(db_name, key.data.db_name, sizeof(key.data.db_name));
	if(db_table_id != NULL)
		*db_table_id = key.data.db_table_id;
	if(channel_name != NULL)
		dave_strcpy(channel_name, key.data.channel_name, sizeof(key.data.channel_name));
	if(validity_date != NULL)
		*validity_date = key.data.validity_date;

	return RetCode_OK;
}

static dave_bool
_str_to_auth_key(u8 auth_key[DAVE_AUTH_KEY_LEN], s8 auth_key_str[DAVE_AUTH_KEY_STR_LEN])
{
	u8 auth_key_decode_buf_ptr[DAVE_AUTH_KEY_LEN + 8];
	ub auth_key_decode_buf_len;

	auth_key_decode_buf_len = sizeof(auth_key_decode_buf_ptr);
	if(t_crypto_base64_decode(auth_key_str, dave_strlen(auth_key_str), auth_key_decode_buf_ptr, &auth_key_decode_buf_len) == dave_false)
	{
		UIPLOG("auth_key:%s decode failed!", auth_key_str);
		dave_memset(auth_key, 0x00, DAVE_AUTH_KEY_LEN);
		return dave_false;
	}
	if(auth_key_decode_buf_len != DAVE_AUTH_KEY_LEN)
	{
		UIPLOG("auth_key:%s decode length:%d failed!", auth_key_str, auth_key_decode_buf_len);
		t_stdio_print_hex("auth_key_decode_buf_ptr", auth_key_decode_buf_ptr, auth_key_decode_buf_len);
		dave_memset(auth_key, 0x00, DAVE_AUTH_KEY_LEN);
		return dave_false;
	}

	dave_memcpy(auth_key, auth_key_decode_buf_ptr, DAVE_AUTH_KEY_LEN);

	return dave_true;
}

static dave_bool
_auth_key_to_str(s8 auth_key_str[DAVE_AUTH_KEY_STR_LEN], u8 auth_key[DAVE_AUTH_KEY_LEN])
{
	u8 auth_key_check[DAVE_AUTH_KEY_LEN];
	ub auth_key_len;

	auth_key_len = t_crypto_base64_encode((const u8 *)auth_key, DAVE_AUTH_KEY_LEN, auth_key_str, DAVE_AUTH_KEY_STR_LEN);
	if(auth_key_len >= (DAVE_AUTH_KEY_STR_LEN - 1))
	{
		UIPLOG("invalid auth_key:%s", auth_key_str);
		return dave_false;
	}
	auth_key_str[auth_key_len] = '\0';

	_str_to_auth_key(auth_key_check, auth_key_str);

	if(dave_memcmp(auth_key, auth_key_check, DAVE_AUTH_KEY_LEN) == dave_false)
	{
		UIPLOG("invalid auth_key:%s auth_key_len:%d", auth_key_str, auth_key_len);
		t_stdio_print_hex("auth_key", auth_key, DAVE_AUTH_KEY_LEN);
		t_stdio_print_hex("auth_key_check", auth_key_check, DAVE_AUTH_KEY_LEN);
		return dave_false;
	}

	return dave_true;
}

// =====================================================================

s8 *
uip_auth_key_build(s8 auth_key_str[DAVE_AUTH_KEY_STR_LEN], s8 *db_name, s8 *channel_name)
{
	u8 auth_key[DAVE_AUTH_KEY_LEN];
	DateStruct validity_date;

	t_time_get_date(&validity_date);

	validity_date.year += 100;

	if(_auth_key_encode(auth_key, db_name, 0, channel_name, validity_date) != RetCode_OK)
	{
		UIPLOG("auth key encode failed! db_name:%s channel_name:%s", db_name, channel_name);
		return NULL;
	}

	if(_auth_key_to_str(auth_key_str, auth_key) == dave_false)
	{
		UIPLOG("auth key to string failed! db_name:%s channel_name:%s", db_name, channel_name);
		return NULL;
	}

	return auth_key_str;
}

dave_bool
uip_auth_key_check(s8 auth_key_str[DAVE_AUTH_KEY_STR_LEN], s8 *channel_name)
{
	u8 auth_key[DAVE_AUTH_KEY_LEN];
	s8 channel_name_from_key[AUTH_CHANNEL_NAME_LEN];

	if(_str_to_auth_key(auth_key, auth_key_str) == dave_false)
	{
		UIPLOG("invalid auth_key_str:%s", auth_key_str);
		return dave_false;
	}

	if(_auth_key_decode(NULL, NULL, channel_name_from_key, NULL, auth_key) != RetCode_OK)
	{
		UIPLOG("invalid auth_key_str:%s", auth_key_str);
		return dave_false;
	}

	if(dave_strcmp(channel_name_from_key, channel_name) == dave_false)
	{
		UIPLOG("auth_key_str:%s channel mismatch:%s/%s",
			auth_key_str, channel_name_from_key, channel_name);
		return dave_false;
	}

	return dave_true;
}

