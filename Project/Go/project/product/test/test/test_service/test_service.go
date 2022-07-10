package test_service

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"dave/product/test/refittesting"
	"dave/product/test/test/test_service/service"
	"strings"
)

var _input_dir = "/project/product/test/test/service_test/input"
var _output_dir = "/project/product/test/test/service_test/output"

var _test_service_table = map[string]func(t *refittesting.T){
	"base": service.Base_case,
}

// =====================================================================

func Test_service(gid string, service string, id uint64) {

	service = strings.ToLower(service)

	test_case, exists := _test_service_table[service]
	if exists {
		refittesting.TestSuite(service, test_case)
	}
}
