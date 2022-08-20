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

HOMEPRJDIR=${HOMEPATH}/project
DEPLOYMODE='operation'
if [ ! -d ${HOMEPRJDIR} ]; then
   HOMEPRJDIR=${HOMEPATH}/../../project
   DEPLOYMODE='develop'
fi
if [ ! -d ${HOMEPRJDIR} ]; then
   HOMEPRJDIR=${HOMEPATH}/../../../project
   DEPLOYMODE='develop'
fi

echo -e "update.sh deploy mode:\033[35m${DEPLOYMODE}\033[0m"

############## copy_project_file function ##############
copy_python_project_to_container()
{
   if [ -d ${HOMEPRJDIR} ]; then
      echo update.sh copy ${HOMEPRJDIR} to ${PROJECTNAME}

      chmod a+x rm-project.sh
      docker cp rm-project.sh ${PROJECTNAME}:/
      docker exec -it ${PROJECTNAME} sh -c "cd / && ./rm-project.sh && rm -rf rm-project.sh"

      docker cp ${HOMEPRJDIR}/dave_main.py ${PROJECTNAME}:/project
      docker cp ${HOMEPRJDIR}/components ${PROJECTNAME}:/project
      docker cp ${HOMEPRJDIR}/public ${PROJECTNAME}:/project

      if [ -d ${HOMEPRJDIR}/product/${PROJECT} ]; then
         docker exec -it ${PROJECTNAME} mkdir -p /project/product
         docker cp ${HOMEPRJDIR}/product/dave_product.py ${PROJECTNAME}:/project/product
         docker cp ${HOMEPRJDIR}/product/${PROJECT} ${PROJECTNAME}:/project/product
      fi
   fi
}

backup_python_project_from_container()
{
   if [ ${DEPLOYMODE} == 'develop' ]; then
      DEPLOYPRJDIR=${HOMEPATH}/../../../../Deploy/deploy/${PROJECT}
      echo update.sh backup project from ${PROJECTNAME} to ${DEPLOYPRJDIR}
      if [ -d ${DEPLOYPRJDIR}/project ]; then
         rm -rf ${DEPLOYPRJDIR}/project
      fi
      mkdir -p ${DEPLOYPRJDIR}/project
      docker cp ${PROJECTNAME}:/project ${DEPLOYPRJDIR}
   fi
}

copy_python_project_file()
{
   copy_python_project_to_container
   backup_python_project_from_container
}

copy_bin_project_file()
{
   PRJBINFILE=$(cd `dirname $0`; pwd)/deploy/${PROJECT}/project/${PROJECT^^}-BIN

   if [ -f ${PRJBINFILE} ]; then
      echo update.sh copy ${PRJBINFILE} to ${PROJECTNAME}:/project ...
      chmod a+x ${PRJBINFILE}
      docker cp ${PRJBINFILE} ${PROJECTNAME}:/project
      docker exec -it ${PROJECTNAME} sh -c "chmod a+x /project/${PROJECT^^}-BIN"
   fi
}

copy_project_file()
{
   if [ -f ${HOMEPRJDIR}/dave_main.py ]; then
      copy_python_project_file
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

      SHCMDLINE=`cat -n $SHFILE | grep 'goto_debug #___FLAG_FOR_UPDATE.SH___' | awk '{print $1}'`
      if [ "$SHCMDLINE" == "" ]; then
         SHCMDLINE=`cat -n $SHFILE | grep 'python3 ./dave_main.py' | awk '{print $1}'`
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

         echo update.sh copy $SHFILE to ${PROJECTNAME}/project
         docker cp $SHFILE ${PROJECTNAME}:/project

         sed -i "${SHCMDLINE}c goto_debug #___FLAG_FOR_UPDATE.SH___" $SHFILE
      else
         echo update.sh empty grep data from $SHFILE !!!
      fi
   else
      echo update.sh where is $SHFILE ???
   fi
}

copy_sh_file()
{
   # running file
   SHFILE=${HOMEPATH}/../../../../../Deploy/dave-running.sh
   if [ ! -f ${SHFILE} ]; then
      SHFILE=${HOMEPATH}/../../../../Deploy/dave-running.sh
   fi
   if [ ! -f ${SHFILE} ]; then
      SHFILE=${HOMEPATH}/../../../Deploy/dave-running.sh
   fi
   __copy_sh_file__

   # debug file
   SHFILE=${HOMEPATH}/../../../../../Deploy/dave-debug.sh
   if [ ! -f ${SHFILE} ]; then
      SHFILE=${HOMEPATH}/../../../../Deploy/dave-debug.sh
   fi
   if [ ! -f ${SHFILE} ]; then
      SHFILE=${HOMEPATH}/../../../Deploy/dave-debug.sh
   fi
   __copy_sh_file__
}
############## copy_sh_file function ##############

############## modify_project_attributes function ##############
modify_project_attributes()
{
   echo update.sh chown -R root:root /project
   docker exec -it ${PROJECTNAME} sh -c "cd / && chown -R root:root /project"
}
############## modify_project_attributes function ##############

if [ ! "$PROJECTMAPPING" != "" ]; then
   copy_project_file
   copy_sh_file
   modify_project_attributes
else
   copy_sh_file
fi

if [ -f jupyter.sh ]; then
   ./jupyter.sh ${PROJECTNAME} ${JUPYTERPORT}
fi

./release.sh ${PROJECTNAME} ${PROJECTMAPPING}