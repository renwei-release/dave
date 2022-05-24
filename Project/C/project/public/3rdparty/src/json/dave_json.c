/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(JSON_3RDPARTY)
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h> 
#include <netinet/in.h>
#include <time.h> 
#include <dirent.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "debug.h"
#include "linkhash.h"
#include "arraylist.h"
#include "json_util.h"
#include "json_object.h"
#include "json_pointer.h"
#include "json_tokener.h"
#include "json_object_iterator.h"
#include "json_object_private.h"
#include "json_c_version.h"
#ifdef __cplusplus
}
#endif
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_base.h"
#include "dave_json.h"
#include "party_log.h"

// =====================================================================

s8 *
dave_json_version(void)
{
	return (s8 *)json_c_version();
}

void *
__dave_string_to_json__(s8 *string_data, sb string_length, s8 *fun, ub line)
{
	enum json_tokener_error jerr_ignored;
	struct json_object *object;

	if(string_data == NULL)
	{
		PARTYLOG("string_data is NULL! <%s:%d>", fun, line);
		return NULL;
	}

	if((string_length > 0)
		&& (string_data[string_length] != '\0')
		&& (string_data[string_length - 1] != '\0'))
	{
		/*
		 * (string_data[string_length - 1] != '\0')
		 * 有时候 string_length的长度可能是加了\0的。
		 */
		PARTYLOG("The string is not 0 ending! %d <%s:%d>", string_length, fun, line);
		return NULL;
	}

	object = json_tokener_parse_verbose((const char *)string_data, &jerr_ignored);
	if((object == NULL) || (jerr_ignored != json_tokener_success))
	{
		PARTYLOG("<%s:%d> json parse error:%d<%s>", fun, line, jerr_ignored, string_data);

		if(object != NULL)
		{
			dave_json_free(object);
		}

		return NULL;
	}

	return (void *)object;
}

s8 *
dave_json_to_string(void *object, ub *length)
{
	const char *string;
	size_t string_length;

	if(object == NULL)
	{
		*length = 0;
		return NULL;
	}

	string = json_object_to_json_string_length((struct json_object *)object, JSON_C_TO_STRING_SPACED, &string_length);

	if(length != NULL)
	{
		*length = string_length;
	}

	return (s8 *)string;
}

void *
dave_json_malloc(void)
{
	return (void *)json_object_new_object();
}

void
dave_json_free(void *object)
{
	if(object != NULL)
	{
		if(json_object_put(object) != 1)
		{
			PARTYABNOR("json object put failed!");
		}
	}
}

void *
dave_json_array_malloc(void)
{
	return (void *)json_object_new_array();
}

