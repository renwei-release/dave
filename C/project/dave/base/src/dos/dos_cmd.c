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
	s8 cmd[DOS_CMD_LEN + 1];
	cmd_process_fun fun;
	help_process_fun help_fun;
	sb life_cycle;
	void *next;
} DOSCmdStruct;

static DOSCmdStruct *_cmd_list_head = NULL;

static DOSCmdStruct *
_find_the_cmd(s8 *cmd, ub cmd_len)
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
_add_cmd_list(DOSCmdStruct *pNewCmd)
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
_show_invalid_cmd_screen(s8 *input, ub input_len)
{
	s8 *invalid_cmd;
	ub invalid_cmd_len = 256 + input_len;
	ub invalid_index;

	invalid_cmd = dave_ralloc(invalid_cmd_len);

	invalid_index = 0;
	invalid_index += dave_sprintf(&invalid_cmd[invalid_index], "Sorry, you entered an invalid command:");
	dave_memcpy(&invalid_cmd[invalid_index], input, input_len);
	invalid_index += input_len;
	invalid_cmd[invalid_index++] = '\r';
	invalid_cmd[invalid_index++] = '\n';
	dave_sprintf(&invalid_cmd[invalid_index], "Please enter help or lscmd to get information!");
	dos_print(invalid_cmd);

	dave_free(invalid_cmd);
}

static void
_show_not_support_cmd_screen(s8 *cmd, ub cmd_len)
{
	ub show_index;
	s8 *support_cmd;
	ub support_cmd_len = 128 + cmd_len;

	support_cmd = dave_malloc(support_cmd_len);
	if(support_cmd != NULL)
	{
		show_index = 0;
		dave_memset(support_cmd, 0x00, support_cmd_len);
		dave_sprintf(&support_cmd[show_index], "Sorry, you entered an unsupported command:%s\r\n", cmd);
		dos_print(support_cmd);
		dave_free(support_cmd);
	}
}

static void
_show_run_cmd_failed_screen(s8 *cmd, ub cmd_len, s8 *param, ub param_len, ErrCode ret)
{
	s8 *cmd_failed;
	ub cmd_failed_len = 128 + cmd_len + param_len;

	cmd_failed = dave_malloc(cmd_failed_len);
	if(cmd_failed != NULL)
	{
		dave_memset(cmd_failed, 0x00, cmd_failed_len);
		dave_sprintf(cmd_failed, "Sorry(%s), you entered an invalid command:\r\n%s or param:\r\n%s", errorstr(ret), cmd, param);
		dos_print(cmd_failed);
		dave_free(cmd_failed);
	}
}

static void
_show_run_help_failed_screen(s8 *cmd, ub cmd_len)
{
	s8 *help_failed;
	ub help_failed_len = 128 + cmd_len;

	help_failed = dave_malloc(help_failed_len);
	if(help_failed != NULL)
	{
		dave_memset(help_failed, 0x00, help_failed_len);
		dave_sprintf(help_failed, "Sorry,help(%s) failed", cmd);
		dos_print(help_failed);
		dave_free(help_failed);
	}
}

static void
_dos_cmd_analysis(s8 *input, ub input_len, s8 *cmd, ub cmd_len, s8 *param, ub param_len)
{
	DOSCmdStruct *pCmd;
	ErrCode ret;

	if(_dos_get_cmd_and_param(cmd, &cmd_len, param, &param_len, input, input_len) == dave_false)
	{
		_show_invalid_cmd_screen(input, input_len);
		return;
	}

	pCmd = _find_the_cmd(cmd, cmd_len);
	if(pCmd == NULL)
	{
		_show_not_support_cmd_screen(cmd, cmd_len);
		return;
	}

	DOSLOG("%s %s", cmd, param);

	ret = ERRCODE_Arithmetic_error;
	if(pCmd->fun != NULL)
	{
		ret = (pCmd->fun)(param, param_len);
	}
	if(ret != ERRCODE_OK)
	{
		if(pCmd->help_fun == NULL)
		{
			_show_run_cmd_failed_screen(cmd, cmd_len, param, param_len, ret);
		}
		else
		{
			pCmd->help_fun();
		}
	}
}

static ErrCode
_register_cmd_list(s8 *cmd, cmd_process_fun fun, help_process_fun help_fun, sb life_cycle)
{
	ub cmd_len = dave_strlen(cmd);
	DOSCmdStruct *pNewCmd;

	if((cmd_len > DOS_CMD_LEN)
		|| (fun == NULL))
		return ERRCODE_Invalid_parameter;
	
	if(_find_the_cmd(cmd, cmd_len) != NULL)
		return ERRCODE_Resource_conflicts;

	pNewCmd = dave_malloc(sizeof(DOSCmdStruct));
	if(pNewCmd == NULL)
		return ERRCODE_Memory_full;

	dave_memset(pNewCmd->cmd, 0x00, DOS_CMD_LEN + 1);
	dave_memcpy(pNewCmd->cmd, cmd, cmd_len);
	pNewCmd->fun = fun;
	pNewCmd->help_fun = help_fun;
	pNewCmd->life_cycle = life_cycle;
	pNewCmd->next = NULL;

	_add_cmd_list(pNewCmd);

	return ERRCODE_OK;
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
	dos_cmd_reset();
}

void
dos_cmd_reset(void)
{
	DOSCmdStruct *temp;

	while(_cmd_list_head != NULL)
	{
		temp = (DOSCmdStruct *)(_cmd_list_head->next);
		dave_free(_cmd_list_head);
		_cmd_list_head = temp;
	}
}

void
dos_cmd_analysis(s8 *input, ub input_len)
{
	s8 *cmd;
	ub cmd_len;
	s8 *param;
	ub param_len;

	cmd = dave_ralloc(input_len + 1);
	param = dave_ralloc(input_len + 1);
	cmd_len = input_len;
	param_len = input_len;

	_dos_cmd_analysis(input, input_len, cmd, cmd_len, param, param_len);

	dave_free(cmd);
	dave_free(param);	
}

void
dos_help_analysis(s8 *cmd, ub cmd_len)
{
	DOSCmdStruct *pCmd;
	ErrCode ret = ERRCODE_OK;

	pCmd = _find_the_cmd(cmd, cmd_len);
	if(pCmd == NULL)
	{
		_show_not_support_cmd_screen(cmd, cmd_len);
		return;
	}
	
	if(pCmd->help_fun != NULL)
	{
		ret = (pCmd->help_fun)();
	}
	if(ret != ERRCODE_OK)
	{
		_show_run_help_failed_screen(cmd, cmd_len);
	}
}

ErrCode
dos_cmd_register(char *cmd, cmd_process_fun fun, help_process_fun help_fun)
{
	return _register_cmd_list((s8 *)cmd, fun, help_fun, -1);
}

ErrCode
dos_cmd_talk_register(s8 *cmd, cmd_process_fun fun, help_process_fun help_fun)
{
	return _register_cmd_list(cmd, fun, help_fun, 1);
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

#endif

