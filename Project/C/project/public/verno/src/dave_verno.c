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
static s8 __dave_verno__[DAVE_VERNO_STR_LEN + 1] = { "\0" };
static s8 _product_str[128] = { '\0' };

static s8 *
_verno_product(s8 *verno, s8 *temp_ptr, ub temp_len)
{
	dave_strfind(verno, '.', temp_ptr, temp_len);

	return temp_ptr;
}

static dave_bool
_verno_number(ub *main_num, ub *sub_num, ub *rev_num, s8 *product_head)
{
	s8 product_ptr[128], misc_ptr[128], main_ptr[128], sub_ptr[128], rev_ptr[128];
	s8 *misc_head, *main_head, *sub_head, *rev_head;

	misc_head = dave_strfind(product_head, '.', product_ptr, sizeof(product_ptr));
	if(misc_head == NULL)
		return dave_false;

	main_head = dave_strfind(misc_head, '.', misc_ptr, sizeof(misc_ptr));
	if(main_head == NULL)
		return dave_false;

	sub_head = dave_strfind(main_head, '.', main_ptr, sizeof(main_ptr));
	if(sub_head == NULL)
		return dave_false;

	rev_head = dave_strfind(sub_head, '.', sub_ptr, sizeof(sub_ptr));
	if(rev_head == NULL)
		return dave_false;

	dave_strfind(rev_head, '.', rev_ptr, sizeof(rev_ptr));

	*main_num = stringdigital(main_ptr);
	*sub_num = stringdigital(sub_ptr);
	*rev_num = stringdigital(rev_ptr);

	return dave_true;
}

// =====================================================================

s8 *
dave_verno(void)
{
	if(__dave_verno__[0] == '\0')
		dave_strcpy(__dave_verno__, &__const_verno__[13], sizeof(__dave_verno__));

	return __dave_verno__;
}

s8 *
dave_verno_reset(s8 *verno)
{
	dave_memset(_product_str, 0x00, sizeof(_product_str));

	dave_strcpy(__dave_verno__, verno, sizeof(__dave_verno__));

	return dave_verno();
}

s8 *
dave_verno_product(s8 *verno, s8 *buf_ptr, ub buf_len)
{
	static s8 product_str[64];

	if(verno == NULL)
	{
		verno = dave_verno();
	}
	if(buf_ptr == NULL)
	{
		buf_ptr = product_str;
		buf_len = sizeof(product_str);
	}

	_verno_product(verno, buf_ptr, buf_len);

	return buf_ptr;
}

dave_bool
dave_verno_number(ub *main_num, ub *sub_num, ub *rev_num, s8 *verno)
{
	if(verno == NULL)
	{
		*main_num = stringdigital(VERSION_MAIN);
		*sub_num = stringdigital(VERSION_SUB);
		*rev_num = stringdigital(VERSION_REV);
	}
	else
	{
		if(_verno_number(main_num, sub_num, rev_num, verno) == dave_false)
		{
			*main_num = 0;
			*sub_num = 0;
			*rev_num = 0;
			return dave_false;
		}
	}

	return dave_true;
}

s8 *
dave_verno_my_product(void)
{
	if(_product_str[0] != '\0')
		return _product_str;

	return dave_verno_product(dave_verno(), _product_str, sizeof(_product_str));
}

