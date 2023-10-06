FROM golang:1.19.2

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update
RUN apt install -y vim
RUN apt install -y bash-completion

ENV GOPROXY=https://goproxy.io,direct

WORKDIR /
COPY ./file_system .

WORKDIR /project