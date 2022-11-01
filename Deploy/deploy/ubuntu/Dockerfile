FROM ubuntu:18.04

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt update -y
RUN apt install -y build-essential
RUN apt install -y software-properties-common
RUN apt install -y python3.7
RUN apt install -y wget
RUN apt install -y libjpeg-dev
RUN apt install -y libpng-dev
RUN apt install -y libtiff-dev
RUN apt install -y libopenexr-dev

RUN mkdir -p /project/DAVE

WORKDIR /project
COPY dave-running.sh .
CMD ["./dave-running.sh"]