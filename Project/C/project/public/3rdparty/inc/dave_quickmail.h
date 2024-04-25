/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __DAVE_QUICKMAIL_H__
#define __DAVE_QUICKMAIL_H__

dave_bool dave_quickmail(s8 *username, s8 *password, s8 *smtp_url, s8 *from_email, s8 *to_email, s8 *subject, s8 *body, s8 *attachment);

#endif

