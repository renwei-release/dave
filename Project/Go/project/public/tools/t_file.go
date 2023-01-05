package tools

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"errors"
	"fmt"
	"os"
)

func _file_creat_dir(file_path string) {
	for index:=1; index<len(file_path); index++ {
		if file_path[index] == '/' {
			cur_path := file_path[0:index]

			_ , err :=os.Stat(cur_path)
			if err != nil {
				os.Mkdir(cur_path, 0666)
			}
		}
    }
}

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

func T_file_write(file_data []byte, file_name string) error {
	_file_creat_dir(file_name)

	file, err := os.OpenFile(file_name, os.O_RDWR|os.O_CREATE, 0666)
	if err != nil {
		fmt.Println("open file:%s err:%v", file_name, err)
		return err
	}
	defer file.Close()

	wlen, err := file.Write(file_data)
	if err != nil {
		fmt.Println("write file:%s err:%v", file_name, err)
		return err
	}
	if wlen != len(file_data) {
		fmt.Println("write file:%s len:%d/%d err:%v", file_name, wlen, len(file_data), err)
		return errors.New("write file error!")
	}

	return nil
}

func T_dir_remove(dir_path string) {
	os.RemoveAll(dir_path)
}