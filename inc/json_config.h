/*
Copyright (C) 2025 Lishaocheng <https://shaocheng.li>

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License version 3 as published by the
Free Software Foundation.
*/

#ifndef __JSON_CONFIG_H__
#define __JSON_CONFIG_H__

typedef struct {
	int number;     /* 标签号 */
	char *hex_data; /* HEX数据字符串 */
	int delay;      /* 延时（毫秒） */
	int enable;     /* 是否启用 */
} send_item_t;

typedef struct {
	char *group_name;       /* 配置组名称 */
	int cycle_count;        /* 循环次数 */
	send_item_t *send_list; /* 发送列表数组 */
	int send_list_count;    /* 发送列表元素个数 */
} json_config_t;

/*
 * 解析JSON配置文件
 * 参数: filename - JSON文件路径
 * 返回: 配置结构体指针，失败返回NULL
 */
json_config_t *parse_json_file(const char *filename);

/*
 * 验证JSON配置的有效性
 * 参数: config - 配置结构体
 * 返回: 0 成功, -1 失败
 */
int validate_json_config(json_config_t *config);

/*
 * 释放JSON配置结构体的内存
 * 参数: config - 配置结构体
 */
void free_json_config(json_config_t *config);

#endif /* __JSON_CONFIG_H__ */
