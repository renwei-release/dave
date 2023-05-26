/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_os.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "tools_log.h"

#define CFG_LOCALHOST "LOCALHOST"
#define CFG_PRODUCT_NAME "ProductName"

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

	return "127.0.0.1";
}

s8 *
t_gp_product_name(void)
{
	static s8 product_name[128] = { 0x00 };
	s8 config_name[64];

	if(cfg_get(CFG_PRODUCT_NAME, (u8 *)config_name, sizeof(config_name)) == dave_true)
	{
		if(t_is_all_digit_or_alpha((u8 *)config_name, dave_strlen(config_name)) == dave_true)
		{
			dave_snprintf(product_name, sizeof(product_name), "%s_%s", dave_verno_my_product(), config_name);
		}
		else
		{
			TOOLSABNOR("Effective %s can only contain numbers and letters! <%s>", CFG_PRODUCT_NAME, config_name);
		}
	}

	if(product_name[0] == 0x00)
	{
		dave_strcpy(product_name, dave_verno_my_product(), sizeof(product_name));
	}

	return product_name;
}

s8 *
t_gp_base_path(s8 *path_ptr, ub path_len)
{
	dave_snprintf(path_ptr, path_len, "/dave/%s", dave_verno_my_product());

	lower(path_ptr);

	return path_ptr;
}

