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

#include "net.hpp"
#include "log.hpp"

/**
 * secrets.h must define the following things:
 * #define TEST_SSID "somewifissid"
 * #define TEST_PSK "somewifipsk"
 */
#include "secrets.h"
#define TEST_NUM_RETRY 5


namespace net   // private
{
    // event group used to notify the networking application task
    // about the wifi state from event handlers
    static EventGroupHandle_t wifi_event_group;
    // bit definition for the above event group
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

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
     * @brief sends an http get request
     *
     * @param _url url to request (e.g. "http://elektron.work/" or "http://192.168.1.23/api/some/endpoint")
     */
    static void http_send_get(const char *_url);
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
            /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (pasword len => 8).
             * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
             * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
             * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
             */
            .threshold = {
                .authmode = WIFI_AUTH_WPA2_PSK
            }
        },
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

static void net::task_fn(void *_arg)
{
    for (;;)
    {
        /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
        * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see below) */
        EventBits_t bits = xEventGroupWaitBits(
            wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdTRUE,     // clear bits when received, so event can be received again
            pdFALSE,
            portMAX_DELAY
        );

        /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
        * happened. */
        if (bits & WIFI_CONNECTED_BIT)
        {
            LOGI("connected to ap SSID:%s password:%s", TEST_SSID, TEST_PSK);
            LOGI("sending http in 5 sec...");
            sleep(5);
            LOGI("sending http request");

            http_send_get("http://ip.jsontest.com/");

        }
        else if (bits & WIFI_FAIL_BIT)
        {
            LOGI("Failed to connect to SSID:%s, password:%s", TEST_SSID, TEST_PSK);
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
    static int retry_num = 0;

    if (_event_base == WIFI_EVENT && _event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (_event_base == WIFI_EVENT && _event_id == WIFI_EVENT_STA_CONNECTED)
    {
        LOGI("Station connected to AP, waiting for DHCP");
    }
    else if (_event_base == WIFI_EVENT && _event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        LOGI("connect to the AP fail");
        if (retry_num < TEST_NUM_RETRY)
        {
            esp_wifi_connect();
            retry_num++;
            LOGI("retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(wifi_event_group, WIFI_FAIL_BIT);
        }
    }
    else if (_event_base == IP_EVENT && _event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)_event_data;
        LOGI("got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_err_t net::http_event_handler(
    esp_http_client_event_t *_evt
)
{
    static char *output_buffer;  // Buffer to store response of http request from event handler
    static int output_len;       // Stores number of bytes read
    switch(_evt->event_id) {
        case HTTP_EVENT_ERROR:
            LOGI("HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            LOGI("HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            LOGI("HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            LOGI("HTTP_EVENT_ON_HEADER, key=%s, value=%s", _evt->header_key, _evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            LOGI("HTTP_EVENT_ON_DATA, len=%d", _evt->data_len);
            /*
             *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
             *  However, event handler can also be used in case chunked encoding is used.
             */
            if (!esp_http_client_is_chunked_response(_evt->client)) {
                // If user_data buffer is configured, copy the response into the buffer
                if (_evt->user_data) {
                    memcpy(_evt->user_data + output_len, _evt->data, _evt->data_len);
                } else {
                    if (output_buffer == NULL) {
                        output_buffer = (char *) malloc(esp_http_client_get_content_length(_evt->client));
                        output_len = 0;
                        if (output_buffer == NULL) {
                            LOGE("Failed to allocate memory for output buffer");
                            return ESP_FAIL;
                        }
                    }
                    memcpy(output_buffer + output_len, _evt->data, _evt->data_len);
                }
                output_len += _evt->data_len;
            }

            break;
        case HTTP_EVENT_ON_FINISH:
            LOGI("HTTP_EVENT_ON_FINISH");
            if (output_buffer != NULL) {
                LOGI("Finished, response:\n%.*s", output_len, output_buffer);
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
            break;
        case HTTP_EVENT_DISCONNECTED:
        {
            LOGI("HTTP_EVENT_DISCONNECTED");
            int mbedtls_err = 0;
            esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)_evt->data, &mbedtls_err, NULL);
            if (err != 0) {
                LOGI("Last esp error code: 0x%x", err);
                LOGI("Last mbedtls failure: 0x%x", mbedtls_err);
            }
            if (output_buffer != NULL) {
                free(output_buffer);
                output_buffer = NULL;
            }
            output_len = 0;
        }
            break;
        
        case HTTP_EVENT_REDIRECT:
            LOGI("HTTP_EVENT_REDIRECT");
            esp_http_client_set_header(_evt->client, "From", "user@example.com");
            esp_http_client_set_header(_evt->client, "Accept", "text/html");
            esp_http_client_set_redirection(_evt->client);
            break;
        
        default:
            break;
    }
    return ESP_OK;
}

void net::http_send_get(const char *_url)
{
    LOGI("== creating structure...");
    esp_http_client_config_t config = {
        .url = _url,
        .method = HTTP_METHOD_GET,
        .event_handler = http_event_handler
    };

    LOGI("== initializing http...");
    esp_http_client_handle_t client = esp_http_client_init(&config);

    LOGI("== performing request...");
    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK)
    {
        LOGI("== perform done, status=%d, content_len=%lld", 
            esp_http_client_get_status_code(client),
            esp_http_client_get_content_length(client)
        );
    }
    else
    {
        LOGE("== perform done with error: %s", esp_err_to_name(err));
    }

    LOGI("== http cleanup");
    esp_http_client_cleanup(client);
}