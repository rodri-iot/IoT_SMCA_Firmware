#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_manager.h"
#include "wifi_credentials.h"
#include "mqtt_client.h" 
#include "smca_mqtt_client.h"
#include "spiffs_manager.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"

void app_main(void) {
    ESP_LOGI("MAIN", "🚀 Iniciando SMCA - Heltec V3");

    spiffs_mount();
    read_certificates();
    
    wifi_init_sta(WIFI_SSID, WIFI_PASS);

    vTaskDelay(pdMS_TO_TICKS(5000));

    mqtt_start();

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}