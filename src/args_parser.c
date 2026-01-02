/*
Copyright (C) 2025 Lishaocheng <https://shaocheng.li>

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License version 3 as published by the
Free Software Foundation.
*/

#include "args_parser.h"
#include "Config.h"
#include "mydebug.h"
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_DEVICE "/dev/ttyAMA0"
#define DEFAULT_BAUD 115200
#define DEFAULT_DATA_BIT 8
#define DEFAULT_PARITY 'N'
#define DEFAULT_STOP_BIT 1
#define DEFAULT_SEND_STRING "123456"
#define DEFAULT_SEND_INTERVAL 1000
#define DEFAULT_SEND_COUNT 0
#define DEFAULT_FORMAT OUTPUT_ASCII

static const struct option long_options[] = {{"device", required_argument, 0, 'd'},
                                             {"baud", required_argument, 0, 'b'},
                                             {"config", required_argument, 0, 'c'},
                                             {"mode", required_argument, 0, 'm'},
                                             {"send", required_argument, 0, 's'},
                                             {"interval", required_argument, 0, 'i'},
                                             {"count", required_argument, 0, 'n'},
                                             {"format", required_argument, 0, 'f'},
                                             {"file", required_argument, 0, 'F'},
                                             {"help", no_argument, 0, 'h'},
                                             {0, 0, 0, 0}};

int parse_uart_config(const char *str, int *data_bit, char *parity, int *stop_bit)
{
	int len;
	int d, s;
	char p;

	if (str == NULL || data_bit == NULL || parity == NULL || stop_bit == NULL) {
		errno = EINVAL;
		return -1;
	}

	len = strlen(str);
	if (len < 3 || len > 4) {
		pr_error("Invalid uart config format: %s (should be like 8N1)\n", str);
		return -1;
	}

	/* 解析数据位 */
	d = str[0] - '0';
	if (d < 5 || d > 8) {
		pr_error("Invalid data bit: %d (should be 5-8)\n", d);
		return -1;
	}

	/* 解析校验位 */
	p = toupper(str[1]);
	if (p != 'N' && p != 'E' && p != 'O') {
		pr_error("Invalid parity: %c (should be N/E/O)\n", p);
		return -1;
	}

	/* 解析停止位 */
	if (len == 3) {
		s = str[2] - '0';
	} else {
		/* 可能是 8N12 格式，停止位是两位数 */
		s = (str[2] - '0') * 10 + (str[3] - '0');
	}

	if (s != 1 && s != 2) {
		pr_error("Invalid stop bit: %d (should be 1 or 2)\n", s);
		return -1;
	}

	*data_bit = d;
	*parity = p;
	*stop_bit = s;

	pr_debug("Parsed uart config: %d%c%d\n", *data_bit, *parity, *stop_bit);

	return 0;
}

void print_usage(const char *program_name)
{
	printf("%s version %s\n", PROJECT, VERSION);
	printf("Build Time: %s\n", BUILD_TIME);
	if (strlen(GIT_HASH) > 0) {
		printf("Git Commit: %s, Date: %s\n", GIT_HASH, GIT_DATE);
	}
	printf("\n");
	printf("Usage: %s [OPTIONS]\n", program_name);
	printf("\n");
	printf("Common Options (all modes):\n");
	printf("  -m, --mode <mode>          Working mode: "
	       "loopback/send/recv/file (required)\n");
	printf("  -d, --device <device>       Serial port device (default: %s)\n", DEFAULT_DEVICE);
	printf("  -b, --baud <baudrate>      Baud rate (default: %d)\n", DEFAULT_BAUD);
	printf("  -c, --config <config>      UART config, format: databits "
	       "parity stopbits (default: %d%c%d)\n",
	       DEFAULT_DATA_BIT, DEFAULT_PARITY, DEFAULT_STOP_BIT);
	printf("                            Examples: 8N1, 7E1, 8O2\n");
	printf("  -h, --help                 Show this help message\n");
	printf("\n");
	printf("Loopback Mode Options:\n");
	printf("  -s, --send <string>        Send string for loopback test "
	       "(default: %s)\n",
	       DEFAULT_SEND_STRING);
	printf("  -f, --format <format>       Send format: ascii/hex (default: "
	       "ascii)\n");
	printf("                            If hex, string is parsed as hex "
	       "(e.g., af37126b4A = 5 bytes)\n");
	printf("\n");
	printf("Send Mode Options:\n");
	printf("  -s, --send <string>        Send string (default: %s)\n", DEFAULT_SEND_STRING);
	printf("  -i, --interval <ms>        Send interval in milliseconds, "
	       "1-10000 (default: %d)\n",
	       DEFAULT_SEND_INTERVAL);
	printf("  -n, --count <count>        Send count, 0 means infinite "
	       "(default: %d)\n",
	       DEFAULT_SEND_COUNT);
	printf("  -f, --format <format>       Send format: ascii/hex (default: "
	       "ascii)\n");
	printf("                            If hex, string is parsed as hex "
	       "(e.g., af37126b4A = 5 bytes)\n");
	printf("\n");
	printf("Receive Mode Options:\n");
	printf("  -f, --format <format>      Output format: ascii/hex "
	       "(default: ascii)\n");
	printf("\n");
	printf("File Mode Options:\n");
	printf("  -F, --file <json file>     JSON configuration file "
	       "(required)\n");
	printf("\n");
	printf("Examples:\n");
	printf("  %s -m loopback -d /dev/ttyUSB0 -s \"Hello\"\n", program_name);
	printf("  %s -m loopback -d /dev/ttyUSB0 -s \"af37126b4A\" -f hex\n", program_name);
	printf("  %s -m send -d /dev/ttyUSB0 -s \"Hello\" -i 500 -n 10\n", program_name);
	printf("  %s -m send -d /dev/ttyUSB0 -s \"af37126b4A\" -f hex -i 1000\n", program_name);
	printf("  %s -m recv -d /dev/ttyUSB0 -f hex\n", program_name);
}

