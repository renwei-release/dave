package tools
/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"time"
	"runtime"
	"github.com/shirou/gopsutil/cpu"
	"github.com/shirou/gopsutil/mem"
	"github.com/shirou/gopsutil/disk"
	"github.com/shirou/gopsutil/net"
)

// =====================================================================

func T_sys_GetCpuPercent() float64 {
	percent, _:= cpu.Percent(time.Second, false)
	return percent[0]
}

func T_sys_GetMemPercent() float64 {
	memInfo, _ := mem.VirtualMemory()
	return memInfo.UsedPercent
}
 
func T_sys_GetDiskPercent() float64 {
	parts, _ := disk.Partitions(true)
	diskInfo, _ := disk.Usage(parts[0].Mountpoint)
	return diskInfo.UsedPercent
}
 
func T_sys_GetNetPercent() []net.ConnectionStat {
	info, _ := net.Connections("all")
	return info
}

func T_sys_go_version() string {
	return runtime.Version()
}