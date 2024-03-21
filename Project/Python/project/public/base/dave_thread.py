# -*- coding: utf-8 -*-
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
from ctypes import *
from .dave_dll import dave_dll
from .dave_log import *
from .dave_tools import *
from .dave_verno import *
from ..tools import *


davelib = dave_dll()


def thread_id(thread_name):
    if isinstance(thread_name, str) == True:
        thread_name = bytes(thread_name, encoding='utf8')

    davelib.dave_dll_thread_id.restype = c_int
    return davelib.dave_dll_thread_id(c_char_p(thread_name))


def thread_self():
    __func__, __LINE__ = t_sys_myline(2)

    self_id = davelib.base_thread_get_self(c_char_p(__func__), c_int(__LINE__))

    davelib.base_thread_get_name.restype = c_char_p
    return str(davelib.base_thread_get_name(c_longlong(self_id), c_char_p(__func__), c_int(__LINE__)), encoding = "utf-8")


def thread_msg(class_struct):
    __func__, __LINE__ = t_sys_myline(2)

    davelib.dave_dll_thread_msg.restype = POINTER(class_struct)
    return davelib.dave_dll_thread_msg(sizeof(class_struct), c_char_p(__func__), c_int(__LINE__))


def thread_msg_release(struct_pointer):
    __func__, __LINE__ = t_sys_myline(2)

    davelib.dave_dll_thread_msg_release(struct_pointer, c_char_p(__func__), c_int(__LINE__))
    return


def write_msg(dst, msg_id, class_instance):
    __func__, __LINE__ = t_sys_myline(2)

    if isinstance(dst, c_char) or isinstance(dst, c_char_p) or isinstance(dst, str) or isinstance(dst, bytes):
        if isinstance(dst, str) == True:
            dst = bytes(dst, encoding='utf8')
        davelib.dave_dll_thread_name_msg(c_char_p(dst), c_int(msg_id), c_int(sizeof(class_instance.contents)), class_instance, c_char_p(__func__), c_int(__LINE__))
    else:
        davelib.dave_dll_thread_id_msg(c_uint64(dst), c_int(msg_id), c_int(sizeof(class_instance.contents)), class_instance, c_char_p(__func__), c_int(__LINE__))
    return


def write_qmsg(dst, msg_id, class_instance):
    __func__, __LINE__ = t_sys_myline(2)

    if isinstance(dst, c_char) or isinstance(dst, c_char_p) or isinstance(dst, str) or isinstance(dst, bytes):
        if isinstance(dst, str) == True:
            dst = bytes(dst, encoding='utf8')
        davelib.dave_dll_thread_name_qmsg(c_char_p(dst), c_int(msg_id), c_int(sizeof(class_instance.contents)), class_instance, c_char_p(__func__), c_int(__LINE__))
    else:
        davelib.dave_dll_thread_id_qmsg(c_uint64(dst), c_int(msg_id), c_int(sizeof(class_instance.contents)), class_instance, c_char_p(__func__), c_int(__LINE__))
    return


def write_co(dst, req_id, pReq, rsp_id, rsp_class):
    __func__, __LINE__ = t_sys_myline(2)

    if isinstance(dst, c_char) or isinstance(dst, c_char_p) or isinstance(dst, str) or isinstance(dst, bytes):
        if isinstance(dst, str) == True:
            dst = bytes(dst, encoding='utf8')
        davelib.dave_dll_thread_name_co.restype = c_void_p
        pRsp = davelib.dave_dll_thread_name_co(c_char_p(dst), c_int(req_id), c_int(sizeof(pReq.contents)), pReq, c_int(rsp_id), c_char_p(__func__), c_int(__LINE__))
    else:
        davelib.dave_dll_thread_id_co.restype = c_void_p
        pRsp = davelib.dave_dll_thread_id_co(c_uint64(dst), c_int(req_id), c_int(sizeof(pReq.contents)), pReq, c_int(rsp_id), c_char_p(__func__), c_int(__LINE__))

    if pRsp == None:
        return None

    return struct_copy(rsp_class, pRsp, sizeof(rsp_class))


