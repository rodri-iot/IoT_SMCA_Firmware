#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"
#include "spiffs_manager.h"
#include "esp_sntp.h"
#include <time.h>
#include <sys/time.h>

static const char *TAG = "MQTT";
static esp_mqtt_client_handle_t client = NULL;

extern char *ca_crt;
extern char *client_crt;
extern char *client_key;

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

esp_mqtt_client_handle_t mqtt_start(void) {
    if (!ca_crt || !client_crt || !client_key) {
        ESP_LOGE(TAG, "❌ Certificados no cargados, no se puede iniciar MQTT");
        return NULL;
    }

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = "mqtts://192.168.0.13:8883",
        .credentials = {
            .authentication = {
                .certificate = client_crt,
                .key = client_key
            },
            .client_id = "heltec_v3"
        },
        .broker.verification.certificate = ca_crt
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (!client) {
        ESP_LOGE(TAG, "❌ Fallo al inicializar el cliente MQTT");
        return NULL;
    }

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    ESP_LOGI(TAG, "Iniciando cliente MQTT...");

    return client;
}