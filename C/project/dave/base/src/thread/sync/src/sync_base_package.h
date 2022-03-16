/*
 * ================================================================================
 * (c) Copyright 2017 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2017.04.26.
 * The synchronization service in the future may try to connect to all within
 * the framework of the server.
 * ================================================================================
 */

#ifndef __SYNC_BASE_PACKAGE_H__
#define __SYNC_BASE_PACKAGE_H__
#include "base_macro.h"

ub sync_str_packet(u8 *frame, ub frame_len, s8 *str);

ub sync_str_unpacket(u8 *frame, ub frame_len, s8 *str, ub str_len);

MBUF * sync_heartbeat_packet(ub recv_data_counter, ub send_data_counter);

ub sync_heartbeat_unpacket(u8 *frame, ub frame_len, ub *recv_data_counter, ub *send_data_counter);

MBUF * sync_thread_name_packet(s8 *verno, s8 *globally_identifier, s8 *thread_name, sb thread_index);

ub sync_thread_name_unpacket(u8 *frame, ub frame_len, s8 *verno, s8 *globally_identifier, s8 *thread_name, sb *thread_index);

ub sync_msg_packet(
	u8 *frame, ub frame_len,
	ThreadId route_src, ThreadId route_dst, s8 *src, s8 *dst, ub msg_id,
	BaseMsgType msg_type, TaskAttribute src_attrib, TaskAttribute dst_attrib,
	ub msg_len, void *msg_body);

ub sync_msg_unpacket(
	u8 *frame, ub frame_len,
	ThreadId *route_src, ThreadId *route_dst, s8 *src, s8 *dst, ub *msg_id,
	BaseMsgType *msg_type, TaskAttribute *src_attrib, TaskAttribute *dst_attrib,
	ub *msg_len, u8 **msg_body);

ub sync_msg_unpacket_msg_id(u8 *frame, ub frame_len);

MBUF * sync_link_packet(
	s8 *verno,
	u8 ip[16], u16 port,
	s8 *globally_identifier);

ub sync_link_unpacket(
	u8 *frame, ub frame_len,
	s8 *verno, ub verno_len,
	u8 ip[16], u16 *port,
	s8 *globally_identifier, ub globally_identifier_len);

ub sync_ip_packet(
	u8 *frame, ub frame_len,
	u8 ip[16]);

ub sync_ip_unpacket(
	u8 *frame, ub frame_len,
	u8 ip[16]);

MBUF * sync_rpcver_packet(sb rpcver);

ub sync_rpcver_unpacket(
	u8 *frame, ub frame_len,
	sb *rpcver);

#endif

