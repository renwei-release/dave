/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#include "thread_sync.h"
#if defined(SYNC_STACK_CLIENT) || defined(SYNC_STACK_SERVER)
#include "dave_tools.h"
#include "sync_param.h"
#include "sync_log.h"

typedef enum {
	SyncDataType_ub = 0,
	SyncDataType_str,
	SyncDataType_bin,
	SyncDataType_date,
} SyncDataType;

#define byte_to_u32(d) {u32 t; t=((((u32)(frame[frame_index++]))<<24)&0xff000000); t+=((((u32)(frame[frame_index++]))<<16)&0xff0000); t+=((((u32)(frame[frame_index++]))<<8)&0xff00); t+=(((u32)(frame[frame_index++]))&0xff); (d)=t;}
#define u32_to_byte(d) {u32 t; t=(u32)d; (frame[frame_index++])=(u8)((t)>>24); (frame[frame_index++])=(u8)((t)>>16); (frame[frame_index++])=(u8)((t)>>8); (frame[frame_index++])=(u8)(t);}
#define byte_to_sb(d) {s64 t; t=((((s64)(msg[msg_index++]))<<56)&0xff00000000000000); t+=((((s64)(msg[msg_index++]))<<48)&0xff000000000000); t+=((((s64)(msg[msg_index++]))<<40)&0xff0000000000); t+=((((s64)(msg[msg_index++]))<<32)&0xff00000000); t+=((((s64)(msg[msg_index++]))<<24)&0xff000000); t+=((((s64)(msg[msg_index++]))<<16)&0xff0000); t+=((((s64)(msg[msg_index++]))<<8)&0xff00); t+=(((s64)(msg[msg_index++]))&0xff); (d)=t;}
#define sb_to_byte(d) {s64 t; t=(sb)d; (msg[msg_index++])=(u8)((t)>>56); (msg[msg_index++])=(u8)((t)>>48); (msg[msg_index++])=(u8)((t)>>40); (msg[msg_index++])=(u8)((t)>>32); (msg[msg_index++])=(u8)((t)>>24); (msg[msg_index++])=(u8)((t)>>16); (msg[msg_index++])=(u8)((t)>>8); (msg[msg_index++])=(u8)(t);}
#define byte_to_ub(d) {u64 t; t=((((u64)(msg[msg_index++]))<<56)&0xff00000000000000); t+=((((u64)(msg[msg_index++]))<<48)&0xff000000000000); t+=((((u64)(msg[msg_index++]))<<40)&0xff0000000000); t+=((((u64)(msg[msg_index++]))<<32)&0xff00000000); t+=((((u64)(msg[msg_index++]))<<24)&0xff000000); t+=((((u64)(msg[msg_index++]))<<16)&0xff0000); t+=((((u64)(msg[msg_index++]))<<8)&0xff00); t+=(((u64)(msg[msg_index++]))&0xff); (d)=t;}
#define ub_to_byte(d) {u64 t; t=(ub)d; (msg[msg_index++])=(u8)((t)>>56); (msg[msg_index++])=(u8)((t)>>48); (msg[msg_index++])=(u8)((t)>>40); (msg[msg_index++])=(u8)((t)>>32); (msg[msg_index++])=(u8)((t)>>24); (msg[msg_index++])=(u8)((t)>>16); (msg[msg_index++])=(u8)((t)>>8); (msg[msg_index++])=(u8)(t);}

static inline ub
_sync_ub_packet(u8 *msg, ub msg_len, ub ub_data)
{
	ub msg_index = 0;

	if(9 > msg_len)
	{
		SYNCABNOR("short msg_len:%d", msg_len);
		return msg_len;
	}

	msg[msg_index ++] = SyncDataType_ub;

	ub_to_byte(ub_data);

	return msg_index;
}

static inline ub
_sync_ub_unpacket(u8 *msg, ub msg_len, ub *ub_data)
{
	ub msg_index = 0;

	*ub_data = 0;

	if(msg_len < 9)
	{
		SYNCABNOR("short frame_len:%d", msg_len);
		return msg_len;
	}

	if(msg[msg_index] != SyncDataType_ub)
	{
		SYNCABNOR("%x not ub type data! msg_len:%d", msg[msg_index], msg_len);
		return msg_len;
	}

	msg_index ++;

	byte_to_ub(*ub_data);

	return msg_index;
}

