#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "config.h"
#if USE_REAL_SCD40
#include "sensor_scd40.h"
#endif
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

static const char *TAG = "SENSOR_SIM";

static float random_float(float min, float max) {
    return min + (float)rand() / ((float)RAND_MAX / (max - min));
}

static const char* get_timestamp(void) {
    static char timestamp[64];
    struct timeval tv;
    gettimeofday(&tv, NULL);
    // Usar gmtime() para obtener UTC (estándar para APIs y bases de datos)
    // Esto asegura que los timestamps sean consistentes independientemente
    // de la zona horaria del dispositivo o del servidor (AWS US East)
    struct tm *timeinfo = gmtime(&tv.tv_sec);
    snprintf(timestamp, sizeof(timestamp), "%04d-%02d-%02dT%02d:%02d:%02dZ",
             timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
             timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    return timestamp;
}

static void sensor_simulation_task(void *pvParameters) {
    esp_mqtt_client_handle_t client = (esp_mqtt_client_handle_t) pvParameters;
    while (1) {
        char payload[256];

        float pm25 = random_float(5.0, 50.0);
        float pm10 = random_float(10.0, 80.0);
        float co = random_float(0.1, 5.0);
        float no2 = random_float(0.01, 0.2);

#if USE_REAL_SCD40
        float co2, temp, hum;
        esp_err_t connected = scd40_is_connected();
        if (connected != ESP_OK) {
            ESP_LOGW(TAG, "SCD40 no responde (I2C)");
            snprintf(payload, sizeof(payload),
                     "{\"sensorId\":\"%s\",\"timestamp\":\"%s\",\"pm25\":%.2f,\"pm10\":%.2f,\"co\":%.2f,\"no2\":%.3f}",
                     SENSOR_ID, get_timestamp(), pm25, pm10, co, no2);
        } else {
            esp_err_t read_err = scd40_read_measurement(&co2, &temp, &hum);
            if (read_err == ESP_OK) {
                ESP_LOGI(TAG, "SCD40 conectado, lectura OK (CO2=%.0f ppm)", co2);
                snprintf(payload, sizeof(payload),
                         "{\"sensorId\":\"%s\",\"timestamp\":\"%s\",\"pm25\":%.2f,\"pm10\":%.2f,\"co\":%.2f,\"co2\":%.2f,\"no2\":%.3f,\"temp\":%.1f,\"hum\":%.1f}",
                         SENSOR_ID, get_timestamp(), pm25, pm10, co, co2, no2, temp, hum);
            } else {
                ESP_LOGW(TAG, "SCD40 conectado pero sin datos nuevos (enviando sin co2/temp/hum)");
                snprintf(payload, sizeof(payload),
                         "{\"sensorId\":\"%s\",\"timestamp\":\"%s\",\"pm25\":%.2f,\"pm10\":%.2f,\"co\":%.2f,\"no2\":%.3f}",
                         SENSOR_ID, get_timestamp(), pm25, pm10, co, no2);
            }
        }
#else
        float co2 = random_float(400.0, 800.0);
        float temp = random_float(20.0, 35.0);
        float hum = random_float(30.0, 70.0);
        snprintf(payload, sizeof(payload),
                 "{\"sensorId\":\"%s\",\"timestamp\":\"%s\",\"pm25\":%.2f,\"pm10\":%.2f,\"co\":%.2f,\"co2\":%.2f,\"no2\":%.3f,\"temp\":%.1f,\"hum\":%.1f}",
                 SENSOR_ID, get_timestamp(), pm25, pm10, co, co2, no2, temp, hum);
#endif

        int msg_id = esp_mqtt_client_publish(client, MQTT_TOPIC, payload, 0, 1, 0);

        if (msg_id >= 0) {
            ESP_LOGI(TAG, "📤 Publicado (id=%d): %s", msg_id, payload);
        } else {
            ESP_LOGE(TAG, "❌ Error publicando el mensaje");
        }

        vTaskDelay(pdMS_TO_TICKS(SEND_INTERVAL_MS));
    }
}

void start_sensor_simulation(esp_mqtt_client_handle_t client) {
    if (client == NULL) {
        ESP_LOGE(TAG, "❌ Cliente MQTT es NULL, no se puede iniciar simulación");
        return;
    }
    xTaskCreate(sensor_simulation_task, "sensor_sim_task", 8192, client, 5, NULL);
    ESP_LOGI(TAG, "✅ Tarea de simulación de sensores creada");
}