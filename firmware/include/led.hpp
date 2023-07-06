/**
 * @file led.hpp
 * @author melektron
 * @brief status led driver
 * @version 0.1
 * @date 2023-07-04
 *
 * @copyright Copyright FrenchBakery (c) 2023
 *
 */

#pragma once

namespace led
{
    /**
     * @brief configures the default LED pin state
     * and starts the LED task 
     */
    void init();

    // functions to put the LED in specific modes
    void set_permanent_off();
    void set_permanent_on();
    void set_blink_notice_alive();
    void set_blink_charging();
    void set_blink_warning();
    void set_blink_alarm();
};