/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __T_A2B_H__
#define __T_A2B_H__

#define dave_byte_8_32(d, a0, a1, a2, a3) {u32 t; t=((((u32)(a0))<<24)&0xff000000); t+=((((u32)(a1))<<16)&0xff0000); t+=((((u32)(a2))<<8)&0xff00); t+=(((u32)(a3))&0xff); (d)=t;}
#define dave_byte_32_8(a0, a1, a2, a3, d) {u32 t; t=d; (a0)=(u8)((t)>>24); (a1)=(u8)((t)>>16); (a2)=(u8)((t)>>8); (a3)=(u8)(t);}
#define dave_byte_16(d, a0, a1) {u16 t; t=((((u16)(a0))<<8)&0xff00); t+=(((u16)(a1))&0xff); (d)=t;}
#define dave_byte_8(a0, a1, d) {u16 t; t=d; (a0)=(u8)((t)>>8); (a1)=(u8)(t);}

MBUF * __t_a2b_bin_to_mbuf__(s8 *bin_ptr, ub bin_len, s8 *fun, ub line);
#define t_a2b_bin_to_mbuf(bin_ptr, bin_len) __t_a2b_bin_to_mbuf__(bin_ptr, bin_len, (s8 *)__func__, (ub)__LINE__)
MBUF * __t_a2b_str_to_mbuf__(s8 *str_ptr, sb str_len, s8 *fun, ub line);
#define t_a2b_str_to_mbuf(str_ptr, str_len) __t_a2b_str_to_mbuf__(str_ptr, str_len, (s8 *)__func__, (ub)__LINE__)
MBUF * t_a2b_param_to_mbuf(const char *args, ...);
ub t_a2b_mbuf_to_buf(u8 *buf_ptr, ub buf_len, MBUF *m);
void * t_a2b_mbuf_to_json(MBUF *m);

ub t_a2b_digital_to_string(s8 *str_ptr, ub str_len, ub digital);
ub t_a2b_string_to_digital(s8 *str_ptr);
ub t_a2b_stringhex_to_digital(s8 *str_ptr);
ub t_a2b_bin_to_hex_string(s8 *buf_ptr, ub buf_len, u8 *bin_ptr, ub bin_len);
double t_a2b_string_to_double(s8 *string);
ub t_a2b_double_to_string(s8 *string_ptr, ub string_len, double double_data);

s8 * t_a2b_net_ipv4_to_str(u8 *ip, u16 port);
s8 * t_a2b_net_ipv4_to_str_2(u8 *ip, u16 port);
s8 * t_a2b_net_ip_to_str(u8 *ip_ptr, ub ip_len, s8 *str_ptr, ub str_len);
ub t_a2b_net_str_to_ip(s8 *str_ptr, ub str_len, u8 *ip_ptr, ub ip_len);
s8 * t_a2b_net_mac_to_str(u8 *mac);
dave_bool t_a2b_net_domain_to_ip_and_port(u8 ipv4[DAVE_IP_V4_ADDR_LEN], u16 *port, s8 *domain);

s8 * t_a2b_date_str(DateStruct *pDate);
s8 * t_a2b_date_str_2(DateStruct *pDate);
s8 * t_a2b_date_str_3(DateStruct *pDate);
s8 * t_a2b_date_str_4(DateStruct *pDate);
s8 * t_a2b_date_str_5(DateStruct *pDate);
s8 * t_a2b_date_str_6(DateStruct *pDate);

#define ipv4str t_a2b_net_ipv4_to_str
#define ipv4str2 t_a2b_net_ipv4_to_str_2
#define datestr t_a2b_date_str
#define datestr2 t_a2b_date_str_2
#define ipstr t_a2b_net_ip_to_str
#define strip t_a2b_net_str_to_ip
#define macstr t_a2b_net_mac_to_str
#define domainip t_a2b_net_domain_to_ip_and_port
#define digitalstring t_a2b_digital_to_string
#define stringdigital t_a2b_string_to_digital
#define stringhexdigital t_a2b_stringhex_to_digital
#define stringdouble t_a2b_string_to_double
#define doublestring t_a2b_double_to_string

#endif

