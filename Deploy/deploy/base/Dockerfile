FROM fedora:latest

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN chmod 777 /tmp

RUN yum update -y
RUN yum install -y vim
RUN yum install -y libgomp
RUN yum install -y python3

WORKDIR /
COPY ./file_system .

WORKDIR /project
CMD ["./dave-running.sh"]