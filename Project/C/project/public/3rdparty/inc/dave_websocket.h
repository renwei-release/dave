/*
 * Copyright (c) 2025 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */
#ifndef __DAVE_WEBSOCKET_H__
#define __DAVE_WEBSOCKET_H__

typedef void (* websocket_plugin)(void *wsi);
typedef void (* websocket_plugout)(void *wsi);
typedef void (* websocket_read)(void *wsi, s8 *data_ptr, ub data_len);

void dave_websocket_init(websocket_plugin plugin, websocket_plugout plugout, websocket_read data_read);
void dave_websocket_exit(void);
void dave_websocket_data_write(void *wsl, s8 *data_ptr, ub data_len);

#endif

