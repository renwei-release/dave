/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "3rdparty_macro.h"
#if defined(QUICKMAIL_3RDPARTY)
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "party_log.h"
#include "quickmail.h"

static void
__quickmail_add_attachment(quickmail mailobj, void *pJson)
{
	EmailJsonAttachment *pAttachment;

	pAttachment = t_a2b_email_json_build_attachment(pJson);
	if(pAttachment != NULL)
	{
		char * binbody = malloc(pAttachment->binlen);

		memcpy(binbody, pAttachment->binbody, pAttachment->binlen);

		quickmail_add_attachment_memory(mailobj, pAttachment->filename, pAttachment->mimetype, binbody, pAttachment->binlen, 1);

		t_a2b_email_json_release_attachment(pAttachment);
	}
	else
	{
		PARTYLOG("build attachment failed:%s", dave_json_to_string(pJson, NULL));
	}
}

static void
_quickmail_add_attachment(quickmail mailobj, s8 *attachment)
{
	if(attachment == NULL)
		return;

	void *pArray = dave_string_to_json(attachment, 0);
	if(pArray == NULL)
		return;

	sb array_length = dave_json_get_array_length(pArray);
	sb array_index;

	for(array_index=0; array_index<array_length; array_index++)
	{
		void *pJson = dave_json_array_get_object(pArray, array_index);

		if(pJson != NULL)
		{
			__quickmail_add_attachment(mailobj, pJson);
		}
	}

	dave_json_free(pArray);
}

static void
_quickmail_add_to(quickmail mailobj, s8 *to_email)
{
	void *pToEmailArray;
	sb array_length, array_index;
	s8 *to_user_email;

	pToEmailArray = dave_string_to_json(to_email, dave_strlen(to_email));
	if(pToEmailArray == NULL)
	{
		quickmail_add_to(mailobj, to_email);
	}
	else
	{
		array_length = dave_json_get_array_length(pToEmailArray);
		for(array_index=0; array_index<array_length; array_index++)
		{
			to_user_email = dave_json_array_get_str(pToEmailArray, array_index, NULL);

			quickmail_add_to(mailobj, to_user_email);
		}

		dave_json_free(pToEmailArray);
	}
}

// =====================================================================

dave_bool
dave_quickmail(s8 *username, s8 *password, s8 *smtp_url, s8 *from_email, s8 *to_email, s8 *subject, s8 *body, s8 *attachment)
{
	const char* errmsg;
	dave_bool ret;

	quickmail_initialize();

	quickmail mailobj = quickmail_create(from_email, subject);

	_quickmail_add_to(mailobj, to_email);
	quickmail_set_body(mailobj, body);
	_quickmail_add_attachment(mailobj, attachment);

	errmsg = quickmail_send(mailobj, smtp_url, 0, username, password);
	if(errmsg != NULL)
	{
		PARTYLOG("errmsg:%s %s/%s/%s/%s/%s",
			errmsg, username, password, smtp_url, from_email, to_email);
		ret = dave_false;
	}
	else
	{
		ret = dave_true;
	}

	quickmail_destroy(mailobj);

	quickmail_cleanup();

	return ret;
}

#endif

