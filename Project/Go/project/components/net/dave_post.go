package net

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"crypto/tls"
	"encoding/json"
	"github.com/parnurzeal/gorequest"
	"dave/public/base"
	"net/http"
	"errors"
)

// =====================================================================

func Post(url string, req_header string, req_body interface{}) (*http.Response, string, error) {
	if len(url) == 0 {
		return nil, "", errors.New("invalid url")
	}

	request := gorequest.New().Post(url).TLSClientConfig(&tls.Config{InsecureSkipVerify: true})
	request.BounceToRawString = true
	m := make(map[string]string)
	json.Unmarshal([]byte(req_header), &m)

	for k, v := range m {
		if len(k) == 0 || len(v) == 0 {
			continue
		}
		request.Header.Set(k, v)
	}
	rsp_res, rsp_body, errs := request.Send(req_body).End()

	base.DAVEDEBUG("rsp_res:%v rsp_body:%v errs:%v", rsp_res, rsp_body, errs)

	return rsp_res, rsp_body, nil
}