FROM fedora:30

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN yum update -y
RUN yum groupinstall -y "Development Tools"
RUN yum install -y libtool
RUN yum install -y findutils
RUN yum install -y vim
RUN yum install -y wget
RUN yum install -y libgomp
RUN yum install -y gcc-c++
RUN yum install -y cmake
RUN yum install -y python3
RUN yum install -y python3-devel
RUN yum install -y golang
RUN yum install -y libstdc++
RUN yum install -y libstdc++-devel
RUN yum install -y libstdc++-static
RUN yum install -y flex
RUN yum install -y bison
RUN yum install -y boost
RUN yum install -y boost-devel
RUN yum install -y openssl
RUN yum install -y openssl-devel
RUN yum install -y protobuf
RUN yum install -y protobuf-devel
RUN yum install -y grpc
RUN yum install -y grpc-devel
RUN yum install -y grpc-plugins
RUN yum install -y cpprest
RUN yum install -y cpprest-devel
RUN yum install -y java-openjdk

RUN mkdir -p /project/DAVE

ENV GOPROXY=https://goproxy.io

WORKDIR /project