package tools

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"fmt"
	"os"
)

// =====================================================================

func T_file_read(file_name string) []byte{
	file, err := os.Open(file_name)
	if err != nil {
		fmt.Println(err)
		return nil
	}
	defer file.Close()

	fileinfo, err := file.Stat()
	if err != nil {
		fmt.Println(err)
		return nil
	}

	filesize := fileinfo.Size()
	buffer := make([]byte, filesize)

	bytesread, err := file.Read(buffer)
	if err != nil {
		fmt.Println(err)
		return nil
	}

	if int64(bytesread) != filesize {
		fmt.Println("bytesread:%d filesize:%d mismatch!", bytesread, filesize)
	}

	return buffer
}