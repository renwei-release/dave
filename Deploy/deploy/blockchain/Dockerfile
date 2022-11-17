FROM golang:1.19.2

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

ENV GOPROXY=https://goproxy.io,direct

RUN apt update
RUN apt install -y vim

WORKDIR /
COPY ./file_system .

WORKDIR /project
CMD ["./dave-running.sh"]