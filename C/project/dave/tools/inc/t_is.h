/*
 * ================================================================================
 * (c) Copyright 2022 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2022.03.09.
 * ================================================================================
 */

#ifndef __T_IS_H__
#define __T_IS_H__

dave_bool t_is_alpha(u8 c);
dave_bool t_is_digit(u8 c);
dave_bool t_is_digit_or_alpha(u8 c);
dave_bool t_is_all_digit(u8 *data, sb data_len);
dave_bool t_is_all_digit_or_alpha(u8 *data, sb data_len);
dave_bool t_is_separator(u8 c);
dave_bool t_is_show_char(u8 c);
dave_bool t_is_all_show_char(u8 *data, sb data_len);
dave_bool t_is_all_show_char_or_rn(u8 *data, sb data_len);
dave_bool t_is_legal_char(u8 c);
dave_bool t_is_empty_str(s8 *str);
dave_bool t_is_ipv4(s8 *ip_str);

#endif

