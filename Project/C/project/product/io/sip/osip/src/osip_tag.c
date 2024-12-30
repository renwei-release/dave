/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "osip_log.h"

// =====================================================================

s8 *
osip_tag(s8 *tag_ptr, ub tag_len)
{
	dave_snprintf(tag_ptr, tag_len,
		"%lx-%lx-%lx-%lx-%lx",
		(u32)t_rand(), (u16)t_rand(), (u16)t_rand(), (u16)t_rand(), (u32)t_rand());
	return tag_ptr;
}

s8 *
osip_branch(s8 *branch_ptr, ub branch_len)
{
	ub branch_index;

	dave_memset(branch_ptr, 0x00, branch_len);

	branch_len -= 1;

	branch_index = dave_strcpy(branch_ptr, "z9hG4bK", branch_len);

	osip_tag(&branch_ptr[branch_index], branch_len-branch_index);

	return branch_ptr;
}


