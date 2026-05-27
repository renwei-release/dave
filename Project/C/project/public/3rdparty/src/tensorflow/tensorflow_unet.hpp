/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __TENSORFLOW_UNET_HPP__
#define __TENSORFLOW_UNET_HPP__

void tensorflow_unet_init(void);

void tensorflow_unet_exit(void);

Matrix __tensorflow_unet_predict__(u8 *pic_data, ub pic_length, s8 *fun, ub line);

#endif

