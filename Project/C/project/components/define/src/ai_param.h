/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __AI_PARAM_H__
#define __AI_PARAM_H__

typedef struct {
	s8 label[DAVE_LABEL_STR_MAX];
	float score;
	MCardContentType description_type;
	MBUF *description;
	MBUF *engine_answer;
} AIReportInfo;

#endif

