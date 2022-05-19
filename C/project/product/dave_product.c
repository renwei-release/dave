/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

void dave_product_init(void);
void dave_product_exit(void);

// =====================================================================

void
product_init(void)
{
	dave_product_init();
}

void
product_exit(void)
{
	dave_product_exit();
}

