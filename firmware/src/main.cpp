/**
 * @brief
 *
 */

#include <stdbool.h>
#include <unistd.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_heap_trace.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>

#include "battery.hpp"
#include "buzzer.hpp"
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

    LOGI("Initializing ADC");
    env::init_adc();

    LOGI("Initializing LED blink controller");
    led::init();

    LOGI("Initializing buzzer");
    buzzer::init();

    LOGI("Initialization done");
    led::set_blink_notice_alive();
    buzzer::play_startup();
    msleep(1000);

    for (;;)
    {
        int c1_voltage = battery::read_cell1();
        int c2_voltage = battery::read_cell2();
        LOGI("C1 (L): %1.2f V,\tC2 (H): %1.2f", c1_voltage * 0.001, c2_voltage * 0.001);

        buzzer::play_quiet();
        msleep(5000);
        buzzer::play_battery_low();
        msleep(5000);
        buzzer::play_battery_alarm();
        msleep(5000);
    }
    


    LOGE("exited main loop, shouldn't happen, restart");
    esp_restart();
}