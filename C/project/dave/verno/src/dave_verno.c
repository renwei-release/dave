/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "verno_macro.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "dave_verno.h"

static const s8 __const_verno__[] = "++DAVEVERNO++"VERSION_PRODUCT"."VERSION_MISC"."VERSION_MAIN"."VERSION_SUB"."VERSION_REV"."VERSION_DATE_TIME"."VERSION_LEVEL"\0";

static s8 *
_verno_product(s8 *verno, s8 *temp_ptr, ub temp_len)
{
	dave_strfind(verno, '.', temp_ptr, temp_len);

	return temp_ptr;
}

// =====================================================================

s8 *
dave_verno(void)
{
	return (s8 *)(&__const_verno__[13]);
}

s8 *
dave_verno_product(s8 *verno, s8 *buf_ptr, ub buf_len)
{
	static s8 product_str[128];

	if(verno == NULL)
	{
		verno = dave_verno();
	}
	if(buf_ptr == NULL)
	{
		buf_ptr = product_str;
		buf_len = sizeof(product_str);
	}

	_verno_product(verno, buf_ptr, sizeof(buf_len));

	return buf_ptr;
}

