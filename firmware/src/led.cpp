/**
 * @file led.cpp
 * @author melektron
 * @brief status led driver
 * @version 0.1
 * @date 2023-07-04
 *
 * @copyright Copyright FrenchBakery (c) 2023
 *
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <unistd.h>

#include "led.hpp"
#include "env.hpp"
#include "utils.hpp"
#include "log.hpp"

namespace led // private
{
    // led task symbols

    // The statically allocated memory for the task's stack
#define TASK_STACK_SIZE 3000
    static StackType_t task_stack[TASK_STACK_SIZE];

    // Handle buffer and handle to led task
    static StaticTask_t task_static_buffer;
    static TaskHandle_t task_handle = nullptr;

    /**
     * @brief entry point of task
     */
    static void task_fn(void *);

    // led modes
    enum class led_mode_t
    {
        PERMANENT_ON,       // LED permanent on mode, just turns the LED on once
        PERMANENT_OFF,      // LED permanent off mode, just turns the LED off once
        BLINK_NOTICE_ALIVE, // short pulse every few seconds to indicate the device is turned on
        BLINK_CHARGING,     // 1 Hz 50% duty flashing indicating battery is being charged
        BLINK_WARNING,      // two quick medium duration pulses every second to indicate the battery is low
        BLINK_ALARM,        // fast, rapid pulses indicating a critical battery alarm
    };
    static led_mode_t led_mode = led_mode_t::PERMANENT_OFF;

    /**
     * @brief activates a mode and notifies the task
     *
     * @param _mode mode to activate
     */
    static void activate_mode(led_mode_t _mode);
};

void led::init()
{
    // set to off state
    set_permanent_off();

    // start the led task
    task_handle = xTaskCreateStatic(
        task_fn,
        "led",
        TASK_STACK_SIZE,
        nullptr,
        1,
        task_stack,
        &task_static_buffer
    );
}

void led::set_permanent_off()
{
    activate_mode(led_mode_t::PERMANENT_OFF);
}
void led::set_permanent_on()
{
    activate_mode(led_mode_t::PERMANENT_ON);
}
void led::set_blink_notice_alive()
{
    if (led_mode != led_mode_t::BLINK_NOTICE_ALIVE)
        activate_mode(led_mode_t::BLINK_NOTICE_ALIVE);
}
void led::set_blink_charging()
{
    if (led_mode != led_mode_t::BLINK_CHARGING)
        activate_mode(led_mode_t::BLINK_CHARGING);
}
void led::set_blink_warning()
{
    if (led_mode != led_mode_t::BLINK_WARNING)
        activate_mode(led_mode_t::BLINK_WARNING);
}
void led::set_blink_alarm()
{
    if (led_mode != led_mode_t::BLINK_ALARM)
        activate_mode(led_mode_t::BLINK_ALARM);
}

static void led::activate_mode(led_mode_t _mode)
{
    led_mode = _mode;

    // send a notification to the task so it can immediately abort any sleeps
    // and start processing the new mode
    if (task_handle != nullptr)
        xTaskNotify(task_handle, 0, eNotifyAction::eNoAction);
}

static void led::task_fn(void *)
{
    for (;;)
    {
        switch (led_mode)
        {
        case led_mode_t::PERMANENT_ON:
            gpio_set_level(env::LED, 1);
            wait_until_notified();
            break;

        case led_mode_t::PERMANENT_OFF:
            gpio_set_level(env::LED, 0);
            wait_until_notified();
            break;

        case led_mode_t::BLINK_NOTICE_ALIVE:
            gpio_set_level(env::LED, 1);
            msleep_unless_notified(60);
            gpio_set_level(env::LED, 0);
            msleep_unless_notified(2940);
            break;

        case led_mode_t::BLINK_CHARGING:
            gpio_set_level(env::LED, 1);
            msleep_unless_notified(500);
            gpio_set_level(env::LED, 0);
            msleep_unless_notified(500);
            break;

        case led_mode_t::BLINK_WARNING:
            gpio_set_level(env::LED, 1);
            msleep_unless_notified(150);
            gpio_set_level(env::LED, 0);
            msleep_unless_notified(150);
            gpio_set_level(env::LED, 1);
            msleep_unless_notified(150);
            gpio_set_level(env::LED, 0);
            msleep_unless_notified(1550);
            break;

        case led_mode_t::BLINK_ALARM:
            gpio_set_level(env::LED, 1);
            msleep_unless_notified(150);
            gpio_set_level(env::LED, 0);
            msleep_unless_notified(150);
            break;

        default:
            LOGE("Invalid led state, ignoring");
            wait_until_notified();
            break;
        }
    }

    // set to nullptr before deleting, as any code after this line
    // is not run and the variable isn't referenced here anyway.
    task_handle = nullptr;
    vTaskDelete(NULL);
}
