FROM fedora:30

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN yum update -y
RUN yum groupinstall -y "Development Tools"
RUN yum install -y python3-devel
RUN yum install -y vim
RUN yum install -y procps
RUN yum install -y wget

WORKDIR /
COPY ./file_system .

WORKDIR /project
CMD ["./dave-running.sh"]