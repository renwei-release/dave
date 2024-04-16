/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_BDATA__
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_os.h"
#include "dave_verno.h"
#include "bdata_tools.h"
#include "bdata_msg.h"
#include "bdata_log.h"

static void
_reporter_save_file(s8 *file_path, void *pJson)
{
	EmailJsonAttachment *pAttachment;
	s8 file_name[256];

	pAttachment = t_a2b_email_json_build_attachment(pJson);
	if(pAttachment != NULL)
	{
		dave_snprintf(file_name, sizeof(file_name), "%s/%s", file_path, pAttachment->filename);
		if(dave_os_file_valid(file_name) == dave_true)
		{
			dave_snprintf(file_name, sizeof(file_name), "%s/%s.%lx", file_path, pAttachment->filename, t_rand());
		}

		if(dave_os_file_write(CREAT_WRITE_FLAG|DIRECT_FLAG, file_name, 0, pAttachment->binlen, (u8 *)(pAttachment->binbody)) == dave_false)
		{
			BDLOG("write file:%d/%s failed!", pAttachment->binlen, file_name);
		}
		else
		{
			BDLOG("write file:%d/%s success!", pAttachment->binlen, file_name);
		}

		t_a2b_email_json_release_attachment(pAttachment);
	}
	else
	{
		BDLOG("build attachment failed:%s", dave_json_to_string(pJson, NULL));
	}
}

// =====================================================================

void
reporter_save_file(s8 *file_path, s8 *file_data)
{
	if(file_data == NULL)
		return;

	void *pArray = dave_string_to_json(file_data, 0);
	if(pArray == NULL)
		return;

	sb array_length = dave_json_get_array_length(pArray);
	sb array_index;

	for(array_index=0; array_index<array_length; array_index++)
	{
		void *pJson = dave_json_array_get_object(pArray, array_index);

		if(pJson != NULL)
		{
			_reporter_save_file(file_path, pJson);
		}
	}

	dave_json_free(pArray);
}

#endif

