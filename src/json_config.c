/*
Copyright (C) 2025 Lishaocheng <https://shaocheng.li>

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License version 3 as published by the
Free Software Foundation.
*/

#include "json_config.h"
#include "../third_party/cjson/cJSON.h"
#include "mydebug.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

json_config_t *parse_json_file(const char *filename)
{
	FILE *fp;
	long file_size;
	char *json_string;
	cJSON *json, *item, *send_list, *send_item;
	int i;
	json_config_t *config;
	send_item_t *send_items;

	if (filename == NULL) {
		errno = EINVAL;
		return NULL;
	}

	/* 打开文件 */
	fp = fopen(filename, "r");
	if (fp == NULL) {
		pr_error("Failed to open JSON file: %s\n", filename);
		return NULL;
	}

	/* 获取文件大小 */
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (file_size <= 0) {
		pr_error("JSON file is empty: %s\n", filename);
		fclose(fp);
		return NULL;
	}

	/* 读取文件内容 */
	json_string = (char *)malloc(file_size + 1);
	if (json_string == NULL) {
		pr_error("Failed to allocate memory for JSON string\n");
		fclose(fp);
		return NULL;
	}

	if (fread(json_string, 1, file_size, fp) != (size_t)file_size) {
		pr_error("Failed to read JSON file: %s\n", filename);
		free(json_string);
		fclose(fp);
		return NULL;
	}
	json_string[file_size] = '\0';
	fclose(fp);

	/* 解析JSON */
	json = cJSON_Parse(json_string);
	free(json_string);

	if (json == NULL) {
		const char *error_ptr = cJSON_GetErrorPtr();
		if (error_ptr != NULL) {
			pr_error("JSON parse error before: %s\n", error_ptr);
		}
		return NULL;
	}

	/* 分配配置结构体 */
	config = (json_config_t *)calloc(1, sizeof(json_config_t));
	if (config == NULL) {
		pr_error("Failed to allocate memory for config\n");
		cJSON_Delete(json);
		return NULL;
	}

	/* 解析 GroupName */
	item = cJSON_GetObjectItemCaseSensitive(json, "GroupName");
	if (cJSON_IsString(item) && item->valuestring != NULL) {
		config->group_name = strdup(item->valuestring);
		if (config->group_name == NULL) {
			pr_error("Failed to allocate memory for group name\n");
			cJSON_Delete(json);
			free(config);
			return NULL;
		}
	} else {
		pr_error("GroupName is missing or invalid\n");
		cJSON_Delete(json);
		free(config);
		return NULL;
	}

	/* 解析 CycleCount */
	item = cJSON_GetObjectItemCaseSensitive(json, "CycleCount");
	if (cJSON_IsNumber(item)) {
		config->cycle_count = item->valueint;
	} else {
		pr_error("CycleCount is missing or invalid\n");
		cJSON_Delete(json);
		free(config->group_name);
		free(config);
		return NULL;
	}

	/* 解析 SendList */
	send_list = cJSON_GetObjectItemCaseSensitive(json, "SendList");
	if (!cJSON_IsArray(send_list)) {
		pr_error("SendList is missing or not an array\n");
		cJSON_Delete(json);
		free(config->group_name);
		free(config);
		return NULL;
	}

	config->send_list_count = cJSON_GetArraySize(send_list);
	if (config->send_list_count == 0) {
		pr_error("SendList is empty\n");
		cJSON_Delete(json);
		free(config->group_name);
		free(config);
		return NULL;
	}

	/* 分配发送列表数组 */
	send_items = (send_item_t *)calloc(config->send_list_count, sizeof(send_item_t));
	if (send_items == NULL) {
		pr_error("Failed to allocate memory for send list\n");
		cJSON_Delete(json);
		free(config->group_name);
		free(config);
		return NULL;
	}
	config->send_list = send_items;

	/* 解析每个发送项 */
	for (i = 0; i < config->send_list_count; i++) {
		send_item = cJSON_GetArrayItem(send_list, i);
		if (!cJSON_IsObject(send_item)) {
			pr_error("SendList[%d] is not an object\n", i);
			cJSON_Delete(json);
			free_json_config(config);
			return NULL;
		}

		/* 解析 Number */
		item = cJSON_GetObjectItemCaseSensitive(send_item, "Number");
		if (cJSON_IsNumber(item)) {
			send_items[i].number = item->valueint;
		} else {
			pr_error("SendList[%d].Number is missing or invalid\n", i);
			cJSON_Delete(json);
			free_json_config(config);
			return NULL;
		}

		/* 解析 HexData */
		item = cJSON_GetObjectItemCaseSensitive(send_item, "HexData");
		if (cJSON_IsString(item) && item->valuestring != NULL) {
			send_items[i].hex_data = strdup(item->valuestring);
			if (send_items[i].hex_data == NULL) {
				pr_error("Failed to allocate memory for HexData\n");
				cJSON_Delete(json);
				free_json_config(config);
				return NULL;
			}
		} else {
			pr_error("SendList[%d].HexData is missing or invalid\n", i);
			cJSON_Delete(json);
			free_json_config(config);
			return NULL;
		}

		/* 解析 Delay */
		item = cJSON_GetObjectItemCaseSensitive(send_item, "Delay");
		if (cJSON_IsNumber(item)) {
			send_items[i].delay = item->valueint;
		} else {
			pr_error("SendList[%d].Delay is missing or invalid\n", i);
			cJSON_Delete(json);
			free_json_config(config);
			return NULL;
		}

		/* 解析 Enable */
		item = cJSON_GetObjectItemCaseSensitive(send_item, "Enable");
		if (cJSON_IsNumber(item)) {
			send_items[i].enable = item->valueint;
		} else {
			pr_error("SendList[%d].Enable is missing or invalid\n", i);
			cJSON_Delete(json);
			free_json_config(config);
			return NULL;
		}
	}

	cJSON_Delete(json);
	return config;
}

int validate_json_config(json_config_t *config)
{
	int i;
	int len;

	if (config == NULL) {
		errno = EINVAL;
		return -1;
	}

	/* 验证 CycleCount */
	if (config->cycle_count < 1) {
		pr_error("CycleCount must be >= 1\n");
		return -1;
	}

	/* 验证 SendList */
	if (config->send_list == NULL || config->send_list_count == 0) {
		pr_error("SendList is empty\n");
		return -1;
	}

	/* 验证每个发送项 */
	for (i = 0; i < config->send_list_count; i++) {
		/* 验证 Delay 范围 */
		if (config->send_list[i].delay < 1 || config->send_list[i].delay > 1000) {
			pr_error("SendList[%d].Delay must be 1-1000, got %d\n", i,
			         config->send_list[i].delay);
			return -1;
		}

		/* 验证 HexData 格式 */
		if (config->send_list[i].hex_data == NULL) {
			pr_error("SendList[%d].HexData is NULL\n", i);
			return -1;
		}

		len = strlen(config->send_list[i].hex_data);
		if (len == 0) {
			pr_error("SendList[%d].HexData is empty\n", i);
			return -1;
		}

		if (len % 2 != 0) {
			pr_error("SendList[%d].HexData length must be even, "
			         "got %d\n",
			         i, len);
			return -1;
		}
	}

	return 0;
}

void free_json_config(json_config_t *config)
{
	int i;

	if (config == NULL)
		return;

	if (config->group_name)
		free(config->group_name);

	if (config->send_list) {
		for (i = 0; i < config->send_list_count; i++) {
			if (config->send_list[i].hex_data)
				free(config->send_list[i].hex_data);
		}
		free(config->send_list);
	}

	free(config);
}