dave_bool
dave_json_add_object(void *object, char *key, void *obj)
{
	if((object == NULL) || (obj == NULL))
	{
		return dave_false;
	}

	if(json_object_object_add((struct json_object *)object, (const char *)key, (struct json_object *)obj) < 0)
	{
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

void
dave_json_del_object(void *object, char *key)
{
	if ((NULL == object) || (NULL == key))
	{
		PARTYABNOR("json object del failed!");
		
		return;
	}
	
	json_object_object_del((struct json_object *)object, (const char *)key);
}

void *
dave_json_get_object(void *object, char *key)
{
	struct json_object *object_object;

	if(object == NULL)
	{
		return NULL;
	}

	object_object = json_object_object_get(object, (const char *)key);
	if(object_object == NULL)
	{
		return NULL;
	}

	return (void *)object_object;
}

dave_bool
dave_json_add_array(void *object, char *key, void *array)
{
	if(object == NULL)
	{
		return dave_false;
	}

	if(json_object_object_add((struct json_object *)object, (const char *)key, (struct json_object *)array) < 0)
	{
		PARTYABNOR("add array key:%s failed!", key);
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

void *
dave_json_get_array(void *object, char *key)
{
	struct json_object *array_object;
	json_type type;

	if(object == NULL)
	{
		return NULL;
	}

	array_object = json_object_object_get(object, (const char *)key);
	if(array_object == NULL)
		return NULL;

	type = json_object_get_type(array_object);

	if(type != json_type_array)
	{
		PARTYABNOR("key:%s type:%d mismatch!", key, type);
		return NULL;
	}

	return (void *)array_object;
}

dave_bool
dave_array_add_json(void *array, void *object)
{
	if(json_object_array_add(array, object) < 0)
	{
		PARTYABNOR("add object failed!");
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

dave_bool
dave_json_add_ub(void *object, char *key, ub ub_data)
{
	if(object == NULL)
	{
		return dave_false;
	}

	if(json_object_object_add((struct json_object *)object, (const char *)key, json_object_new_int64((int64_t)ub_data)) < 0)
	{
		PARTYABNOR("add json key:%s failed!", key);

		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

dave_bool
__dave_json_get_ub__(void *object, char *key, ub *ub_data, s8 *fun, ub line)
{
	struct json_object *ub_object;
	s8 *json_string;

	*ub_data = 0;

	if(object == NULL)
	{
		PARTYABNOR("object is NULL!");
		return dave_false;
	}

	if(key != NULL)
	{
		if(json_object_get_type(object) != json_type_object)
		{
			PARTYABNOR("%s object type not object! <%s:%d>", key, fun, line);
			return dave_false;
		}

		ub_object = json_object_object_get(object, (const char *)key);
		if(ub_object == NULL)
		{
			return dave_false;
		}
	}
	else
	{
		ub_object = object;
	}

	if(json_object_get_type(ub_object) == json_type_int)
	{
		*ub_data = json_object_get_int64(ub_object);
	}
	else if(json_object_get_type(ub_object) == json_type_string)
	{
		json_string = (s8 *)json_object_get_string(ub_object);
		*ub_data = stringdigital(json_string);
	}
	else
	{
		PARTYABNOR("ub_object is not int type[%d] <%s:%d>", json_object_get_type(ub_object), fun, line);
		return dave_false;
	}

	return dave_true;
}

dave_bool
dave_json_add_sb(void *object, char *key, sb sb_data)
{
	if(object == NULL)
	{
		return dave_false;
	}

	if(json_object_object_add((struct json_object *)object, (const char *)key, json_object_new_int((int32_t)sb_data)) < 0)
	{
		PARTYABNOR("add json key:%s failed!", key);

		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

dave_bool
dave_json_get_sb(void *object, char *key, sb *sb_data)
{
	struct json_object *sb_object;

	*sb_data = 0;

	if(object == NULL)
	{
		PARTYABNOR("%s object is NULL!", key);
		return dave_false;
	}

	if(key != NULL)
	{
		if(json_object_get_type(object) != json_type_object)
		{
			PARTYABNOR("%s object type not object!", key);
			return dave_false;
		}

		sb_object = json_object_object_get(object, (const char *)key);
		if(sb_object == NULL)
		{
			PARTYDEBUG("%s sb_object is NULL!", key);
			return dave_false;
		}
	}
	else
	{
		sb_object = object;
	}

	if(json_object_get_type(sb_object) != json_type_int)
	{
		PARTYABNOR("sb_object is not int type[%d]", json_object_get_type(sb_object));
		return dave_false;
	}

	*sb_data = json_object_get_int(sb_object);

	return dave_true;
}

dave_bool
dave_json_add_double(void *object, char *key, double double_data)
{
	if(object == NULL)
	{
		return dave_false;
	}

	if(json_object_object_add((struct json_object *)object, (const char *)key, json_object_new_double(double_data)) < 0)
	{
		PARTYABNOR("add json key:%s failed!", key);
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

dave_bool
__dave_json_get_double__(void *object, char *key, double *double_data, s8 *fun, ub line)
{
	struct json_object *double_object;

	*double_data = 0;

	if(object == NULL)
	{
		return dave_false;
	}

	if(key != NULL)
	{
		if(json_object_get_type(object) != json_type_object)
		{
			return dave_false;
		}

		double_object = json_object_object_get(object, (const char *)key);
		if(double_object == NULL)
		{
			return dave_false;
		}
	}
	else
	{
		double_object = object;
	}

	if(json_object_get_type(double_object) != json_type_double)
	{
		PARTYLOG("invalid double_object:%d! <%s:%d>", json_object_get_type(double_object), fun, line);
		return dave_false;
	}

	*double_data = json_object_get_double(double_object);

	return dave_true;
}

dave_bool
dave_json_add_str(void *object, char *key, s8 *str_data)
{
	if((object == NULL) || (key == NULL) || (str_data == NULL))
	{
		PARTYABNOR("some ptr is NULL:%lx,%lx,%lx", object, key, str_data);
		return dave_false;
	}

	if(json_object_object_add((struct json_object *)object, (const char *)key, json_object_new_string((const char *)str_data)) < 0)
	{
		PARTYABNOR("add json key:%s failed!", key);
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

dave_bool
dave_json_add_str_len(void *object, char *key, s8 *str_data, ub str_len)
{
	if((object == NULL) || (key == NULL) || (str_data == NULL))
	{
		PARTYABNOR("some ptr is NULL:%lx,%lx,%lx", object, key, str_data);
		return dave_false;
	}

	if(json_object_object_add((struct json_object *)object, (const char *)key, json_object_new_string_len((const char *)str_data, (int)str_len)) < 0)
	{
		PARTYABNOR("add json key:%s failed!", key);
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

dave_bool
__dave_json_get_str__(void *object, char *key, s8 *str_data, ub *str_length, s8 *fun, ub line)
{
	struct json_object *string_object;
	s8 *json_string;
	ub str_data_length, json_string_length;

	if((str_data == NULL) || (str_length == NULL))
	{
		return dave_false;
	}

	str_data[0] = '\0';
	str_data_length = *str_length;
	if(str_data_length > 0)
	{
		str_data_length --;
	}
	*str_length = 0;

	if(object == NULL)
	{
		return dave_false;
	}

	if(key != NULL)
	{
		if(json_object_get_type(object) != json_type_object)
		{
			return dave_false;
		}

		string_object = json_object_object_get(object, (const char *)key);
		if(string_object == NULL)
		{
			return dave_false;
		}
	}
	else
	{
		string_object = object;
	}

	if(json_object_get_type(string_object) != json_type_string)
	{
		PARTYLOG("invalid string_object:%d key:%s!<%s:%d>", json_object_get_type(string_object), key, fun, line);
		return dave_false;
	}

	json_string = (s8 *)json_object_get_string(string_object);
	if(json_string == NULL)
	{
		return dave_false;
	}

	json_string_length = json_object_get_string_len(string_object);

	while(json_string_length > str_data_length) json_string_length --;

	dave_memcpy(str_data, json_string, json_string_length);
	*str_length = json_string_length;

	str_data[json_string_length] = '\0';

	if(json_string_length > str_data_length)
	{
		PARTYLOG("Maybe the character data is not copied completely, please increase the str_data buffer space.(%d/%d)<%s:%d>",
			json_string_length, str_data_length, fun, line);
	}

	return dave_true;
}

ub
__dave_json_get_str_v2__(void *object, char *key, s8 *str_data, ub str_length, s8 *fun, ub line)
{
	if(__dave_json_get_str__(object, key, str_data, &str_length, fun, line) == dave_false)
	{
		return 0;
	}

	return str_length;
}

sb
dave_json_get_array_length(void *array)
{
	struct json_object *object_object = (struct json_object *)array;

	if(array == NULL)
	{
		return 0;
	}

	if(object_object == NULL)
	{
		return 0;
	}
	else
	{
		return json_object_array_length(object_object);
	}
}

void *
dave_json_get_array_idx(void *array, sb index)
{
	struct json_object *object_object;

	if(array == NULL)
	{
		return NULL;
	}

	object_object = json_object_array_get_idx(array, index);
	if(object_object == NULL)
	{
		return NULL;
	}

	return (void *)object_object;
}

dave_bool
dave_json_add_boolean(void *object, char *key, dave_bool bool_data)
{
	if(object == NULL)
	{
		return dave_false;
	}

	if(json_object_object_add((struct json_object *)object, \
	(const char *)key, json_object_new_boolean((json_bool)bool_data)) < 0)
	{
		PARTYABNOR("add json key:%s failed!", key);
		return dave_false;
	}
	else
	{
		return dave_true;
	}
}

dave_bool
dave_json_get_boolean(void *object, char *key, dave_bool *bool_data)
{
	struct json_object *ub_object;

	if(object == NULL)
	{
		return dave_false;
	}

	*bool_data = dave_false;

	if(json_object_get_type(object) != json_type_object)
	{
		return dave_false;
	}

	ub_object = json_object_object_get(object, (const char *)key);
	if(ub_object == NULL)
	{
		return dave_false;
	}
	
	if(json_object_get_type(ub_object) != json_type_boolean)
	{
		return dave_false;
	}

	*bool_data = json_object_get_boolean(ub_object);

	return dave_true;
}

dave_bool
dave_json_array_add_str(void *array, s8 *str_data)
{
	struct json_object *pObject;

	pObject = json_object_new_string((const char *)str_data);

	if(pObject != NULL)
	{
		json_object_array_add((struct json_object *)array, pObject);
		return dave_true;
	}
	else
	{
		PARTYABNOR("the array add str failed!");
		return dave_false;
	}
}

dave_bool
dave_json_array_add_str_len(void *array, s8 *str_data, ub str_len)
{
	struct json_object *pObject;

	pObject = json_object_new_string_len((const char *)str_data, (int)str_len);

	if(pObject != NULL)
	{
		json_object_array_add((struct json_object *)array, pObject);
		return dave_true;
	}
	else
	{
		PARTYABNOR("the array add str failed!");
		return dave_false;
	}
}

s8 *
dave_json_array_get_str(void *array, sb index, ub *string_len)
{
	struct json_object *pObject;

	pObject = dave_json_get_array_idx(array, index);

	if(pObject != NULL)
	{
		if(string_len != NULL)
		{
			*string_len = (ub)json_object_get_string_len(pObject);
		}
		return (s8 *)json_object_get_string(pObject);
	}
	else
	{
		PARTYABNOR("the array get str failed!");
		return NULL;
	}
}

dave_bool
dave_json_array_add_sb(void *array, sb ub_data)
{
	struct json_object *pObject;

	pObject = json_object_new_int64((int64_t)ub_data);

	if(pObject != NULL)
	{
		json_object_array_add((struct json_object *)array, pObject);
		return dave_true;
	}
	else
	{
		PARTYABNOR("the array add sb failed!");
		return dave_false;
	}
}

sb
dave_json_array_get_sb(void *array, sb index)
{
	struct json_object *pObject;

	pObject = dave_json_get_array_idx(array, index);

	if(pObject != NULL)
	{
		return (sb)json_object_get_int64(pObject);
	}
	else
	{
		PARTYABNOR("the array get sb failed!");
		return -1;
	}
}

dave_bool
dave_json_array_add_double(void *array, double double_data)
{
	struct json_object *pObject;

	pObject = json_object_new_double(double_data);

	if(pObject != NULL)
	{
		json_object_array_add((struct json_object *)array, pObject);

		return dave_true;
	}
	else
	{
		PARTYABNOR("the array add double failed!");

		return dave_false;
	}
}

double
dave_json_array_get_double(void *array, sb index)
{
	struct json_object *pObject;

	pObject = dave_json_get_array_idx(array, index);

	if(pObject != NULL)
	{
		return json_object_get_double(pObject);
	}
	else
	{
		PARTYABNOR("the array add double failed!");

		return 0;
	}
}

dave_bool
dave_json_array_add_int(void *array, ub int_data)
{
	struct json_object *pObject;

	pObject = json_object_new_int((int)int_data);

	if(pObject != NULL)
	{
		json_object_array_add((struct json_object *)array, pObject);

		return dave_true;
	}
	else
	{
		PARTYABNOR("the array add int failed!");

		return dave_false;
	}
}

void *
dave_json_array_get_object(void *array, sb index)
{
	return dave_json_get_array_idx(array, index);
}

dave_bool
__dave_json_array_add_object__(void *array, void *pObject, s8 *fun, ub line)
{
	if(pObject != NULL)
	{
		json_object_array_add((struct json_object *)array, pObject);

		return dave_true;
	}
	else
	{
		PARTYABNOR("the array add object failed! <%s:%d>", fun, line);

		return dave_false;
	}
}

dave_bool 
__dave_json_array_del_object__(void *array, ub index, s8 *fun, ub line)
{
    if (NULL != array)
    {
        if (json_object_array_del_idx((struct json_object *)array, index, 1) == 0)
        {
            return dave_true;
        }
    }
    return dave_false;
}

dave_bool
dave_json_array_add_array(void *array_dst, void *array_src)
{
	json_object_array_add((struct json_object *)array_dst, (struct json_object *)array_src);

	return dave_true;
}

dave_bool
dave_json_bool_copy(void *pDstJson, void *pSrcJson, char *key)
{
	dave_bool bool_data;
	dave_bool ret;

	ret = dave_json_get_boolean(pSrcJson, key, &bool_data);
	if(ret == dave_true)
	{
		ret = dave_json_add_boolean(pDstJson, key, bool_data);
	}

	return ret;
}

dave_bool
dave_json_ub_copy(void *pDstJson, void *pSrcJson, char *key)
{
	ub ub_data;
	dave_bool ret;

	ret = dave_json_get_ub(pSrcJson, key, &ub_data);
	if(ret == dave_true)
	{
		ret = dave_json_add_ub(pDstJson, key, ub_data);
	}

	return ret;
}

dave_bool
dave_json_str_copy(void *pDstJson, void *pSrcJson, char *key)
{
	s8 *str_data;
	ub str_length = 2048;
	dave_bool ret;

	str_data = dave_malloc(str_length);

	ret = dave_json_get_str(pSrcJson, key, str_data, &str_length);
	if(ret == dave_true)
	{
		ret = dave_json_add_str(pDstJson, key, str_data);
	}

	dave_free(str_data);

	return ret;
}

dave_bool
dave_json_array_copy(void *pDstJson, void *pSrcJson, char *key)
{
	void *pArray, *pNewArray;
	s8 *pObjectStr;
	ub pObjectLength;
	dave_bool ret = dave_false;

	pArray = dave_json_get_array(pSrcJson, key);
	if(pArray != NULL)
	{
		pObjectStr = dave_json_to_string(pArray, &pObjectLength);

		if(pObjectStr != NULL)
		{
			pNewArray = dave_string_to_json(pObjectStr, (sb)pObjectLength);

			if(pNewArray != NULL)
			{
				ret = dave_json_add_array(pDstJson, key, pNewArray);
			}
		}
	}

	return ret;
}

MBUF *
dave_json_to_mbuf(void *pJson)
{
	s8 *str;
	ub str_len;
	MBUF *mbuf_data;

	if(pJson == NULL)
	{
		return NULL;
	}

	str = dave_json_to_string(pJson, &str_len);

	if(str == NULL)
	{
		return NULL;
	}

	mbuf_data = dave_mmalloc(str_len + 1);
	dave_memcpy(mbuf_data->payload, str, str_len);
	mbuf_data->len = mbuf_data->tot_len = (sb)(str_len);
	((s8 *)(mbuf_data->payload))[str_len] = '\0';

	return mbuf_data;
}

void *
dave_json_clone(void *pJson)
{
	void *pCloneJson = NULL;
	s8 *json_str;
	ub json_length;

	if(pJson != NULL)
	{
		json_str = dave_json_to_string(pJson, &json_length);
		if((json_str != NULL) && (json_length > 0))
		{
			pCloneJson = dave_string_to_json(json_str, json_length);
		}
	}

	return pCloneJson;
}

dave_bool
dave_json_write(void *pJson, s8 *file_name)
{
	s8 *json_str;
	size_t json_length;

	if(pJson == NULL)
		return dave_false;

	json_str = (s8 *)json_object_to_json_string_length((struct json_object *)pJson, JSON_C_TO_STRING_PRETTY, &json_length);

	return dave_os_file_write(CREAT_WRITE_FLAG|DIRECT_FLAG, file_name, 0, json_length, (u8 *)json_str);
}

#endif

