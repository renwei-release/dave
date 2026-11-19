FROM ubuntu:20.04

ENV TZ=Asia/Shanghai
RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN chmod 777 /tmp

RUN apt-get update && \
	DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
	ca-certificates \
	tzdata \
	libgomp1 \
	mariadb-client \
	postgresql-client \
	libpq5 \
	libmariadb3 && \
	rm -rf /var/lib/apt/lists/*

WORKDIR /
COPY ./file_system .

WORKDIR /project
CMD ["./dave-running.sh"]