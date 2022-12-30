package base

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
    "dave/public/auto"
    "sync"
    "unsafe"
)

type msg_function_define func(string, string, uint64, uint64, unsafe.Pointer)

func fun_None(src_gid string, src_name string, src_id uint64, msg_len uint64, msg_body unsafe.Pointer) {

}

// =====================================================================

var (
    system_function_table = map[int]msg_function_define{
        auto.MSGID_TEST:                       fun_None,
        auto.MSGID_TIMER:                      fun_None,
        auto.MSGID_WAKEUP:                     fun_None,
        auto.MSGID_RUN_FUNCTION:               fun_None,
        auto.MSGID_DEBUG_REQ:                  fun_None,
        auto.MSGID_DEBUG_RSP:                  fun_None,
        auto.MSGID_RESTART_REQ:                fun_None,
        auto.MSGID_RESTART_RSP:                fun_None,
        auto.MSGID_POWER_OFF:                  fun_None,
        auto.MSGID_REMOTE_THREAD_READY:        fun_None,
        auto.MSGID_REMOTE_THREAD_REMOVE:       fun_None,
        auto.MSGID_TRACE_SWITCH:               fun_None,
        auto.MSGID_PROCESS_MSG_TIMER_OUT:      fun_None,
        auto.MSGID_TEMPORARILY_DEFINE_MESSAGE: fun_None,
        auto.MSGID_CALL_FUNCTION:              fun_None,
        auto.MSGID_SYSTEM_MOUNT:               fun_None,
        auto.MSGID_SYSTEM_DECOUPLING:          fun_None,
        auto.MSGID_MEMORY_WARNING:             fun_None,
        auto.MSGID_ECHO:                       fun_None,
        auto.MSGID_INTERNAL_EVENTS:            fun_None,
        auto.MSGID_THREAD_BUSY:                fun_None,
        auto.MSGID_THREAD_IDLE:                fun_None,
        auto.MSGID_CLIENT_BUSY:                fun_None,
        auto.MSGID_CLIENT_IDLE:                fun_None,
        auto.MSGID_REMOTE_THREAD_ID_READY:     fun_None,
        auto.MSGID_REMOTE_THREAD_ID_REMOVE:    fun_None,
        auto.MSGID_LOCAL_THREAD_READY:         fun_None,
        auto.MSGID_LOCAL_THREAD_REMOVE:        fun_None,
        auto.MSGID_CFG_UPDATE:                 fun_None,
        auto.MSGID_CFG_REMOTE_UPDATE:          fun_None,
    }
    rwMutex  = sync.RWMutex{}
)

func Dave_system_function_table_add(msg_id int, msg_function msg_function_define) {
    rwMutex.Lock()
    defer rwMutex.Unlock()

    system_function_table[msg_id] = msg_function
}

func Dave_system_function_table_del(msg_id int) {
    rwMutex.Lock()
    defer rwMutex.Unlock()

    delete(system_function_table, msg_id)
}

func Dave_system_function_table_inq(msg_id int) (msg_function_define, bool) {
    rwMutex.RLock()
    defer rwMutex.RUnlock()

    fun, exists := system_function_table[msg_id]
    return fun, exists
}
