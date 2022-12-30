#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

PROJECTNAME=$1
JUPYTERPORT=$2
JUPYTERCFGFILE=/root/.jupyter/jupyter_notebook_config.py
File=$(basename $0)

if [ $JUPYTERPORT != 8888 ]; then
   echo ${File} find jupyter, replece ${PROJECTNAME} jupyter port to ${JUPYTERPORT}
   JUPYTERPORTLINE=`docker exec -t ${PROJECTNAME} cat -n ${JUPYTERCFGFILE} | grep 'c.NotebookApp.port = ' | awk '{print $1}'`
   docker exec -t ${PROJECTNAME} sed -i "${JUPYTERPORTLINE}c c.NotebookApp.port = ${JUPYTERPORT}" ${JUPYTERCFGFILE}
fi