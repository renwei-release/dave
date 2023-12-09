/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_verno.h"
#include "dave_tools.h"
#include "dll_tools.h"
#include "dll_log.h"

// =====================================================================

void
dave_dll_log(char *func, int line, char *log_msg, int log_type)
{
	ub log_len = 1024 + dave_strlen(log_msg);
	s8 *log_ptr = dave_malloc(log_len);

	if(log_msg != NULL)
	{
		log_len = dave_strcpy(log_ptr, log_msg, log_len);
	}
	else
	{
		log_ptr[0] = '\0';
		log_len = 0;
	}

	dll_remove_some_data(log_ptr, log_len);

	if(func == NULL)
	{
		func = "NULL";
	}

	switch(log_type)
	{
		case 0:
				DAVEDEBUG("[%s]<%s:%d>%s\n", dave_verno_my_product(), func, line, log_ptr);
			break;
		case 1:
				DAVETRACE("[%s]<%s:%d>%s\n", dave_verno_my_product(), func, line, log_ptr);
			break;
		case 2:
				DAVELOG("[%s]<%s:%d>%s\n", dave_verno_my_product(), func, line, log_ptr);
			break;
		case 3:
				DAVEABNORMAL("[%s]<%s:%d>%s\n", dave_verno_my_product(), func, line, log_ptr);				
			break;
		default:
				DAVETRACE("[%s]<%s:%d>%s\n", dave_verno_my_product(), func, line, log_ptr);
			break;
	}

	dave_free(log_ptr);
}

#endif

