FROM ubuntu:18.04

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update -y
RUN apt install -y build-essential
RUN apt install -y python3.7 wget unzip git
RUN ln -s /usr/bin/python3.7 /usr/bin/python
RUN ln -s /usr/bin/python3.7 /usr/bin/python3
RUN apt install -y libjpeg-dev libpng-dev libtiff-dev libopenexr-dev
RUN wget --no-check-certificate https://github.com/bazelbuild/bazel/releases/download/0.25.2/bazel-0.25.2-installer-linux-x86_64.sh && chmod +x bazel-0.25.2-installer-linux-x86_64.sh && ./bazel-0.25.2-installer-linux-x86_64.sh

WORKDIR /
COPY ./file_system .

WORKDIR /project/DAVE/ThirdParty/install/setup-boost
RUN setup-boost.sh

WORKDIR /project