int parse_args(int argc, char *argv[], uart_config_t *config)
{
	int opt;
	int option_index = 0;
	char *endptr;
	int data_bit = DEFAULT_DATA_BIT;
	char parity = DEFAULT_PARITY;
	int stop_bit = DEFAULT_STOP_BIT;
	int mode_set = 0;

	if (config == NULL) {
		errno = EINVAL;
		return -1;
	}

	/* 初始化默认值 */
	config->device = NULL;
	config->baud = DEFAULT_BAUD;
	config->data_bit = DEFAULT_DATA_BIT;
	config->parity = DEFAULT_PARITY;
	config->stop_bit = DEFAULT_STOP_BIT;
	config->mode = MODE_LOOPBACK;
	config->send_string = NULL;
	config->send_interval = DEFAULT_SEND_INTERVAL;
	config->send_count = DEFAULT_SEND_COUNT;
	config->format = DEFAULT_FORMAT;
	config->json_file = NULL;

	while ((opt = getopt_long(argc, argv, "d:b:c:m:s:i:n:f:F:h", long_options,
	                          &option_index)) != -1) {
		switch (opt) {
		case 'd':
			config->device = strdup(optarg);
			if (config->device == NULL) {
				pr_error("Failed to allocate memory for device "
				         "name\n");
				return -1;
			}
			break;

		case 'b':
			config->baud = (int)strtol(optarg, &endptr, 10);
			if (*endptr != '\0' || config->baud <= 0) {
				pr_error("Invalid baud rate: %s\n", optarg);
				return -1;
			}
			break;

		case 'c':
			if (parse_uart_config(optarg, &data_bit, &parity, &stop_bit) < 0) {
				return -1;
			}
			config->data_bit = data_bit;
			config->parity = parity;
			config->stop_bit = stop_bit;
			break;

		case 'm':
			if (strcmp(optarg, "loopback") == 0) {
				config->mode = MODE_LOOPBACK;
			} else if (strcmp(optarg, "send") == 0) {
				config->mode = MODE_SEND;
			} else if (strcmp(optarg, "recv") == 0) {
				config->mode = MODE_RECV;
			} else if (strcmp(optarg, "file") == 0) {
				config->mode = MODE_FILE;
			} else {
				pr_error("Invalid mode: %s (should be "
				         "loopback/send/recv/file)\n",
				         optarg);
				return -1;
			}
			mode_set = 1;
			break;

		case 's':
			config->send_string = strdup(optarg);
			if (config->send_string == NULL) {
				pr_error("Failed to allocate memory for send "
				         "string\n");
				return -1;
			}
			break;

		case 'i':
			config->send_interval = (int)strtol(optarg, &endptr, 10);
			if (*endptr != '\0' || config->send_interval < 1 ||
			    config->send_interval > 10000) {
				pr_error("Invalid interval: %s (should be "
				         "1-10000)\n",
				         optarg);
				return -1;
			}
			break;

		case 'n':
			config->send_count = (int)strtol(optarg, &endptr, 10);
			if (*endptr != '\0' || config->send_count < 0) {
				pr_error("Invalid count: %s (should be >= 0)\n", optarg);
				return -1;
			}
			break;

		case 'f':
			if (strcmp(optarg, "ascii") == 0) {
				config->format = OUTPUT_ASCII;
			} else if (strcmp(optarg, "hex") == 0) {
				config->format = OUTPUT_HEX;
			} else {
				pr_error("Invalid format: %s (should be "
				         "ascii/hex)\n",
				         optarg);
				return -1;
			}
			break;

		case 'F':
			config->json_file = strdup(optarg);
			if (config->json_file == NULL) {
				pr_error("Failed to allocate memory for JSON "
				         "file name\n");
				return -1;
			}
			break;

		case 'h':
			print_usage(argv[0]);
			return 1; /* 特殊返回值，表示显示帮助后退出 */

		default:
			print_usage(argv[0]);
			return -1;
		}
	}

	/* 检查必需参数 */
	if (!mode_set) {
		pr_error("Mode is required (-m loopback/send/recv/file)\n");
		print_usage(argv[0]);
		return -1;
	}

	/* 检查 file 模式是否需要 json_file */
	if (config->mode == MODE_FILE && config->json_file == NULL) {
		pr_error("JSON file is required for file mode (-F <json file>)\n");
		print_usage(argv[0]);
		return -1;
	}

	/* 设置默认设备名 */
	if (config->device == NULL) {
		config->device = strdup(DEFAULT_DEVICE);
		if (config->device == NULL) {
			pr_error("Failed to allocate memory for device name\n");
			return -1;
		}
	}

	/* 设置默认发送字符串 */
	if (config->send_string == NULL) {
		config->send_string = strdup(DEFAULT_SEND_STRING);
		if (config->send_string == NULL) {
			pr_error("Failed to allocate memory for send string\n");
			return -1;
		}
	}

	return 0;
}

void free_config(uart_config_t *config)
{
	if (config == NULL)
		return;

	if (config->device)
		free(config->device);

	if (config->send_string)
		free(config->send_string);

	if (config->json_file)
		free(config->json_file);
}
