#!/bin/bash
#/*
# * Copyright (c) 2024 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */
homedir=$(cd `dirname $0`; pwd)

PRODUCT=$1
BIN=`echo $PRODUCT | tr '[a-z]' '[A-Z]'`-BIN

cd $PRODUCT

python3 ../../../../Tools/refresh_version/refresh_version.py "../../../../" ${PRODUCT^^}

if [ -d "bin_data" ]; then
    rm -rf bin_data
fi
mkdir bin_data
cd bin_data

cp $homedir/build.spec .
cp $homedir/boot.py .

sed -i "s/___FLAG_FOR_PRODUCT___/$PRODUCT/g" build.spec
sed -i "s/___FLAG_FOR_BIN___/$BIN/g" build.spec
sed -i "s/___FLAG_FOR_PRODUCT___/$PRODUCT/g" boot.py

if [ ! -f "/usr/local/bin/pyinstaller" ]; then
    pip install pyinstaller
fi
if [ ! -f "/usr/bin/jmpy" ]; then
    pip install jmpy3
fi

if [ -d "project" ]; then
    rm -rf project
fi
mkdir project
cd project
# cp weights
python3 $homedir/cp_weights.py $PRODUCT
# cp components
python3 $homedir/cp_components.py $PRODUCT ./
# cp product
mkdir ./product
cp ../../../../project/product/dave_product.py ./product
cp -r ../../../../project/product/$PRODUCT ./product
cd ../

jmpy -i "project" -o "./dist" -m 0
rm -rf ./project/*
cp -r ./dist/* ./project
# cp dave_main.py
cp -r ../../../project/dave_main.py ./project
# cp public
cp -r ../../../project/public ./project

pyinstaller build.spec

if [ ! -d "../../../../../Deploy/deploy/$PRODUCT/file_system/project" ]; then
    mkdir -p ../../../../../Deploy/deploy/$PRODUCT/file_system/project
fi
cp ./dist/$PRODUCT/$BIN ../../../../../Deploy/deploy/$PRODUCT/file_system/project

cd ..

rm -rf bin_data