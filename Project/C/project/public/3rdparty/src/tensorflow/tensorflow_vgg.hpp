/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __TENSORFLOW_VGG_H__
#define __TENSORFLOW_VGG_H__

void tensorflow_vgg_init(void);

void tensorflow_vgg_exit(void);

Matrix __tensorflow_vgg_feature__(u8 *pic_data, ub pic_length, s8 *fun, ub line);

#endif

