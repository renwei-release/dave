/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifdef __DAVE_PRODUCT_STORE__
#include "dave_tools.h"
#include "dave_3rdparty.h"
#include "dave_verno.h"
#include "dave_base.h"
#include "store_msg.h"

// =====================================================================

void
store_mysql_init(void)
{
	dave_mysql_init();
}

void
store_mysql_exit(void)
{
	dave_mysql_exit();
}

#endif

