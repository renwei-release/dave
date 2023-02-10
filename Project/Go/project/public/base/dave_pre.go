//go:build !__DAVE_PRODUCT_API__
/*
 * API products do not need pre-initialization
 */

package base

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

 var pre_init = Dave_go_system_pre_init()