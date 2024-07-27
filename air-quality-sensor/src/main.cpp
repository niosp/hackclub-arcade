/*
    part of the air measurement project
*/
#include <stdio.h>
#include <memory.h>
#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_task_wdt.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi_types.h"
#include "esp_system.h"
#include "sensirion-lib/sen5x_i2c.h"
#include "sensirion-lib/scd4x_i2c.h"
#include "sensirion-lib/sensirion_i2c_hal.h"
#include "sensirion-lib/sensirion_common.h"

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
#define EX_UART_NUM UART_NUM_1
#define FIRMWARE_VERSION 1

int s_retry_num = 0;
uint8_t connected_bit = 0;
EventGroupHandle_t s_wifi_event_group;

wifi_config_t g_wifi_config = {
        .sta = {
            .ssid = "redacted",
            .password = "redacted",
        },
};

int8_t get_sen5x_values(uint16_t *arr){
    // read some basic values ...
    int16_t error_code = 0;
    // reset device
    error_code = sen5x_device_reset();
    if (error_code) {
        ESP_LOGE("get_sen5x_values", "Error executing sen5x_device_reset(): %i\n", error_code);
        return -1;
    }
    // start the measurement!
    error_code = sen5x_start_measurement();
    if (error_code) {
        ESP_LOGE("get_sen5x_values", "Error executing sen5x_start_measurement(): %i\n", error_code);
        return -2;
    }
    //
    sensirion_i2c_hal_sleep_usec(1000000);

    uint16_t mass_concentration_pm1p0;
    uint16_t mass_concentration_pm2p5;
    uint16_t mass_concentration_pm4p0;
    uint16_t mass_concentration_pm10p0;
    int16_t ambient_humidity;
    int16_t ambient_temperature;
    int16_t voc_index;
    int16_t nox_index;
    
    // give references to defined vars to the function
    error_code = sen5x_read_measured_values(
        &mass_concentration_pm1p0, &mass_concentration_pm2p5,
        &mass_concentration_pm4p0, &mass_concentration_pm10p0,
        &ambient_humidity, &ambient_temperature, &voc_index, &nox_index);
    if (error_code) {
        ESP_LOGE("get_sen5x_values", "Error executing sen5x_read_measured_values(): %i\n", error_code);
    } else {
        arr[0] = mass_concentration_pm1p0;
        arr[1] = mass_concentration_pm2p5;
        arr[2] = mass_concentration_pm4p0;
        arr[3] = mass_concentration_pm10p0;
        arr[4] = mass_concentration_pm10p0;
        arr[5] = ambient_humidity;
        arr[6] = ambient_temperature;
        arr[7] = voc_index;
        arr[8] = nox_index;
        ESP_LOGI("get_sen5x_values", "Current measurement: \r\nPM1.0: %.1f\r\nPM2.5: %.1f\r\nPM4.0: %.1f\r\nPM10.0: %.1f\r\nTemperature: %.1f\r\nHumidity: %.1f\r\nNOX: %.1f\r\nVOC: %.1f\r\n", mass_concentration_pm1p0 / 10.0f, mass_concentration_pm2p5 / 10.0f, mass_concentration_pm4p0 / 10.0f, mass_concentration_pm10p0 / 10.0f, ambient_humidity / 100.0f, ambient_temperature / 200.0f, voc_index / 10.0f, nox_index / 10.0f);
        // measurement successful, return 0
        return 0;
    }

    arr[0] = mass_concentration_pm1p0;
    arr[1] = mass_concentration_pm2p5;
    arr[2] = mass_concentration_pm4p0;
    arr[3] = mass_concentration_pm10p0;
    arr[4] = mass_concentration_pm10p0;
    arr[5] = ambient_humidity;
    arr[6] = ambient_temperature;
    arr[7] = voc_index;
    arr[8] = nox_index;
    // measurement failed, return -1
    return -3;
}

