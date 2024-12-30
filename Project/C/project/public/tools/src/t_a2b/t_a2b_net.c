/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "dave_base.h"
#include "dave_tools.h"
#include "dave_os.h"
#include "tools_log.h"

static s8 *
_t_a2b_net_ip_to_str(s8 *str_ptr, ub str_len, u8 *ip_ptr, ub ip_len, u16 port)
{
	ub str_index, ip_index;

	if(NULL == ip_ptr)
	{
		dave_snprintf(str_ptr, str_len, "this ip is NULL");
	}

	dave_memset(str_ptr, 0x00, str_len);

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
	if(str_len == 0)
	{
		str_len = dave_strlen(str_ptr);
	}

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
				ip_ptr[ip_index ++] = (u8)stringdigital(temp_str);
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
			ip_ptr[ip_index ++] = (u8)stringdigital(temp_str);
		}
	}

	return ip_index;
}

static dave_bool
_t_a2b_net_domain_to_ip_and_port(s8 *domain_ptr, ub domain_len, u16 *port, s8 *domain)
{
	#define MAX_DOMAIN_STRING 2048
	ub index, safe_counter;
	s8 port_str[32];
	ub domain_index, port_index;

	dave_memset(domain_ptr, 0x00, domain_len);
	*port = 0;

	index = safe_counter = 0;

	domain_index = 0;

	while((domain[index] != '\0') && (domain_index < domain_len) && ((++ safe_counter) < MAX_DOMAIN_STRING))
	{
		if(domain[index] == '/')
		{
			domain_index = 0;
		}
		else if(domain[index] == ':')
		{
			index ++;
			break;
		}
		else
		{
			domain_ptr[domain_index ++] = domain[index];
		}

		index ++;		
	}

	if(domain_index == 0)
	{
		TOOLSLOG("From inside <%s> I can't find the domain!", domain);
		return dave_false;
	}

	dave_memset(port_str, 0x00, sizeof(port_str));
	port_index = 0;

	while((domain[index] != '\0') && ((++ safe_counter) < MAX_DOMAIN_STRING))
	{
		if(t_is_digit(domain[index]) == dave_false)
		{
			break;
		}
		else
		{
			port_str[port_index ++] = domain[index];
		}

		index ++;		
	}

	if(port_index > 0)
	{
		*port = stringdigital(port_str);
		if(*port == 0)
		{
			return dave_false;
		}
	}

	return dave_true;
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

dave_bool
t_a2b_net_domain_to_ip_and_port(u8 ipv4[DAVE_IP_V4_ADDR_LEN], u16 *port, s8 *domain)
{
	s8 domain_str[1024];
	s8 ip_str[32];

	dave_memset(ipv4, 0x00, DAVE_IP_V4_ADDR_LEN);
	*port = 0;

	if(_t_a2b_net_domain_to_ip_and_port(domain_str, sizeof(domain_str), port, domain) == dave_false)
	{
		return dave_false;
	}

	if(t_is_ipv4(domain) == dave_true)
	{
		dave_strcpy(ip_str, domain, sizeof(ip_str));
	}
	else
	{
		if(dave_os_gethostbyname(ip_str, sizeof(ip_str), domain_str) == dave_false)
		{
			TOOLSLOG("can't get the host by domain:%s", domain_str);
			return dave_false;
		}
	}

	if(strip(ip_str, dave_strlen(ip_str), ipv4, DAVE_IP_V4_ADDR_LEN) != DAVE_IP_V4_ADDR_LEN)
	{
		TOOLSLOG("invalid ip string:%s", ip_str);
		return dave_false;
	}

	return dave_true;
}

