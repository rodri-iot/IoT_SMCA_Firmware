#include "mqtt_client.h"
#include "config.h"
#include "spiffs_manager.h"
#include "sensor_sim.h"

#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t client = NULL;

static void publish_data_loop(void *param) {
    while (1) {
        char payload[256];
        snprintf(payload, sizeof(payload),
                 "{\"pm25\": %.2f, \"pm10\": %.2f, \"co2\": %.2f, \"no2\": %.2f, \"co\": %.2f, \"voc\": %.2f, \"temp\": %.2f, \"hum\": %.2f}",
                 simular_pm25(), simular_pm10(), simular_co2(), simular_no2(),
                 simular_co(), simular_voc(), simular_temp(), simular_hum());

        int msg_id = esp_mqtt_client_publish(client, MQTT_TOPIC_PUB, payload, 0, MQTT_QOS, 0);
        ESP_LOGI(TAG, "📤 Publicado (ID: %d): %s", msg_id, payload);

        vTaskDelay(pdMS_TO_TICKS(10000));  // Cada 10 segundos
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_t *event = event_data;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "✅ Conectado al broker MQTT");
            xTaskCreate(publish_data_loop, "mqtt_pub_task", 4096, NULL, 5, NULL);
            break;

        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "🔌 Desconectado del broker MQTT");
            break;

        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "❌ Error en el cliente MQTT");
            break;

        default:
            break;
    }
}

void mqtt_start(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = MQTT_BROKER_URI,
            .verification = {
                .certificate = get_cert_path_ca()
            }
        },
        .credentials = {
            .authentication = {
                .certificate = get_cert_path_client_crt(),
                .key = get_cert_path_client_key(),
                .username = MQTT_USERNAME,
                .password = MQTT_PASSWORD,
            },
            .client_id = MQTT_CLIENT_ID
        }
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}
