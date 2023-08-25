FROM fedora:30

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN chmod 777 /tmp

RUN yum update -y
RUN yum install -y libgomp

WORKDIR /
COPY ./file_system .

WORKDIR /project
CMD ["./dave-running.sh"]