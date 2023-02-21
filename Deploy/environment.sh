#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

if [[ -f /usr/bin/yum ]] || [[ -f /usr/sbin/yum ]] || [[ -f /bin/yum ]] || [[ -f /sbin/yum ]]; then
   INSTALL=yum
else
   INSTALL=apt
fi

exit_program=$(type docker)
if [ "$exit_program" == "" ]; then
   echo environment.sh setup docker ...
   sudo ${INSTALL} -y install docker
   sudo systemctl start docker
   sudo systemctl enable docker
fi

exit_program=$(type docker-compose)
if [ "$exit_program" == "" ]; then
   echo environment.sh setup docker-compose ...
   sudo ${INSTALL} -y install docker-compose
fi

exit_program=$(type bzip2)
if [ "$exit_program" == "" ]; then
   echo environment.sh setup bzip2 ...
   sudo ${INSTALL} -y install bzip2
fi

exit_program=$(type wget)
if [ "$exit_program" == "" ]; then
   echo environment.sh setup wget ...
   sudo ${INSTALL} -y install wget
fi