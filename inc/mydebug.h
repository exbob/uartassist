#ifndef __MYLOG_H__
#define __MYLOG_H__

#include <stdio.h>

#define pr_error(fmt, ...) printf("Error : " fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...) printf("Info : " fmt, ##__VA_ARGS__)
/* 定义 __DEBUG__ 时，打印调试信息 */
#ifdef __DEBUG__
#define pr_debug(fmt, ...)                                                                         \
	printf("[%s:%04d]%s() : " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#else
#define pr_debug(fmt, ...)
#endif

#endif