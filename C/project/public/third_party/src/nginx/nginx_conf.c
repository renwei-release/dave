/*
 * Copyright (c) 2022 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include "third_party_macro.h"
#if defined(NGINX_3RDPARTY)
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h> 
#include <netinet/in.h>
#include <time.h> 
#include <dirent.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <dlfcn.h>
#include "dave_os.h"
#include "dave_tools.h"
#include "dave_base.h"
#include "dave_verno.h"
#include "dave_http.h"
#include "nginx_conf.h"
#include "party_log.h"

#define NGINX_CONF_MAX (16384)
#define NGINX_SERVER_CONF_FLAG (s8 *)"#########"
#define NGINX_CONF_NAME (s8 *)"/dave/tools/nginx/conf/nginx.conf"

static dave_bool
_nginx_conf_write(s8 *name, s8 *conf, ub conf_len)
{
	dave_bool ret;

	if(dave_os_file_delete(DIRECT_FLAG, name) == dave_false)
	{
		PARTYLOG("name:%s conf:%s write failed!", name, conf);
	}

	ret = dave_os_file_write(DIRECT_FLAG|CREAT_WRITE_FLAG, name, 0, conf_len, (u8 *)conf);
	if(ret == dave_false)
	{
		PARTYLOG("name:%s conf:%s write failed!", name, conf);
	}

	return ret;
}

static ub
_nginx_conf_read(s8 *name, s8 *conf, ub conf_len)
{
	return dave_os_file_read(DIRECT_FLAG|READ_FLAG, name, 0, conf_len, (u8 *)conf);
}

static s8 *
_nginx_cpu_affinity(ub work_process)
{
	s8 *affinity_str;

	switch(work_process)
	{
		case 1:
				affinity_str = (s8 *)"01";
			break;
		case 2:
				affinity_str = (s8 *)"0101 1010";
			break;
		case 4:
				affinity_str = (s8 *)"0001 0010 0100 1000";
			break;
		case 8:
				affinity_str = (s8 *)"00000001 00000010 00000100 00001000 00010000 00100000 01000000 10000000";
			break;
		default:
				affinity_str = (s8 *)"auto";
			break;
	}

	return affinity_str;
}

static ub
_nginx_conf_head(s8 *conf, ub conf_len, ub work_process)
{
	ub conf_index;
	DateStruct date;

	dave_timer_get_date(&date);

	conf_index = 0;

	conf_index += dave_sprintf(&conf[conf_index], "# nginx.conf build date : %s \n", t_a2b_date_str(&date));
	conf_index += dave_sprintf(&conf[conf_index], "\n");

	// http://nginx.org/en/docs/ngx_core_module.html#worker_cpu_affinity
	conf_index += dave_sprintf(&conf[conf_index], "worker_processes %d;\n", work_process);
	conf_index += dave_sprintf(&conf[conf_index], "worker_cpu_affinity %s;\n", _nginx_cpu_affinity(work_process));
	conf_index += dave_sprintf(&conf[conf_index], "\n");

	conf_index += dave_sprintf(&conf[conf_index], "events {\n");
	conf_index += dave_sprintf(&conf[conf_index], "    worker_connections  %d;\n", work_process*65535);
	conf_index += dave_sprintf(&conf[conf_index], "}\n");
	conf_index += dave_sprintf(&conf[conf_index], "\n");

	conf_index += dave_sprintf(&conf[conf_index], "http {\n");

	conf_index += dave_sprintf(&conf[conf_index], "    access_log nul;\n");
	conf_index += dave_sprintf(&conf[conf_index], "    include		mime.types;\n");
	conf_index += dave_sprintf(&conf[conf_index], "    default_type  application/octet-stream;\n");
	conf_index += dave_sprintf(&conf[conf_index], "    sendfile        on;\n");
	conf_index += dave_sprintf(&conf[conf_index], "    keepalive_timeout  %d;\n", 240);
	conf_index += dave_sprintf(&conf[conf_index], "    client_header_timeout  %d;\n", 60);
	conf_index += dave_sprintf(&conf[conf_index], "    client_body_timeout   %d;\n", 60);

	conf_index += dave_sprintf(&conf[conf_index], "    client_header_buffer_size   %dk;\n", 64);
	conf_index += dave_sprintf(&conf[conf_index], "    large_client_header_buffers  %d %dk;\n", 4, 64);
	conf_index += dave_sprintf(&conf[conf_index], "    client_body_buffer_size   %dm;\n", 20);
	conf_index += dave_sprintf(&conf[conf_index], "    client_max_body_size   %dm;\n", 16);
	conf_index += dave_sprintf(&conf[conf_index], "    fastcgi_buffer_size   %dk;\n", 128);
	conf_index += dave_sprintf(&conf[conf_index], "    fastcgi_buffers  %d %dk;\n", 4, 128);
	conf_index += dave_sprintf(&conf[conf_index], "    fastcgi_busy_buffers_size   %dk;\n", 256);
	conf_index += dave_sprintf(&conf[conf_index], "    gzip_buffers  %d %dk;\n", 16, 8);
	conf_index += dave_sprintf(&conf[conf_index], "    proxy_buffer_size   %dk;\n", 64);
	conf_index += dave_sprintf(&conf[conf_index], "    proxy_buffers  %d %dk;\n", 4, 128);
	conf_index += dave_sprintf(&conf[conf_index], "    proxy_busy_buffers_size   %dk;\n", 256);
	
	conf_index += dave_sprintf(&conf[conf_index], "    fastcgi_connect_timeout   %d;\n", 60);
	conf_index += dave_sprintf(&conf[conf_index], "    fastcgi_send_timeout   %d;\n", 60);
	conf_index += dave_sprintf(&conf[conf_index], "    fastcgi_read_timeout   %d;\n", 60);
	
	conf_index += dave_sprintf(&conf[conf_index], "    proxy_connect_timeout   %ds;\n", 60);
	conf_index += dave_sprintf(&conf[conf_index], "    proxy_send_timeout   %d;\n", 60);
	conf_index += dave_sprintf(&conf[conf_index], "    proxy_read_timeout   %d;\n", 60);

	return conf_index;
}

static ub
_nginx_conf_end(s8 *conf, ub conf_len)
{
	ub conf_index;

	conf_index = 0;

	conf_index += dave_sprintf(&conf[conf_index], "}\n");
	conf_index += dave_sprintf(&conf[conf_index], "\n");

	return conf_index;
}

static ub
_nginx_conf_write_allow_cross_domain_cfg(s8 *conf, ub conf_len)
{
	ub conf_index;

	conf_index = 0;

	conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 	   add_header Access-Control-Allow-Credentials true;\n");
	conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "	   add_header Access-Control-Allow-Origin '*';\n");
	conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "	   add_header Access-Control-Max-Age 600;\n");
	conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "	   add_header Access-Control-Allow-Headers 'x-requested-with,content-type,Cache-Control,Pragma,Date,x-timestamp';\n");
	conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "	   add_header Access-Control-Allow-Methods 'POST,GET,PUT';\n");
	conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "	   add_header Access-Control-Expose-Headers 'WWW-Authenticate,Server-Authorization';\n");
	conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "	   add_header P3P 'policyref=\"/w3c/p3p.xml\", CP=\"NOI DSP PSAa OUR BUS IND ONL UNI COM NAV INT LOC\"';\n");

	return conf_index;
}

static ub
_nginx_conf_write_https_server(s8 *conf, ub conf_len, ub https_port, ub cgi_port, s8 *nginx_path, s8 *pem_path, s8 *key_path)
{
	ub conf_index;

	conf_index = 0;

	if(https_port != 0)
	{
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "%s%d%s\n", NGINX_SERVER_CONF_FLAG, https_port, NGINX_SERVER_CONF_FLAG);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "    server {\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 	   listen	%d ssl;\n", https_port);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 	   server_name	https_%d;\n", https_port);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 	   error_page   500 502 503 504 /50x.html;\n");
		conf_index += _nginx_conf_write_allow_cross_domain_cfg(&conf[conf_index], conf_len-conf_index);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 	   ssl_session_cache shared:SSL:10m;\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 	   ssl_session_timeout 10m;\n");
		if(pem_path != NULL)
		{
			conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 	   ssl_certificate %s;\n", pem_path);
			conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 	   ssl_certificate_key %s;\n", key_path);
		}
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 	   ssl_protocols TLSv1.2 TLSv1.3;\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 	   ssl_ciphers ECDHE-RSA-AES128-GCM-SHA256:ECDHE:ECDH:AES:HIGH:!NULL:!aNULL:!MD5:!ADH:!RC4;\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 	   location %s {\n", nginx_path);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 		   fastcgi_pass   127.0.0.1:%d;\n", cgi_port);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 		   fastcgi_index   index.cgi;\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 		   include	 fastcgi.conf;\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 	   }\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, " 	   access_log off;\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "    }\n\n");
	}

	return conf_index;
}

static ub
_nginx_conf_write_http_server(s8 *conf, ub conf_len, ub http_port, ub cgi_port, s8 *nginx_path)
{
	ub conf_index;

	conf_index = 0;

	if(http_port != 0)
	{
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "%s%d%s\n", NGINX_SERVER_CONF_FLAG, http_port, NGINX_SERVER_CONF_FLAG);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "    server {\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "        listen       %d;\n", http_port);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "        server_name  http_%d;\n", http_port);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "        error_page   500 502 503 504 /50x.html;\n");
		conf_index += _nginx_conf_write_allow_cross_domain_cfg(&conf[conf_index], conf_len-conf_index);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "        location %s {\n", nginx_path);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "            fastcgi_pass   127.0.0.1:%d;\n", cgi_port);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "            fastcgi_index   index.cgi;\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "            include   fastcgi.conf;\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "        }\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "        access_log off;\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "    }\n\n");
	}

	return conf_index;
}

static ub
_nginx_conf_write_web_server(s8 *conf, ub conf_len, ub web_port, ub cgi_port, s8 *nginx_path)
{
	ub conf_index;

	conf_index = 0;

	if(web_port != 0)
	{
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "%s%d%s\n", NGINX_SERVER_CONF_FLAG, web_port, NGINX_SERVER_CONF_FLAG);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "    server {\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "        listen       %d;\n", web_port);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "        server_name  web_%d;\n", web_port);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "        location / { \n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "          root   html; \n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "          index  index.html index.htm;\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "        }\n\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "        error_page   500 502 503 504 /50x.html;\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "        location %s {\n", nginx_path);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "            fastcgi_pass   127.0.0.1:%d;\n", cgi_port);
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "            fastcgi_index   index.cgi;\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "            include   fastcgi.conf;\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "        }\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "        access_log off;\n");
		conf_index += dave_snprintf(&conf[conf_index], conf_len-conf_index, "    }\n\n");
	}

	return conf_index;
}


static ub
_nginx_conf_read_server(s8 *conf, ub conf_len, s8 *server, ub server_len, ub *server_port)
{
	ub conf_index, server_index;
	ub conf_flag_len = dave_strlen(NGINX_SERVER_CONF_FLAG);
	ub front_flag, back_flag;

	server[0] = '\0';

	front_flag = back_flag =0;
	conf_index = server_index = 0;
	*server_port = 0;

	PARTYDEBUG("conf:%d/%d server:%d/%d", conf_index, conf_len, server_index, server_len);

	while(((conf_index + 8) < conf_len) && (server_index < (server_len - 1)))
	{
		if(dave_memcmp(&conf[conf_index], NGINX_SERVER_CONF_FLAG, conf_flag_len) == dave_true)
		{
			conf_index += dave_strlen(NGINX_SERVER_CONF_FLAG);
			*server_port = stringdigital(&conf[conf_index]);
			PARTYDEBUG("%d/%d:%s/%d", conf_index, conf_len, &conf[conf_index], *server_port);
			break;
		}

		conf_index ++;
	}
	if(*server_port == 0)
	{
		return conf_len;
	}

	while((conf_index < conf_len) && (server_index < (server_len - 1)))
	{
		if(conf[conf_index] == '\n')
		{
			conf_index ++;
			break;
		}
		conf_index ++;
	}

	if(conf_index >= conf_len)
	{
		return conf_len;
	}

	server_index += dave_snprintf(&server[server_index], server_len-server_index, "%s%d%s\n", NGINX_SERVER_CONF_FLAG, *server_port, NGINX_SERVER_CONF_FLAG);

	while((conf_index < conf_len) && (server_index < (server_len - 1)))
	{
		if (conf[conf_index] == 0x7B)
		{
			front_flag++;
		}
		else if (conf[conf_index] == 0x7D)
		{
			back_flag++;
		}
		
		server[server_index ++] = conf[conf_index ++];
		
		if((front_flag > 0) && (front_flag == back_flag))
		{
			conf_index += 2;

			server[server_index ++] = '\n';
			server[server_index ++] = '\n';
			break;
		}
	}

	server[server_index] = '\0';

	return conf_index;
}

// =====================================================================

dave_bool
nginx_conf_add(ub work_process, ub nginx_port, HTTPListenType type, ub cgi_port, s8 *nginx_path, s8 *pem_path, s8 *key_path)
{
	s8 *old_conf, *new_conf, *server_conf;
	ub old_conf_len, old_conf_index, new_conf_index;
	ub server_port;
	dave_bool ret;

	old_conf = dave_malloc(NGINX_CONF_MAX);
	new_conf = dave_malloc(NGINX_CONF_MAX);
	server_conf = dave_malloc(NGINX_CONF_MAX);

	old_conf_len = _nginx_conf_read(NGINX_CONF_NAME, old_conf, NGINX_CONF_MAX);

	old_conf_index = new_conf_index = 0;

	new_conf_index += _nginx_conf_head(&new_conf[new_conf_index], NGINX_CONF_MAX-new_conf_index, work_process);

	PARTYDEBUG("old:%d/%d", old_conf_index, old_conf_len);

	while(old_conf_index < old_conf_len)
	{
		old_conf_index += _nginx_conf_read_server(&old_conf[old_conf_index], old_conf_len-old_conf_index, server_conf, NGINX_CONF_MAX, &server_port);
		if(server_port == 0)
			break;

		if(server_port != nginx_port)
		{
			new_conf_index += dave_strcpy(&new_conf[new_conf_index], server_conf, NGINX_CONF_MAX-new_conf_index);
		}
	}

	switch (type){
		case ListenHttps:
				new_conf_index += _nginx_conf_write_https_server(&new_conf[new_conf_index], NGINX_CONF_MAX-new_conf_index, nginx_port, cgi_port, nginx_path, pem_path, key_path);
			break;
		case ListenHttp:
				new_conf_index += _nginx_conf_write_http_server(&new_conf[new_conf_index], NGINX_CONF_MAX-new_conf_index, nginx_port, cgi_port, nginx_path);
			break;
		case ListenWeb:
				new_conf_index += _nginx_conf_write_web_server(&new_conf[new_conf_index], NGINX_CONF_MAX-new_conf_index, nginx_port, cgi_port, nginx_path);
			break;
		default:
				PARTYABNOR("Invalid listen type[%d]", type);
			break;
	}

	new_conf_index += _nginx_conf_end(&new_conf[new_conf_index], NGINX_CONF_MAX-new_conf_index);

	ret = _nginx_conf_write(NGINX_CONF_NAME, new_conf, new_conf_index);

	dave_free(old_conf);
	dave_free(new_conf);
	dave_free(server_conf);

	return ret;
}

ub
nginx_conf_del(ub work_process, ub nginx_port)
{
	dave_bool has_modify;
	ub has_server_number;
	s8 *old_conf, *new_conf, *server_conf;
	ub old_conf_len, old_conf_index, new_conf_index;
	ub server_port;

	has_modify = dave_false;

	has_server_number = 0;

	old_conf = dave_malloc(NGINX_CONF_MAX);
	new_conf = dave_malloc(NGINX_CONF_MAX);
	server_conf = dave_malloc(NGINX_CONF_MAX);

	old_conf_len = _nginx_conf_read(NGINX_CONF_NAME, old_conf, NGINX_CONF_MAX);

	old_conf_index = new_conf_index = 0;

	new_conf_index += _nginx_conf_head(&new_conf[new_conf_index], NGINX_CONF_MAX-new_conf_index, work_process);

	while(old_conf_index < old_conf_len)
	{
		old_conf_index += _nginx_conf_read_server(&old_conf[old_conf_index], old_conf_len-old_conf_index, server_conf, NGINX_CONF_MAX, &server_port);
		if(server_port == 0)
			break;

		PARTYDEBUG("server_port:%d", server_port);		

		if(server_port != nginx_port)
		{
			new_conf_index += dave_strcpy(&new_conf[new_conf_index], server_conf, NGINX_CONF_MAX-new_conf_index);

			has_server_number ++;
		}
		else
		{
			has_modify = dave_true;
		}
	}

	new_conf_index += _nginx_conf_end(&new_conf[new_conf_index], NGINX_CONF_MAX-new_conf_index);

	if(has_modify == dave_true)
	{
		_nginx_conf_write(NGINX_CONF_NAME, new_conf, new_conf_index);
	}

	dave_free(old_conf);
	dave_free(new_conf);
	dave_free(server_conf);

	return has_server_number;
}

ub
nginx_conf_number(void)
{
	ub has_server_number;
	s8 *old_conf, *server_conf;
	ub old_conf_len, old_conf_index;
	ub server_port;

	has_server_number = 0;

	old_conf = dave_malloc(NGINX_CONF_MAX);
	server_conf = dave_malloc(NGINX_CONF_MAX);

	old_conf_len = _nginx_conf_read(NGINX_CONF_NAME, old_conf, NGINX_CONF_MAX);

	old_conf_index = 0;

	while(old_conf_index < old_conf_len)
	{
		old_conf_index += _nginx_conf_read_server(&old_conf[old_conf_index], old_conf_len-old_conf_index, server_conf, NGINX_CONF_MAX, &server_port);
		if(server_port == 0)
			break;

		PARTYDEBUG("server_port:%d", server_port);		

		has_server_number ++;
	}

	dave_free(old_conf);
	dave_free(server_conf);

	return has_server_number;
}

#endif

