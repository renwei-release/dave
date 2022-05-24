FROM centos:centos7.9.2009

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN yum update -y
RUN yum groups mark convert
RUN yum install -y vim
RUN yum install -y libgomp
RUN yum groupinstall -y "Development Tools"
RUN yum install -y python3

RUN mkdir -p /project/DAVE
WORKDIR /project
COPY dave-running.sh .
CMD ["./dave-running.sh"]