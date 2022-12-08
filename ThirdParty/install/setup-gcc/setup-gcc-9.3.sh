#!/bin/sh

yum -y install centos-release-scl

yum -y install devtoolset-9-*

scl enable devtoolset-9 bash

echo "source /opt/rh/devtoolset-9/enable" >> /etc/profile
