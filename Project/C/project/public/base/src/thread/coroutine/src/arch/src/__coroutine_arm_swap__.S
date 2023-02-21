/*
 * Copyright (c) 2023 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#if defined(__aarch64__)

.globl coroutine_swap
.type  coroutine_swap, @function

coroutine_swap:
    mov    x3, sp

    stp    x19, x20, [x0]
    stp    x21, x22, [x0, #0x10]
    stp    x23, x24, [x0, #0x20]
    stp    x25, x26, [x0, #0x30]
    stp    x27, x28, [x0, #0x40]
    stp    x29, x30, [x0, #0x50]
    stp    x3, x0,   [x0, #0x60]

    ldp    x19, x20, [x1]
    ldp    x21, x22, [x1,#0x10]
    ldp    x23, x24, [x1,#0x20]
    ldp    x25, x26, [x1,#0x30]
    ldp    x27, x28, [x1,#0x40]
    ldp    x29, x30, [x1,#0x50]
    ldp    x3,  x0,  [x1,#0x60]

    mov    sp, x3
    ret
#endif