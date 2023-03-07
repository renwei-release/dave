/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_tools.h"
#include "dos_cmd.h"
#include "dos_show.h"
#include "dos_log.h"

#define DOS_CMD_LEN (128)

typedef struct {
	s8 owner_thread[DAVE_THREAD_NAME_LEN];
	s8 cmd[DOS_CMD_LEN];
	dos_cmd_fun cmd_fun;
	dos_help_fun help_fun;
	sb life_cycle;
	void *next;
} DOSCmdStruct;

static DOSCmdStruct *_cmd_list_head = NULL;

static DOSCmdStruct *
_dos_find_the_cmd(s8 *cmd, ub cmd_len)
{
	DOSCmdStruct *search;

	if(cmd_len > DOS_CMD_LEN)
		return NULL;

	search = _cmd_list_head;
	while(search != NULL)
	{
		if(dave_strcmp(cmd, search->cmd) == dave_true)
			return search;
		search = (DOSCmdStruct *)(search->next);
	}
	return NULL;
}

static void
_dos_add_cmd_list(DOSCmdStruct *pNewCmd)
{
	DOSCmdStruct *temp;

	if(_cmd_list_head == NULL)
	{
		_cmd_list_head = pNewCmd;
	}
	else
	{
		temp = _cmd_list_head;
		while((temp != NULL) && (temp->next != NULL))
		{
			temp = (DOSCmdStruct *)(temp->next);
		}
		if(temp != NULL)
		{
			temp->next = pNewCmd;
		}
	}
}

static dave_bool
_dos_get_cmd(s8 *cmd, ub *cmd_len, s8 *input, ub input_len)
{
	ub index;
	dave_bool result;

	if((input == NULL) || (input_len == 0) 
		|| (cmd == NULL) || (*cmd_len == 0))
		return dave_false;

	index = 0;
	(*cmd_len) = 0;
	result = dave_false;

	while((index < input_len)
		&& (input[index] != '\0')
		&& (input[index] != '\r')
		&& (input[index] != '\n')
		&& (t_is_alpha(input[index]) == dave_false))
	{
		index ++;
	}

	while((index < input_len)
		&& (input[index] != '\0')
		&& (input[index] != '\r')
		&& (input[index] != '\n')
		&& ((t_is_alpha(input[index]) == dave_true) || (t_is_digit(input[index]) == dave_true)))
	{
		cmd[(*cmd_len) ++] = input[index ++];
	}

	if((*cmd_len) != 0)
	{
		result = dave_true;
		cmd[(*cmd_len)] = '\0';
		t_stdio_tolowers(cmd);
	}

	return result;
}

static dave_bool
_dos_get_param(s8 *param, ub *param_len, s8 *input, ub input_len)
{
	ub index;

	if((input == NULL) || (input_len == 0) 
		|| (param == NULL) || (*param_len == 0))
		return dave_false;

	index = 0;

	while((index < input_len) 
		&& (input[index] != '\0')
		&& (input[index] != '\r')
		&& (input[index] != '\n')
		&& (input[index] == ' '))
	{
		index ++;
	}
	
	(*param_len) = 0;
	while((index < input_len) 
		&& (input[index] != '\0')
		&& (input[index] != '\r')
		&& (input[index] != '\n'))
	{
		param[(*param_len) ++] = input[index ++];
	}

	if((*param_len) != 0)
	{
		param[(*param_len)] = '\0';
	}
	return dave_true;
}

static dave_bool
_dos_get_cmd_and_param(s8 *cmd, ub *cmd_len, s8 *param, ub *param_len, s8 *input, ub input_len)
{
	dave_bool result;

	if((input == NULL)
		|| (input_len == 0) 
		|| (cmd == NULL)
		|| (*cmd_len == 0)
		|| (param == NULL)
		|| (*param_len == 0))
	{
		return dave_false;
	}

	result = _dos_get_cmd(cmd, cmd_len, input, input_len);

	if((result == dave_true) && ((*cmd_len) < input_len))
	{
		result = _dos_get_param(param, param_len, &input[(*cmd_len)], input_len-(*cmd_len));
	}
	return result;
}

static void
_dos_show_invalid_cmd_screen(s8 *input_ptr, ub input_len)
{
	dos_print("Sorry, you entered an invalid command:%s\nPlease enter ls to get information!", input_ptr);
}

