FROM golang:1.15.0

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update
RUN apt install -y python3
RUN apt install -y openssl
RUN apt install -y libssl-dev
RUN apt install -y vim

ENV GOPROXY=https://goproxy.io,direct

RUN mkdir -p /project/DAVE

WORKDIR /project