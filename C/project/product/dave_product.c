/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "product_macro.h"

void dave_product_init(void);
void dave_product_exit(void);

// =====================================================================

void
product_init(void)
{
#ifndef __DAVE_PRODUCT_NULL__
	dave_product_init();
#endif
}

void
product_exit(void)
{
#ifndef __DAVE_PRODUCT_NULL__
	dave_product_exit();
#endif
}

