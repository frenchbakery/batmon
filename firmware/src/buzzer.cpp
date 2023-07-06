/**
 * @file buzzer.cpp
 * @author melektron
 * @brief buzzer control functions
 * @version 0.1
 * @date 2023-07-05
 * 
 * @copyright Copyright FrenchBakery (c) 2023
 * 
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/ledc.h>
#include <unistd.h>

#include "buzzer.hpp"
#include "pitches.h"
#include "utils.hpp"
#include "env.hpp"
#include "log.hpp"


namespace buzzer    // private
{
    // buzzer task symbols
    // the statically allocated memory for the task's stack
#define TASK_STACK_SIZE 3000
    static StackType_t task_stack[TASK_STACK_SIZE];
    // handle to stack buffer and handle to task
    static StaticTask_t task_static_buffer;
    static TaskHandle_t task_handle = nullptr;
    
    // buzzer timer configuration
    static ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,              // we are arbitrarily selecting timer 0 for the task
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    // buzzer output channel configuration
    static ledc_channel_config_t channel_config = {
        .gpio_num = env::BUZZER,
        .speed_mode = timer_config.speed_mode,
        .channel = LEDC_CHANNEL_0,              // we are arbitrarily selecting channel 0 for the task
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = timer_config.timer_num,
        .duty = 0,                              // off at first
        .hpoint = 0,
        .flags = {
            .output_invert = 0
        }
    };

    // buzzer mode/function
    enum class buzzer_mode_t
    {
        QUIET,              // quiet, nothing plays
        STARTUP,            // charming three notes played at startup
        BATTERY_WARNING,    // short medium pitch pulse every few seconds to indicate the battery is starting to get low
        BATTERY_ALARM,      // short, rapid high pitch pulses indicating the battery is critically low or there is a large
                            // cell voltage difference
    };
    static buzzer_mode_t buzzer_mode = buzzer_mode_t::QUIET;

    /**
     * @brief sets the buzzer to a specific mode. 
     * Use this if there is no special initialization needed.
     * 
     * @param _mode mode to activate
     */
    static void set_mode(buzzer_mode_t _mode);

    /**
     * @brief entry point of task
     */
    static void task_fn(void *);
    
    /**
     * @brief starts the task if it is not running already.
     * If it is already running it stops the task and restarts it.
     */
    static void start_task();

    /**
     * @brief stops the task if it is running
     */
    static void stop_task();


    /**
     * @brief sets the output frequency of the buzzer's timer
     * 
     * @param _freq 
     */
    static void set_frequency(int _freq);

    // turns the buzzer on
    static void turn_on();
    // turns the buzzer off
    static void turn_off();
}

void buzzer::init()
{
    // initialize the timer with a default frequency
    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    // configure the output channel
    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
}

void buzzer::play_quiet()
{
    if (buzzer_mode == buzzer_mode_t::QUIET) return;

    stop_task();
    turn_off();
    buzzer_mode = buzzer_mode_t::QUIET;
    // don't need the task to run for this
}
void buzzer::play_startup()
{
    set_mode(buzzer_mode_t::STARTUP);
}
void buzzer::play_battery_warning()
{
    set_mode(buzzer_mode_t::BATTERY_WARNING);
}
void buzzer::play_battery_alarm()
{
    set_mode(buzzer_mode_t::BATTERY_ALARM);
}

static void buzzer::set_mode(buzzer_mode_t _mode)
{
    // if the mode is already active, do nothing
    if (buzzer_mode == _mode) return;

    stop_task();
    turn_off();
    buzzer_mode = _mode;
    start_task();
}

static void buzzer::set_frequency(int _freq)
{
    ESP_ERROR_CHECK(ledc_set_freq(
        timer_config.speed_mode,
        timer_config.timer_num,
        _freq
    ));
}

static void buzzer::turn_on()
{
    ESP_ERROR_CHECK(ledc_set_duty(
        timer_config.speed_mode,
        channel_config.channel,
        0x1ff   // 511 (half of the 10 bit)
    ));
    ESP_ERROR_CHECK(ledc_update_duty(
        timer_config.speed_mode,
        channel_config.channel
    ));
}

static void buzzer::turn_off()
{
    ESP_ERROR_CHECK(ledc_set_duty(
        timer_config.speed_mode,
        channel_config.channel,
        0
    ));
    ESP_ERROR_CHECK(ledc_update_duty(
        timer_config.speed_mode,
        channel_config.channel
    ));
}

static void buzzer::start_task()
{
    if (task_handle != nullptr)
        stop_task();
    
    task_handle = xTaskCreateStatic(
        task_fn,
        "buzzer",
        TASK_STACK_SIZE,
        nullptr,
        1,
        task_stack,
        &task_static_buffer
    );
}
static void buzzer::stop_task()
{
    if (task_handle == nullptr)
        return;
    
    // temp var in case the task stops itself 
    auto task_handle_tmp = task_handle;
    task_handle = nullptr;
    vTaskDelete(task_handle_tmp);
}

static void buzzer::task_fn(void *)
{
    for (;;)
    {
        switch (buzzer_mode)
        {
        case buzzer_mode_t::QUIET:
            goto cleanup;
            break;

        case buzzer_mode_t::STARTUP:
            set_frequency(HZ_NOTE_E3);
            turn_on();
            msleep(100);
            set_frequency(HZ_NOTE_B3);
            msleep(100);
            set_frequency(HZ_NOTE_E4);
            msleep(100);
            turn_off();
            goto cleanup;
            break;

        case buzzer_mode_t::BATTERY_WARNING:
            set_frequency(HZ_NOTE_A4);
            turn_on();
            msleep(100);
            turn_off();
            msleep(1900);
            break;

        case buzzer_mode_t::BATTERY_ALARM:
            set_frequency(HZ_NOTE_F5);
            turn_on();
            msleep(150);
            turn_off();
            msleep(150);
            break;

        default:
            LOGE("Invalid led state, turning off");
            play_quiet();
            break;
        }
    }

cleanup:    
    // set to nullptr before deleting, as any code after this line
    // is not run and the variable isn't referenced here anyway
    task_handle = nullptr;
    vTaskDelete(NULL);
}