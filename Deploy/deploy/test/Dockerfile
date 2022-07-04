FROM jenkins/jenkins:latest

USER root

RUN apt update
RUN apt install -y sudo
RUN apt install -y vim
RUN apt install -y procps
RUN apt install -y python3
RUN apt install -y pip

WORKDIR /

COPY ./file_system .

RUN mkdir /dave
WORKDIR /project