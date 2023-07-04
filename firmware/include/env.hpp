/**
 * @file env.hpp
 * @author melektron
 * @brief environment definitions (such as pinout) and setup functions
 * @version 0.1
 * @date 2023-07-03
 *
 * @copyright Copyright FrenchBakery (c) 2023
 *
 */

#pragma once

#include <driver/gpio.h>

namespace env
{
    // GPIO Outputs
    extern const gpio_num_t BUZZER;
    extern const gpio_num_t LED;

    // GPIO Inputs
    extern const gpio_num_t WRONG_C1I;
    extern const gpio_num_t WRONG_C2I;
    extern const gpio_num_t C1I;
    extern const gpio_num_t C2I;

    /**
     * @brief configures the GPIO pins
     *
     */
    void init_gpio();
};
