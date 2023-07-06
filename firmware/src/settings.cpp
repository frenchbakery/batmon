/**
 * @file settings.cpp
 * @author melektron
 * @brief helpers for NVS settings
 * @version 0.1
 * @date 2023-07-06
 *
 * @copyright Copyright FrenchBakery (c) 2023
 *
 */

#include <inttypes.h>
#include <nvs_flash.h>
#include <nvs.h>

#include "settings.hpp"
#include "log.hpp"

namespace settings // private
{

#define NR_OF_SETTINGS __SETTING_END

    // an array of all the settings names 
    // (according to the order in the key_t enum)
    static const char *setting_names[NR_OF_SETTINGS] = {
        "c1_warn_v",
        "c2_warn_v",
        "c1_alarm_v",
        "c2_alarm_v",
        "c_alarm_diff_v",
    };

    // default values for all the settings (in order)
    static int32_t setting_defaults[NR_OF_SETTINGS] = {
        3000,
        3000,
        2800,
        2800,
        1000
    };

    // cache of setting values stored in RAM (in order)
    static int32_t setting_read_cache[NR_OF_SETTINGS] = { 0 };

    // handle for settings NVS namespace
    static nvs_handle_t settings_handle;
};

void settings::init()
{
    esp_err_t err;

    // initialize the NVS flash
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
        err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        // if there are some errors with flash layout, clear the NVS and 
        // try again
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    // do the regular error checking after handling special cases
    ESP_ERROR_CHECK(err);

    // Open the NVS namespace used for settings
    ESP_ERROR_CHECK(nvs_open("settings", NVS_READWRITE, &settings_handle));

    // Prepare all the setting keys
    bool changes = false;
    for (size_t setting_index = 0; setting_index < NR_OF_SETTINGS; setting_index++)
    {
        // load the default value in case it needs to be set up
        int32_t setting_value = setting_defaults[setting_index];

        // try to load the stored value from NVS
        err = nvs_get_i32(
            settings_handle,
            setting_names[setting_index],
            &setting_value
        );

        switch (err)
        {
        case ESP_OK:
            // Setting found in NVS

            LOGI(
                "Successfully read setting: %s=%" PRIi32,
                setting_names[setting_index],
                setting_value
            );
            break;

        case ESP_ERR_NVS_NOT_FOUND:
            // Setting not found in NVS

            // initialize with default value
            ESP_ERROR_CHECK(nvs_set_i32(
                settings_handle,
                setting_names[setting_index],
                setting_value
            ));
            changes = true;

            LOGI(
                "Setting not found, initializing to default: %s=%" PRIi32,
                setting_names[setting_index],
                setting_value
            );
            break;

        default:
            // Other unknown error, just call default error handler
            ESP_ERROR_CHECK(err);
            break;
        }

        // store value in read cache
        setting_read_cache[setting_index] = setting_value;
    }

    // if anything was written to NVS, commit the changes
    if (changes)
        ESP_ERROR_CHECK(nvs_commit(settings_handle));

}

int32_t settings::get(key_t _key)
{
    return setting_read_cache[_key];
}

void settings::set(key_t _key, int32_t _value)
{
    // if there is no change, we don't want to
    // unnecessarily write to the flash
    if (_value == setting_read_cache[_key]) return;

    // write the new value to flash
    ESP_ERROR_CHECK(nvs_set_i32(
        settings_handle,
        setting_names[_key],
        _value
    ));
    ESP_ERROR_CHECK(nvs_commit(settings_handle));

    // update the read cache
    setting_read_cache[_key] = _value;

    LOGI(
        "Setting changed: %s=%" PRIi32,
        setting_names[_key],
        _value
    );
}
