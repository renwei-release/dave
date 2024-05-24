FROM golang:1.21.9

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update
RUN apt install -y python3 openssl libssl-dev vim

ENV GOPROXY=https://goproxy.io,direct

RUN mkdir -p /project/DAVE

WORKDIR /project