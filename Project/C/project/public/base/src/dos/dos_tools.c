/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "base_macro.h"
#if defined(__DAVE_BASE__)
#include "dave_base.h"
#include "dave_tools.h"
#include "dos_tools.h"
#include "dos_show.h"
#include "dos_log.h"

typedef enum {
	DosUserOpt_SET,
	DosUserOpt_GET,
	DosUserOpt_ERROR
} DosUserOpt;

static RetCode
__dos_resolution_u16(s8 *cmd, ub cmd_len, u16 *u16_value)
{
	ub input_index;
	s8 u16_value_string[64];
	
	if((cmd == NULL) || (cmd_len == 0))
		return RetCode_Invalid_parameter;

	dave_memset(u16_value_string, 0x00, 64);

	input_index = 0;

	dos_get_one_parameters(&cmd[input_index], cmd_len-input_index, u16_value_string, sizeof(u16_value_string));

	if(dave_strlen(u16_value_string) == 0)
		return RetCode_Invalid_parameter;

	*u16_value = stringdigital(u16_value_string);

	return RetCode_OK;
}

static DosUserOpt
_dos_u16_resolution(s8 *cmd_ptr, ub cmd_len, u16 *u16_value)
{
	sb param_index;
	DosUserOpt type;

	if(cmd_len < 2)
		return DosUserOpt_ERROR;

	param_index = 0;

	if(cmd_ptr[param_index] != '-')
		return DosUserOpt_ERROR;

	param_index ++;

	type = DosUserOpt_ERROR;

	switch(cmd_ptr[param_index])
	{
		case 's':
				param_index ++;
				if(__dos_resolution_u16(&cmd_ptr[param_index], cmd_len-param_index, u16_value) == RetCode_OK)
				{
					type = DosUserOpt_SET;
				}
			break;
		case 'g':
				param_index ++;
				type = DosUserOpt_GET;
			break;
		default:
			break;
	}

	return type;
}

static RetCode
__dos_resolution_ip_and_port(s8 *cmd_ptr, ub cmd_len, u8 ip_v4[DAVE_IP_V4_ADDR_LEN], u16 *port)
{
	ub input_index;
	s8 ip_string[64], port_string[64];

	if((cmd_ptr == NULL) || (cmd_len == 0))
	{
		return RetCode_Invalid_parameter;
	}

	dave_memset(ip_string, 0x00, sizeof(ip_string));
	dave_memset(port_string, 0x00, sizeof(port_string));

	input_index = 0;

	input_index += dos_get_one_parameters(&cmd_ptr[input_index], cmd_len-input_index, ip_string, sizeof(ip_string));
	dos_get_one_parameters(&cmd_ptr[input_index], cmd_len-input_index, port_string, sizeof(port_string));

	if((dave_strlen(ip_string) == 0)
		|| (t_is_ipv4(ip_string) == dave_false))
	{
		return RetCode_Invalid_parameter;
	}

	t_a2b_net_str_to_ip(ip_string, dave_strlen(ip_string), ip_v4, DAVE_IP_V4_ADDR_LEN);
	*port = stringdigital(port_string);

	return RetCode_OK;
}

static DosUserOpt
_dos_ip_port_resolution(s8 *cmd_ptr, ub cmd_len, u8 ip_v4[DAVE_IP_V4_ADDR_LEN], u16 *port)
{
	sb param_index;
	DosUserOpt type;

	if(cmd_len < 2)
		return DosUserOpt_ERROR;

	param_index = 0;

	if(cmd_ptr[param_index] != '-')
		return DosUserOpt_ERROR;

	param_index ++;

	type = DosUserOpt_ERROR;

	switch(cmd_ptr[param_index])
	{
		case 's':
				param_index ++;
				if(__dos_resolution_ip_and_port(&cmd_ptr[param_index], cmd_len-param_index, ip_v4, port) == RetCode_OK)
				{
					type = DosUserOpt_SET;
				}
			break;
		case 'g':
				param_index ++;
				type = DosUserOpt_GET;
			break;
		default:
			break;
	}

	return type;
}

static RetCode
__dos_ip_resolution(s8 *cmd_ptr, ub cmd_len, u8 ip_v4[DAVE_IP_V4_ADDR_LEN])
{
	ub input_index;
	s8 ip_string[64];

	if((cmd_ptr == NULL) || (cmd_len == 0))
		return RetCode_Invalid_parameter;

	dave_memset(ip_string, 0x00, sizeof(ip_string));

	input_index = 0;

	dos_get_one_parameters(&cmd_ptr[input_index], cmd_len-input_index, ip_string, sizeof(ip_string));

	if((dave_strlen(ip_string) == 0)
		|| (t_is_ipv4(ip_string) == dave_false))
	{
		return RetCode_Invalid_parameter;
	}

	t_a2b_net_str_to_ip(ip_string, dave_strlen(ip_string), ip_v4, DAVE_IP_V4_ADDR_LEN);

	return RetCode_OK;
}

