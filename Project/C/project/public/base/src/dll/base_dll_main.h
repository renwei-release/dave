/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_DLL_MAIN_H__
#define __BASE_DLL_MAIN_H__
#include "base_dll.h"

typedef enum {
	BaseDllRunningMode_Inner_Loop,
	BaseDllRunningMode_Outer_Loop,
	BaseDllRunningMode_Coroutine_Inner_Loop,
	BaseDllRunningMode_Coroutine_Outer_Loop,
	BaseDllRunningMode_max
} BaseDllRunningMode;

void dave_dll_main_init(
	BaseDllRunningMode mode,
	int thread_number,
	dll_callback_fun dll_init_fun, dll_callback_fun dll_main_fun, dll_callback_fun dll_exit_fun);

void dave_dll_main_exit(void);

ThreadId dave_dll_main_thread_id(void);

#endif

