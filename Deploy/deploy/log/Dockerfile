FROM centos:centos7.9.2009

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN chmod 777 /tmp

RUN yum update -y
RUN yum install -y libgomp
RUN yum install -y python3

RUN mkdir /dave
WORKDIR /project
COPY dave-running.sh .
CMD ["./dave-running.sh"]