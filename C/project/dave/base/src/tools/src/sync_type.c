/*
 * ================================================================================
 * (c) Copyright 2021 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2021.10.30.
 * ================================================================================
 */
#include "base_macro.h"
#ifdef __DAVE_BASE__
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_verno.h"
#include "sync_type.h"

static SYNCType
_sync_type(void)
{
	s8 product_type[128];

	dave_verno_product(dave_verno(), product_type, sizeof(product_type));

	if(dave_strcmp(product_type, "SYNC") == dave_true)
	{
		return SYNC_SERVER;
	}
	else if(dave_strcmp(product_type, "LOG") == dave_true)
	{
		return SYNC_MAX;
	}
	else
	{
		return SYNC_CLIENT;
	}
}

// =====================================================================

SYNCType
T_sync_type(void)
{
	return _sync_type();
}

#endif