static void
_dos_show_not_support_cmd_screen(s8 *cmd_ptr, ub cmd_len)
{
	dos_print("Sorry, you entered an unsupported command:%s", cmd_ptr);

	dos_cmd_list_show();
}

static void
_dos_show_run_cmd_failed_screen(s8 *cmd_ptr, ub cmd_len, s8 *param, ub param_len, RetCode ret)
{
	dos_print("Sorry(%s), you entered an invalid command:\n%s or param:\n%s", retstr(ret), cmd_ptr, param);
}

static void
_dos_show_run_help_failed_screen(s8 *cmd_ptr, ub cmd_len)
{
	dos_print("Sorry,help(%s) failed!", cmd_ptr);
}

static void
_dos_record_cmd(s8 *cmd_ptr, s8 *param)
{
	if(dave_strcmp(cmd_ptr, "log") == dave_false)
	{
		DOSLOG("%s %s", cmd_ptr, param);
	}
}

static void
_dos_execute_cmd(DOSCmdStruct *pCmd, s8 *cmd_ptr, ub cmd_len, s8 *param_ptr, ub param_len)
{
	RetCode ret;

	ret = RetCode_Arithmetic_error;
	if(pCmd->cmd_fun != NULL)
	{
		ret = (pCmd->cmd_fun)(param_ptr, param_len);
	}
	if(ret != RetCode_OK)
	{
		if(pCmd->help_fun == NULL)
		{
			_dos_show_run_cmd_failed_screen(cmd_ptr, cmd_len, param_ptr, param_len, ret);
		}
		else
		{
			pCmd->help_fun();
		}
	}
}

static void
_dos_external_cmd(DOSCmdStruct *pCmd, s8 *cmd_ptr, ub cmd_len, s8 *param_ptr, ub param_len)
{
	DosForward *pForward = thread_msg(pForward);

	pForward->cmd = t_a2b_str_to_mbuf(cmd_ptr, cmd_len);
	pForward->param = t_a2b_str_to_mbuf(param_ptr, param_len);
	pForward->ptr = pCmd;

	name_msg(pCmd->owner_thread, MSGID_DOS_FORWARD, pForward);
}

static void
_dos_forward_msg(MSGBODY *msg)
{
	DosForward *pForward = (DosForward *)(msg->msg_body);
	DOSCmdStruct *pCmd = pForward->ptr;

	_dos_execute_cmd(pCmd, ms8(pForward->cmd), mlen(pForward->cmd), ms8(pForward->param), mlen(pForward->param));

	dave_mfree(pForward->cmd);
	dave_mfree(pForward->param);
}

static void
_dos_cmd_analysis(s8 *input_ptr, ub input_len, s8 *cmd_ptr, ub cmd_len, s8 *param_ptr, ub param_len)
{
	DOSCmdStruct *pCmd;

	if(_dos_get_cmd_and_param(cmd_ptr, &cmd_len, param_ptr, &param_len, input_ptr, input_len) == dave_false)
	{
		_dos_show_invalid_cmd_screen(input_ptr, input_len);
		return;
	}

	pCmd = _dos_find_the_cmd(cmd_ptr, cmd_len);
	if(pCmd == NULL)
	{
		_dos_show_not_support_cmd_screen(cmd_ptr, cmd_len);
		return;
	}

	_dos_record_cmd(cmd_ptr, param_ptr);

	if(dave_strcmp(pCmd->owner_thread, DOS_THREAD_NAME) == dave_true)
	{
		_dos_execute_cmd(pCmd, cmd_ptr, cmd_len, param_ptr, param_len);
	}
	else
	{
		_dos_external_cmd(pCmd, cmd_ptr, cmd_len, param_ptr, param_len);
	}
}