int8_t get_scd4x_values(int32_t *arr){
    // error code used later (return val of sensirion library functions)
    int16_t error_code = 0;
    scd4x_wake_up();
    scd4x_stop_periodic_measurement();
    scd4x_reinit();
    // start measurement!
    error_code = scd4x_start_periodic_measurement();
    if(error_code){
        ESP_LOGE("get_sen5x_values", "Error while executing the measurement function: %i", error_code);
    }
    for (;;) {
        sensirion_i2c_hal_sleep_usec(100000);
        bool data_ready = false;
        error_code = scd4x_get_data_ready_flag(&data_ready);
        if (error_code) {
            printf("Error while calling data_ready_flag: %i\n", error_code);
            continue;
        }
        if (!data_ready) {
            continue;
        }
        // vars to store measured values
        uint16_t carbon_dioxide;
        int32_t ambient_temperature;
        int32_t ambient_humidity;
        error_code = scd4x_read_measurement(&carbon_dioxide, &ambient_temperature, &ambient_humidity);
        if (error_code) {
            ESP_LOGE("get_scd4x_values", "Error while trying to read measured values: %i", error_code);
            return -1;
        } else if (carbon_dioxide == 0) {
            // invalid measurement!
            ESP_LOGE("get_scd4x_values", "Measurement invalid");
        } else {
            arr[0] = carbon_dioxide;
            arr[1] = ambient_temperature;
            arr[2] = ambient_humidity;
            // measurement successful, return 0
            return 0;
        }
    }
}

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    // STA_START signal -> connect
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 10) {
            // try to reconnect after disconnect (10 times)
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI("event_handler", "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        // connection failed
        ESP_LOGI("event_handler","connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        // GOT_IP signal -> print ip to console
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI("event_handler", "got ip: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

esp_netif_t *sta_ptr;
int8_t connect_to_wifi(){
    s_wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_netif_init());
    // fix: https://github.com/espressif/esp-idf/issues/5609
    esp_event_loop_create_default();
    // ESP_ERROR_CHECK(esp_event_loop_create_default());
    sta_ptr = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &g_wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start() );
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);
    // connected to wifi!
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI("connect_to_wifi", "connected to ap");
        connected_bit = 1;
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        // connection failed
        ESP_LOGI("connect_to_wifi", "Failed to connect");
    } else {
        ESP_LOGE("connect_to_wifi", "UNEXPECTED EVENT");
    }
    // no connection, return fail
    return ESP_FAIL;
}

extern "C" void app_main(void)
{
    // set the esp32 log level 
    esp_log_level_set("app_main", ESP_LOG_DEBUG);
    // print firmware version for logging purposes
    ESP_LOGI("Firmware", "Version: %d", FIRMWARE_VERSION);
    // init nvs flash (should store kv-pairs later on)
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGE("nvs", "no free nvs pages, recreating nvs!");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    // create nvs storage handle
    nvs_handle_t nvs_str_handle;
    err = nvs_open("storage", NVS_READWRITE, &nvs_str_handle);
    if(err != ESP_OK){
        ESP_LOGE("nvs_prepare", "Error: %s", esp_err_to_name(err));
    }else{
        // proceed with main execution, because no nvs errir was thrown before
        uint8_t nvs_result = 0;
        err = nvs_get_u8(nvs_str_handle, "provisioned", &nvs_result);
        // todo : implement later use of the value 
    }

    // init the sensirion hal so it can be used by both following functions
    sensirion_i2c_hal_init();
    // alloc memory for the arr passed after
    uint16_t* sen5x_values = new uint16_t[8];
    int8_t return_code_sen = get_sen5x_values(sen5x_values);
    // scd4x carbon dioxide measurement
    int32_t* scd4x_values = new int32_t[3];
    int8_t return_code_scd = get_scd4x_values(scd4x_values);

    // send measured values over wifi to a server (to be implemented)
    connect_to_wifi();

    // free resources, to save memory!!!
    delete[] sen5x_values;
    delete[] scd4x_values;
}
