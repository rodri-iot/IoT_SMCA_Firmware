#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_manager.h"
#include "wifi_credentials.h"
#include "mqtt_client.h" 
#include "smca_mqtt_client.h"
#include "spiffs_manager.h"
#include "sensor_sim.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"

#include "esp_netif_sntp.h"
#include <time.h>

static void sync_time_init_and_wait(void);

void app_main(void) {
    ESP_LOGI("MAIN", "🚀 Iniciando SMCA - Heltec V3");

    esp_log_level_set("esp-tls", ESP_LOG_DEBUG);
    esp_log_level_set("mqtt_client", ESP_LOG_DEBUG);
    esp_log_level_set("esp-tls-mbedtls", ESP_LOG_DEBUG);

    spiffs_mount();
    read_certificates();
    
    wifi_init_sta(WIFI_SSID, WIFI_PASS);

    vTaskDelay(pdMS_TO_TICKS(5000));

    sync_time_init_and_wait();

    esp_mqtt_client_handle_t mqtt_client = mqtt_start();

    start_sensor_simulation(mqtt_client);

    vTaskDelay(pdMS_TO_TICKS(5000));

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void sync_time_init_and_wait(void) {
    // Opcional: setea zona horaria si querés logs con hora local
    setenv("TZ", "BOT4", 1);  // Cochabamba (sin DST); ajusta si quieres
    tzset();

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    esp_netif_sntp_init(&config);

    // Esperar hasta que SNTP tenga tiempo válido (tm_year >= 120 ~ año >= 2020)
    const int max_retries = 30;
    for (int i = 0; i < max_retries; i++) {
        time_t now = 0;
        struct tm ti = {0};
        time(&now);
        localtime_r(&now, &ti);
        if (ti.tm_year >= 125) {
            ESP_LOGI("SNTP", "Tiempo sincronizado: %s", asctime(&ti));
            break;
        }
        ESP_LOGI("SNTP", "Esperando sincronización de tiempo... (%d/%d), intentos", i + 1, max_retries);
        vTaskDelay(pdMS_TO_TICKS(250));
        if (i == max_retries - 1) {
            ESP_LOGE("SNTP", "Fallo al sincronizar el tiempo");
        }
    }
}