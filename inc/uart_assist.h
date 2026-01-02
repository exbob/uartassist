/*
Copyright (C) 2025 Lishaocheng <https://shaocheng.li>

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License version 3 as published by the
Free Software Foundation.
*/

#ifndef __UART_ASSIST_H__
#define __UART_ASSIST_H__

#include "args_parser.h"
#include "uartdev.h"

#define RECV_TIMEOUT_SEC 2 /* 接收超时时间（秒） */

/*
 * 解析hex字符串并转换为字节数组
 * 参数: hex_str - hex字符串（如 "af37126b4A"）
 *       buf - 输出缓冲区
 *       buf_len - 缓冲区长度
 * 返回: 解析的字节数，失败返回-1
 */
int parse_hex_string(const char *hex_str, char *buf, int buf_len);

/*
 * 自测模式：自发自收
 * 参数: dev - 串口设备
 *       send_str - 发送字符串
 *       format - 发送格式（ASCII/HEX）
 * 返回: 0 成功, -1 失败
 */
int uart_loopback_test(uartdev_t *dev, const char *send_str, output_format_t format);

/*
 * 发送模式：按间隔和次数发送数据
 * 参数: dev - 串口设备
 *       send_str - 发送字符串
 *       interval_ms - 发送间隔（毫秒）
 *       count - 发送次数（0=无限）
 *       format - 发送格式（ASCII/HEX）
 * 返回: 0 成功, -1 失败
 */
int uart_send_test(uartdev_t *dev, const char *send_str, int interval_ms, int count,
                   output_format_t format);

/*
 * 接收模式：持续接收并打印数据
 * 参数: dev - 串口设备
 *       format - 打印格式（ASCII/HEX）
 * 返回: 0 成功, -1 失败
 */
int uart_recv_test(uartdev_t *dev, output_format_t format);

/*
 * 文件模式：根据JSON配置文件发送数据
 * 参数: dev - 串口设备
 *       json_file - JSON配置文件路径
 * 返回: 0 成功, -1 失败
 */
int uart_file_test(uartdev_t *dev, const char *json_file);

/*
 * 带超时的接收数据
 * 参数: dev - 串口设备
 *       buf - 接收缓冲区
 *       len - 缓冲区长度
 *       timeout_sec - 超时时间（秒）
 * 返回: 接收的字节数，超时返回0，错误返回-1
 */
int uart_recv_with_timeout(uartdev_t *dev, char *buf, int len, int timeout_sec);

/*
 * 打印时间戳
 */
void print_timestamp(void);

/*
 * 按ASCII格式打印数据
 */
void print_ascii(const char *buf, int len);

/*
 * 按16进制格式打印数据
 */
void print_hex(const char *buf, int len);

#endif /* __UART_ASSIST_H__ */
