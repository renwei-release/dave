FROM nginx:latest

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN chmod 777 /tmp

RUN apt update -y
RUN apt -y install libgomp1
RUN apt -y install procps
RUN apt -y install vim

RUN mkdir /dave
WORKDIR /project
COPY dave-running.sh .
CMD ["./dave-running.sh"]