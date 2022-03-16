/*
 * ================================================================================
 * (c) Copyright 2016 Renwei(528.ww@163.com) All rights reserved.
 * --------------------------------------------------------------------------------
 * 2016.9.1.
 * ================================================================================
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_tools.h"
#include "dos_show.h"
#include "dos_cmd.h"
#include "dos_log.h"

static ErrCode
_dos_show_support_cmd_list(s8 *param, ub param_len)
{
	MBUF *list = dos_cmd_list(), *search;
	s8 *msg;
	ub msg_len = 4096, msg_index, cmd_counter;
	ub space_len, cmd_len, cmd_len_max = 0;

	if(list != NULL)
	{
		//get cmd len max
		search = list;
		while(search != NULL)
		{
			cmd_len = dave_strlen(search->payload);
			if(cmd_len_max < cmd_len)
				cmd_len_max = cmd_len;

			search = search->next;
		}

		msg = dave_malloc(msg_len);
		if(msg != NULL)
		{
			dave_memset(msg, 0x00, msg_len);
			search = list;
			msg_index = 0;
			cmd_counter = 0;
			msg_index += dave_sprintf(&msg[msg_index], "Support of the command list:\r\n");
			while(search != NULL)
			{
				msg_index += dave_sprintf(&msg[msg_index], "%s ", search->payload);
				space_len = cmd_len_max+2-dave_strlen(search->payload);
				dave_memset(&msg[msg_index],' ',space_len);
				msg_index += space_len;
				if((++ cmd_counter) > 4)
				{
					msg_index += dave_sprintf(&msg[msg_index], "\r\n");
					cmd_counter = 0;
				}
				search = search->next;
			}
			dos_print(msg);
			dave_free(msg);
		}
		dave_mfree(list);
	}
	else
	{
		dos_print("empty command list!");
	}

	return ERRCODE_OK;
}

// =====================================================================

void
dos_ls_reset(void)
{
	dos_cmd_register("ls", _dos_show_support_cmd_list, NULL);
}

#endif

