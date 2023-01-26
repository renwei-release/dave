#!/bin/bash

NVIDIAENABLE=$1

##### setup docket #####
if [ -f get-docker.sh ]; then
   rm -rf get-docker.sh
fi

curl -fsSL https://get.docker.com -o get-docker.sh

chmod a+x get-docker.sh

yum erase docker*

./get-docker.sh

if [ "${NVIDIAENABLE}" == "nvidia" ]; then
   ##### setup docket on nvidia #####
   # https://github.com/NVIDIA/nvidia-docker
   echo setup =================== nvidia ===================

   yum erase nvidia-container-toolkit

   distribution=$(. /etc/os-release;echo $ID$VERSION_ID)
   curl -s -L https://nvidia.github.io/nvidia-docker/$distribution/nvidia-docker.repo | sudo tee /etc/yum.repos.d/nvidia-docker.repo
   sudo yum install -y nvidia-container-toolkit
fi

sudo systemctl restart docker
sudo systemctl enable docker

##### setup docker-compose #####
VERSION=2.15.1

if [ -f /usr/local/bin/docker-compose ]; then
   rm -rf /usr/local/bin/docker-compose
fi
if [ -f /usr/bin/docker-compose ]; then
   rm -rf /usr/bin/docker-compose
fi

sudo curl -L "https://github.com/docker/compose/releases/download/v${VERSION}/docker-compose-linux-x86_64" -o /usr/local/bin/docker-compose
chmod +x /usr/local/bin/docker-compose
ln -s /usr/local/bin/docker-compose /usr/bin/docker-compose