static inline ub
_sync_str_packet(u8 *frame, ub frame_len, s8 *str)
{
	u32 str_len = (u32)dave_strlen(str);
	ub frame_index = 0;

	if((5 + str_len + 1) > frame_len)
	{
		SYNCABNOR("short frame_len:%d", frame_len);
		return frame_len;
	}

	frame[frame_index ++] = SyncDataType_str;

	u32_to_byte(str_len);

	dave_strcpy((s8 *)(&frame[frame_index]), str, str_len + 1);

	frame_index += str_len;

	return frame_index;
}

static inline ub
_sync_str_unpacket(u8 *frame, ub frame_len, s8 *str, ub str_len)
{
	u32 thread_name_len;
	ub frame_index = 0;

	str[0] = '\0';

	if(frame_len < 5)
	{
		SYNCABNOR("short frame_len:%d", frame_len);
		return frame_len;
	}

	if(frame[frame_index] != SyncDataType_str)
	{
		SYNCABNOR("%x not str type data!", frame[frame_index]);
		return frame_len;
	}

	frame_index ++;

	byte_to_u32(thread_name_len);

	if((frame_len < (5 + thread_name_len))
		|| (thread_name_len > (str_len - 1)))
	{
		SYNCABNOR("short len:%d,%d,%d", frame_len, thread_name_len, str_len);
		return frame_len;
	}

	if(thread_name_len > 0)
	{
		dave_strcpy(str, (s8 *)(&frame[frame_index]), thread_name_len + 1);
		frame_index += thread_name_len;
	}

	return frame_index;
}

static inline ub
_sync_bin_packet(u8 *frame, ub frame_len, ub bin_len, u8 *bin, s8 *src, s8 *dst, ub msg_id)
{
	ub frame_index = 0;
    u32 u32_bin_len = (u32)bin_len;
    
	if(5 > frame_len)
	{
		SYNCABNOR("short frame_len:%d<max>,%d<5+data> %s->%s %d",
			frame_len, bin_len, src, dst, msg_id);
		return frame_len;
	}

	frame[frame_index ++] = SyncDataType_bin;

	u32_to_byte(u32_bin_len);

	if((bin != NULL) && ((frame_index + bin_len) <= frame_len))
	{
		dave_memcpy(&frame[frame_index], bin, bin_len);
		frame_index += bin_len;
	}

	return frame_index;
}

static inline ub
_sync_bin_unpacket(u8 *frame, ub frame_len, u8 **bin, ub *bin_len)
{
	ub frame_index = 0;
	ub u32_bin_len;

	if(bin != NULL)
	{
		*bin = NULL;
	}
	*bin_len = 0;

	if(frame_len < 5)
	{
		SYNCABNOR("short frame_len:%d", frame_len);
		return frame_len;
	}

	if(frame[frame_index] != SyncDataType_bin)
	{
		SYNCABNOR("%x not bin type data!", frame[frame_index]);
		return frame_len;
	}

	frame_index ++;

	byte_to_u32(u32_bin_len);

	if((5 + u32_bin_len) < frame_len)
	{
		SYNCABNOR("invalid frame len:%d bin len:%d", frame_len, u32_bin_len);
		return frame_len;
	}

	if(bin != NULL)
	{
		*bin = (u8 *)(&frame[frame_index]);
	}
	*bin_len = (ub)u32_bin_len;

	return frame_index + u32_bin_len;
}

static inline ub
_sync_date_packet(u8 *msg, ub msg_len, DateStruct date_data)
{
	ub msg_index = 0;

	if(57 > msg_len)
	{
		SYNCABNOR("short msg_len:%d", msg_len);
		return msg_len;
	}

	msg[msg_index ++] = SyncDataType_date;

	ub_to_byte(date_data.year);
	ub_to_byte(date_data.month);
	ub_to_byte(date_data.day);
	ub_to_byte(date_data.hour);
	ub_to_byte(date_data.minute);
	ub_to_byte(date_data.second);
	sb_to_byte(date_data.zone);

	return msg_index;
}

static inline ub
_sync_date_unpacket(u8 *msg, ub msg_len, DateStruct *date_data)
{
	DateStruct unpacket_date;
	ub msg_index = 0;

	if(date_data != NULL)
		dave_memset(date_data, 0x00, sizeof(DateStruct));

	if(msg_len < 57)
	{
		SYNCABNOR("short frame_len:%d", msg_len);
		return msg_len;
	}

	if(msg[msg_index] != SyncDataType_date)
	{
		SYNCABNOR("%x not ub type data! msg_len:%d", msg[msg_index], msg_len);
		return msg_len;
	}

	msg_index ++;

	byte_to_ub(unpacket_date.year);
	byte_to_ub(unpacket_date.month);
	byte_to_ub(unpacket_date.day);
	byte_to_ub(unpacket_date.hour);
	byte_to_ub(unpacket_date.minute);
	byte_to_ub(unpacket_date.second);
	byte_to_sb(unpacket_date.zone);

	if(date_data != NULL)
		*date_data = unpacket_date;

	return msg_index;
}

