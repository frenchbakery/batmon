/**
 * @file buzzer.hpp
 * @author melektron
 * @brief buzzer control functions
 * @version 0.1
 * @date 2023-07-05
 * 
 * @copyright Copyright FrenchBakery (c) 2023
 * 
 */

#pragma once

namespace buzzer
{
    /**
     * @brief initializes LEDC to drive buzzer
     * 
     */
    void init();

    // functions to play different sounds
    void play_quiet();
    void play_startup();
    void play_battery_low();
    void play_battery_alarm();

} // namespace buzzer
