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
#include <freertos/timers.h>
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
    // Entry point of LED task
    static void task_fn(void *);
    
    /**
     * @brief starts the LED task if it is not running already.
     * If it is already running it stops the task and restarts it.
     */
    static void start_task();

    /**
     * @brief stops the led task if it is running
     * 
     */
    static void stop_task();

    // led modes
    enum class led_mode_t
    {
        PERMANENT,          // permanent on/off mode, no actions need to happen in background
        BLINK_NOTICE_ALIVE, // short pulse every few seconds to indicate the device is turned on
        BLINK_CHARGING,     // 1 Hz 50% flashing indicating battery is being charged
        BLINK_ALARM,        // two quick medium duration pulses every second to indicate an error
    };
    static led_mode_t led_mode = led_mode_t::PERMANENT;
};

void led::init()
{
    // turn LED off
    gpio_set_level(env::LED, 0);

    // set to default state
    set_permanent_off();
}

void led::set_permanent_off()
{
    stop_task();
    led_mode = led_mode_t::PERMANENT;
    gpio_set_level(env::LED, 0);
}
void led::set_permanent_on()
{
    stop_task();
    led_mode = led_mode_t::PERMANENT;
    gpio_set_level(env::LED, 1);
}
void led::set_blink_notice_alive()
{
    gpio_set_level(env::LED, 0);
    led_mode = led_mode_t::BLINK_NOTICE_ALIVE;
    start_task();
}
void led::set_blink_charging()
{
    gpio_set_level(env::LED, 0);
    led_mode = led_mode_t::BLINK_CHARGING;
    start_task();
}
void led::set_blink_alarm()
{
    gpio_set_level(env::LED, 0);
    led_mode = led_mode_t::BLINK_ALARM;
    start_task();
}

static void led::start_task()
{
    if (task_handle != nullptr)
        stop_task();
    
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
static void led::stop_task()
{
    if (task_handle == nullptr)
        return;
    
    auto task_handle_tmp = task_handle;
    task_handle = nullptr;
    vTaskDelete(task_handle_tmp);
}

static void led::task_fn(void *)
{
    for (;;)
    {
        switch (led_mode)
        {
        case led_mode_t::PERMANENT:
            sleep(1);
            break;

        case led_mode_t::BLINK_NOTICE_ALIVE:
            gpio_set_level(env::LED, 1);
            msleep(60);
            gpio_set_level(env::LED, 0);
            msleep(2940);
            break;

        case led_mode_t::BLINK_CHARGING:
            gpio_set_level(env::LED, 1);
            msleep(500);
            gpio_set_level(env::LED, 0);
            msleep(500);
            break;

        case led_mode_t::BLINK_ALARM:
            gpio_set_level(env::LED, 1);
            msleep(150);
            gpio_set_level(env::LED, 0);
            msleep(150);
            gpio_set_level(env::LED, 1);
            msleep(150);
            gpio_set_level(env::LED, 0);
            msleep(1050);
            break;

        default:
            LOGE("Invalid led state, turning off");
            set_permanent_off();
            break;
        }
    }
    
    // set to nullptr before deleting, as any code after this line
    // is not run and the variable isn't referenced here anyway
    task_handle = nullptr;
    vTaskDelete(NULL);
}
