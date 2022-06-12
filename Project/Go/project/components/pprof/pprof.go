package pprof

import (
	"fmt"
	"net/http"
	_ "net/http/pprof"
)

func _PprofListen(addr string) error {
	// For load balance keep alive and pprof debug
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		w.Write([]byte("ok"))
	})

	var err error = http.ListenAndServe(addr, nil)
	if nil != err {
		return err
	}

	return nil
}

func PprofListen(httpport int) (int, int) {
	addr := fmt.Sprintf(":%d", httpport)
	safe_counter := 0
	success := -1

	fmt.Printf("PprofListen:%d\n", httpport)
	for {
		safe_counter++
		if safe_counter >= 256 {
			break
		}

		error := _PprofListen(addr)
		if error == nil {
			success = 0
			fmt.Printf("pprof listen on:http://127.0.0.1:%d/debug/pprof success!", httpport)
			break
		}

		fmt.Printf("pprof listen on:http://127.0.0.1:%d/debug/pprof failed!", httpport)
		httpport++
		addr = fmt.Sprintf(":%d", httpport)
	}

	fmt.Printf("PprofListen result:%d\n", success)
	return success, httpport
}

// =====================================================================

func StartPprof(httpport int) int {
	fmt.Printf("start pprof:%d\n", httpport)
	go PprofListen(httpport)
	return httpport
}