static RetCode
_dos_register_cmd_list(s8 *cmd, dos_cmd_fun cmd_fun, dos_help_fun help_fun, sb life_cycle)
{
	ub cmd_len = dave_strlen(cmd);
	DOSCmdStruct *pNewCmd;

	if((cmd_len > DOS_CMD_LEN) || (cmd_fun == NULL))
	{
		return RetCode_Invalid_parameter;
	}

	if(_dos_find_the_cmd(cmd, cmd_len) != NULL)
	{
		return RetCode_Resource_conflicts;
	}

	pNewCmd = dave_ralloc(sizeof(DOSCmdStruct));

	dave_strcpy(pNewCmd->owner_thread, thread_name(self()), sizeof(pNewCmd->owner_thread));
	dave_strcpy(pNewCmd->cmd, cmd, sizeof(pNewCmd->cmd));
	pNewCmd->cmd_fun = cmd_fun;
	pNewCmd->help_fun = help_fun;
	pNewCmd->life_cycle = life_cycle;
	pNewCmd->next = NULL;

	_dos_add_cmd_list(pNewCmd);

	if(dave_strcmp(pNewCmd->owner_thread, DOS_THREAD_NAME) == dave_false)
	{
		reg_msg(MSGID_DOS_FORWARD, _dos_forward_msg);
	}

	return RetCode_OK;
}

static void
_dos_cmd_reset(void)
{
	DOSCmdStruct *temp;

	while(_cmd_list_head != NULL)
	{
		temp = (DOSCmdStruct *)(_cmd_list_head->next);
		dave_free(_cmd_list_head);
		_cmd_list_head = temp;
	}
}

// =====================================================================

void
dos_cmd_init(void)
{
	_cmd_list_head = NULL;
}

void
dos_cmd_exit(void)
{
	_dos_cmd_reset();
}

void
dos_cmd_analysis(s8 *input_ptr, ub input_len)
{
	s8 *cmd_ptr;
	ub cmd_len;
	s8 *param_ptr;
	ub param_len;

	cmd_ptr = dave_ralloc(input_len + 1);
	param_ptr = dave_ralloc(input_len + 1);
	cmd_len = input_len;
	param_len = input_len;

	_dos_cmd_analysis(input_ptr, input_len, cmd_ptr, cmd_len, param_ptr, param_len);

	dave_free(cmd_ptr);
	dave_free(param_ptr);	
}

void
dos_help_analysis(s8 *cmd_ptr, ub cmd_len)
{
	DOSCmdStruct *pCmd;
	RetCode ret;

	pCmd = _dos_find_the_cmd(cmd_ptr, cmd_len);
	if(pCmd == NULL)
	{
		_dos_show_not_support_cmd_screen(cmd_ptr, cmd_len);
		return;
	}

	if(pCmd->help_fun != NULL)
	{
		ret = (pCmd->help_fun)();
		if(ret != RetCode_OK)
		{
			_dos_show_run_help_failed_screen(cmd_ptr, cmd_len);
		}
	}
}

RetCode
dos_cmd_reg(const char *cmd, dos_cmd_fun cmd_fun, dos_help_fun help_fun)
{
	return _dos_register_cmd_list((s8 *)cmd, cmd_fun, help_fun, -1);
}

RetCode
dos_cmd_talk_reg(s8 *cmd, dos_cmd_fun cmd_fun, dos_help_fun help_fun)
{
	return _dos_register_cmd_list(cmd, cmd_fun, help_fun, 1);
}

MBUF *
dos_cmd_list(void)
{
	DOSCmdStruct *search;
	MBUF *list, *temp;
	ub cmd_len;

	search = _cmd_list_head;
	list = temp = NULL;

	while(search != NULL)
	{
		cmd_len = dave_strlen(search->cmd);
		temp = dave_mmalloc(cmd_len + 1);
		if(temp == NULL)
			break;
		dave_memset(temp->payload, 0x00, cmd_len + 1);
		dave_memcpy(temp->payload, search->cmd, cmd_len);
		if(list != NULL)
			dave_mchain(temp, list);
		list = temp;
		search = (DOSCmdStruct *)(search->next);
	}
	return list;
}

void
dos_cmd_list_show(void)
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
			msg_index += dave_sprintf(&msg[msg_index], "Support of the command list:\n");
			while(search != NULL)
			{
				msg_index += dave_sprintf(&msg[msg_index], "%s ", search->payload);
				space_len = cmd_len_max+2-dave_strlen(search->payload);
				dave_memset(&msg[msg_index],' ',space_len);
				msg_index += space_len;
				if((++ cmd_counter) > 4)
				{
					msg_index += dave_sprintf(&msg[msg_index], "\n");
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
}

#endif