static inline ub
_sync_msg_packet_msg_id_up(
	u8 *frame, ub frame_len,
	ThreadId route_src, ThreadId route_dst, s8 *src, s8 *dst, ub msg_id)
{
	ub ub_route_src, ub_route_dst, ub_msg_id;
	ub frame_index = 0;

	ub_route_src = (ub)route_src;
	ub_route_dst = (ub)route_dst;
	ub_msg_id = (ub)msg_id;

	frame_index += _sync_ub_packet(&frame[frame_index], frame_len-frame_index, ub_route_src);
	frame_index += _sync_ub_packet(&frame[frame_index], frame_len-frame_index, ub_route_dst);
	frame_index += _sync_str_packet(&frame[frame_index], frame_len-frame_index, src);
	frame_index += _sync_str_packet(&frame[frame_index], frame_len-frame_index, dst);
	frame_index += _sync_ub_packet(&frame[frame_index], frame_len-frame_index, ub_msg_id);

	return frame_index;
}

static inline ub
_sync_msg_unpacket_msg_id_up(
	u8 *frame, ub frame_len,
	ThreadId *route_src, ThreadId *route_dst, s8 *src, s8 *dst, ub *msg_id)
{
	ub frame_index = 0;
	ub ub_route_src, ub_route_dst, ub_msg_id;
	s8 s8_src[SYNC_THREAD_NAME_LEN], s8_dst[SYNC_THREAD_NAME_LEN];

	frame_index += _sync_ub_unpacket(&frame[frame_index], frame_len-frame_index, &ub_route_src);
	frame_index += _sync_ub_unpacket(&frame[frame_index], frame_len-frame_index, &ub_route_dst);
	frame_index += _sync_str_unpacket(&frame[frame_index], frame_len-frame_index, s8_src, SYNC_THREAD_NAME_LEN);
	frame_index += _sync_str_unpacket(&frame[frame_index], frame_len-frame_index, s8_dst, SYNC_THREAD_NAME_LEN);
	frame_index += _sync_ub_unpacket(&frame[frame_index], frame_len-frame_index, &ub_msg_id);

	if(route_src != NULL)
	{
		*route_src = (ThreadId)ub_route_src;
	}
	if(route_dst != NULL)
	{
		*route_dst = (ThreadId)ub_route_dst;
	}
	if(src != NULL)
	{
		dave_strcpy(src, s8_src, SYNC_THREAD_NAME_LEN);
	}
	if(dst != NULL)
	{
		dave_strcpy(dst, s8_dst, SYNC_THREAD_NAME_LEN);
	}
	if(msg_id != NULL)
	{
		*msg_id = (ub)ub_msg_id;
	}

	return frame_index;
}

// =====================================================================

ub
sync_str_packet(u8 *frame, ub frame_len, s8 *str)
{
	return _sync_str_packet(frame, frame_len, str);
}

ub
sync_str_unpacket(u8 *frame, ub frame_len, s8 *str, ub str_len)
{
	return _sync_str_unpacket(frame, frame_len, str, str_len);
}

MBUF *
sync_heartbeat_packet(ub recv_data_counter, ub send_data_counter, DateStruct date)
{
	u8 *frame;
	ub frame_len = 2048;
	ub frame_index;
	MBUF *snd_buffer;

	snd_buffer = dave_mmalloc(frame_len);

	frame = dave_mptr(snd_buffer);

	frame_index = 0;
	frame_index += _sync_ub_packet(&frame[frame_index], frame_len-frame_index, recv_data_counter);
	frame_index += _sync_ub_packet(&frame[frame_index], frame_len-frame_index, send_data_counter);
	frame_index += _sync_date_packet(&frame[frame_index], frame_len-frame_index, date);

	snd_buffer->len = snd_buffer->tot_len = frame_index;

	return snd_buffer;
}

ub
sync_heartbeat_unpacket(u8 *frame, ub frame_len, ub *recv_data_counter, ub *send_data_counter, DateStruct *date)
{
	ub frame_index = 0;

	frame_index += _sync_ub_unpacket(&frame[frame_index], frame_len-frame_index, recv_data_counter);
	frame_index += _sync_ub_unpacket(&frame[frame_index], frame_len-frame_index, send_data_counter);
	if(frame_index < frame_len)
	{
		frame_index += _sync_date_unpacket(&frame[frame_index], frame_len-frame_index, date);
	}

	return frame_index;
}

