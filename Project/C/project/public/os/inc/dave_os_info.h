/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_OS_INFO_H__
#define __DAVE_OS_INFO_H__

RetCode dave_os_load_mac(u8 *mac);

RetCode dave_os_load_ip(u8 ip_v4[DAVE_IP_V4_ADDR_LEN], u8 ip_v6[DAVE_IP_V6_ADDR_LEN]);

RetCode dave_os_load_host_name(s8 *hostname, ub hostname_len);

ub dave_os_cpu_process_number(void);

ub dave_os_memory_use_percentage(void);

#endif

