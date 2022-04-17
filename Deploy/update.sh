#!/bin/bash
#/*
# * Copyright (c) 2022 Renwei
# *
# * This is a free software; you can redistribute it and/or modify
# * it under the terms of the MIT license. See LICENSE for details.
# */

HOMEPATH=$1
PROJECT=$2
PROJECTNAME=$3
JUPYTERPORT=$4
PROJECTMAPPING=$5
HOMEPRJDIR=${HOMEPATH}/../../project
HOMEBUILDDIR=${HOMEPATH}/../../build

if [ ! -d ${HOMEPRJDIR} ]; then
   HOMEPRJDIR=${HOMEPATH}/../../../project
fi
if [ ! -d ${HOMEBUILDDIR} ]; then
   HOMEBUILDDIR=${HOMEPATH}/../../../build
fi

############## copy_project_file function ##############
copy_python_project_to_container()
{
   if [ -d ${PRJPYFILE} ]; then
      echo update.sh copy ${PRJPYFILE} to ${PROJECTNAME}

      docker exec -it ${PROJECTNAME} mkdir -p /project/public
      docker exec -it ${PROJECTNAME} mkdir -p /project/product

      docker cp ${PRJPYFILE}/dave_main.py ${PROJECTNAME}:/project
      docker cp ${PRJPYFILE}/public/__init__.py ${PROJECTNAME}:/project/public

      docker cp ${PRJPYFILE}/public/base ${PROJECTNAME}:/project/public
      docker cp ${PRJPYFILE}/public/tools ${PROJECTNAME}:/project/public

      if [ -d ${PRJPYFILE}/product/${PROJECT} ]; then
         docker cp ${PRJPYFILE}/product/dave_product.py ${PROJECTNAME}:/project/product
         docker cp ${PRJPYFILE}/product/${PROJECT} ${PROJECTNAME}:/project/product
      fi
   fi
}

copy_python_project_file()
{
   PRJPYFILE=${HOMEPRJDIR}

   copy_python_project_to_container
}

copy_ai_project_to_container()
{
   if [ -d ${PRJAIFILE} ]; then
      echo update.sh copy ${PRJAIFILE} to ${PROJECTNAME}:/project/public
      docker exec -it ${PROJECTNAME} mkdir -p /project/public
      docker cp ${PRJAIFILE} ${PROJECTNAME}:/project/public
   fi
}

copy_ai_project_file()
{
   if [ ${PROJECT} == "aesthetics" ] \
      || [ ${PROJECT} == "aip" ] \
      || [ ${PROJECT} == "bagword" ] \
      || [ ${PROJECT} == "sculptures" ] \
      || [ ${PROJECT} == "style" ]; then
      PRJAIFILE=${HOMEPRJDIR}/public/neural_network

      copy_ai_project_to_container
   fi
}

copy_bin_project_container()
{
   if [ -f ${BINFILE} ]; then
      echo update.sh copy ${BINFILE} to ${PROJECTNAME}:/project ...
      chmod a+x ${BINFILE}
      docker cp ${BINFILE} ${PROJECTNAME}:/project
   fi
}

backup_bin_project_file()
{
   if [ ${BINFILE} != ${DEPLOYBINFILE} ]; then
      echo update.sh backup ${BINFILE} to ${DEPLOYBINFILE}
      cp -r ${BINFILE} ${DEPLOYBINFILE}
   fi
}

copy_bin_project_file()
{
   DEPLOYBINFILE=$(cd `dirname $0`; pwd)/deploy/${PROJECT}/${PROJECT^^}-BIN
   BINFILE=${HOMEBUILDDIR}/linux/${PROJECT}/${PROJECT^^}-BIN
   if [ ! -f ${BINFILE} ]; then
      BINFILE=${HOMEBUILDDIR}/${PROJECT}/${PROJECT^^}-BIN
   fi
   if [ ! -f ${BINFILE} ]; then
      BINFILE=${DEPLOYBINFILE}
   fi

   copy_bin_project_container
   backup_bin_project_file
}

copy_project_file()
{
   if [ -f ${HOMEPRJDIR}/dave_main.py ]; then
      copy_python_project_file
      copy_ai_project_file
   else
      copy_bin_project_file
   fi
}
############## copy_project_file function ##############

############## copy_sh_file function ##############
__copy_sh_file__()
{
   if [ -f ${SHFILE} ]; then   
      chmod a+x $SHFILE

      SHCMDLINE=`cat -n $SHFILE | grep '___FLAG_FOR_UPDATE.SH___' | awk '{print $1}'`
      if [ "$SHCMDLINE" == "" ]; then
         SHCMDLINE=`cat -n $SHFILE | grep 'python ./dave_main.py' | awk '{print $1}'`
      fi
      if [ "$SHCMDLINE" == "" ]; then
         SHCMDLINE=`cat -n $SHFILE | grep '-BIN' | awk '{print $1}'`
      fi

      if [ "$SHCMDLINE" != "" ]; then
         if [ -f ${HOMEPRJDIR}/dave_main.py ]; then
            echo update.sh config $SHFILE to Python mode!
            sed -i "${SHCMDLINE}c python3 ./dave_main.py ${PROJECT}" $SHFILE
         else
            echo update.sh config $SHFILE to C or Go mode!
            sed -i "${SHCMDLINE}c ./${PROJECT^^}-BIN" $SHFILE
         fi

         if [ ! "$PROJECTMAPPING" != "" ]; then
            echo update.sh copy $SHFILE to ${PROJECTNAME}/project
            docker cp $SHFILE ${PROJECTNAME}:/project
         fi

         sed -i "${SHCMDLINE}c ___FLAG_FOR_UPDATE.SH___" $SHFILE
      else
         echo update.sh empty $SHFILE !!!
      fi
   fi
}

copy_sh_file()
{
   # running file
   SHFILE=${HOMEPRJDIR}/dave-running.sh
   __copy_sh_file__

   # debug file
   SHFILE=${HOMEPRJDIR}/dave-debug.sh
   __copy_sh_file__
}
############## copy_sh_file function ##############

############## modify_project_attributes function ##############
modify_project_attributes()
{
   echo update.sh chown -R root:root /project
   docker exec -it ${PROJECTNAME} bash -c "cd / && chown -R root:root /dave && chown -R root:root /project"
}
############## modify_project_attributes function ##############

if [ ! "$PROJECTMAPPING" != "" ]; then
   copy_project_file
fi

copy_sh_file

if [ ! "$PROJECTMAPPING" != "" ]; then
   modify_project_attributes
fi

if [ -f jupyter.sh ]; then
   ./jupyter.sh ${PROJECTNAME} ${JUPYTERPORT}
fi

echo update.sh restart ${PROJECTNAME} user:${USER} project:${PROJECT} ...
docker restart ${PROJECTNAME}