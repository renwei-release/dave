package base

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

/*
#include <dave_base.h>
#include <stdio.h>
#include <stdlib.h>
*/
import "C"
import (
	"dave/public/tools"
	"fmt"
	"reflect"
	"unsafe"
)

// =====================================================================

func Thread_self() string {
	c_self_name := C.dave_dll_self()
	return C.GoString(c_self_name)
}

func Thread_msg(msg_len int) unsafe.Pointer {
	msg_ptr := C.dave_dll_thread_msg(C.int(msg_len), nil, C.int(0))
	return msg_ptr
}

func Write_msg(dst interface{}, msg_id int, msg_len int, msg_ptr unsafe.Pointer) bool {
	ret := C.int(-1)

    thread_ptr := Thread_msg(msg_len)
    if thread_ptr == nil {
        return false
    }
    tools.T_cgo_memcpy(thread_ptr, msg_ptr, int64(msg_len))

	switch dst.(type) {
		case uint64:
			ret = C.dave_dll_thread_id_msg(C.ulonglong(dst.(uint64)), C.int(msg_id), C.int(msg_len), thread_ptr, nil, C.int(0))
		case string:
			c_thread_name := C.CString(dst.(string))
			ret = C.dave_dll_thread_name_msg(c_thread_name, C.int(msg_id), C.int(msg_len), thread_ptr, nil, C.int(0))
			C.free(unsafe.Pointer(c_thread_name))
		default:
			fmt.Printf("Write_msg invalid type:%s\n", reflect.TypeOf(dst).Name())
	}

	if ret < 0 {
		return false
	}
	return true
}

func Write_qmsg(dst interface{}, msg_id int, msg_len int, msg_ptr unsafe.Pointer) bool {
	ret := C.int(-1)

    thread_ptr := Thread_msg(msg_len)
    if thread_ptr == nil {
        return false
    }
    tools.T_cgo_memcpy(thread_ptr, msg_ptr, int64(msg_len))

	switch dst.(type) {
		case uint64:
			ret = C.dave_dll_thread_id_qmsg(C.ulonglong(dst.(uint64)), C.int(msg_id), C.int(msg_len), thread_ptr, nil, C.int(0))
		case string:
			c_thread_name := C.CString(dst.(string))
			ret = C.dave_dll_thread_name_qmsg(c_thread_name, C.int(msg_id), C.int(msg_len), thread_ptr, nil, C.int(0))
			C.free(unsafe.Pointer(c_thread_name))
		default:
			fmt.Printf("Write_msg invalid type:%s\n", reflect.TypeOf(dst).Name())
	}

	if ret < 0 {
		return false
	}
	return true
}

func Write_co(dst interface{}, req_id int, req_len int, req_ptr unsafe.Pointer, rsp_id int) unsafe.Pointer {
	ret := unsafe.Pointer(nil)

    thread_ptr := Thread_msg(req_len)
    if thread_ptr == nil {
        return ret
    }
    tools.T_cgo_memcpy(thread_ptr, req_ptr, int64(req_len))

	switch dst.(type) {
		case uint64:
			ret = C.dave_dll_thread_id_co(C.ulonglong(dst.(uint64)), C.int(req_id), C.int(req_len), thread_ptr, C.int(rsp_id), nil, C.int(0))
		case string:
			c_thread_name := C.CString(dst.(string))
			ret = C.dave_dll_thread_name_co(c_thread_name, C.int(req_id), C.int(req_len), thread_ptr, C.int(rsp_id), nil, C.int(0))
			C.free(unsafe.Pointer(c_thread_name))
		default:
			fmt.Printf("Write_co invalid type:%s\n", reflect.TypeOf(dst).Name())
	}

	return ret
}

func Write_qco(dst interface{}, req_id int, req_len int, req_ptr unsafe.Pointer, rsp_id int) unsafe.Pointer {
	ret := unsafe.Pointer(nil)

    thread_ptr := Thread_msg(req_len)
    if thread_ptr == nil {
        return ret
    }
    tools.T_cgo_memcpy(thread_ptr, req_ptr, int64(req_len))

	switch dst.(type) {
		case uint64:
			ret = C.dave_dll_thread_id_qco(C.ulonglong(dst.(uint64)), C.int(req_id), C.int(req_len), thread_ptr, C.int(rsp_id), nil, C.int(0))
		case string:
			c_thread_name := C.CString(dst.(string))
			ret = C.dave_dll_thread_name_qco(c_thread_name, C.int(req_id), C.int(req_len), thread_ptr, C.int(rsp_id), nil, C.int(0))
			C.free(unsafe.Pointer(c_thread_name))
		default:
			fmt.Printf("Write_co invalid type:%s\n", reflect.TypeOf(dst).Name())
	}

	return ret
}

