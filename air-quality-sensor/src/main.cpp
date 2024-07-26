/*
    part of the air measurement project
*/
#include <stdio.h>
#include <memory.h>
#include <string>
#include "nvs_flash.h"
#include "esp_log.h"
#include "sensirion-lib/sen5x_i2c.h"
#include "sensirion-lib/sensirion_i2c_hal.h"
#include "sensirion-lib/sensirion_common.h"

#define FIRMWARE_VERSION 1

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

    // read some basic values ...
    int16_t error_code = 0;
    // init the HAL library, implementation located in sensirion_i2c_hal.c
    sensirion_i2c_hal_init();
    // reset device
    error_code = sen5x_device_reset();
    if (error_code) {
        printf("Error executing sen5x_device_reset(): %i\n", error_code);
    }
    // start the measurement!
    error_code = sen5x_start_measurement();
    if (error_code) {
        printf("Error executing sen5x_start_measurement(): %i\n", error_code);
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
        printf("Error executing sen5x_read_measured_values(): %i\n", error_code);
    } else {
        // measurement successful!
        // print values
        printf("Current measurement: \r\nPM1.0: %.1f\r\nPM2.5: %.1f\r\nPM4.0: %.1f\r\nPM10.0: %.1f\r\nTemperature: %.1f\r\nHumidity: %.1f\r\nNOX: %.1f\r\nVOC: %.1f\r\n", mass_concentration_pm1p0 / 10.0f, mass_concentration_pm2p5 / 10.0f, mass_concentration_pm4p0 / 10.0f, mass_concentration_pm10p0 / 10.0f, ambient_humidity / 100.0f, ambient_temperature / 200.0f, voc_index / 10.0f, nox_index / 10.0f);
    }

    // todo: add measurements of carbon dioxide sensor, send them to a server, visualize them with grafana or similar vis sofwtware
}
