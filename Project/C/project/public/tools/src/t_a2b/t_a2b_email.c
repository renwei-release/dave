/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "tools_log.h"

// =====================================================================

EmailJsonAttachment *
t_a2b_email_json_build_attachment(void *pJson)
{
	EmailJsonAttachment *pAttachment = dave_ralloc(sizeof(EmailJsonAttachment));
	ub filelen;

	if(dave_json_get_str_v2(pJson, "filename", pAttachment->filename, sizeof(pAttachment->filename)) == 0)
	{
		TOOLSLOG("can't find filename:%s", dave_json_to_string(pJson, NULL));
		t_a2b_email_json_release_attachment(pAttachment);
		return NULL;
	}

	/*
	 * https://developer.mozilla.org/zh-CN/docs/Web/HTTP/Basics_of_HTTP/MIME_types/Common_types
	 */
	if(dave_json_get_str_v2(pJson, "mimetype", pAttachment->mimetype, sizeof(pAttachment->mimetype)) == 0)
	{
		TOOLSLOG("can't find mimetype:%s", dave_json_to_string(pJson, NULL));
		t_a2b_email_json_release_attachment(pAttachment);
		return NULL;
	}

	filelen = dave_json_get_str_length(pJson, "filebody");
	if(filelen == 0)
	{
		TOOLSLOG("can't find filebody:%s", dave_json_to_string(pJson, NULL));
		t_a2b_email_json_release_attachment(pAttachment);
		return NULL;
	}

	filelen += 16;
	s8 *filebody = dave_malloc(filelen);
	pAttachment->binlen = filelen;
	pAttachment->binbody = dave_malloc(pAttachment->binlen);

	filelen = dave_json_get_str_v2(pJson, "filebody", filebody, filelen);
	filebody[filelen] = '\0';

	if(t_crypto_base64_decode(filebody, filelen, (u8 *)(pAttachment->binbody), &(pAttachment->binlen)) == dave_false)
	{
		TOOLSLOG("base64 decode failed:%s/%d", pAttachment->filename, filelen);
		dave_free(filebody);
		t_a2b_email_json_release_attachment(pAttachment);
		return NULL;
	}

	return pAttachment;
}

void
t_a2b_email_json_release_attachment(EmailJsonAttachment *pAttachment)
{
	if(pAttachment != NULL)
	{
		if(pAttachment->binbody != NULL)
			dave_free(pAttachment->binbody);

		dave_free(pAttachment);
	}
}

