package base

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
    "dave/components/cpanic"
    "dave/components/pprof"
    "fmt"
    "log"
    "os"
    "strconv"
    "strings"
    "time"
    "dave/components/metrics"
)

var (
    gPainicFile   *os.File
    gPainicLogger *log.Logger
)

var (
    _PanicFileName string
)

func _ModuleCpanicInit() {
    var (
        err error
    )
    
    lNow := time.Now().Format("2006-01-02 15:04:05")
    lNow = strings.Replace(lNow, " ", "-", 1)
    lNow = strings.Replace(lNow, ":", "-", 2)
    
    _PanicFileName = fmt.Sprintf("%s-%v-panic.log", VERSION_PRODUCT, lNow)
    
    gPainicFile, gPainicLogger, err = cpanic.NewPanicFile(_PanicFileName)
    if nil != err {
        log.Printf("cpanic.NewPanicFile %v, ", time.Now())
        return
    }
    
    DAVELOG("\033[35mSet panic file name:%s\033[0m", _PanicFileName)
    DAVELOG("\033[35mIf you exit normally, the system will automatically delete the file!\033[0m")
}

func _ModuleCpanicExit() {
    os.Remove(_PanicFileName)
}

func _ModulePprofLoad() {
    CFG_PPROF_ACTION := "PProfAction"
    
    action := Cfg_get(CFG_PPROF_ACTION)
    if action == "enable" {
        CFG_LISTEN_PORT := "PProfHttpPort"
        
        httpportstr := Cfg_get(CFG_LISTEN_PORT)
        if httpportstr == "" {
            httpportstr := "10108"
            Cfg_set(CFG_LISTEN_PORT, httpportstr)
        }
        httpport, err := strconv.Atoi(httpportstr)
        if err != nil {
            httpport = 10108
        }
        pprof.StartPprof(httpport)
        
        DAVELOG("\033[35mNow you can use the following link to access the pprof service:\033[0m")
        DAVELOG("\033[35mhttp://127.0.0.1:%d/debug/pprof/\033[0m", httpport)
    }
}

func _ModuleMetricsLoad() {
    CFG_PPROF_ACTION := "MetricsAction"
    
    action := Cfg_get(CFG_PPROF_ACTION)
    if action == "enable" {
        CFG_LISTEN_PORT := "MetricsHttpPort"
        
        httpportstr := Cfg_get(CFG_LISTEN_PORT)
        if httpportstr == "" {
            httpportstr := "10109"
            Cfg_set(CFG_LISTEN_PORT, httpportstr)
        }
        httpport, err := strconv.Atoi(httpportstr)
        if err != nil {
            httpport = 10109
        }
        metrics.StartMetrics(httpport)
        
        DAVELOG("\033[35mNow you can use the following link to access the metrics service:\033[0m")
        DAVELOG("\033[35mhttp://127.0.0.1:%d/metrics\033[0m", httpport)
    }
}

// =====================================================================

func dave_module_init() {
    _ModuleCpanicInit()
    
    _ModulePprofLoad()
    _ModuleMetricsLoad()
}

func dave_module_exit() {
    _ModuleCpanicExit()
}
