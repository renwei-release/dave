package pprof

/*
 * Copyright (c) 2022 chenjun
 * Git: https://github.com/chenjunpc2008
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"fmt"
	"net/http"
	_ "net/http/pprof"
)

func _PprofListen(addr string, bPanicIfFailed bool) {
	// For load balance keep alive and pprof debug
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		w.Write([]byte("ok"))
	})

	var err error = http.ListenAndServe(addr, nil)
	sErrMsg := fmt.Sprintf("%s http.ListenAndServe failed, err:%v", addr, err)
	fmt.Printf(sErrMsg)
	if bPanicIfFailed {
		panic(sErrMsg)
	}
}

// =====================================================================

func StartPprof(httpport int) {
	addr := fmt.Sprintf(":%d", httpport)
	go _PprofListen(addr, false)
}
