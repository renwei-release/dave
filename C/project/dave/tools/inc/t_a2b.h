/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __T_A2B_H__
#define __T_A2B_H__

MBUF * t_a2b_str_to_mbuf(s8 *str);
MBUF * t_a2b_param_to_mbuf(const char *args, ...);
ub t_a2b_mbuf_to_buf(MBUF *m, u8 *buf, ub buf_len);
ub t_a2b_mbufs_to_mbuf(MBUF **dst, MBUF *src);

ub t_a2b_digital_to_string(s8 *str_ptr, ub str_len, ub digital);
ub t_a2b_string_to_digital(s8 *str_ptr);
ub t_a2b_bin_to_hex_string(s8 *buf_ptr, ub buf_len, u8 *bin_ptr, ub bin_len);

s8 * t_a2b_errorstr(ErrCode code);

s8 * t_a2b_net_ipv4_to_str(u8 *ip, u16 port);
s8 * t_a2b_net_ipv4_to_str_2(u8 *ip, u16 port);
s8 * t_a2b_net_ip_to_str(u8 *ip_ptr, ub ip_len, s8 *str_ptr, ub str_len);
ub t_a2b_net_str_to_ip(s8 *str_ptr, ub str_len, u8 *ip_ptr, ub ip_len);
s8 * t_a2b_net_mac_to_str(u8 *mac);

s8 * t_a2b_date_str(DateStruct *pDate);
s8 * t_a2b_date_str_2(DateStruct *pDate);
s8 * t_a2b_date_str_3(DateStruct *pDate);
s8 * t_a2b_date_str_4(DateStruct *pDate);
s8 * t_a2b_date_str_5(DateStruct *pDate);
s8 * t_a2b_date_str_6(DateStruct *pDate);

#define errorstr t_a2b_errorstr
#define ipv4str t_a2b_net_ipv4_to_str
#define ipv4str2 t_a2b_net_ipv4_to_str_2
#define ipstr t_a2b_net_ip_to_str
#define strip t_a2b_net_str_to_ip
#define macstr t_a2b_net_mac_to_str


// $$$$$$$$$$$$$$$$$$interface compatible$$$$$$$$$$$$$$$$$$$$
#define format_ip_v4 t_a2b_net_ipv4_to_str
#define format_mac t_a2b_net_mac_to_str


#endif