MBUF *
sync_thread_name_packet(s8 *verno, s8 *globally_identifier, s8 *thread_name, sb thread_index)
{
	u8 *frame;
	ub frame_len = 2048;
	ub frame_index;
	MBUF *snd_buffer;

	snd_buffer = dave_mmalloc(frame_len);

	frame = dave_mptr(snd_buffer);

	frame_index = 0;
	frame_index += _sync_str_packet(&frame[frame_index], frame_len-frame_index, verno);
	frame_index += _sync_str_packet(&frame[frame_index], frame_len-frame_index, thread_name);
	frame_index += _sync_str_packet(&frame[frame_index], frame_len-frame_index, globally_identifier);
	frame_index += _sync_ub_packet(&frame[frame_index], frame_len-frame_index, (ub)thread_index);

	snd_buffer->len = snd_buffer->tot_len = frame_index;

	return snd_buffer;
}

ub
sync_thread_name_unpacket(u8 *frame, ub frame_len, s8 *verno, s8 *globally_identifier, s8 *thread_name, sb *thread_index)
{
	ub frame_index = 0;

	*thread_index  = -1;

	frame_index += _sync_str_unpacket(&frame[frame_index], frame_len-frame_index, verno, DAVE_VERNO_STR_LEN);
	frame_index += _sync_str_unpacket(&frame[frame_index], frame_len-frame_index, thread_name, SYNC_THREAD_NAME_LEN);
	if(frame_index < frame_len)
	{
		frame_index += _sync_str_unpacket(&frame[frame_index], frame_len-frame_index, globally_identifier, DAVE_GLOBALLY_IDENTIFIER_LEN);	
	}
	if(frame_index < frame_len)
	{
		frame_index += _sync_ub_unpacket(&frame[frame_index], frame_len-frame_index, (ub *)thread_index);
	}

	return frame_index;
}

ub
sync_msg_packet(
	u8 *frame, ub frame_len,
	ThreadId route_src, ThreadId route_dst, s8 *src, s8 *dst, ub msg_id,
	BaseMsgType msg_type, TaskAttribute src_attrib, TaskAttribute dst_attrib,
	ub msg_len, void *msg_body)
{
	ub ub_msg_type, ub_src_attrib, ub_dst_attrib;
	ub frame_index = 0;

	SYNCDEBUG("src:%s dst:%s msg_id:%d msg_type:%d src_attrib:%d dst_attrib:%d",
		src, dst, msg_id, msg_type, src_attrib, dst_attrib);

	ub_msg_type = (ub)msg_type;
	ub_src_attrib = (ub)src_attrib;
	ub_dst_attrib = (ub)dst_attrib;

	frame_index += _sync_msg_packet_msg_id_up(
		&frame[frame_index], frame_len-frame_index,
		route_src, route_dst, src, dst, msg_id);
	frame_index += _sync_ub_packet(&frame[frame_index], frame_len-frame_index, ub_msg_type);
	frame_index += _sync_ub_packet(&frame[frame_index], frame_len-frame_index, ub_src_attrib);
	frame_index += _sync_ub_packet(&frame[frame_index], frame_len-frame_index, ub_dst_attrib);
	frame_index += _sync_bin_packet(&frame[frame_index], frame_len-frame_index, msg_len, (u8 *)msg_body, src, dst, msg_id);

	return frame_index;	
}

ub
sync_msg_unpacket(
	u8 *frame, ub frame_len,
	ThreadId *route_src, ThreadId *route_dst, s8 *src, s8 *dst, ub *msg_id,
	BaseMsgType *msg_type, TaskAttribute *src_attrib, TaskAttribute *dst_attrib,
	ub *msg_len, u8 **msg_body)
{
	ub frame_index = 0;
	ub ub_msg_type, ub_src_attrib, ub_dst_attrib;

	frame_index += _sync_msg_unpacket_msg_id_up(
		&frame[frame_index], frame_len-frame_index,
		route_src, route_dst, src, dst, msg_id);
	frame_index += _sync_ub_unpacket(&frame[frame_index], frame_len-frame_index, &ub_msg_type);
	if(msg_type != NULL)
	{
		*msg_type = (BaseMsgType)ub_msg_type;
	}
	frame_index += _sync_ub_unpacket(&frame[frame_index], frame_len-frame_index, &ub_src_attrib);
	if(src_attrib != NULL)
	{
		*src_attrib = (TaskAttribute)ub_src_attrib;
	}
	frame_index += _sync_ub_unpacket(&frame[frame_index], frame_len-frame_index, &ub_dst_attrib);
	if(dst_attrib != NULL)
	{
		*dst_attrib = (TaskAttribute)ub_dst_attrib;
	}
	frame_index += _sync_bin_unpacket(&frame[frame_index], frame_len-frame_index, msg_body, msg_len);

	return frame_index;
}

