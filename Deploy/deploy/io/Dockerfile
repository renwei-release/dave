FROM ubuntu:24.04

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get -o Acquire::Retries=0 update && \
    DEBIAN_FRONTEND=noninteractive apt-get -o Acquire::Retries=0 install -y \
        --only-upgrade apt libapt-pkg6.0t64 && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
	ca-certificates \
	openssl \
	libcurl4t64 \
	libbrotli1 \
	libgomp1 \
	procps \
	libcrypt1 \
	nginx \
	&& rm -rf /var/lib/apt/lists/*

RUN useradd -r -s /sbin/nologin nginx
RUN mkdir -p /var/cache/nginx && chown nginx:nginx /var/cache/nginx

WORKDIR /
COPY ./file_system .

WORKDIR /project
ENTRYPOINT ["./dave-running.sh"]