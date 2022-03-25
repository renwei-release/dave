package base
/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import "unsafe"

type msg_function_define func(string, uint64, uint64, unsafe.Pointer)

func fun_None(src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer) {

}

// =====================================================================

var system_function_table = map[int]msg_function_define{
    MSGID_TEST:                       fun_None,
    MSGID_TIMER:                      fun_None,
    MSGID_WAKEUP:                     fun_None,
    MSGID_RUN_FUNCTION:               fun_None,
    MSGID_DEBUG_REQ:                  fun_None,
    MSGID_DEBUG_RSP:                  fun_None,
    MSGID_RESTART_REQ:                fun_None,
    MSGID_RESTART_RSP:                fun_None,
    MSGID_POWER_OFF:                  fun_None,
    MSGID_REMOTE_THREAD_READY:        fun_None,
    MSGID_REMOTE_THREAD_REMOVE:       fun_None,
    MSGID_TRACE_SWITCH:               fun_None,
    MSGID_REMOTE_MSG_TIMER_OUT:       fun_None,
    MSGID_TEMPORARILY_DEFINE_MESSAGE: fun_None,
    MSGID_CALL_FUNCTION:              fun_None,
    MSGID_SYSTEM_MOUNT:               fun_None,
    MSGID_SYSTEM_DECOUPLING:          fun_None,
    MSGID_MEMORY_WARNING:             fun_None,
    MSGID_ECHO:                       fun_None,
    MSGID_INTERNAL_EVENTS:            fun_None,
    MSGID_THREAD_BUSY:                fun_None,
    MSGID_THREAD_IDLE:                fun_None,
    MSGID_CLIENT_BUSY:                fun_None,
    MSGID_CLIENT_IDLE:                fun_None,
    MSGID_REMOTE_THREAD_ID_READY:     fun_None,
    MSGID_REMOTE_THREAD_ID_REMOVE:    fun_None,
    MSGID_LOCAL_THREAD_READY:         fun_None,
    MSGID_LOCAL_THREAD_REMOVE:        fun_None,
}

func Dave_system_function_table_add(msg_id int, msg_function msg_function_define) {
    system_function_table[msg_id] = msg_function
}

func Dave_system_function_table_del(msg_id int) {
    delete(system_function_table, msg_id)
}