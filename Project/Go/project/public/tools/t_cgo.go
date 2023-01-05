package tools

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void go_string_copy_to_c_bin(void *c_bin_ptr, int c_bin_len, void *go_string_ptr, int go_string_len) {
    if(c_bin_len > go_string_len)
        c_bin_len = go_string_len;

    memcpy(c_bin_ptr, go_string_ptr, c_bin_len);
}
*/
import "C"
import (
    "unsafe"
)

type stringStruct struct {
	str unsafe.Pointer
	len int
}

type sliceStruct struct {
	str unsafe.Pointer
	len int
    cap int
}

func stringStructOf(sp *string) *stringStruct {
	return (*stringStruct)(unsafe.Pointer(sp))
}

func byteStructOf(sp *[]byte) *sliceStruct {
	return (*sliceStruct)(unsafe.Pointer(sp))
}

// =====================================================================

func T_cgo_gobyte2gostring(go_byte []byte) string {
	byte_string_len := 0
	for ; byte_string_len < len(go_byte); byte_string_len++ {
		if go_byte[byte_string_len] == 0 {
			break
		}
	}

    go_string := string(go_byte)
    stringStructOf(&go_string).len = byte_string_len
    return go_string
}

func T_cgo_gostring2cbin(bin_len int64, bin_ptr unsafe.Pointer, go_string string) {
    go_string_struct := stringStructOf(&go_string)

    C.go_string_copy_to_c_bin(bin_ptr, C.int(bin_len), go_string_struct.str, C.int(go_string_struct.len))
}

func T_cgo_gobyte2cbin(bin_len int64, bin_ptr unsafe.Pointer, go_byte []byte) {
    go_byte_struct := byteStructOf(&go_byte)

    C.go_string_copy_to_c_bin(bin_ptr, C.int(bin_len), go_byte_struct.str, C.int(go_byte_struct.len))
}

func T_cgo_cbin2gostring(bin_len int64, bin_ptr unsafe.Pointer) string {
    return C.GoStringN((*C.char)(bin_ptr), C.int(bin_len))
}

func T_cgo_cbin2gobyte(bin_len int64, bin_ptr unsafe.Pointer) []byte {
	return C.GoBytes((unsafe.Pointer)(bin_ptr), C.int(bin_len))
}

func T_cgo_byte_clone(src []byte) []byte {
	dst := make([]byte, len(src))
	copy(dst, src)
	return dst
}