#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

exit_program=$(type docker)
if [ "$exit_program" == "" ]; then
   echo environment.sh setup docker ...
   sudo yum -y install docker
   sudo systemctl start docker
   sudo systemctl enable docker
fi

exit_program=$(type docker-compose)
if [ "$exit_program" == "" ]; then
   echo environment.sh setup docker-compose ...
   sudo ./curl -L "https://github.com/docker/compose/releases/download/v2.3.4/docker-compose-$(uname -s)-$(uname -m)" -o /usr/local/bin/docker-compose
   sudo chmod +x /usr/local/bin/docker-compose
   sudo ln -s /usr/local/bin/docker-compose /usr/bin/docker-compose
fi

exit_program=$(type bzip2)
if [ "$exit_program" == "" ]; then
   echo environment.sh setup bzip2 ...
   sudo yum -y install bzip2
fi

exit_program=$(type wget)
if [ "$exit_program" == "" ]; then
   echo environment.sh setup wget ...
   sudo yum -y install wget
fi

exit_dll=/lib64/libboost_thread-mt.so.1.53.0
if [ ! -f ${exit_dll} ]; then
   echo environment.sh setup libboost
   sudo yum install boost-devel
fi