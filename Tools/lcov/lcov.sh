#!/bin/bash

#
# Check if lcov is installed
#
if [ $(type lcov) == "" ]; then
   ./build-lcov
fi

#
# Setup public parameters
#
homepath=$(cd `dirname $0`; pwd)
product=$1
if [ "$product" == "" ]; then
   product="base"
fi
PRODUCTDIR=$homepath/../../Project/C/build/linux/$product
LCOVFILEDIR=$homepath/lcov_file/$product
File=$(basename $0)

#
echo
echo ${File} Make target
echo
#
cd ${PRODUCTDIR}
./clean
./build profiling nodocker

#
echo
echo ${File} Build work directory
echo
#
cd $homepath
rm -rf ${LCOVFILEDIR}
mkdir -p ${LCOVFILEDIR}
cp ${PRODUCTDIR}/${product^^}-BIN ${LCOVFILEDIR}

#
echo
echo ${File} Collect operation information
echo
#
cd ${LCOVFILEDIR}
GCNO=$(find $homepath/../../Project/C/project -name '*.gcno')
mv $GCNO ./

#
echo
echo ${File} Setup the working environment
echo
#
cd ${LCOVFILEDIR}
lcov -d ./ -z
lcov -c -i -d ./ -o init.info

#
echo
echo ${File} Run the ${product}
echo
#
cd ${LCOVFILEDIR}
sudo ./${product^^}-BIN

#
echo
echo ${File} Collect operation information
echo
#
cd ${LCOVFILEDIR}
GCDA=$(find $homepath/../../Project/C/project -name '*.gcda')
mv $GCDA ./

#
echo
echo ${File} Construct code run coverage results
echo
#
cd ${LCOVFILEDIR}
lcov -c -d ./ -o cover.info
lcov -a init.info -a cover.info -o total.info
lcov --remove total.info '*/usr/include/*' '*/usr/lib/*' '*/usr/lib64/*' '*/usr/local/include/*' '*/usr/local/lib/*' '*/usr/local/lib64/*' -o final.info
genhtml -o report --legend --title "lcov"  --prefix=./ final.info
gprof ./${product^^}-BIN gmon.out > ./report/pg.txt