/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __BASE_ENUM_H__
#define __BASE_ENUM_H__

typedef enum {
	TYPE_SOCK_STREAM = 0,
	TYPE_SOCK_DGRAM,
	TYPE_SOCK_RAW,
	TYPE_SOCK_SCTP,
	TYPE_SOCK_max,
	TYPE_SOCK_MAX = 0x1fffffff
} SOCTYPE;

typedef enum {
	NetAddrIPType = 0,
	NetAddrIPBroadcastType,
	NetAddrURLType = 0x12345678,
} NetAddrType;

typedef enum {
	FixedPort = 0x01234567,
	NotFixedPort = 0x09abcdef
} FixedPortFlag;

typedef enum {
	KeepAlive_enable = 0x01234567,
	KeepAlive_disable = 0x09abcdef
} EnableKeepAliveFlag;

typedef enum {
	NetCardBind_enable = 0x01234567,
	NetCardBind_disable = 0x09abcdef
} EnableNetCardBindFlag;

typedef enum {
	SOCKETINFO_BIND = 0,
	SOCKETINFO_BIND_OK,
	SOCKETINFO_BIND_FAIL,
	SOCKETINFO_CONNECT,
	SOCKETINFO_CONNECT_OK,
	SOCKETINFO_CONNECT_FAIL,
	SOCKETINFO_CONNECT_WAIT,
	SOCKETINFO_DISCONNECT,
	SOCKETINFO_DISCONNECT_OK,
	SOCKETINFO_DISCONNECT_FAIL,
	SOCKETINFO_DISCONNECT_WAIT,
	SOCKETINFO_CREAT,
	SOCKETINFO_WAIT_CREAT,
	SOCKETINFO_ACCEPT,
	SOCKETINFO_REV,
	SOCKETINFO_REV_MBUF,
	SOCKETINFO_SND,
	SOCKETINFO_CLOSE,
	SOCKETINFO_SILENCE,
	SOCKETINFO_DEVICE_CONNECT,
	SOCKETINFO_DEVICE_DISCONNECT,
	SOCKETINFO_LINK_LOST,
	SOCKETINFO_SEND_TIMER_OUT,
	SOCKETINFO_PORT_EXIST,
	SOCKETINFO_WRITE_THEN_CLOSE = 0x7857aaeb,
	SOCKETINFO_SND_URG,
	SOCKETINFO_RAW_EVENT_RECV_LENGTH,
	SOCKETINFO_MAX = 0x1fffffff
} SOCKETINFO;

typedef enum {
	SOC_EVENT_START = 0,
	SOC_EVENT_WAIT_CREAT,
	SOC_EVENT_CONNECT,
    SOC_EVENT_CONNECT_FAIL,
	SOC_EVENT_WAIT_CONNECT,
	SOC_EVENT_ACCEPT,
	SOC_EVENT_REV,
	SOC_EVENT_SND,
	SOC_EVENT_CLOSE,
	SOC_EVENT_SILENCE,
	SOC_EVENT_MAX
} SOCEVENT;

typedef enum {
	IPVER_IPV4 = 4,
	IPVER_IPV6 = 6,
	IPVER_MAX = 0x1fffffff
} IPVER;

typedef enum {
	DM_SOC_PF_INET = 0,
	DM_SOC_PF_INET6,
	DM_SOC_PF_UART,
	DM_SOC_PF_LOCAL_INET,
	DM_SOC_PF_LOCAL_INET6,
	DM_SOC_PF_RAW,
	DM_SOC_PF_RAW_INET,
	SOCDOMAIN_MAX = 0x1fffffff
} SOCDOMAIN;

typedef enum {
	IPProtocol_ICMP = 1,
	IPProtocol_TCP = 6,
	IPProtocol_UDP = 17,
	IPProtocol_GRE = 47,
	
	IPProtocol_MAX,
	IPProtocol_max = 0x1fffffff
} IPProtocol;

typedef enum {
	SOC_CNT_OK = 0,
	SOC_CNT_FAIL,
	SOC_CNT_WAIT,
	SOC_CNT_MAX
} SOCCNTTYPE;

typedef enum {
	CREAT_FLAG = 0x01,
	READ_FLAG = 0x02,
	WRITE_FLAG = 0x04,
	DIRECT_FLAG = 0x08,
	CREAT_READ_FLAG = 0x03,
	CREAT_WRITE_FLAG = 0x05,
	READ_WRITE_FLAG = 0x06,
	CREAT_READ_WRITE_FLAG = 0x07
} FileOptFlag;

typedef enum {
	TRACELEVEL_DEBUG = 0,
	TRACELEVEL_CATCHER,
	TRACELEVEL_TRACE,
	TRACELEVEL_LOG,
	TRACELEVEL_ABNORMAL,
	TRACELEVEL_ASSERT,
	TRACELEVEL_MAX
} TraceLevel;

typedef enum {
	BuildingBlocksOpt_none,
	BuildingBlocksOpt_inq,
	BuildingBlocksOpt_mount,
	BuildingBlocksOpt_decoupling,
	BuildingBlocksOpt_State_exchange,
	BuildingBlocksOpt_valve,
	BuildingBlocksOpt_max
} BuildingBlocksOpt;

#endif
