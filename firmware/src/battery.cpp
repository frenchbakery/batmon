/**
 * @file battery.cpp
 * @author melektron
 * @brief battery cell functions
 * @version 0.1
 * @date 2023-07-05
 * 
 * @copyright Copyright FrenchBakery (c) 2023
 * 
 */

#include <esp_adc/adc_continuous.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>

#include "battery.hpp"
#include "env.hpp"
#include "log.hpp"


namespace battery   // private
{
    /**
     * @brief reads an adc channel multiple times and returns the average.
     * This could also be achieved more efficiently with the continuous mode 
     * ADC driver but for this simple use case that would be overkill.
     * 
     * @param _unit ADC unit to read from
     * @param _chan ADC channel to read from
     * @param _n_samples number of sample to read
     * @return int average ADC value
     */
    static int adc_read_multisample(
        adc_oneshot_unit_handle_t _unit, 
        adc_channel_t _chan, 
        size_t _n_samples
    );
}


static int battery::adc_read_multisample(
    adc_oneshot_unit_handle_t _unit, 
    adc_channel_t _chan, 
    size_t _n_samples
)
{
    int sum = 0;

    for (size_t sample_index = 0; sample_index < _n_samples; sample_index++)
    {
        int sample_data;
        ESP_ERROR_CHECK(adc_oneshot_read(_unit, _chan, &sample_data));
        sum += sample_data;
    }

    return sum / _n_samples;
}

int battery::read_cell1()
{
    int adc_raw;
    int adc_voltage;
    ESP_ERROR_CHECK(adc_oneshot_read(env::adc1_handle, env::c1i_adc_channel, &adc_raw));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(env::adc1_calibration_handle, adc_raw, &adc_voltage));
    LOGI("ADC1 C1I raw data: %4d, voltage: %4d mV", adc_raw, adc_voltage);
    return adc_voltage * 3;
}

int battery::read_cell2()
{
    int adc_raw;
    int adc_voltage;
    ESP_ERROR_CHECK(adc_oneshot_read(env::adc1_handle, env::c2i_adc_channel, &adc_raw));
    ESP_ERROR_CHECK(adc_cali_raw_to_voltage(env::adc1_calibration_handle, adc_raw, &adc_voltage));
    LOGI("ADC1 C2I raw data: %4d, voltage: %4d mV", adc_raw, adc_voltage);
    return adc_voltage * 3;
}