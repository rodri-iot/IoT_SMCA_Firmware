#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_manager.h"
#include "wifi_credentials.h"
#include "mqtt_client.h"
#include "esp_log.h"

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

void read_certificates() {
    FILE *f = fopen("/spiffs/client_cert.pem", "r");
    if (f == NULL) {
        ESP_LOGE("CERT", "Failed to open client_cert.pem");
        return;
    }
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        ESP_LOGI("CERT", "%s", line);
    }
    fclose(f);
}