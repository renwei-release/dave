FROM fedora:30

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN yum update -y
RUN yum groupinstall -y "Development Tools"
RUN yum install -y findutils
RUN yum install -y vim
RUN yum install -y libgomp
RUN yum install -y gcc-c++.x86_64
RUN yum install -y cmake
RUN yum install -y python3
RUN yum install -y golang.x86_64

RUN mkdir -p /project/DAVE

WORKDIR /project
COPY dave-running.sh .
CMD ["./dave-running.sh"]