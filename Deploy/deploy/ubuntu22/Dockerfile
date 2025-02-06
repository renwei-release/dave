FROM ubuntu:22.04

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update -y
RUN apt install -y build-essential
RUN apt install -y python3 python3-pip wget unzip git cmake vim
RUN ln -s /usr/bin/python3 /usr/bin/python
RUN apt install -y libjpeg-dev libpng-dev libtiff-dev libopenexr-dev libssl-dev
RUN apt install -y software-properties-common 
#RUN wget --no-check-certificate https://github.com/bazelbuild/bazel/releases/download/0.25.2/bazel-0.25.2-installer-linux-x86_64.sh && chmod +x bazel-0.25.2-installer-linux-x86_64.sh && ./bazel-0.25.2-installer-linux-x86_64.sh

RUN pip install opencv-python==4.10.0.84 imageio==2.34.2 ultralytics==8.2.64 torch==2.3.1 imageio[ffmpeg] imageio[pyav]
RUN pip install filterpy==1.4.5 lap==0.4.0 tensorrt==10.2.0.post1

WORKDIR /
COPY ./file_system .

WORKDIR /project