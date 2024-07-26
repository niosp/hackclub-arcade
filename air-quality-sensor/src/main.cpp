/*
    part of the air measurement project
*/
#include <stdio.h>
#include <memory.h>
#include <string>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_task_wdt.h"
#include "esp_log.h"

#define FIRMWARE_VERSION 1

extern "C" void app_main(void)
{
    ESP_LOGI("Firmware", "Version: %d", FIRMWARE_VERSION);
}
