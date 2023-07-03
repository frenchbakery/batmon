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
idf::GPIONum env::BUZZER(14);
idf::GPIONum env::LED(26);

// GPIO Inputs
idf::GPIONum env::WRONG_C1I(16);
idf::GPIONum env::WRONG_C2I(17);
idf::GPIONum env::C1I(35);
idf::GPIONum env::C2I(34);