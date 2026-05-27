/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __IMREAD_OPERATION_HPP__
#define __IMREAD_OPERATION_HPP__

extern "C" dave_bool __imread_matrix_malloc__(Matrix *pMatrix, s8 *pic_path, u8 *pic_data, ub pic_length, s8 *fun, ub line);

extern "C" void imread_matrix_free(Matrix *pMatrix);

#endif

