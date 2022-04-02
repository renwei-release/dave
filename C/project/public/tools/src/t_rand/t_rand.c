/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_tools.h"

static ub _rand;

// =====================================================================

ub
t_rand(void)
{
	static ub out_put_rand;

	_rand = _rand + rand();

	if(out_put_rand == _rand)
	{
		_rand = _rand + 1;
	}

	out_put_rand = _rand;

	return out_put_rand;
}

