#!/usr/bin/expect
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

#./terminal.sh base-renwei ls

set container_name [lindex $argv 0]
set input_string [lindex $argv 1]

spawn docker attach $container_name

expect "$ "

send "$input_string\r"