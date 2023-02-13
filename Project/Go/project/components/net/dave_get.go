package net

/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"github.com/parnurzeal/gorequest"
	"dave/public/base"
	"net/http"
	"errors"
)

// =====================================================================

func Get(url string) (*http.Response, string, error) {
	if len(url) == 0 {
		return nil, "", errors.New("invalid url")
	}

	rsp_res, rsp_body, errs := gorequest.New().Get(url).End()

	base.DAVEDEBUG("rsp_res:%v rsp_body:%v errs:%v", rsp_res, rsp_body, errs)

	return rsp_res, rsp_body, nil
}