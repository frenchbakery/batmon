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

#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>

#include "env.hpp"

// GPIO Outputs
const gpio_num_t env::BUZZER = GPIO_NUM_14;
const gpio_num_t env::LED = GPIO_NUM_26;

// GPIO Inputs
const gpio_num_t env::WRONG_C1I = GPIO_NUM_16;
const gpio_num_t env::WRONG_C2I = GPIO_NUM_17;
const gpio_num_t env::C1I = GPIO_NUM_35;
const gpio_num_t env::C2I = GPIO_NUM_34;

// ADC 
#define USED_ADC1_ATTENUATION adc_atten_t::ADC_ATTEN_DB_11
#define USED_ADC1_BITWIDTH adc_bitwidth_t::ADC_BITWIDTH_DEFAULT
adc_oneshot_unit_handle_t env::adc1_handle;
adc_cali_handle_t env::adc1_calibration_handle;
adc_channel_t env::c1i_adc_channel;
adc_channel_t env::c2i_adc_channel;

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

void env::init_adc()
{
    // initialize ADC1
    const adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = adc_unit_t::ADC_UNIT_1,
        .ulp_mode = adc_ulp_mode_t::ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    // initialize all needed ADC channels
    const adc_oneshot_chan_cfg_t config = {
        .atten = USED_ADC1_ATTENUATION,
        .bitwidth = USED_ADC1_BITWIDTH
    };
    // we don't need the unit result of adc_oneshot_io_to_channel so we store it here
    adc_unit_t unit;

    ESP_ERROR_CHECK(adc_oneshot_io_to_channel(env::C1I, &unit, &c1i_adc_channel));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, c1i_adc_channel, &config));
    ESP_ERROR_CHECK(adc_oneshot_io_to_channel(env::C2I, &unit, &c2i_adc_channel));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, c2i_adc_channel, &config));

    // apply line fitting calibration
    const adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = USED_ADC1_ATTENUATION,
        .bitwidth = USED_ADC1_BITWIDTH,
    };
    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config, &adc1_calibration_handle));

    
}