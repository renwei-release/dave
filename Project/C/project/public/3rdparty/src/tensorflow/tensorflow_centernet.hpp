/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __TENSORFLOW_CENTERNET_HPP__
#define __TENSORFLOW_CENTERNET_HPP__

void tensorflow_centernet_init(void);

void tensorflow_centernet_exit(void);

Matrix __tensorflow_centernet_predict__(u8 *pic_data, ub pic_length, s8 *fun, ub line);

#endif

