/**
 * @brief
 *
 */

#include <freertos/FreeRTOS.h>

#include "log.h"
#include "env.hpp"

extern "C" void app_main()
{
    LOGE("=== BatMon Firmware v0.1 starting ===");
    LOGV("Configuring pins");
    idf::GPIO_Output led_pin(env::LED);
    idf::GPIO_Output buzzer_pin(env::BUZZER);
    idf::GPIOInput wrong_c1i_pin(env::WRONG_C1I);
    idf::GPIOInput wrong_c2i_pin(env::WRONG_C2I);
    idf::GPIOInput c1i_pin(env::C1I);
    idf::GPIOInput c2i_pin(env::C2I);

    LOGV("Setting default pin states");
    led_pin.set_low();
    buzzer_pin.set_low();
    wrong_c1i_pin.set_pull_mode(idf::GPIOPullMode::FLOATING());
    wrong_c2i_pin.set_pull_mode(idf::GPIOPullMode::FLOATING());
    c1i_pin.set_pull_mode(idf::GPIOPullMode::FLOATING());
    c2i_pin.set_pull_mode(idf::GPIOPullMode::FLOATING());

    for (;;)
    {
        vPortYield();
        led_pin.set_high();
        vTaskDelay(500 / portTICK_PERIOD_MS);
        led_pin.set_low();
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}