/**
 * @file env.cpp
 * @author melektron
 * @brief environment definitions (such as pinout)
 * @version 0.1
 * @date 2023-07-03
 * 
 * @copyright Copyright FrenchBakery (c) 2023
 * 
 */

#include "env.hpp"

#include <driver/gpio.h>

// GPIO Outputs
const gpio_num_t env::BUZZER = GPIO_NUM_14;
const gpio_num_t env::LED = GPIO_NUM_26;

// GPIO Inputs
const gpio_num_t env::WRONG_C1I = GPIO_NUM_16;
const gpio_num_t env::WRONG_C2I = GPIO_NUM_17;
const gpio_num_t env::C1I = GPIO_NUM_35;
const gpio_num_t env::C2I = GPIO_NUM_34;

void env::init_gpio()
{
    gpio_config_t io_conf;

    // Output Pins
    io_conf.pin_bit_mask = (1ull << env::BUZZER) | (1ull << env::LED);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    // Input Pins
    io_conf.pin_bit_mask = (1ull << env::WRONG_C1I) | (1ull << env::WRONG_C2I) | (1ull << env::C1I) | (1ull << env::C2I);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
}