def write_qco(dst, req_id, pReq, rsp_id, rsp_class):
    __func__, __LINE__ = t_sys_myline(2)

    if isinstance(dst, c_char) or isinstance(dst, c_char_p) or isinstance(dst, str) or isinstance(dst, bytes):
        if isinstance(dst, str) == True:
            dst = bytes(dst, encoding='utf8')
        davelib.dave_dll_thread_name_co.restype = c_void_p
        pRsp = davelib.dave_dll_thread_name_qco(c_char_p(dst), c_int(req_id), c_int(sizeof(pReq.contents)), pReq, c_int(rsp_id), c_char_p(__func__), c_int(__LINE__))
    else:
        davelib.dave_dll_thread_id_co.restype = c_void_p
        pRsp = davelib.dave_dll_thread_id_qco(c_uint64(dst), c_int(req_id), c_int(sizeof(pReq.contents)), pReq, c_int(rsp_id), c_char_p(__func__), c_int(__LINE__))

    if pRsp == None:
        return None

    return struct_copy(rsp_class, pRsp, sizeof(rsp_class))


def gid_msg(gid, dst, msg_id, class_instance):
    __func__, __LINE__ = t_sys_myline(2)
    pRsp = davelib.dave_dll_thread_gid_msg(c_char_p(gid), c_char_p(dst), c_int(msg_id), c_int(sizeof(class_instance.contents)), class_instance, c_char_p(__func__), c_int(__LINE__))
    return    


def gid_qmsg(gid, dst, msg_id, class_instance):
    __func__, __LINE__ = t_sys_myline(2)
    davelib.dave_dll_thread_gid_qmsg(c_char_p(gid), c_char_p(dst), c_int(msg_id), c_int(sizeof(class_instance.contents)), class_instance, c_char_p(__func__), c_int(__LINE__))
    return


def gid_co(gid, dst, req_id, pReq, rsp_id, rsp_class):
    __func__, __LINE__ = t_sys_myline(2)
    davelib.dave_dll_thread_gid_co.restype = c_void_p
    pRsp = davelib.dave_dll_thread_gid_co(c_char_p(gid), c_char_p(dst), c_int(req_id), c_int(sizeof(pReq.contents)), pReq, c_int(rsp_id), c_char_p(__func__), c_int(__LINE__))
    return struct_copy(rsp_class, pRsp, sizeof(rsp_class))


def gid_qco(gid, dst, req_id, pReq, rsp_id, rsp_class):
    __func__, __LINE__ = t_sys_myline(2)
    davelib.dave_dll_thread_gid_co.restype = c_void_p
    pRsp = davelib.dave_dll_thread_gid_qco(c_char_p(gid), c_char_p(dst), c_int(req_id), c_int(sizeof(pReq.contents)), pReq, c_int(rsp_id), c_char_p(__func__), c_int(__LINE__))
    return struct_copy(rsp_class, pRsp, sizeof(rsp_class))


def sync_msg(dst, req_id, pReq, rsp_id, rsp_class):
    __func__, __LINE__ = t_sys_myline(2)

    if isinstance(dst, str) == True:
        dst = bytes(dst, encoding='utf8')

    pRsp = thread_msg(rsp_class)

    davelib.dave_dll_thread_sync_msg.restype = c_void_p
    ret = davelib.dave_dll_thread_sync_msg(c_char_p(dst), c_int(req_id), c_int(sizeof(pReq.contents)), pReq, c_int(rsp_id), c_int(sizeof(pRsp.contents)), pRsp, c_char_p(__func__), c_int(__LINE__))
    if ret == None:
        rsp_ret = None
    else:
        rsp_ret = struct_copy(rsp_class, pRsp, sizeof(rsp_class))
    thread_msg_release(pRsp)
    return rsp_ret


def broadcast_msg(thread_name, msg_id, class_instance):
    __func__, __LINE__ = t_sys_myline(2)

    if isinstance(thread_name, str) == True:
        thread_name = bytes(thread_name, encoding='utf8')
    davelib.dave_dll_thread_broadcast_msg(c_char_p(thread_name), c_int(msg_id), c_int(sizeof(class_instance.contents)), class_instance, c_char_p(__func__), c_int(__LINE__))
    return