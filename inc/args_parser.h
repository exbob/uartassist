/*
Copyright (C) 2025 Lishaocheng <https://shaocheng.li>

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License version 3 as published by the
Free Software Foundation.
*/

#ifndef __ARGS_PARSER_H__
#define __ARGS_PARSER_H__

#include <stdint.h>

typedef enum {
	MODE_LOOPBACK, /* 自测模式 */
	MODE_SEND,     /* 发送模式 */
	MODE_RECV,     /* 接收模式 */
	MODE_FILE      /* 文件模式 */
} test_mode_t;

typedef enum {
	OUTPUT_ASCII, /* ASCII打印 */
	OUTPUT_HEX    /* 16进制打印 */
} output_format_t;

typedef struct {
	char *device;           /* 串口设备名 */
	int baud;               /* 波特率 */
	int data_bit;           /* 数据位 */
	char parity;            /* 校验位 */
	int stop_bit;           /* 停止位 */
	test_mode_t mode;       /* 工作模式 */
	char *send_string;      /* 发送字符串 */
	int send_interval;      /* 发送间隔（毫秒） */
	int send_count;         /* 发送次数（0=无限） */
	output_format_t format; /* 接收打印格式 */
	char *json_file;        /* JSON配置文件（file模式） */
} uart_config_t;

/*
 * 解析命令行参数
 * 参数: argc, argv - 命令行参数
 *       config - 输出配置结构体
 * 返回: 0 成功, -1 失败
 */
int parse_args(int argc, char *argv[], uart_config_t *config);

/*
 * 解析串口参数字符串（如 "8N1"）
 * 参数: str - 参数字符串
 *       data_bit - 输出数据位
 *       parity - 输出校验位
 *       stop_bit - 输出停止位
 * 返回: 0 成功, -1 失败
 */
int parse_uart_config(const char *str, int *data_bit, char *parity, int *stop_bit);

/*
 * 打印使用说明
 */
void print_usage(const char *program_name);

/*
 * 释放配置结构体中的动态分配内存
 */
void free_config(uart_config_t *config);

#endif /* __ARGS_PARSER_H__ */
