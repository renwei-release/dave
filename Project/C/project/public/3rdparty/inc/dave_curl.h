/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef __DAVE_CURL_H__
#define __DAVE_CURL_H__

dave_bool dave_curl_email(s8 *username, s8 *password, s8 *smtp_url, s8 *from_email, s8 *to_email, s8 *subject, s8 *body);

#endif

