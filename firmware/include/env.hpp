/**
 * @file env.hpp
 * @author melektron
 * @brief environment definitions (such as pinout)
 * @version 0.1
 * @date 2023-07-03
 * 
 * @copyright Copyright FrenchBakery (c) 2023
 * 
 */

#pragma once

#include <gpio_cxx.hpp>

namespace env
{
    // GPIO Outputs
    extern idf::GPIONum BUZZER;
    extern idf::GPIONum LED;

    // GPIO Inputs
    extern idf::GPIONum WRONG_C1I;
    extern idf::GPIONum WRONG_C2I;
    extern idf::GPIONum C1I;
    extern idf::GPIONum C2I;

};
