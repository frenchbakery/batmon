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

// GPIO Outputs
const gpio_num_t env::BUZZER = GPIO_NUM_14;
const gpio_num_t env::LED = GPIO_NUM_26;

// GPIO Inputs
const gpio_num_t env::WRONG_C1I = GPIO_NUM_16;
const gpio_num_t env::WRONG_C2I = GPIO_NUM_17;
const gpio_num_t env::C1I = GPIO_NUM_35;
const gpio_num_t env::C2I = GPIO_NUM_34;