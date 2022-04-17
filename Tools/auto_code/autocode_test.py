# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from autocode_tools import *


def _autocode_test_1():
    data = 'typedef struct json_tokener json_tokener;\
\
/**\
 * Be strict when parsing JSON input.  Use caution with\
 * this flag as what is considered valid may become more\
 * restrictive from one release to the next, causing your\
 * code to fail on previously working input.\
 *\
 * Note that setting this will also effectively disable parsing\
 * of multiple json objects in a single character stream\
 * (e.g. {"foo":123}{"bar":234}); if you want to allow that\
 * also set JSON_TOKENER_ALLOW_TRAILING_CHARS\
 *\
 * This flag is not set by default.\
 *\
 * @see json_tokener_set_flags()\
 */'

    data = remove_annotation_data(data)
    print(f'{data}')
    return


def _autocode_test_2():
    data = '#ifndef __DAVE_STRUCT_H__ #define __DAVE_STRUCT_H__ #include "dave_macro.h" #include "dave_list.h"  typedef u64 ThreadId;  typedef void (*mbuf_external_free_fun)(void *param);  #define MBUF dave_mbuf  typedef struct {    void *next;     void *payload;       sb tot_len;    sb len;  sb ref;  sb alloc_len;      u8 external_param[16];   mbuf_external_free_fun external_free; } dave_mbuf;  typedef struct {  PRODUC product_type;  s8 product[64];  s8 misc[64];  u32 main_verno;  u32 sub_verno;  u32 rev_verno;  u16 year;  u8 month;  u8 day;  u8 hour;  u8 minute;  u8 second;  VERLEVEL level; } DAVEVERNOSTRUCT;  typedef struct {  u16 year;  u8 month;  u8 day;  u8 hour;  u8 minute;  u8 second;  u8 week; } DateStruct;  typedef struct {  IPVER ver;  u8 ip_addr[16]; } SocNetInfoIp;  typedef union {  SocNetInfoIp ip;  s8 url[DAVE_URL_LEN]; } SocNetInfoAddr;  typedef struct {  SOCDOMAIN domain;  SOCTYPE type;  NetAddrType addr_type;  SocNetInfoAddr addr;  u16 port;  FixedPortFlag fixed_src_flag;  SocNetInfoIp src_ip;  u16 src_port;  EnableKeepAliveFlag enable_keepalive_flag;  sb keepalive_second;  EnableNetCardBindFlag netcard_bind_flag;  s8 netcard_name[DAVE_NORMAL_NAME_LEN]; } SocNetInfo;  typedef struct {  DataTransportAttributeType type;  ub base_package_length; } DataTransportAttribute;  typedef struct {  u8 mac[DAVE_MAC_ADDR_LEN]; } MACBaseInfo;  typedef struct {  IPProtocol protocol;  IPVER ver;  u8 src_ip[16];  u16 src_port;  u8 dst_ip[16];  u16 dst_port;  sb keepalive_second;  s8 netcard_name[DAVE_NORMAL_NAME_LEN];  DataTransportAttribute transport_attribute;  FixedPortFlag fixed_port_flag; } IPBaseInfo;'
    data = get_struct_list(data)
    print(f'{data}')
    return


def _autocode_test_3():
    file_name = './test/autocode_test_3.h'
    with open(file_name, "r", encoding="utf-8") as file_id:
        file_content = file_id.read()
        data = remove_annotation_data(file_content)
        print(f'{data}')
    return


# =====================================================================


def autocode_test():
    _autocode_test_3()


if __name__ == '__main__':
    autocode_test()