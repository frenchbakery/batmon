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

#define MIN(x, y) ((x) > (y) ? (y) : (x))
#define MAX(x, y) ((x) < (y) ? (y) : (x))

#define msleep(__milliseconds) usleep((__milliseconds) * 1000)

// Source: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/FreeRTOS.cpp
#define ms_since_boot() (xTaskGetTickCount() * portTICK_PERIOD_MS)

/**
 * @brief this macro can be used in place of msleep statements in a simple state machine
 * background task. It allows the task to immediately abort the sleep and break out of
 * a switch statement/loop when a notification is received. This is needed
 * so a task can immediately start processing a potential change of state machine state
 */
#define msleep_unless_notified(__milliseconds) {BaseType_t received = xTaskNotifyWait(0, 0, NULL, pdMS_TO_TICKS(__milliseconds)); if (received) break;}

/**
 * @brief  this macro can be used in a state machine task's state that only needs to do
 * something once and then wait forever until the task is notified.
 */
#define wait_until_notified() xTaskNotifyWait(0, 0, NULL, portMAX_DELAY)
