/**
 * @brief
 *
 */

#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "utils.hpp"
#include "log.hpp"
#include "env.hpp"
#include "led.hpp"


extern "C" void app_main()
{
    LOGI("=== BatMon Firmware v0.1 starting ===");
    LOGI("Initializing GPIO Pins");
    env::init_gpio();

    LOGI("Initializing LED");
    led::init();

    LOGI("Initializing buzzer");
    gpio_set_level(env::BUZZER, 0);

    for (;;)
    {
        vPortYield();
        gpio_set_level(env::LED, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(env::LED, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        LOGI("task '%s' is alive", pcTaskGetTaskName(NULL));
    }
}