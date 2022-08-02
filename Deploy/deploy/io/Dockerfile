FROM nginx:1.23.1

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt -y update
RUN apt -y install libgomp1
RUN apt -y install procps

RUN mkdir /dave
WORKDIR /project
COPY dave-running.sh .
ENTRYPOINT ["./dave-running.sh"]