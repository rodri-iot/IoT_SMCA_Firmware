#include "config.h"
#include "spiffs_manager.h"
#include "mqtt_client.h"
#include "sensor_sim.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_system.h"

static const char *TAG = "MAIN";

void app_main(void) {
    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_LOGI(TAG, "🚀 Iniciando nodo SMCA con ESP-IDF");

    // 1. Inicializar NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    // 2. Inicializar red y WiFi
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = { 0 };
    strncpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, WIFI_PASS, sizeof(wifi_config.sta.password));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "📡 WiFi inicializado, conectando...");
    ESP_ERROR_CHECK(esp_wifi_connect());

    // Esperar conexión WiFi (opcional pero recomendable)
    wifi_ap_record_t ap_info;
    int intentos = 0;
    while (esp_wifi_sta_get_ap_info(&ap_info) != ESP_OK && intentos++ < 20) {
        ESP_LOGI(TAG, "⏳ Esperando conexión WiFi...");
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // 3. Montar SPIFFS
    ESP_ERROR_CHECK(spiffs_mount());

    // 4. Iniciar MQTT
    mqtt_start();
}
