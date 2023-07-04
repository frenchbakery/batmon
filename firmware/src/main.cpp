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
    led::set_blink_notice_alive();

    LOGI("Initializing buzzer");
    gpio_set_level(env::BUZZER, 0);


    

    LOGI("Initialization done");

    for (;;)
    {
        //LOGI("task '%s': is alive, start heap tracing", pcTaskGetName(NULL));
        //ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_ALL));
        // some code here
        //ESP_ERROR_CHECK(heap_trace_stop());
        //LOGI("Heap trace results:");
        //heap_trace_dump();

        int adc_raw[2];
        int adc_voltage[2];

        ESP_ERROR_CHECK(adc_oneshot_read(env::adc1_handle, env::c1i_adc_channel, &adc_raw[0]));
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(env::adc1_calibration_handle, adc_raw[0], &adc_voltage[0]));
        LOGI("ADC1 C1I raw data: %4d, voltage: %1.2f", adc_raw[0], adc_voltage[0] * 0.003);

        ESP_ERROR_CHECK(adc_oneshot_read(env::adc1_handle, env::c2i_adc_channel, &adc_raw[1]));
        ESP_ERROR_CHECK(adc_cali_raw_to_voltage(env::adc1_calibration_handle, adc_raw[1], &adc_voltage[1]));
        LOGI("ADC1 C2I raw data: %4d, voltage: %1.2f", adc_raw[1], adc_voltage[1] * 0.003);

        printf("\n");

        msleep(500);
    }

}