FROM fedora:30

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN chmod 777 /tmp

RUN yum update -y
RUN yum -y install libgomp
RUN yum install -y apr-devel apr-util libcurl libcurl-devel
RUN ln -s /usr/lib64/libaprutil-1.so.0 /usr/lib64/libaprutil-1.so

WORKDIR /
COPY ./file_system .

WORKDIR /project
CMD ["./dave-running.sh"]