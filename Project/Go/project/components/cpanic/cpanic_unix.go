//go:build unix
// +build unix

/*
 * Copyright (c) 2022 chenjun
 * Git: https://github.com/chenjunpc2008
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

/*
Package panic log the panic under unix to the log file
*/

package cpanic

import (
	"fmt"
	"log"
	"os"
	"syscall"
)

// redirectStderr to the file passed in
func redirectStderr(f *os.File) error {
	err := syscall.Dup2(int(f.Fd()), int(os.Stderr.Fd()))
	if nil != err {
		log.Fatalf("Failed to redirect stderr to file: %v", err)
		fmt.Printf("Failed to redirect stderr to file: %v", err)
		return err
	}

	return nil
}
