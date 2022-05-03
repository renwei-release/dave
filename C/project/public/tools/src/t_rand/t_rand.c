/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_tools.h"
#include "dave_os.h"

static ub _rand = 0;

static ub
_t_rand_reset(void)
{
	u8 mac_ptr[DAVE_MAC_ADDR_LEN];
	ub mac_index, mac_counter;
	ub rand = dave_os_time_ns();

	mac_index = dave_os_time_ns() % DAVE_MAC_ADDR_LEN;
	dave_os_load_mac(mac_ptr);

	for(mac_counter=0; mac_counter<DAVE_MAC_ADDR_LEN; mac_counter++)
	{
		if(mac_index >= DAVE_MAC_ADDR_LEN)
			mac_index = 0;

		rand += (ub)(mac_ptr[mac_index ++]);
	}

	return rand;
}

// =====================================================================

ub
t_rand(void)
{
	static ub out_put_rand;

	if((_rand == 0) || ((_rand & 0x05) == 0x00))
	{
		_rand = _t_rand_reset();
	}

	_rand = _rand + out_put_rand + rand() + dave_os_time_ns();

	if(out_put_rand == _rand)
	{
		_rand = _rand + dave_os_time_ns();
	}

	out_put_rand = _rand;

	return out_put_rand;
}

