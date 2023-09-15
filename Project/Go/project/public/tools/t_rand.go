package tools

/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

import (
	"fmt"
	"time"
	"math/rand"
)

// =====================================================================

func T_rand_ub() uint64 {
	rand.Seed(time.Now().UnixNano())
	return uint64(rand.Int31n(1000000000))
}

func T_rand() string {
	rand.Seed(time.Now().UnixNano())
	return fmt.Sprintf("%016v", rand.Int31n(1000000000))
}