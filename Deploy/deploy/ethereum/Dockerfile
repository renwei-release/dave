FROM fedora:30

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN yum update -y
RUN yum install -y vim
RUN yum install -y procps

WORKDIR /
COPY ./file_system .

WORKDIR /project
COPY dave-running.sh .
CMD ["./dave-running.sh"]