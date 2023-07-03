/**
 * @file log.h
 * @author melektron
 * @brief automatic logging tags for esp log
 * @version 0.1
 * @date 2023-07-03
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#include <esp_log.h>

#define LOGE(format, ... ) ESP_LOGE(__FILENAME__, format, ##__VA_ARGS__)
#define LOGW(format, ... ) ESP_LOGW(__FILENAME__, format, ##__VA_ARGS__)
#define LOGI(format, ... ) ESP_LOGI(__FILENAME__, format, ##__VA_ARGS__)
#define LOGD(format, ... ) ESP_LOGD(__FILENAME__, format, ##__VA_ARGS__)
#define LOGV(format, ... ) ESP_LOGV(__FILENAME__, format, ##__VA_ARGS__)