FROM ubuntu:22.04

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update -y

RUN apt install -y software-properties-common
RUN add-apt-repository -y ppa:deadsnakes/ppa
RUN apt update -y
RUN apt install -y build-essential python3 python3-dev python3-venv python3-pip wget unzip git cmake vim
RUN apt install -y libjpeg-dev libpng-dev libtiff-dev libopenexr-dev libssl-dev

WORKDIR /
COPY ./file_system .

WORKDIR /pip/jmpy
RUN python3 -m pip uninstall jmpy3
RUN python3 -m pip install -e .
RUN python3 -m pip install pyinstaller

WORKDIR /project