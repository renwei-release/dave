package cpanic

/*
 * Copyright (c) 2022 chenjun
 * Git: https://github.com/chenjunpc2008
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"fmt"
	"log"
	"os"
)

/*
based on https://stackoverflow.com/questions/34772012/capturing-panic-in-golang
*/

/*
NewPanicFile new panic file, redirect stderr to panic file
*/
func NewPanicFile(path string) (*os.File, *log.Logger, error) {

	f, err := os.OpenFile(path, os.O_CREATE|os.O_APPEND|os.O_RDWR, os.ModePerm)
	if nil != err {
		log.Printf("os.Create failed: %v", err)
		fmt.Printf("os.Create failed: %v", err)
		return nil, nil, err
	}

	loger := log.New(f, "", log.LstdFlags)

	err = redirectStderr(f)
	if nil != err {
		log.Printf("redirectStderr failed: %v", err)
		fmt.Printf("redirectStderr failed: %v", err)
		return nil, nil, err
	}

	return f, loger, nil
}
