/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_store.h"
#include "dave_os.h"
#include "dave_echo.h"
#include "rtc_server.h"

#define CFG_RTC_SERVER_PORT "RTCServerPort"

// =====================================================================

ub
rtc_server_port(void)
{
	return cfg_get_ub(CFG_RTC_SERVER_PORT, 5345);
}

