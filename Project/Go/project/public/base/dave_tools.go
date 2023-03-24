package base

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/public/auto"
	"dave/public/tools"
	"encoding/json"
	"errors"
)

// =====================================================================

func T_gostring2mbuf(string_data string) *auto.MBUF {
	mbuf_data := Dave_mmalloc(len(string_data))
	tools.T_cgo_gostring2cbin(mbuf_data.Len, mbuf_data.Payload, string_data)
	return mbuf_data
}

func T_mbuf2gostring(mbuf_data *auto.MBUF) string {
	go_string := tools.T_cgo_cbin2gostring(mbuf_data.Len, mbuf_data.Payload)
	return go_string
}

func T_gobyte2mbuf(byte_data []byte) *auto.MBUF {
	mbuf_data := Dave_mmalloc(len(byte_data))
	tools.T_cgo_gobyte2cbin(mbuf_data.Len, mbuf_data.Payload, byte_data)
	return mbuf_data
}

func T_mbuf2gobyte(mbuf_data *auto.MBUF) ([]byte, error) {
	if mbuf_data == nil {
		return nil, errors.New("mbuf_data is nil")
	}
	go_byte := tools.T_cgo_cbin2gobyte(mbuf_data.Len, mbuf_data.Payload)
	return go_byte, nil
}

func T_mbuf2gojson(mbuf_data *auto.MBUF, json_struct interface{}) error {
	go_byte := tools.T_cgo_cbin2gobyte(mbuf_data.Len, mbuf_data.Payload)

	err := json.Unmarshal(go_byte, json_struct)
	if err != nil {
		DAVELOG("err:%v", err)
		return err
	}
	return nil
}

func T_gojson2mbuf(json_data interface{}) *auto.MBUF {
	json_string, err := json.Marshal(json_data)
	if err != nil {
		DAVELOG("err:%v", err)
		return nil
	}
	return T_gobyte2mbuf(json_string)
}

func T_mbuf2json(mbuf_data *auto.MBUF) (*tools.Json, error) {
	if mbuf_data == nil {
		return nil, errors.New("mbuf_data is nil")
	}

	go_byte, err := T_mbuf2gobyte(mbuf_data)
	if err != nil {
		return nil, err
	}

	return tools.NewJson(go_byte)
}

func T_gojson2string(json_data interface{}) string {
	json_string, err := json.Marshal(json_data)
	if err != nil {
		DAVELOG("err:%v", err)
		return ""
	}
	return string(json_string)
}