static DosUserOpt
_dos_ip_resolution(s8 *cmd_ptr, ub cmd_len, u8 ip_v4[DAVE_IP_V4_ADDR_LEN])
{
	sb param_index;
	DosUserOpt type;

	if(cmd_len < 2)
		return DosUserOpt_ERROR;

	param_index = 0;

	if(cmd_ptr[param_index] != '-')
		return DosUserOpt_ERROR;

	param_index ++;

	type = DosUserOpt_ERROR;

	switch(cmd_ptr[param_index])
	{
		case 's':
				param_index ++;
				if(__dos_ip_resolution(&cmd_ptr[param_index], cmd_len-param_index, ip_v4) == RetCode_OK)
				{
					type = DosUserOpt_SET;
				}
			break;
		case 'g':
				param_index ++;
				type = DosUserOpt_GET;
			break;
		default:
			break;
	}

	return type;
}

// =====================================================================

ub
dos_get_last_parameters(s8 *cmd_ptr, ub cmd_len, s8 *param_ptr, ub param_len)
{
	ub cmd_index;

	if((cmd_ptr == NULL) || (cmd_len == 0) 
		|| (param_ptr == NULL) || (param_len == 0))
	{
		if((param_ptr != NULL) && (param_len > 0))
		{
			param_ptr[0] = '\0';
		}
		return 0;
	}

	cmd_index = 0;

	while((cmd_index < cmd_len) 
		&& ((t_is_show_char(cmd_ptr[cmd_index]) == dave_false) || (cmd_ptr[cmd_index] == ' ')))
	{
		cmd_index ++;
	}

	if(param_len > (cmd_len - cmd_index))
	{
		param_len = (cmd_len - cmd_index) + 1;
	}

	return dave_strcpy(param_ptr, &cmd_ptr[cmd_index], param_len);
}

ub
dos_get_one_parameters(s8 *cmd_ptr, ub cmd_len, s8 *param_ptr, ub param_len)
{
	ub cmd_index, param_index;

	if((cmd_ptr == NULL) || (cmd_len == 0) 
		|| (param_ptr == NULL) || (param_len == 0))
	{
		if((param_ptr != NULL) && (param_len > 0))
		{
			param_ptr[0] = '\0';
		}
		return 0;
	}

	cmd_index = 0;

	while((cmd_index < cmd_len) 
		&& ((t_is_show_char(cmd_ptr[cmd_index]) == dave_false) || (cmd_ptr[cmd_index] == ' ')))
	{
		cmd_index ++;
	}

	param_index = 0;

	while((cmd_index < cmd_len) 
		&& (cmd_ptr[cmd_index] != ' ')
		&& ((param_index + 1) < param_len))
	{
		param_ptr[param_index ++] = cmd_ptr[cmd_index ++];
	}

	if((param_index + 1) >  param_len)
	{
		param_ptr[0] = '\0';
	}
	else
	{
		param_ptr[param_index] = '\0';
	}

	return cmd_index;
}

ub
dos_load_ub(s8 *cmd_ptr, ub cmd_len, ub *ub_data)
{
	ub cmd_index;
	s8 ub_data_str[64];

	if(ub_data != NULL)
	{
		*ub_data = 0;
	}

	cmd_index = 0;

	cmd_index += dos_get_one_parameters(&cmd_ptr[cmd_index], cmd_len-cmd_index, ub_data_str, sizeof(ub_data_str));

	if(ub_data_str[0] != '\0')
	{
		if(t_is_all_digit((u8 *)ub_data_str, dave_strlen(ub_data_str)) == dave_true)
		{
			if(ub_data != NULL)
			{
				*ub_data = stringdigital(ub_data_str);
			}
		}
	}

	return cmd_index;
}

ub
dos_load_bool(s8 *cmd_ptr, ub cmd_len, dave_bool *bool_value)
{
	s8 bool_value_str[32];
	ub param_len;

	param_len = dos_get_one_parameters(cmd_ptr, cmd_len, bool_value_str, sizeof(bool_value_str));

	if(dave_strcmp(bool_value_str, "true"))
		*bool_value = dave_true;
	else
		*bool_value = dave_false;

	return param_len;
}

ub
dos_load_string(s8 *cmd_ptr, ub cmd_len, s8 *str_ptr, ub str_len)
{
	return dos_get_one_parameters(cmd_ptr, cmd_len, str_ptr, str_len);
}

