/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __TENSORFLOW_TOOLS_H__
#define __TENSORFLOW_TOOLS_H__
#include "dave_define.h"

dave_bool tensorflow_open(
	TF_Graph **ppGraph, TF_Session **ppSession,
	TF_Output *pInputOpt, TF_Output *pOutputOpt,
	const char *input_graph, const char *output_graph,
	const char *model_file,
	const char *device);

void tensorflow_close(TF_Graph *pGraph, TF_Session *pSession);

TF_Tensor * tensorflow_malloc_tensor(TF_DataType type, const int64_t *dims, int num_dims, MatrixData matrix, s8 *fun, ub line);
#define malloc_tensor(type, dims, matrix, fun, line) tensorflow_malloc_tensor(type, dims, sizeof(dims)/sizeof(int64_t), matrix, fun, line)

void tensorflow_free_tensor(TF_Tensor **ppTensor, ub tensor_num);
#define free_tensor(ppTensor, tensor_num) tensorflow_free_tensor(ppTensor, tensor_num)

TF_Output tensorflow_load_ops(TF_Graph *pGraph, const char *graph_name, ub index);

dave_bool __tensorflow_run__(
	MatrixData matrix,
	TF_Session *pSession,
	TF_Output *pInputOpt, ub ninputs,
	TF_Output *pOutputOpt, ub noutputs,
	TF_Tensor **output_tensors,
	ub *time,
	s8 *fun, ub line);
#define tensorflow_run(matrix, pSession, pInputOpt, ninputs, pOutputOpt, noutputs, output_tensors, time) __tensorflow_run__(matrix, pSession, pInputOpt, ninputs, pOutputOpt, noutputs, output_tensors, time, (s8 *)__func__, (ub)__LINE__)

#endif