func Gid_msg(gid string, thread_name string, msg_id int, msg_len int, msg_ptr unsafe.Pointer) bool {
    thread_ptr := Thread_msg(msg_len)
    if thread_ptr == nil {
        return false
    }
    tools.T_cgo_memcpy(thread_ptr, msg_ptr, int64(msg_len))

	c_gid_name := C.CString(gid)
	c_thread_name := C.CString(thread_name)

	ret := C.dave_dll_thread_gid_msg(c_gid_name, c_thread_name, C.int(msg_id), C.int(msg_len), thread_ptr, nil, C.int(0))

	C.free(unsafe.Pointer(c_thread_name))
	C.free(unsafe.Pointer(c_gid_name))

	if ret < 0 {
		return false
	}
	return true
}

func Gid_qmsg(gid string, thread_name string, msg_id int, msg_len int, msg_ptr unsafe.Pointer) bool {
    thread_ptr := Thread_msg(msg_len)
    if thread_ptr == nil {
        return false
    }
    tools.T_cgo_memcpy(thread_ptr, msg_ptr, int64(msg_len))

	c_gid_name := C.CString(gid)
	c_thread_name := C.CString(thread_name)

	ret := C.dave_dll_thread_gid_qmsg(c_gid_name, c_thread_name, C.int(msg_id), C.int(msg_len), thread_ptr, nil, C.int(0))

	C.free(unsafe.Pointer(c_thread_name))
	C.free(unsafe.Pointer(c_gid_name))

	if ret < 0 {
		return false
	}
	return true
}

func Gid_co(gid string, thread_name string, req_id int, req_len int, req_ptr unsafe.Pointer, rsp_id int) unsafe.Pointer {
    thread_ptr := Thread_msg(req_len)
    if thread_ptr == nil {
        return nil
    }
    tools.T_cgo_memcpy(thread_ptr, req_ptr, int64(req_len))

	c_gid_name := C.CString(gid)
	c_thread_name := C.CString(thread_name)

	ret := C.dave_dll_thread_gid_co(c_gid_name, c_thread_name, C.int(req_id), C.int(req_len), thread_ptr, C.int(rsp_id), nil, C.int(0))

	C.free(unsafe.Pointer(c_gid_name))
	C.free(unsafe.Pointer(c_thread_name))

	return ret
}

func Gid_qco(gid string, thread_name string, req_id int, req_len int, req_ptr unsafe.Pointer, rsp_id int) unsafe.Pointer {
    thread_ptr := Thread_msg(req_len)
    if thread_ptr == nil {
        return nil
    }
    tools.T_cgo_memcpy(thread_ptr, req_ptr, int64(req_len))

	c_gid_name := C.CString(gid)
	c_thread_name := C.CString(thread_name)

	ret := C.dave_dll_thread_gid_qco(c_gid_name, c_thread_name, C.int(req_id), C.int(req_len), thread_ptr, C.int(rsp_id), nil, C.int(0))

	C.free(unsafe.Pointer(c_gid_name))
	C.free(unsafe.Pointer(c_thread_name))

	return ret
}

func Sync_msg(dst string, req_id int, req_len int, req_ptr unsafe.Pointer, rsp_id int, rsp_len int, rsp_ptr unsafe.Pointer) bool {
	c_dst := C.CString(dst)

	ret := C.dave_dll_thread_name_sync_msg(c_dst, C.int(req_id), C.int(req_len), req_ptr, C.int(rsp_id), C.int(rsp_len), rsp_ptr, nil, C.int(0))

	C.free(unsafe.Pointer(c_dst))

	if ret == nil {
		return false
	}

	return true
}

func Broadcast_msg(dst string, msg_id int, msg_len int, msg_ptr unsafe.Pointer) bool {
	c_dst := C.CString(dst)

	ret := C.dave_dll_thread_broadcast_msg(c_dst, C.int(msg_id), C.int(msg_len), msg_ptr, nil, C.int(0))

	C.free(unsafe.Pointer(c_dst))

	if ret < 0 {
		return false
	}
	return true
}
