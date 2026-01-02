/*
Copyright (C) 2025 Lishaocheng <https://shaocheng.li>

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License version 3 as published by the
Free Software Foundation.
*/

#include "args_parser.h"
#include "mydebug.h"
#include "uart_assist.h"
#include "uartdev.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 全局运行标志，用于信号处理 */
volatile int g_running = 1;

/* 信号处理函数 */
static void signal_handler(int sig)
{
	if (sig == SIGINT) {
		pr_info("\nReceived SIGINT (Ctrl+C), exiting...\n");
		g_running = 0;
	}
}

int main(int argc, char *argv[])
{
	uart_config_t config;
	uartdev_t *dev = NULL;
	int ret = 0;

	/* 注册信号处理 */
	signal(SIGINT, signal_handler);

	/* 解析命令行参数 */
	ret = parse_args(argc, argv, &config);
	if (ret < 0) {
		/* 解析失败 */
		return EXIT_FAILURE;
	} else if (ret > 0) {
		/* 显示帮助信息 */
		free_config(&config);
		return EXIT_SUCCESS;
	}

	/* 创建串口设备 */
	dev = uartdev_new(config.device, config.baud, config.data_bit, config.parity,
	                  config.stop_bit);
	if (dev == NULL) {
		pr_error("Failed to create uart device: %s\n", strerror(errno));
		free_config(&config);
		return EXIT_FAILURE;
	}

	/* 打开并配置串口 */
	if (uartdev_setup(dev) < 0) {
		pr_error("Failed to setup uart device: %s\n", strerror(errno));
		uartdev_del(dev);
		free_config(&config);
		return EXIT_FAILURE;
	}

	pr_info("UART device opened: %s, %d, %d%c%d\n", config.device, config.baud, config.data_bit,
	        config.parity, config.stop_bit);

	/* 根据模式执行测试 */
	switch (config.mode) {
	case MODE_LOOPBACK:
		ret = uart_loopback_test(dev, config.send_string, config.format);
		break;

	case MODE_SEND:
		ret = uart_send_test(dev, config.send_string, config.send_interval,
		                     config.send_count, config.format);
		break;

	case MODE_RECV:
		ret = uart_recv_test(dev, config.format);
		break;

	case MODE_FILE:
		ret = uart_file_test(dev, config.json_file);
		break;

	default:
		pr_error("Unknown mode\n");
		ret = -1;
		break;
	}

	/* 清理资源 */
	uartdev_del(dev);
	free_config(&config);

	if (ret < 0) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
