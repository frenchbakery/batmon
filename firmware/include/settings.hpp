/**
 * @file settings.hpp
 * @author melektron
 * @brief helpers for NVS settings
 * @version 0.1
 * @date 2023-07-06
 *
 * @copyright Copyright FrenchBakery (c) 2023
 *
 */

#pragma once

#include <stdint.h>

namespace settings
{
    enum key_t
    {
        // voltage at which (and below) low battery warning should be played
        CELL1_WARN_VOLTAGE = 0,
        CELL2_WARN_VOLTAGE,
        // voltage at which (and below) battery alarm should be played
        CELL1_ALARM_VOLTAGE,
        CELL2_ALARM_VOLTAGE,
        // voltage difference between cells at which (and above) the alarm
        // for too high voltage difference should be played
        CELL_ALARM_VOLTAGE_DIFFERENCE,

        // Iterator end value
        __SETTING_END
    };

    /**
     * @brief initializes NVS to load and store settings
     */
    void init();

    /**
     * @brief reads a setting and returns it's value.
     * This is fast, as it always uses the read cache.
     *
     * @param _key the setting to read (use enum constants)
     * @return int32_t value of the setting
     */
    int32_t get(key_t _key);

    /**
     * @brief updates a setting with a new value and stores
     * that value in NVS
     *
     * @param _key the setting to write (use enum constants)
     * @param _value the new value to be stored
     */
    void set(key_t _key, int32_t _value);
}