ub
sync_msg_unpacket_msg_id(u8 *frame, ub frame_len)
{
	ub msg_id;

	_sync_msg_unpacket_msg_id_up(frame, frame_len, NULL, NULL, NULL, NULL, &msg_id);

	return msg_id;
}

MBUF *
sync_link_packet(
	s8 *verno,
	u8 ip[16], u16 port,
	s8 *globally_identifier)
{
	u8 *frame;
	ub frame_len = 2048;
	ub frame_index;
	MBUF *snd_buffer;
	ub ip_index;
	ub test_data = 0;

	snd_buffer = dave_mmalloc(frame_len);

	frame = dave_mptr(snd_buffer);

	frame_index = 0;
	frame_index += _sync_str_packet(&frame[frame_index], frame_len-frame_index, verno);
	for(ip_index=0; ip_index<16; ip_index++)
	{
		frame_index += _sync_ub_packet(&frame[frame_index], frame_len-frame_index, (ub)(ip[ip_index]));
	}
	frame_index += _sync_ub_packet(&frame[frame_index], frame_len-frame_index, (ub)(port));
	frame_index += _sync_ub_packet(&frame[frame_index], frame_len-frame_index, test_data);
	frame_index += _sync_str_packet(&frame[frame_index], frame_len-frame_index, globally_identifier);

	snd_buffer->len = snd_buffer->tot_len = frame_index;

	return snd_buffer;
}

ub
sync_link_unpacket(
	u8 *frame, ub frame_len,
	s8 *verno, ub verno_len,
	u8 ip[16], u16 *port,
	s8 *globally_identifier, ub globally_identifier_len)
{
	ub frame_index = 0;
	ub ip_index, ub_data, test_data;

	frame_index += _sync_str_unpacket(&frame[frame_index], frame_len-frame_index, verno, verno_len);
	for(ip_index=0; ip_index<16; ip_index++)
	{
		frame_index += _sync_ub_unpacket(&frame[frame_index], frame_len-frame_index, &ub_data);
		ip[ip_index] = (u8)ub_data;
	}
	frame_index += _sync_ub_unpacket(&frame[frame_index], frame_len-frame_index, &ub_data);
	if(port != NULL)
	{
		*port = (u16)ub_data;
	}
	frame_index += _sync_ub_unpacket(&frame[frame_index], frame_len-frame_index, &test_data);
	if(test_data != 0)
	{
		SYNCLOG("error test_data:%d", test_data);
	}
	if(frame_index < frame_len)
	{
		frame_index += _sync_str_unpacket(&frame[frame_index], frame_len-frame_index, globally_identifier, globally_identifier_len);
	}
	else
	{
		globally_identifier[0] = '\0';
	}

	return frame_index;
}

ub
sync_ip_packet(
	u8 *frame, ub frame_len,
	u8 ip[16])
{
	ub frame_index = 0;
	ub ip_index;

	for(ip_index=0; ip_index<16; ip_index++)
	{
		frame_index += _sync_ub_packet(&frame[frame_index], frame_len-frame_index, (ub)(ip[ip_index]));
	}

	return frame_index;
}

ub
sync_ip_unpacket(
	u8 *frame, ub frame_len,
	u8 ip[16])
{
	ub frame_index = 0;
	ub ip_index, ub_data;

	if(frame_len < 16 * sizeof(ub))
	{
		dave_memset(ip, 0x00, 16);
		return frame_len;
	}

	for(ip_index=0; ip_index<16; ip_index++)
	{
		frame_index += _sync_ub_unpacket(&frame[frame_index], frame_len-frame_index, &ub_data);
		ip[ip_index] = (u8)ub_data;
	}

	return frame_index;
}

MBUF *
sync_rpcver_packet(sb rpcver)
{
	u8 *frame;
	ub frame_len = 2048;
	ub frame_index;
	MBUF *snd_buffer;	

	snd_buffer = dave_mmalloc(frame_len);

	frame = dave_mptr(snd_buffer);

	frame_index = _sync_ub_packet(frame, frame_len, (ub)rpcver);

	snd_buffer->len = snd_buffer->tot_len = frame_index;

	return snd_buffer;
}

ub
sync_rpcver_unpacket(
	u8 *frame, ub frame_len,
	sb *rpcver)
{
	return _sync_ub_unpacket(frame, frame_len, (ub *)rpcver);
}

#endif

