/*
 * Copyright (c) 2024 Renwei
 *
 * This is a free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef __TLV_TAG_H__
#define __TLV_TAG_H__

#define TLV_TAG_START 0x00000001
#define TLV_TAG_END 0x00000002

#define TLV_TAG_TOKEN 0x00000003
#define TLV_TAG_TERMINAL_TYPE 0x00000004
#define TLV_TAG_TERMINAL_ID 0x00000005

#define TLV_TAG_DATA_BUILD 0x00010000
#define TLV_TAG_DATA_CLOSE 0x00020000
#define TLV_TAG_DATA_FORMAT 0x00030000
#define TLV_TAG_DATA_SERIAL 0x00040000
#define TLV_TAG_DATA_BODY 0x00050000

/*
 * 语音包携带规则，一个完整的语音流由4个TLV字段构成，分别是：
 * TLV_TAG_START + TLV_TAG_TOKEN + TLV_TAG_DATA_SERIAL + TLV_TAG_DATA_FORMAT + TLV_TAG_DATA_BODY + TLV_TAG_END
 *
 * 其中 TLV_TAG_START与TLV_TAG_END 是长度为 0 的TLV。
 *
 * TLV_TAG_TOKEN 用于携带本次数据流的认证数据，由UIP协议获取
 *
 * TLV_TAG_DATA_SERIAL 序号从0开始，每增加一个数据包，序号加1，最大值65535，然后回到0，依此循环。这是为保证音频等时间序列数据在接收方按序组包。
 * TLV_TAG_DATA_FORMAT 字段的value用来描述TLV_TAG_DATA_BODY携带的是什么格式的语音数据，
 * 如PCM格式，TLV_TAG_DATA_FORMAT字段携带的value是'PCM'字符串。
 *
 * TLV_TAG_DATA_BODY携带语音数据。
 */

#endif

