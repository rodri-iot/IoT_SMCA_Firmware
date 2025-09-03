#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "smca_mqtt_client.h"
#include <stdlib.h>

#define SENSOR_ID "nodo01"
#define ZONA_ID "zona1"

static const char *TAG = "SENSOR_SIM";

static float random_float(float min, float max) {
    return min + (float)rand() / ((float)RAND_MAX / (max - min));
}

static void sensor_simulation_task(void *pvParameters) {
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t) pvParameters;
    while (1) {
        char payload[256];

        float pm25 = random_float(5.0, 50.0);
        float pm10 = random_float(10.0, 80.0);
        float co = random_float(0.1, 5.0);
        float co2 = random_float(400.0, 800.0);
        float no2 = random_float(0.01, 0.2);
        float voc = random_float(100.0, 600.0);
        float temp = random_float(20.0, 35.0);
        float hum = random_float(30.0, 70.0);

        snprintf(payload, sizeof(payload),
                 "{\"sensorId\":\"%s\",\"zona\":\"%s\",\"pm25\":%.2f,\"pm10\":%.2f,\"co\":%.2f,\"co2\":%.2f,\"no2\":%.3f,\"voc\":%.1f,\"temp\":%.1f,\"hum\":%.1f}",
                 SENSOR_ID, ZONA_ID, pm25, pm10, co, co2, no2, voc, temp, hum);

        int msg_id = esp_mqtt_client_publish(client, "smca/nodo01/mediciones", payload, 0, 1, 0);

        if (msg_id >= 0) {
            ESP_LOGI(TAG, "📤 Publicado (id=%d): %s", msg_id, payload);
        } else {
            ESP_LOGE(TAG, "❌ Error publicando el mensaje");
        }

        vTaskDelay(pdMS_TO_TICKS(5000));  // Enviar cada 5s
    }
}

void start_sensor_simulation(esp_mqtt_client_handle_t client) {
    xTaskCreate(sensor_simulation_task, "sensor_sim_task", 4096, client, 5, NULL);
}

/*
void start_sensor_simulation() {
    xTaskCreate(sensor_simulation_task, "sensor_sim_task", 4096, NULL, 5, NULL);
}
*/