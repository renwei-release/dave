/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_os.h"
#include "dave_tools.h"
#include "tools_log.h"

#define CFG_LOCALHOST "LOCALHOST"

// =====================================================================

s8 *
t_gp_localhost(void)
{
	static s8 localhost[64];

	if(cfg_get(CFG_LOCALHOST, (u8 *)localhost, sizeof(localhost)) == dave_true)
	{
		if(t_is_ipv4((s8 *)localhost) == dave_true)
		{
			return localhost;
		}

		TOOLSABNOR("find invalid localhost:%s!", localhost);
	}

	if(dave_os_on_docker() == dave_true)
	{
		// 在docker中默认连接到宿主机上。 "172.17.0.1"
		// 如果--network host模式，那么这里需要修改成"localhost"
		return "172.17.0.1";
	}
	else
	{
		return "localhost";
	}
}

