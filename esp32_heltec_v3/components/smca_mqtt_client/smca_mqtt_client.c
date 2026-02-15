#include "mqtt_client.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"
#include "spiffs_manager.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

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
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "🔌 Desconectado del broker MQTT");
            break;
        case MQTT_EVENT_ERROR:
            if (event->error_handle) {
                ESP_LOGE(TAG, "❌ Error MQTT: error_type=%d, esp_tls_last_esp_err=0x%x", 
                         event->error_handle->error_type, 
                         event->error_handle->esp_tls_last_esp_err);
            } else {
                ESP_LOGE(TAG, "❌ Error en el cliente MQTT");
            }
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGD(TAG, "📤 Mensaje publicado (msg_id=%d)", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGD(TAG, "📥 Datos recibidos (topic=%.*s, data_len=%d)", 
                     event->topic_len, event->topic, event->data_len);
            break;
        default:
            ESP_LOGD(TAG, "Evento MQTT: %d", event->event_id);
            break;
    }
}

esp_mqtt_client_handle_t mqtt_start(const char *broker_uri, const char *client_id) {
    if (!ca_crt || !client_crt || !client_key) {
        ESP_LOGE(TAG, "❌ Certificados no cargados, no se puede iniciar MQTT");
        return NULL;
    }

    static char lwt_msg[96];
    int lwt_len = snprintf(lwt_msg, sizeof(lwt_msg), "{\"sensorId\":\"%s\",\"estado\":\"offline\"}", SENSOR_ID);
    if (lwt_len < 0 || (size_t)lwt_len >= sizeof(lwt_msg)) {
        lwt_len = (int)strlen(lwt_msg);
    }

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = broker_uri,
        .credentials = {
            .authentication = {
                .certificate = client_crt,
                .key = client_key
            },
            .client_id = client_id
        },
        .broker.verification.certificate = ca_crt,
        .session.last_will = {
            .topic = "iot/aire/status",
            .msg = lwt_msg,
            .msg_len = lwt_len,
            .qos = 1,
            .retain = true
        }
    };

    client = esp_mqtt_client_init(&mqtt_cfg);
    if (!client) {
        ESP_LOGE(TAG, "❌ Fallo al inicializar el cliente MQTT");
        return NULL;
    }

    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
    ESP_LOGI(TAG, "🚀 Iniciando cliente MQTT (broker: %s, client_id: %s)", broker_uri, client_id);

    return client;
}