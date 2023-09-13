/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "echo_log.h"

#define HOW_MANY_CYCLES_DO_STATISTICS 500
#define CONCURRENCY_TPS 5000

static dave_bool _echo_working = dave_false;
static ub _echo_req_counter = 0;

static void
_echo_start(ThreadId src, ThreadId dst, dave_bool concurrent_flag)
{
	MsgIdEchoReq *pReq = thread_msg(pReq);

	if(_echo_working == dave_true)
	{
		ECHOLOG("The ECHO system is working!");
		return;
	}
	_echo_working = dave_true;

	ECHOLOG("%s->%s start %s echo ...",
		thread_name(src), thread_name(dst),
		concurrent_flag==dave_true?"concurrent":"single");

	pReq->echo.type = EchoType_single;

	dave_strcpy(pReq->echo.gid, t_gp_globally_identifier(), sizeof(pReq->echo.gid));
	dave_strcpy(pReq->echo.thread, thread_name(dst), sizeof(pReq->echo.thread));

	pReq->echo.echo_total_counter = 0;
	pReq->echo.echo_total_time = 0;

	pReq->echo.echo_cycle_counter = 0;
	pReq->echo.concurrent_cycle_counter = 0;
	pReq->echo.echo_cycle_time = 0;

	pReq->echo.echo_req_time = dave_os_time_us();
	pReq->echo.echo_rsp_time = 0;

	pReq->echo.concurrent_flag = concurrent_flag;
	pReq->echo.concurrent_tps_time = dave_os_time_us();
	pReq->echo.concurrent_tps_counter = 0;
	pReq->echo.concurrent_total_counter = 0;

	dave_snprintf(pReq->echo.msg, sizeof(pReq->echo.msg), "user start echo!");

	pReq->ptr = NULL;

	broadcast_remote(MSGID_ECHO_REQ, pReq);
}

static void
_echo_stop(ThreadId src, ThreadId dst)
{
	if(_echo_working == dave_false)
	{
		ECHOLOG("The ECHO system is not working!");
		return;
	}
	_echo_working = dave_false;

	ECHOLOG("%s->%s stop echo!",
		thread_name(src), thread_name(dst));
}

static void
_echo_api_req_msg(s8 *gid, s8 *thread, MsgIdEchoReq *pReq)
{
	switch(t_rand() % 6)
	{
		case 0:
				id_msg(thread_id(thread), MSGID_ECHO_REQ, pReq);
			break;
		case 1:
				id_qmsg(thread_id(thread), MSGID_ECHO_REQ, pReq);
			break;
		case 2:
				name_msg(thread, MSGID_ECHO_REQ, pReq);
			break;
		case 3:
				name_qmsg(thread, MSGID_ECHO_REQ, pReq);
			break;
		case 4:
				gid_msg(gid, thread, MSGID_ECHO_REQ, pReq);
			break;
		case 5:
				gid_qmsg(gid, thread, MSGID_ECHO_REQ, pReq);
			break;
		default:
				id_msg(thread_id(thread), MSGID_ECHO_REQ, pReq);
			break;
	}
}

static void
_echo_api_req_co(s8 *gid, s8 *thread, MsgIdEchoReq *pReq)
{
	if(pReq->echo.type != EchoType_random)
	{
		return _echo_api_req_msg(gid, thread, pReq);
	}

	switch(t_rand() % 6)
	{
		case 0:
				id_co(thread_id(thread), MSGID_ECHO_REQ, pReq, MSGID_ECHO_RSP);
			break;
		case 1:
				id_qco(thread_id(thread), MSGID_ECHO_REQ, pReq, MSGID_ECHO_RSP);
			break;
		case 2:
				name_co(thread, MSGID_ECHO_REQ, pReq, MSGID_ECHO_RSP);
			break;
		case 3:
				name_qco(thread, MSGID_ECHO_REQ, pReq, MSGID_ECHO_RSP);
			break;
		case 4:
				gid_co(gid, thread, MSGID_ECHO_REQ, pReq, MSGID_ECHO_RSP);
			break;
		case 5:
				gid_qco(gid, thread, MSGID_ECHO_REQ, pReq, MSGID_ECHO_RSP);
			break;
		default:
				id_co(thread_id(thread), MSGID_ECHO_REQ, pReq, MSGID_ECHO_RSP);
			break;
	}
}

