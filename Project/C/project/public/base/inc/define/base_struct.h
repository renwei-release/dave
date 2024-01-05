/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_STRUCT_H__
#define __BASE_STRUCT_H__
#include "base_macro.h"

typedef struct {
	/* next mbuf in singly linked mbuf chain */
	void *next;

	/* pointer to the actual data in the buffer */
	void *payload;
  
	/*
	 * total length of this buffer and all next buffers in chain
	 * belonging to the same packet.
	 * For non-queue packet chains this is the invariant:
	 * p->tot_len == p->len + (p->next? p->next->tot_len: 0)
	 */
	sb tot_len;
	/* length of this buffer */
	sb len;
	sb ref;
	sb alloc_len;
} MBUF;

typedef struct {
	u16 year;
	u8 month;
	u8 day;
	u8 hour;
	u8 minute;
	u8 second;
	u8 week;
	s8 zone;
} DateStruct;

typedef struct {
	IPVER ver;
	u8 ip_addr[16];
} SocNetInfoIp;

typedef struct {
	SocNetInfoIp ip;
	s8 url[DAVE_URL_LEN];
} SocNetInfoAddr;

typedef struct {
	SOCDOMAIN domain;
	SOCTYPE type;
	NetAddrType addr_type;
	SocNetInfoAddr addr;
	u16 port;
	FixedPortFlag fixed_src_flag;
	SocNetInfoIp src_ip;
	u16 src_port;
	EnableKeepAliveFlag enable_keepalive_flag;
	sb keepalive_second;
	EnableNetCardBindFlag netcard_bind_flag;
	s8 netcard_name[DAVE_NORMAL_NAME_LEN];
} SocNetInfo;

typedef struct {
	IPProtocol protocol;
	IPVER ver;
	u8 src_ip[16];
	u16 src_port;
	u8 dst_ip[16];
	u16 dst_port;
	sb keepalive_second;
	s8 netcard_name[DAVE_NORMAL_NAME_LEN];
	FixedPortFlag fixed_port_flag;
} IPBaseInfo;

typedef struct {
	EchoType type;

	s8 gid[DAVE_GLOBALLY_IDENTIFIER_LEN];
	s8 thread[DAVE_THREAD_NAME_LEN];

	ub echo_total_counter;
	ub echo_total_time;

	ub echo_cycle_counter;
	ub echo_cycle_time;

	ub echo_req_time;
	ub echo_rsp_time;

	dave_bool concurrent_flag;
	ub concurrent_tps_time;
	ub concurrent_tps_counter;
	ub concurrent_cycle_counter;
	ub concurrent_total_counter;

	s8 s8_echo;
	u8 u8_echo;
	s16 s16_echo;
	u16 u16_echo;
	s32 s32_echo;
	u32 u32_echo;
	s64 s64_echo;
	u64 u64_echo;
	float float_echo;
	double double_echo;
	void *void_echo;
	s8 string_echo[256];
	MBUF *mbuf_echo;
} MsgIdEcho;

#endif

