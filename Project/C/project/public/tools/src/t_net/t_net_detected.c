/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "dave_tools.h"
#include "dave_os.h"
#include "tools_log.h"

static dave_bool
_net_detected(s8 *hostname, ub port)
{
	int sockfd;
	struct sockaddr_in servaddr;
	struct hostent *server;
	dave_bool ret;

	server = gethostbyname(hostname);
	if (server == NULL) {
		TOOLSLOG("hostname:%s port:%d gethostbyname failed!", hostname, port);
		return dave_false;
	}

	bzero((char *)&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&servaddr.sin_addr.s_addr, server->h_length);
	servaddr.sin_port = htons(port);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		TOOLSLOG("hostname:%s port:%d open socket failed!", hostname, port);
		return dave_false;
	}

	struct timeval tv;
	tv.tv_sec  = 1;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

	if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
		TOOLSLOG("hostname:%s port:%d open connecting failed!", hostname, port);
		ret = dave_false;
	}
	else {
		TOOLSDEBUG("hostname:%s port:%d open connecting success!", hostname, port);
		ret = dave_true;
	}

	close(sockfd);
	return ret;
}

// =====================================================================

dave_bool
t_net_detected_host_port(s8 *hostname, ub port)
{
	return _net_detected(hostname, port);
}

dave_bool
t_net_detected_ip_port(u8 ip[4], ub port)
{
	s8 hostname[128];

	dave_snprintf(hostname, sizeof(hostname), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);

	return t_net_detected_host_port(hostname, port);
}

