/*
Copyright (C) 2025 Lishaocheng <https://shaocheng.li>

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License version 3 as published by the
Free Software Foundation.
*/

#include "uart_test.h"
#include "mydebug.h"
#include <ctype.h>
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

extern volatile int g_running; /* 全局运行标志，由信号处理设置 */

int parse_hex_string(const char *hex_str, char *buf, int buf_len)
{
	int i, j;
	int len;
	char c1, c2;
	unsigned char byte;

	if (hex_str == NULL || buf == NULL || buf_len <= 0) {
		errno = EINVAL;
		return -1;
	}

	len = strlen(hex_str);
	if (len == 0 || len % 2 != 0) {
		pr_error("Invalid hex string length: %d (must be even)\n", len);
		return -1;
	}

	if (len / 2 > buf_len) {
		pr_error("Hex string too long: %d bytes (max: %d)\n", len / 2,
		         buf_len);
		return -1;
	}

	for (i = 0, j = 0; i < len; i += 2, j++) {
		c1 = hex_str[i];
		c2 = hex_str[i + 1];

		/* 转换为小写并验证 */
		c1 = tolower(c1);
		c2 = tolower(c2);

		if (!((c1 >= '0' && c1 <= '9') || (c1 >= 'a' && c1 <= 'f'))) {
			pr_error("Invalid hex character at position %d: %c\n", i, c1);
			return -1;
		}
		if (!((c2 >= '0' && c2 <= '9') || (c2 >= 'a' && c2 <= 'f'))) {
			pr_error("Invalid hex character at position %d: %c\n", i + 1,
			         c2);
			return -1;
		}

		/* 转换为字节 */
		if (c1 >= '0' && c1 <= '9') {
			byte = (c1 - '0') << 4;
		} else {
			byte = (c1 - 'a' + 10) << 4;
		}

		if (c2 >= '0' && c2 <= '9') {
			byte |= (c2 - '0');
		} else {
			byte |= (c2 - 'a' + 10);
		}

		buf[j] = (char)byte;
	}

	return j;
}

void print_timestamp(void)
{
	struct timeval tv;
	struct tm *tm_info;
	char time_str[64];

	gettimeofday(&tv, NULL);
	tm_info = localtime(&tv.tv_sec);

	strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
	printf("[%s.%03ld] ", time_str, tv.tv_usec / 1000);
}

void print_ascii(const char *buf, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		if (buf[i] >= 32 && buf[i] < 127) {
			/* 可打印字符 */
			printf("%c", buf[i]);
		} else {
			/* 控制字符，显示为转义序列 */
			switch (buf[i]) {
			case '\n':
				printf("\\n");
				break;
			case '\r':
				printf("\\r");
				break;
			case '\t':
				printf("\\t");
				break;
			case '\0':
				printf("\\0");
				break;
			default:
				printf("\\x%02X", (unsigned char)buf[i]);
				break;
			}
		}
	}
}

void print_hex(const char *buf, int len)
{
	int i;
	for (i = 0; i < len; i++) {
		printf("%02X ", (unsigned char)buf[i]);
		if ((i + 1) % 16 == 0) {
			printf("\n");
		}
	}
	if (len % 16 != 0) {
		printf("\n");
	}
}

int uart_recv_with_timeout(uartdev_t *dev, char *buf, int len, int timeout_sec)
{
	struct pollfd pfd;
	int ret;
	int nread = 0;

	if (dev == NULL || buf == NULL || len <= 0 || dev->fd < 0) {
		errno = EINVAL;
		return -1;
	}

	pfd.fd = dev->fd;
	pfd.events = POLLIN;

	/* 使用 poll 实现超时 */
	ret = poll(&pfd, 1, timeout_sec * 1000);
	if (ret < 0) {
		/* 如果被信号中断（如 Ctrl+C），检查 g_running 标志 */
		if (errno == EINTR) {
			/* 信号中断，返回 0 表示没有数据（程序可能正在退出） */
			return 0;
		}
		pr_error("poll() failed: %s\n", strerror(errno));
		return -1;
	} else if (ret == 0) {
		/* 超时 */
		return 0;
	}

	/* 有数据可读 */
	if (pfd.revents & POLLIN) {
		nread = uartdev_recv(dev, buf, len);
		if (nread < 0) {
			pr_error("uartdev_recv() failed: %s\n", strerror(errno));
			return -1;
		}
	}

	return nread;
}

