/**
 * @file net.cpp
 * @author melektron
 * @brief WiFi and networking functionality
 * @version 0.1
 * @date 2023-07-06
 *
 * @copyright Copyright FrenchBakery (c) 2023
 *
 */

#include <unistd.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_tls.h>
#include <esp_http_client.h>    // "esp32_mock.h" not found is only an intellisense error, ignore it.
#include <nlohmann/json.hpp>
#include "net.hpp"
#include "log.hpp"

/**
 * secrets.h must define the following things:
 * #define TEST_SSID "somewifissid"
 * #define TEST_PSK "somewifipsk"
 */
#include "secrets.h"
// the amount of times that an immediate wifi reconnect should be attempted
#define WIFI_RECONNECT_MAX_IMMEDIATE_ATTEMPTS 5
// interval of try reconnecting after immediate reconnect attempts fail (seconds)
#define WIFI_RECONNECT_LONG_PERIOD 30

// maximum size of HTTP responses (any bigger will not be stored)
#define HTTP_RESPONSE_MAX_LEN 500


namespace net   // private
{
    // the report data
    report_t report;

    // event group used to notify the networking application task
    // about the wifi state from event handlers
    static EventGroupHandle_t wifi_event_group;
    // bit definition for the above event group
#define WIFI_CONNECTED_BIT          BIT0
#define WIFI_DISCONNECTED_BIT       BIT1
#define WIFI_RECONNECT_LATER_BIT    BIT2
#define REPORT_READY_FOR_SEND_BIT   BIT3

    // the statically allocated memory for the task's stack
#define TASK_STACK_SIZE 10000   // networking task needs a bit more stack space
    static StackType_t task_stack[TASK_STACK_SIZE];

    // handle to stack buffer and handle to task
    static StaticTask_t task_static_buffer;
    static TaskHandle_t task_handle = nullptr;

    /**
     * @brief entry point for the networking application task
     */
    static void task_fn(void *);

    /**
     * @brief event handler function for all networking related events
     */
    static void wifi_event_handler(
        void *_arg,
        esp_event_base_t _event_base,
        int32_t _event_id,
        void *_event_data
    );

    /**
     * @brief event handler function for all HTTP request events
     */
    static esp_err_t http_event_handler(
        esp_http_client_event_t *_evt
    );

    /**
     * @brief sends a battery status report to the server using http
     * 
     * @retval ok - request was sent
     * @retval err - couldn't send request because not connected or connection interrupted
     */
    el::retcode send_report();
};


void net::init()
{
    wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &wifi_event_handler,
        NULL,
        &instance_any_id
    ));
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        &wifi_event_handler,
        NULL,
        &instance_got_ip
    ));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = TEST_SSID,
            .password = TEST_PSK,
            .threshold = {
                .authmode = WIFI_AUTH_WPA2_PSK  // we want to use WPA2 at a minimum
            }
        }
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    LOGI("wifi_init_sta finished, starting networking task");

    // start the networking task
    task_handle = xTaskCreateStatic(
        task_fn,
        "networking",
        TASK_STACK_SIZE,
        nullptr,
        1,
        task_stack,
        &task_static_buffer
    );
}

void net::update()
{
    // notify task of the new report
    xEventGroupSetBits(wifi_event_group, REPORT_READY_FOR_SEND_BIT);
}

static void net::task_fn(void *_arg)
{
    bool network_ready = false;

    for (;;)
    {
        /**
         * Wait for one of the following events:
         *  - WiFi connection established and got IP
         *  - WiFi disconnected
         *  - WiFi should try to reconnect later
         *  - Report is ready to send
         */
        EventBits_t bits = xEventGroupWaitBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_DISCONNECTED_BIT | WIFI_RECONNECT_LATER_BIT | REPORT_READY_FOR_SEND_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY
        );

        // WIFI_CONNECTED event tells task that a network connection has been established
        if (bits & WIFI_CONNECTED_BIT)
        {
            LOGI("Network connection up");
            network_ready = true;
        }
        // WIFI_DISCONNECTED event tells task that network connection has disconnected
        else if (bits & WIFI_DISCONNECTED_BIT)
        {
            LOGI("Network connection down");
            network_ready = false;
        }
        // WIFI_RECONNECT_LATER event tells task that it should wait a bit and then try to reconnect to the network
        else if (bits & WIFI_RECONNECT_LATER_BIT)
        {
            LOGI("Failed to reconnect to AP, trying again in %d seconds ", WIFI_RECONNECT_LONG_PERIOD);
            sleep(WIFI_RECONNECT_LONG_PERIOD);
            esp_wifi_connect();
        }
        // REPORT_READY_FOR_SEND event tells task that a new report is ready to send and it should send it now if possible
        else if (bits & REPORT_READY_FOR_SEND_BIT)
        {
            if (network_ready)
            {
                // try to send the report (ignore errors)
                send_report();
            }
            else
            {
                LOGI("Cannot send report now because network is down.");
            }
        }
        else
        {
            LOGE("UNEXPECTED NETWORKING EVENT BITS");
        }
    }

    // should never get here
    task_handle = nullptr;
    vTaskDelete(NULL);
}

