FROM nginx:1.24.0

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt -y update
RUN apt -y install libgomp1 procps libwebsockets-dev

WORKDIR /
COPY ./file_system .

WORKDIR /project
ENTRYPOINT ["./dave-running.sh"]