/**
 * @file utils.hpp
 * @author melektron
 * @brief utility functions and definitions
 * @version 0.1
 * @date 2023-07-04
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#pragma once

#define msleep(__milliseconds) usleep((__milliseconds) * 1000)

// Source: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/FreeRTOS.cpp
#define ms_since_boot() (xTaskGetTickCount() * portTICK_PERIOD_MS)