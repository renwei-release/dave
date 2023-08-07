FROM gcc:9

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt list --upgradable
RUN apt update
RUN apt install -y cmake

WORKDIR /project