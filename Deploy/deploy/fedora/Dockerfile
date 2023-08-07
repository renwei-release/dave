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
RUN yum install -y python3 python3-devel
RUN yum install -y libstdc++ libstdc++-devel libstdc++-static
RUN yum install -y flex
RUN yum install -y bison
RUN yum install -y boost boost-devel
RUN yum install -y openssl openssl-devel
RUN yum install -y protobuf protobuf-devel grpc grpc-devel grpc-plugins
RUN yum install -y cpprest cpprest-devel
RUN yum install -y java-openjdk
RUN yum install -y rust cargo
RUN yum install -y golang
RUN ln -s /usr/local/go/bin/go /usr/local/bin/go
RUN yum install -y scala sbt
RUN yum install -y bzip2

RUN mkdir -p /project/DAVE

ENV GOPROXY=https://goproxy.io

WORKDIR /project