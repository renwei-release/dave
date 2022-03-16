/*
 * ================================================================================
 * (c) Copyright 2022 Renwei All rights reserved.
 * --------------------------------------------------------------------------------
 * 2022.03.10.
 * ================================================================================
 */

#include "dave_base.h"
#include "dave_tools.h"

static s8 *
_t_a2b_net_ip_to_str(s8 *str_ptr, ub str_len, u8 *ip_ptr, ub ip_len, u16 port)
{
	ub str_index, ip_index;

	if(NULL == ip_ptr)
	{
		dave_snprintf(str_ptr, str_len, "this ip is NULL");
	}

	str_index = 0;

	for(ip_index=0; ip_index<ip_len; ip_index++)
	{
		if((ip_index + 1) < ip_len)
			str_index += dave_snprintf(&str_ptr[str_index], str_len-str_index, "%d.", ip_ptr[ip_index]);
		else
			str_index += dave_snprintf(&str_ptr[str_index], str_len-str_index, "%d", ip_ptr[ip_index]);
	}

	if(port != 0)
	{
		str_index += dave_snprintf(&str_ptr[str_index], str_len-str_index, ":%d", port);
	}

	return str_ptr;
}

static ub
_t_a2b_net_str_to_ip(u8 *ip_ptr, ub ip_len, s8 *str_ptr, ub str_len)
{
	ub str_index, ip_index;
	s8 temp_str[32];
	ub temp_index;

	dave_memset(ip_ptr, 0x00, ip_len);

	ip_index = 0;

	temp_index = 0;

    for(str_index=0; (str_index<str_len)&&(ip_index<ip_len)&&(temp_index<16); str_index++)
	{
		if(str_ptr[str_index] == '\0')
			break;

		temp_str[temp_index] = (s8)str_ptr[str_index];

		if(temp_str[temp_index] == '.')
		{
			temp_str[temp_index] = '\0';

			if(t_is_all_digit((u8 *)temp_str, dave_strlen(temp_str)) == dave_true)
			{
				ip_ptr[ip_index ++] = (u8)t_a2b_string_to_digital(temp_str);
			}

			temp_index = 0;
		}
		else
		{
			temp_index ++;
		}
	}

	temp_str[temp_index] = '\0';

	if(ip_index < ip_len)
	{
		if(t_is_all_digit((u8 *)temp_str, dave_strlen(temp_str)) == dave_true)
		{
			ip_ptr[ip_index ++] = (u8)t_a2b_string_to_digital(temp_str);
		}
	}

	return ip_index;
}

// =====================================================================

s8 *
t_a2b_net_ipv4_to_str(u8 *ip, u16 port)
{
	static s8 ip_str[DAVE_IP_V4_ADDR_LEN*3 + 16];

	return _t_a2b_net_ip_to_str(ip_str, sizeof(ip_str), ip, 4, port);
}

s8 *
t_a2b_net_ipv4_to_str_2(u8 *ip, u16 port)
{
	static s8 ip_str[DAVE_IP_V4_ADDR_LEN*3 + 16];

	return _t_a2b_net_ip_to_str(ip_str, sizeof(ip_str), ip, 4, port);
}

s8 *
t_a2b_net_ip_to_str(u8 *ip_ptr, ub ip_len, s8 *str_ptr, ub str_len)
{
	return _t_a2b_net_ip_to_str(str_ptr, str_len, ip_ptr, ip_len, 0);
}

ub
t_a2b_net_str_to_ip(s8 *str_ptr, ub str_len, u8 *ip_ptr, ub ip_len)
{
	return _t_a2b_net_str_to_ip(ip_ptr, ip_len, str_ptr, str_len);
}

s8 *
t_a2b_net_mac_to_str(u8 *mac)
{
	static s8 mac_str[DAVE_MAC_ADDR_LEN * 2 + 1];

	t_a2b_bin_to_hex_string(mac_str, sizeof(mac_str), mac, DAVE_MAC_ADDR_LEN);

	return mac_str;
}

