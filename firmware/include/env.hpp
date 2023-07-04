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
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>

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

    // ADC handles (all on unit ADC1, only use after gpio init)
    extern adc_oneshot_unit_handle_t adc1_handle;
    extern adc_cali_handle_t adc1_calibration_handle;
    extern adc_channel_t c1i_adc_channel;
    extern adc_channel_t c2i_adc_channel;

    /**
     * @brief configures the GPIO pins
     *
     */
    void init_gpio();

    /**
     * @brief initializes the ADC and the channel
     * config variables c1i_adc_channel and c2i_adc_channel
     * 
     */
    void init_adc();
};
