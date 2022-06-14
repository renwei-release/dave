//go:build windows
// +build windows

/*
 * Copyright (c) 2022 chenjun
 * Git: https://github.com/chenjunpc2008
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

/*
Package panic log the panic under windows to the log file
*/

package cpanic

import (
	"fmt"
	"log"
	"os"
	"syscall"
)

func setStdHandle(stdhandle int32, handle syscall.Handle) error {
	var (
		kernel32         = syscall.MustLoadDLL("kernel32.dll")
		procSetStdHandle = kernel32.MustFindProc("SetStdHandle")
	)

	r0, _, e1 := syscall.Syscall(procSetStdHandle.Addr(), 2, uintptr(stdhandle), uintptr(handle), 0)
	if 0 == r0 {
		if 0 != e1 {
			return error(e1)
		}

		return syscall.EINVAL
	}

	return nil
}

func redirectStderr(f *os.File) error {
	err := setStdHandle(syscall.STD_ERROR_HANDLE, syscall.Handle(f.Fd()))
	if nil != err {
		log.Fatalf("Failed to redirect stderr to file: %v", err)
		fmt.Printf("Failed to redirect stderr to file: %v", err)
		return err
	}

	// SetStdHandle does not affect prior references to stderr
	os.Stderr = f

	return nil
}
