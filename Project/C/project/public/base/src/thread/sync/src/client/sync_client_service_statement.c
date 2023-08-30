/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT)
#include "dave_verno.h"
#include "dave_base.h"
#include "dave_3rdparty.h"
#include "dave_tools.h"
#include "base_rxtx.h"
#include "thread_tools.h"
#include "sync_client_param.h"
#include "sync_base_package.h"
#include "sync_client_tx.h"
#include "sync_param.h"
#include "sync_log.h"

static s8 *_service_statement_table[] = {
	"support_queue_server",
	"\0"
};

// =====================================================================

void
sync_client_service_statement_reset(SyncServer *pServer)
{
	dave_memset(&(pServer->service_statement), 0x00, sizeof(ServiceStatement));

	pServer->service_statement.support_queue_server = dave_false;
}

void
sync_client_service_statement_rx(SyncServer *pServer, ub frame_len, s8 *frame_ptr)
{
	void *pJson;
	ub table_index;
	dave_bool bool_data;

	pJson = dave_string_to_json(frame_ptr, frame_len);
	if(pJson == NULL)
	{
		SYNCLOG("invalid json:%d/%s", frame_len, frame_ptr);
		return;
	}

	for(table_index=0; table_index<4096; table_index++)
	{
		if(_service_statement_table[table_index][0] == '\0')
			break;

		if(dave_json_c_get_boolean(pJson, _service_statement_table[table_index], &bool_data) == dave_true)
		{
			SYNCLOG("%s/%s %s %s",
				pServer->globally_identifier, pServer->verno,
				bool_data == dave_true ? "support" : "unsupported",
				_service_statement_table[table_index]);

			if(dave_strcmp(_service_statement_table[table_index], "support_queue_server") == dave_true)
			{
				pServer->service_statement.support_queue_server = bool_data;
			}
		}
	}	

	dave_json_free(pJson);
}

void
sync_client_service_statement_tx(SyncServer *pServer)
{
	void *pJson = dave_json_malloc();
	ub table_index;

	for(table_index=0; table_index<4096; table_index++)
	{
		if(_service_statement_table[table_index][0] == '\0')
			break;

		dave_json_c_add_boolean(pJson, _service_statement_table[table_index], dave_true);
	}

	sync_client_tx_service_statement(pServer, dave_json_to_string(pJson, NULL));

	dave_json_free(pJson);
}

#endif

