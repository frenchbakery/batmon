/**
 * @brief
 *
 */

#include <stdbool.h>
#include <unistd.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_heap_trace.h>

#include "utils.hpp"
#include "log.hpp"
#include "env.hpp"
#include "led.hpp"

// variables used for heap tracing during debug mode
#define HEAP_TRACE_NUM_RECORDS 100
static heap_trace_record_t heap_trace_record_buffer[HEAP_TRACE_NUM_RECORDS];


extern "C" void app_main()
{
    LOGI("=== BatMon Firmware v0.1 starting ===");
    LOGI("Initializing heap tracing");
    ESP_ERROR_CHECK(heap_trace_init_standalone(heap_trace_record_buffer, HEAP_TRACE_NUM_RECORDS));

    LOGI("Initializing GPIO Pins");
    env::init_gpio();

    LOGI("Initializing LED");
    led::init();

    LOGI("Initializing buzzer");
    gpio_set_level(env::BUZZER, 0);

    for (;;)
    {
        LOGI("task '%s': is alive, start heap tracing", pcTaskGetName(NULL));
        ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_ALL));

        led::set_permanent_on();
        sleep(10);
        led::set_permanent_off();
        sleep(10);
        led::set_blink_notice_alive();
        sleep(10);
        led::set_blink_charging();
        sleep(10);
        led::set_blink_alarm();
        sleep(10);

        ESP_ERROR_CHECK(heap_trace_stop());
        LOGI("Heap trace results:");
        heap_trace_dump();
    }
}