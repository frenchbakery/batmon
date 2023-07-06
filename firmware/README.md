# Firmware

The firmware for batmon (written using ESP-IDF)

## Notes

These are helpful notes for working with the project 

 - Change ESP-IDF configuration: (in PIO terminal) ```pio run -t menuconfig```
 - Serial monitor that decodes ESP32 crash tracebacks: ```pio device monitor -f esp32_exception_decoder``` (or use the esp32_exception_decoder filter in platformio.ini file, as configured in ./firmware)
 - Info about framework support: https://docs.platformio.org/en/stable/frameworks/espidf.html
 - Info about Logging: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/log.html
 - ESP-IDF GPIO doc: https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/gpio.html+
 - Wrong ESP-IDF Version installed: https://community.platformio.org/t/is-it-possible-to-use-the-latest-esp32-idf-with-platformio/32125
 - different C++ version for ESP-IDF: https://community.platformio.org/t/separate-settings-for-c-and-c-versions/21647
 