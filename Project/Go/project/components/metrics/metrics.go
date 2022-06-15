/*
 * Copyright (c) 2022 zhaojie
 * Email: 496700159@qq.com
 *
 *  This is a free software; you can redistribute it and/or modify
 *  it under the terms of the MIT license. See LICENSE for details.
 *
 */

package metrics


import (
    "fmt"
    "net/http"
    "github.com/prometheus/client_golang/prometheus/promhttp"
)

func _StartMetricsListen(addr string, bPanicIfFailed bool) {
    http.Handle("/metrics", promhttp.Handler())
    
    var err error = http.ListenAndServe(addr, nil)
    sErrMsg := fmt.Sprintf("[metrics] %s http.ListenAndServe failed, err:%v", addr, err)
    fmt.Printf(sErrMsg)
    if bPanicIfFailed {
        panic(sErrMsg)
    }
}

// =====================================================================

func StartMetrics(httpport int) {
    addr := fmt.Sprintf(":%d", httpport)
    go _StartMetricsListen(addr, true)
}
