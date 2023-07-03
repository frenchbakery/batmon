/**
 * @brief
 *
 */

#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#include "log.h"
#include "env.hpp"

gpio_config_t io_conf;

extern "C" void app_main()
{
    LOGI("=== BatMon Firmware v0.1 starting ===");
    LOGI("Configuring pins");

    // Output Pins
    io_conf.pin_bit_mask = (1UL << env::BUZZER) | (1UL << env::LED);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    // Input Pins
    io_conf.pin_bit_mask = (1UL << env::WRONG_C1I) | (1UL << env::WRONG_C2I) | (1UL << env::C1I) | (1UL << env::C2I);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);


    LOGV("Setting default pin states");
    gpio_set_level(env::LED, 0);
    gpio_set_level(env::BUZZER, 0);

    for (;;)
    {
        vPortYield();
        gpio_set_level(env::LED, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(env::LED, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        LOGI("Tick");
    }
}