int uart_loopback_test(uartdev_t *dev, const char *send_str,
                       output_format_t format)
{
	char recv_buf[1024];
	char send_buf[512];
	int recv_len;
	int total_recv = 0;
	const char *send_data;
	int send_data_len;

	if (dev == NULL || send_str == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (format == OUTPUT_HEX) {
		/* 解析hex字符串 */
		send_data_len = parse_hex_string(send_str, send_buf, sizeof(send_buf));
		if (send_data_len < 0) {
			return -1;
		}
		send_data = send_buf;
		pr_info("Loopback test: sending hex \"%s\" (%d bytes)\n", send_str,
		        send_data_len);
	} else {
		/* ASCII格式 */
		send_data_len = strlen(send_str);
		if (send_data_len == 0) {
			pr_error("Send string is empty\n");
			return -1;
		}
		send_data = send_str;
		pr_info("Loopback test: sending \"%s\" (%d bytes)\n", send_str,
		        send_data_len);
	}

	/* 清空缓冲区 */
	uartdev_flush(dev);

	/* 发送数据 */
	if (uartdev_send(dev, send_data, send_data_len) != send_data_len) {
		pr_error("Failed to send data: %s\n", strerror(errno));
		return -1;
	}

	pr_info("Waiting for received data (timeout: %d seconds)...\n",
	        RECV_TIMEOUT_SEC);

	/* 接收数据（带超时） */
	recv_len = uart_recv_with_timeout(dev, recv_buf, sizeof(recv_buf) - 1,
	                                  RECV_TIMEOUT_SEC);
	if (recv_len < 0) {
		pr_error("Failed to receive data: %s\n", strerror(errno));
		return -1;
	} else if (recv_len == 0) {
		pr_error("Receive timeout after %d seconds\n", RECV_TIMEOUT_SEC);
		return -1;
	}

	total_recv += recv_len;
	recv_buf[recv_len] = '\0';

	/* 比较发送和接收的数据 */
	if (recv_len != send_data_len) {
		pr_error("Data length mismatch: sent %d bytes, received %d bytes\n",
		         send_data_len, recv_len);
		if (format == OUTPUT_HEX) {
			pr_info("Sent (hex): \"%s\"\n", send_str);
		} else {
			pr_info("Sent: \"%s\"\n", send_str);
		}
		pr_info("Received: \"%s\"\n", recv_buf);
		return -1;
	}

	if (memcmp(send_data, recv_buf, send_data_len) != 0) {
		pr_error("Data mismatch!\n");
		if (format == OUTPUT_HEX) {
			pr_info("Sent (hex): \"%s\"\n", send_str);
		} else {
			pr_info("Sent: \"%s\"\n", send_str);
		}
		pr_info("Received: \"%s\"\n", recv_buf);
		return -1;
	}

	pr_info("Loopback test PASSED: sent and received %d bytes match\n",
	        send_data_len);
	return 0;
}

int uart_send_test(uartdev_t *dev, const char *send_str, int interval_ms,
                   int count, output_format_t format)
{
	char send_buf[512];
	int i = 0;
	int sent_bytes = 0;
	const char *send_data;
	int send_data_len;

	if (dev == NULL || send_str == NULL) {
		errno = EINVAL;
		return -1;
	}

	if (format == OUTPUT_HEX) {
		/* 解析hex字符串 */
		send_data_len = parse_hex_string(send_str, send_buf, sizeof(send_buf));
		if (send_data_len < 0) {
			return -1;
		}
		send_data = send_buf;
		if (count == 0) {
			pr_info("Send test: hex=\"%s\" (%d bytes), interval=%d ms, count=infinite\n",
			        send_str, send_data_len, interval_ms);
		} else {
			pr_info("Send test: hex=\"%s\" (%d bytes), interval=%d ms, count=%d\n",
			        send_str, send_data_len, interval_ms, count);
		}
	} else {
		/* ASCII格式 */
		send_data_len = strlen(send_str);
		if (send_data_len == 0) {
			pr_error("Send string is empty\n");
			return -1;
		}
		send_data = send_str;
		if (count == 0) {
			pr_info("Send test: string=\"%s\" (%d bytes), interval=%d ms, count=infinite\n",
			        send_str, send_data_len, interval_ms);
		} else {
			pr_info("Send test: string=\"%s\" (%d bytes), interval=%d ms, count=%d\n",
			        send_str, send_data_len, interval_ms, count);
		}
	}

	/* 清空缓冲区 */
	uartdev_flush(dev);

	while (g_running) {
		/* 发送数据 */
		if (uartdev_send(dev, send_data, send_data_len) != send_data_len) {
			pr_error("Failed to send data: %s\n", strerror(errno));
			return -1;
		}

		sent_bytes += send_data_len;
		i++;

		if (format == OUTPUT_HEX) {
			printf("Send [%d] : hex=\"%s\" (%d bytes, total: %d bytes)\n", i,
			       send_str, send_data_len, sent_bytes);
		} else {
			printf("Send [%d] : \"%s\" (%d bytes, total: %d bytes)\n", i,
			       send_str, send_data_len, sent_bytes);
		}

		/* 检查发送次数 */
		if (count > 0 && i >= count) {
			break;
		}

		/* 延时 */
		usleep(interval_ms * 1000);
	}

	pr_info("Send test completed: sent %d times, total %d bytes\n", i,
	        sent_bytes);
	return 0;
}

int uart_recv_test(uartdev_t *dev, output_format_t format)
{
	char recv_buf[256];
	int recv_len;
	int total_bytes = 0;
	int packet_count = 0;

	if (dev == NULL) {
		errno = EINVAL;
		return -1;
	}

	pr_info("Receive test: format=%s, timeout=%d seconds\n",
	        format == OUTPUT_ASCII ? "ASCII" : "HEX", RECV_TIMEOUT_SEC);

	/* 清空缓冲区 */
	uartdev_flush(dev);

	while (g_running) {
		/* 接收数据（带超时） */
		recv_len = uart_recv_with_timeout(dev, recv_buf,
		                                  sizeof(recv_buf) - 1,
		                                  RECV_TIMEOUT_SEC);
		if (recv_len < 0) {
			pr_error("Failed to receive data: %s\n", strerror(errno));
			return -1;
		} else if (recv_len == 0) {
			/* 超时或被信号中断，检查是否需要退出 */
			if (!g_running) {
				/* 收到退出信号，退出循环 */
				break;
			}
			/* 超时，继续等待 */
			pr_info("Receive timeout (%d seconds), waiting for data...\n",
			        RECV_TIMEOUT_SEC);
			continue;
		}

		total_bytes += recv_len;
		packet_count++;

		/* 打印统计信息和数据 */
		if (format == OUTPUT_ASCII) {
			/* 为ASCII格式，先打印数据，然后显示统计信息 */
			printf("Recv [%d] : \"", packet_count);
			print_ascii(recv_buf, recv_len);
			printf("\" (%d bytes, total: %d bytes)\n", recv_len,
			       total_bytes);
		} else {
			/* HEX格式，先显示统计信息，然后打印hex数据 */
			printf("Recv [%d] : (%d bytes, total: %d bytes)\n",
			       packet_count, recv_len, total_bytes);
			print_hex(recv_buf, recv_len);
		}
	}

	pr_info("Receive test completed: received %d packets, total %d bytes\n",
	        packet_count, total_bytes);
	return 0;
}

