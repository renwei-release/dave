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
#include "dos_tty.h"
#include "dos_pop.h"
#include "dos_show.h"
#include "dos_log.h"

typedef enum {
	DOS_POP_OPTION_CONFIRM = 0,
	DOS_POP_INPUT_REQUEST = 1,
	DOS_POP_TYPE_MAX
} DOSPopType;

static DOSPopType _pop_type;
static pop_process_fun _pop_fun1, _pop_fun2;

static void
_dos_pop_analysis_confirm(s8 *param, ub param_len)
{
	pop_process_fun fun;

	if(((param_len == 3) && ((dave_strcmp(param,(s8 *)"yes") == dave_true) || (dave_strcmp(param,(s8 *)"YES") == dave_true)))
		|| ((param_len == 1) && ((param[0] == 'y') || (param[0] == 'Y'))))
	{
		if(_pop_fun1 != NULL)
		{
			fun = _pop_fun1;
			_pop_fun1 = NULL;
			fun(param, param_len);
		}
	}
	else
	{
		if(_pop_fun2 != NULL)
		{
			fun = _pop_fun2;
			_pop_fun2 = NULL;
			fun(param, param_len);
		}
	}
}

static void
_dos_pop_process_input_request(s8 *param, ub param_len)
{
	pop_process_fun fun;

	if(_pop_fun1 != NULL)
	{
		fun = _pop_fun1;
		_pop_fun1 = NULL;
		fun(param, param_len);
	}	
}

static void 
_dos_pop_analysis(s8 *param, ub param_len)
{
	switch(_pop_type)
	{
		case DOS_POP_OPTION_CONFIRM:
				_pop_type = DOS_POP_TYPE_MAX;
				_dos_pop_analysis_confirm(param, param_len);
			break;
		case DOS_POP_INPUT_REQUEST:
				_pop_type = DOS_POP_TYPE_MAX;
				_dos_pop_process_input_request(param, param_len);
			break;
		default:
			break;
	}

	dos_print((s8 *)"");
}

static dave_bool
_dos_pop_state(void)
{
	if(_pop_type >= DOS_POP_TYPE_MAX)
		return dave_false;
	else
		return dave_true;
}

// =====================================================================

void 
dos_pop_init(void)
{	
	_pop_type = DOS_POP_TYPE_MAX;
	_pop_fun1 = _pop_fun2 = NULL;
}

void 
dos_pop_exit(void)
{
	
}

dave_bool 
dos_pop_analysis(s8 *input, ub input_len)
{
	ub index;
	s8 *param;

	if(_dos_pop_state() == dave_false)
	{
		return dave_false;
	}
	
	param = dave_malloc(input_len+1);
	if(input != NULL)
	{
		index = 0;
		dave_memset(param,0x00,input_len+1);

		while(index < input_len)
		{
			if((input[index] == '\0')
				|| (input[index] == '\r')
				|| (input[index] == '\n'))
			{
				break;
			}
			else
			{
				param[index] = input[index];
			}

			index ++;
		}

		_dos_pop_analysis(param, dave_strlen(param));

		dave_free(param);
	}

	return dave_true;
}

ErrCode
dos_pop_confirm(char *msg, pop_process_fun yes, pop_process_fun no)
{
	s8 comfirm_msg[] = {"\r\nYou can choose \"yes\" or \"no\": "};

	if(_pop_type != DOS_POP_TYPE_MAX)
	{
		return ERRCODE_Resource_conflicts;
	}
	else
	{
		_pop_type = DOS_POP_OPTION_CONFIRM;
		_pop_fun1 = yes;
		_pop_fun2 = no;
		dos_tty_write((u8 *)msg, dave_strlen(msg));
		dos_tty_write((u8 *)comfirm_msg, dave_strlen(comfirm_msg));
		return ERRCODE_OK;
	}
}

ErrCode
dos_pop_input_request(s8 *msg, pop_process_fun process_fun)
{
	if(_pop_type != DOS_POP_TYPE_MAX)
	{
		return ERRCODE_Resource_conflicts;
	}
	else
	{
		_pop_type = DOS_POP_INPUT_REQUEST;
		_pop_fun1 = process_fun;
		_pop_fun2 = NULL;
		dos_tty_write((u8 *)msg, dave_strlen(msg));
		return ERRCODE_OK;
	}
}

#endif

