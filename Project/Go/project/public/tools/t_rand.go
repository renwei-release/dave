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

func T_rand() string {
	return fmt.Sprintf("%016v", rand.New(rand.NewSource(time.Now().UnixNano())).Int31n(1000000000))
}