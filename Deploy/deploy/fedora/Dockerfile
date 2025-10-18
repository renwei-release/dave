FROM fedora:30

ENV TZ=Asia/Shanghai \
	GOPROXY=https://goproxy.io

RUN set -eux; \
	ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone; \
	dnf -y update; \
	(dnf -y group install "Development Tools" || dnf -y groupinstall "Development Tools" || dnf -y install @development-tools); \
	dnf -y install \
		libtool vim wget gcc-c++ libstdc++ libstdc++-devel cmake \
		python3 python3-devel python3-pip \
		flex bison boost boost-devel \
		openssl openssl-devel protobuf protobuf-devel \
		grpc grpc-devel grpc-plugins \
		cpprest cpprest-devel \
		rust golang meson libwebsockets-devel findutils \
		bzip2 libicu apr-devel apr-util libcurl libcurl-devel \
		--exclude=openh264* --setopt=install_weak_deps=False || true; \
	ln -sf /usr/lib64/libaprutil-1.so.0 /usr/lib64/libaprutil-1.so || true; \
	python3 -m pip install --upgrade pip setuptools wheel || true; \
	python3 -m pip config set global.index-url https://pypi.tuna.tsinghua.edu.cn/simple || true; \
	dnf clean all; rm -rf /var/cache/dnf /var/cache/yum ~/.cache/pip; \
	mkdir -p /project/DAVE

WORKDIR /
COPY ./file_system .

WORKDIR /project