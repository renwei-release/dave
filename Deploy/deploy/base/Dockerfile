FROM fedora:44

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN dnf -y update && dnf -y upgrade && dnf clean all
RUN dnf install -y vim libgomp python3
RUN dnf install -y java-21-openjdk-devel --exclude=openh264\* --setopt=install_weak_deps=False

WORKDIR /
COPY ./file_system .

WORKDIR /project
CMD ["./dave-running.sh"]