FROM fedora:28

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN yum update -y
RUN yum install -y vim
RUN yum install -y libgomp
RUN yum install -y python3
RUN yum groupinstall -y "Development Tools"
RUN yum install -y gcc-c++.x86_64
RUN yum install -y kfind.x86_64
RUN yum install -y cmake

RUN mkdir -p /project/DAVE

WORKDIR /project
COPY dave-running.sh .
CMD ["./dave-running.sh"]