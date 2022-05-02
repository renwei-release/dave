#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

echo rm-project.sh work inside the container
shopt -s extglob
rm -rf /project/!(model)
shopt -u extglob