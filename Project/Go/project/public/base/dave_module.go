package base

import (
	"dave/components/pprof"
	"strconv"
)

func _module_pprof_load() {
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
	}
}

// =====================================================================

func dave_module_load() {
	_module_pprof_load()
}