static void net::wifi_event_handler(
    void *_arg,
    esp_event_base_t _event_base,
    int32_t _event_id,
    void *_event_data
)
{
    static int reconnect_attempt_count = 0;

    // when wifi first starts, we want to initiate the connection process
    if (_event_base == WIFI_EVENT && _event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    // when wifi is connected, we note that and wait for an IP
    else if (_event_base == WIFI_EVENT && _event_id == WIFI_EVENT_STA_CONNECTED)
    {
        LOGI("Station connected to AP, waiting for DHCP");
    }
    // when wifi disconnects, we try to reconnect a few times.
    // If we fail at that, we notify the task of that an it will block for a bit longer
    // and then try to reconnect again.
    else if (_event_base == WIFI_EVENT && _event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        LOGI("Station disconnected from AP (%d)", reconnect_attempt_count);
        xEventGroupSetBits(wifi_event_group, WIFI_DISCONNECTED_BIT);

        // try to reconnect immediately if still allowed
        if (reconnect_attempt_count < WIFI_RECONNECT_MAX_IMMEDIATE_ATTEMPTS)
        {
            reconnect_attempt_count++;
            LOGI("Attempting to reconnect immediately...");
            esp_wifi_connect();
        }
        // otherwise tell the task to try again at a later time
        else
        {
            xEventGroupSetBits(wifi_event_group, WIFI_RECONNECT_LATER_BIT);
        }
    }
    // once we get an IP we count that as successfully connected
    else if (_event_base == IP_EVENT && _event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)_event_data;
        LOGI("Got IP from DHCP server:" IPSTR, IP2STR(&event->ip_info.ip));
        reconnect_attempt_count = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t net::http_event_handler(
    esp_http_client_event_t *_evt
)
{
    // Stores number of bytes already read (in case of multiple data events)
    // so the buffer offset can be calculated
    static size_t output_len;

    switch (_evt->event_id)
    {
    case HTTP_EVENT_ON_DATA:
        LOGI("Received response data chunk, len=%d", _evt->data_len);
        // only read respose if entire response is transmitted at once
        if (!esp_http_client_is_chunked_response(_evt->client))
        {
            // If user_data buffer is configured, copy the response into the buffer
            if (_evt->user_data)
            {
                if (output_len + _evt->data_len > HTTP_RESPONSE_MAX_LEN)
                {
                    LOGE("HTTP response too long for buffer, may be incomplete");
                    break;
                }
                memcpy((char *)_evt->user_data + output_len, _evt->data, _evt->data_len);
                output_len += _evt->data_len;
                *((char *)_evt->user_data + output_len) = 0;   // null terminator (may be overwritten by next chunk)
            }
        }
        break;

    case HTTP_EVENT_ON_FINISH:
    case HTTP_EVENT_DISCONNECTED:
        output_len = 0;
        break;

    default:
        break;
    }
    return ESP_OK;
}

el::retcode net::send_report()
{
    // return value for single-point return
    el::retcode retval = el::retcode::ok;
    // response buffer
    char http_response_buffer[HTTP_RESPONSE_MAX_LEN + 1];
    // status code
    int status_code;

    esp_http_client_config_t config = {
        .url = "http://elektronlab.local:8080/devtools/http/batt1",
        .method = HTTP_METHOD_POST,
        .event_handler = http_event_handler,
        .user_data = http_response_buffer
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    nlohmann::json post_data{
        {"c1_voltage", report.c1_voltage},
        {"c2_voltage", report.c2_voltage},
        {"c1_warn_threshold", report.c1_warn_threshold},
        {"c2_warn_threshold", report.c2_warn_threshold},
        {"c1_alarm_threshold", report.c1_alarm_threshold},
        {"c2_alarm_threshold", report.c2_alarm_threshold},
        {"diff_alarm_threshold", report.diff_alarm_threshold}
    };
    const std::string &post_data_str = post_data.dump();
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data_str.c_str(), post_data_str.size());

    LOGI("Sending report via HTTP...");
    esp_err_t err = esp_http_client_perform(client);

    if (err != ESP_OK)
    {
        LOGE("Couldn't send report via HTTP: %s", esp_err_to_name(err));
        retval = el::retcode::err;
        goto cleanup;
    }

    status_code = esp_http_client_get_status_code(client);
    if (status_code != 200)
    {
        LOGE("Server responded with non-200 status code %d", status_code);
        retval = el::retcode::err;
        goto cleanup;
    }

    LOGI("Server response (%lld bytes):\n%s",
        esp_http_client_get_content_length(client),
        http_response_buffer
    );

cleanup:
    LOGI("Report done, cleaning up");
    esp_http_client_cleanup(client);

    return retval;
}