RetCode
dos_opt_u16_cfg(char *msg, s8 *cmd_ptr, ub cmd_len, s8 *u16_name)
{
	u16 u16_value;
	DosUserOpt type;
	s8 *out_msg;
	RetCode ret = RetCode_OK;

	out_msg = dave_malloc(1024);
	if(out_msg != NULL)
	{
		type = _dos_u16_resolution(cmd_ptr, cmd_len, &u16_value);

		switch(type)
		{
			case DosUserOpt_SET:
					if(cfg_set(u16_name, (u8 *)(&u16_value), sizeof(u16)) == RetCode_OK)
					{
						dave_sprintf(out_msg, "%s set config success:%d", msg, u16_value);
					}
					else
					{
						dave_sprintf(out_msg, "%s set failed:%d", msg, u16_value);
					}
				break;
			case DosUserOpt_GET:
					if(cfg_get(u16_name, (u8 *)(&u16_value), sizeof(u16)) == dave_true)
					{
						dave_sprintf(out_msg, "%s config infomation:%d", msg, u16_value);
					}
					else
					{
						dave_sprintf(out_msg, "%s get failed!", msg);
					}
				break;
			default:
					ret = RetCode_Invalid_parameter;
				break;
		}

		dos_print(out_msg);

		dave_free(out_msg);
	}

	return ret;
}

RetCode
dos_opt_ip_cfg(char *msg, s8 *cmd_ptr, ub cmd_len, s8 *server_name)
{
	u8 ip_v4[DAVE_IP_V4_ADDR_LEN];
	DosUserOpt type;
	s8 *out_msg;

	out_msg = dave_malloc(1024);
	if(out_msg != NULL)
	{
		type = _dos_ip_resolution(cmd_ptr, cmd_len, ip_v4);

		switch(type)
		{
			case DosUserOpt_SET:
					if(cfg_set(server_name, ip_v4, DAVE_IP_V4_ADDR_LEN) == RetCode_OK)
					{
						dave_sprintf(out_msg, "%s set server ip success:%d.%d.%d.%d",
							msg, ip_v4[0], ip_v4[1], ip_v4[2], ip_v4[3]);
					}
					else
					{
						dave_sprintf(out_msg, "%s set server ip failure:%d.%d.%d.%d",
							msg, ip_v4[0], ip_v4[1], ip_v4[2], ip_v4[3]);
					}
				break;
			case DosUserOpt_GET:
					if(cfg_get(server_name, ip_v4, DAVE_IP_V4_ADDR_LEN) == dave_true)
					{
						dave_sprintf(out_msg, "%s server ip:%d.%d.%d.%d",
							msg, ip_v4[0], ip_v4[1], ip_v4[2], ip_v4[3]);
					}
					else
					{
						dave_sprintf(out_msg, "%s get server ip failure!", msg);
					}
				break;
			default:
					dave_sprintf(out_msg, "invalid format!");
				break;
		}

		dos_print(out_msg);

		dave_free(out_msg);
	}

	return RetCode_OK;
}

RetCode
dos_opt_ip_port_cfg(char *msg, s8 *cmd_ptr, ub cmd_len, s8 *server_name, s8 *port_name)
{
	u8 ip_v4[DAVE_IP_V4_ADDR_LEN];
	u16 port;
	DosUserOpt type;
	s8 *out_msg;

	out_msg = dave_malloc(1024);
	if(out_msg != NULL)
	{
		type = _dos_ip_port_resolution(cmd_ptr, cmd_len, ip_v4, &port);

		switch(type)
		{
			case DosUserOpt_SET:
					if(cfg_set(server_name, ip_v4, DAVE_IP_V4_ADDR_LEN) == RetCode_OK)
					{
						if(port == 0)
						{
							dave_sprintf(out_msg, "%s set config success:%s", msg, ipv4str(ip_v4, port));
						}
					}
					else
					{
						dave_sprintf(out_msg, "%s set server ip failed:%s", msg, ipv4str(ip_v4, port));
					}
					if(port != 0)
					{
						if(cfg_set(port_name, (u8 *)(&port), sizeof(port)) == RetCode_OK)
						{
							dave_sprintf(out_msg, "%s set config success:%s", msg, ipv4str(ip_v4, port));
						}
						else
						{
							dave_sprintf(out_msg, "%s set server port failed:%s", msg, ipv4str(ip_v4, port));
						}
					}
				break;
			case DosUserOpt_GET:
					if(cfg_get(server_name, ip_v4, DAVE_IP_V4_ADDR_LEN) == dave_true)
					{
						if(cfg_get(port_name, (u8 *)(&port), sizeof(port)) == dave_true)
						{
							dave_sprintf(out_msg, "%s config infomation:%s", msg, ipv4str(ip_v4, port));
						}
						else
						{
							dave_sprintf(out_msg, "%s get server port failed!", msg);
						}
					}
					else
					{
						dave_sprintf(out_msg, "%s get server ip failed!", msg);
					}
				break;
			default:
					dave_sprintf(out_msg, "invalid format!");
				break;
		}

		dos_print(out_msg);

		dave_free(out_msg);
	}

	return RetCode_OK;
}

#endif

