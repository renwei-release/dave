#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

SOLFILEPATHNAME=$1
SOLOUTPUTDIR=solc_output
HOMEPATH=$(cd `dirname $0`; pwd)
File=$(basename $0)
SETUPPATH=${HOMEPATH}/../../../../Go/project/product/blockchain/eth

if [ -f ${SOLFILEPATHNAME} ]; then
   SOLFILEPATHNAME=token/nft_token/NFTCollection
fi
SOLFILEPATH=${SOLFILEPATHNAME%/*}
SOLFILENAME=${SOLFILEPATHNAME##*/}

exit_program=$(type npm)
if [ "$exit_program" == "" ]; then
   echo ${File} setup npm ...
   yum install -y npm
fi
exit_program=$(type solc)
if [ "$exit_program" == "" ]; then
   echo ${File} setup solc ...
   npm install -g solc
   ln -s /usr/local/lib/node_modules/solc/solc.js /usr/bin/solc
fi

if [ ! -d ./node_modules/@openzeppelin/contracts ]; then
   npm install @openzeppelin/contracts
fi
if [ ! -d ./node_modules/@openzeppelin/contracts-upgradeable ]; then
   npm install @openzeppelin/contracts-upgradeable
fi

OUTPUTFILE="${HOMEPATH}/${SOLFILEPATHNAME}/sol/${SOLFILENAME}"
OUTPUTFILE=${SOLOUTPUTDIR}/${OUTPUTFILE//\//_}

echo -e "${File} home path  : ${HOMEPATH}"
echo -e "${File} solc output: ${SOLOUTPUTDIR}"
echo -e "${File} output file: ${OUTPUTFILE}"
echo -e "${File} setup path : ${SETUPPATH}"

solc ${SOLFILEPATHNAME}.sol --base-path ${HOMEPATH}/node_modules -o ${SOLOUTPUTDIR} --abi

solc ${SOLFILEPATHNAME}.sol --base-path ${HOMEPATH}/node_modules -o ${SOLOUTPUTDIR} --bin

./abigen --abi ${OUTPUTFILE}.abi --bin ${OUTPUTFILE}.bin --pkg mytoken --out ${SOLFILEPATH}/${SOLFILENAME}.go

if [ $? -ne 0 ];then
   echo -e "${File} \033[35mbuild failed!\033[0m"
else
   echo -e "${File} \033[36msuccess\033[0m    : ${SOLFILEPATHNAME}.sol -> ${SOLFILEPATH}/${SOLFILENAME}.go"
   mkdir -p ${SETUPPATH}/${SOLFILEPATH}
   echo -e "${File} setup to   : ${SETUPPATH}/${SOLFILEPATH}/${SOLFILENAME}.go"
   cp ${SOLFILEPATH}/${SOLFILENAME}.go ${SETUPPATH}/${SOLFILEPATH}/${SOLFILENAME}.go
   rm -rf ${SOLOUTPUTDIR}
fi