static void
_echo_api_req(s8 *gid, s8 *thread, MsgIdEchoReq *pReq)
{
	if((pReq->echo.type == EchoType_random)
		&& (((_echo_req_counter ++) % 256) == 0)
		&& ((t_rand() % 16) != 0))
		_echo_api_req_co(gid, thread, pReq);
	else
		_echo_api_req_msg(gid, thread, pReq);
}

static void
_echo_api_rsp(ThreadId dst, MsgIdEchoRsp *pRsp)
{
	id_msg(dst, MSGID_ECHO_RSP, pRsp);
}

static void
_echo_snd_req(ThreadId src, ThreadId dst, EchoType type, MsgIdEcho *pGetEcho)
{
	MsgIdEchoReq *pReq = thread_msg(pReq);

	pReq->echo = *pGetEcho;

	pReq->echo.type = type;
	dave_strcpy(pReq->echo.gid, t_gp_globally_identifier(), sizeof(pReq->echo.gid));
	dave_strcpy(pReq->echo.thread, thread_name(dst), sizeof(pReq->echo.thread));

	pReq->echo.echo_req_time = dave_os_time_us();

	pReq->ptr = pReq;

	_echo_api_req(pGetEcho->gid, pGetEcho->thread, pReq);
}

static void
_echo_snd_rsp(ThreadId src, ThreadId dst, EchoType type, MsgIdEcho *pGetEcho, void *ptr)
{
	MsgIdEchoRsp *pRsp = thread_msg(pRsp);

	pRsp->echo = *pGetEcho;

	pRsp->echo.type = type;
	dave_strcpy(pRsp->echo.gid, t_gp_globally_identifier(), sizeof(pRsp->echo.gid));
	dave_strcpy(pRsp->echo.thread, thread_name(dst), sizeof(pRsp->echo.thread));

	pRsp->echo.echo_rsp_time = dave_os_time_us();

	pRsp->ptr = ptr;

	_echo_api_rsp(src, pRsp);
}

static void
_echo_random(ThreadId src, ThreadId dst, MsgIdEcho *pGetEcho, ub random_send_times)
{
	ub random_send_index;

	random_send_index = 0;

	while((random_send_index ++) < random_send_times)
	{
		_echo_snd_req(src, dst, EchoType_random, pGetEcho);
	}
}

static void
_echo_concurrent(ThreadId src, ThreadId dst, MsgIdEcho *pGetEcho)
{
	ub random_send_times;

	if(pGetEcho->concurrent_flag == dave_true)
	{
		random_send_times = t_rand() % 128;
		if(random_send_times == 0)
			random_send_times = 1;

		if((dave_os_time_us() - pGetEcho->concurrent_tps_time) >= 1000000)
		{
			pGetEcho->concurrent_tps_time = dave_os_time_us();
			pGetEcho->concurrent_tps_counter = 0;
		}

		if(pGetEcho->concurrent_tps_counter < CONCURRENCY_TPS)
		{
			if((pGetEcho->concurrent_tps_counter + random_send_times) > CONCURRENCY_TPS)
			{
				random_send_times = CONCURRENCY_TPS - pGetEcho->concurrent_tps_counter;
				if(random_send_times > CONCURRENCY_TPS)
					random_send_times = 0;
			}

			pGetEcho->concurrent_cycle_counter += random_send_times;
			pGetEcho->concurrent_tps_counter += random_send_times;
			pGetEcho->concurrent_total_counter += random_send_times;

			_echo_random(src, dst, pGetEcho, random_send_times);
		}
	}
}

