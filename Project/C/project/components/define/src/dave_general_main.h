/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_GENERAL_MAIN_H__
#define __DAVE_GENERAL_MAIN_H__

/* for MAINMSG_PYTHON_REQ message */
typedef struct {
	PythonFun fun;
	ub opt_param;
	s8 file_path[DAVE_PATH_LEN];
	MBUF *req_data;
	void *ptr;
} MainMsgPythonReq;

/* for MAINMSG_PYTHON_RSP message */
typedef struct {
	ErrCode ret;
	ub time;
	MBUF *rsp_data;
	void *ptr;
} MainMsgPythonRsp;

#endif

