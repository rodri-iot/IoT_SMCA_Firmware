#include "mqtt_client.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "esp_err.h"
#include "esp_tls.h"

static const char *TAG = "MQTT";

static esp_mqtt_client_handle_t client = NULL;

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "✅ Conectado al broker MQTT");
            esp_mqtt_client_publish(client, "smca/nodo01/estado", "Nodo conectado", 0, 1, 0);
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
        .broker.address.uri = "mqtts://mosquitto:8883",
        .credentials = {
            .authentication = {
                .certificate = (const char *)"spiffs/client_cert.pem",
                .key = (const char *)"spiffs/client_key.pem"
            },
            .client_id = "heltec_v3"
        },
        .broker.verification.certificate = (const char *)"spiffs/ca_cert.pem"
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}