static void
_echo_single_req(ThreadId src, ThreadId dst, MsgIdEcho *pGetEcho, void *ptr)
{
	_echo_concurrent(src, dst, pGetEcho);

	_echo_snd_rsp(src, dst, EchoType_single, pGetEcho, ptr);
}

static void
_echo_single_rsp(ThreadId src, ThreadId dst, MsgIdEcho *pGetEcho)
{
	ub echo_consume_time = dave_os_time_us() - pGetEcho->echo_req_time;

	pGetEcho->echo_total_counter ++;
	pGetEcho->echo_total_time += echo_consume_time;

	pGetEcho->echo_cycle_counter ++;
	pGetEcho->echo_cycle_time += echo_consume_time;

	if((pGetEcho->echo_cycle_counter % HOW_MANY_CYCLES_DO_STATISTICS) == 0x00)
	{
		ECHOLOG("%s/%s C:%lds/%ld T:%lds/%ld %ldus/%ldus %ld",
			pGetEcho->gid, pGetEcho->thread,
			pGetEcho->echo_cycle_time/1000000, pGetEcho->echo_cycle_counter,
			pGetEcho->echo_total_time/1000000, pGetEcho->echo_total_counter,
			pGetEcho->echo_cycle_time/((pGetEcho->echo_cycle_counter*2)+(pGetEcho->concurrent_cycle_counter*2)),
			pGetEcho->echo_total_time/((pGetEcho->concurrent_total_counter*2)+(pGetEcho->echo_total_counter*2)),
			pGetEcho->concurrent_total_counter);

		pGetEcho->echo_cycle_counter = 0;
		pGetEcho->concurrent_cycle_counter = 0;
		pGetEcho->echo_cycle_time = 0;
	}

	if(_echo_working == dave_true)
	{
		_echo_concurrent(src, dst, pGetEcho);
	
		_echo_snd_req(src, dst, EchoType_single, pGetEcho);
	}
}

static void
_echo_random_req(ThreadId src, ThreadId dst, MsgIdEcho *pGetEcho, void *ptr)
{
	_echo_snd_rsp(src, dst, EchoType_random, pGetEcho, ptr);
}

static void
_echo_random_rsp(ThreadId src, ThreadId dst, MsgIdEcho *pGetEcho)
{
	// Don't do anything.
}

static void
_echo_req(ThreadId src, ThreadId dst, MsgIdEchoReq *pReq)
{
	MsgIdEcho *pEcho = &(pReq->echo);

	switch(pEcho->type)
	{
		case EchoType_start:
				_echo_start(src, dst, pEcho->concurrent_flag);
			break;
		case EchoType_stop:
				_echo_stop(src, dst);
			break;
		case EchoType_single:
				_echo_single_req(src, dst, pEcho, pReq->ptr);
			break;
		case EchoType_random:
				_echo_random_req(src, dst, pEcho, pReq->ptr);
			break;
		default:
				ECHOLOG("%s->%s invalid echo type:%d",
					thread_name(src), thread_name(dst), pEcho->type);
			break;
	}
}

static void
_echo_rsp(ThreadId src, ThreadId dst, MsgIdEchoRsp *pRsp)
{
	MsgIdEcho *pEcho = &(pRsp->echo);

	switch(pEcho->type)
	{
		case EchoType_single:
				_echo_single_rsp(src, dst, pEcho);
			break;
		case EchoType_random:
				_echo_random_rsp(src, dst, pEcho);
			break;
		default:
				ECHOLOG("%s->%s invalid echo type:%d",
					thread_name(src), thread_name(dst), pEcho->type);
			break;
	}
}

// =====================================================================

void
dave_echo(ThreadId src, ThreadId dst, ub msg_id, void *msg_body)
{
	if(msg_id == MSGID_ECHO_REQ)
	{
		_echo_req(src, dst, msg_body);
	}
	else if(msg_id == MSGID_ECHO_RSP)
	{
		_echo_rsp(src, dst, msg_body);
	}
	else
	{
		ECHOLOG("invlaid msg_id:%s", msgstr(msg_id));
	}
}

