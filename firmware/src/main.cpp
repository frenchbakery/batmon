/**
 * @file main.cpp
 * @author melektron
 * @brief main file of batmon firmware
 * @version 0.1
 * @date 2023-07-06
 *
 * @copyright Copyright FrenchBakery (c) 2023
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

#include "settings.hpp"
#include "battery.hpp"
#include "buzzer.hpp"
#include "utils.hpp"
#include "log.hpp"
#include "env.hpp"
#include "led.hpp"
#include "net.hpp"

// variables used for heap tracing during debug mode
#define HEAP_TRACE_NUM_RECORDS 100
static heap_trace_record_t heap_trace_record_buffer[HEAP_TRACE_NUM_RECORDS];


extern "C" void app_main()
{
    LOGI("=== BatMon Firmware v0.1 starting ===");
    LOGI("Initializing heap tracing");
    ESP_ERROR_CHECK(heap_trace_init_standalone(heap_trace_record_buffer, HEAP_TRACE_NUM_RECORDS));

    LOGI("Initializing NVS settings");
    settings::init();

    LOGI("Initializing GPIO pins");
    env::init_gpio();

    LOGI("Initializing ADC");
    env::init_adc();

    LOGI("Initializing LED blink controller");
    led::init();

    LOGI("Initializing buzzer");
    buzzer::init();

    LOGI("Initializing networking");
    net::init();

    LOGI("Initialization done");
    led::set_blink_notice_alive();
    buzzer::play_startup();
    msleep(500);



    for (;;)
    {
        int c1_voltage = battery::read_cell1();
        int c2_voltage = battery::read_cell2();
        LOGI("C1 (L): %1.2f V,\tC2 (H): %1.2f", c1_voltage * 0.001, c2_voltage * 0.001);

        net::report.c1_voltage = c1_voltage;
        net::report.c2_voltage = c2_voltage;
        net::report.c1_warn_threshold = settings::CELL1_WARN_VOLTAGE;
        net::report.c2_warn_threshold = settings::CELL2_WARN_VOLTAGE;
        net::report.c1_alarm_threshold = settings::CELL1_ALARM_VOLTAGE;
        net::report.c2_alarm_threshold = settings::CELL2_ALARM_VOLTAGE;
        net::report.diff_alarm_threshold = settings::CELL_ALARM_VOLTAGE_DIFFERENCE;
        net::update();

        if (
            c1_voltage < settings::get(settings::CELL1_ALARM_VOLTAGE) ||
            c2_voltage < settings::get(settings::CELL2_ALARM_VOLTAGE) ||
            abs(c1_voltage - c2_voltage) > settings::get(settings::CELL_ALARM_VOLTAGE_DIFFERENCE)
            ) {
            buzzer::play_battery_alarm();
            led::set_blink_alarm();
            LOGI("Battery alarm");
        }
        else if (
            c1_voltage < settings::get(settings::CELL1_WARN_VOLTAGE) ||
            c2_voltage < settings::get(settings::CELL2_WARN_VOLTAGE)
            ) {
            buzzer::play_battery_warning();
            led::set_blink_warning();
            LOGI("Battery warning");
        }
        else
        {
            buzzer::play_quiet();
            led::set_blink_notice_alive();
            LOGI("All good");
        }

        sleep(20);
    }



    LOGE("exited main loop, shouldn't happen, restart");
    esp_restart();
}