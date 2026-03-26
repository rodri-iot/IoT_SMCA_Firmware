#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "mqtt_client.h"
#include "config.h"
#if USE_REAL_SCD40
#include "sensor_scd40.h"
#endif
#if USE_REAL_SGP30
#include "sensor_sgp30.h"
#endif
#if USE_REAL_PMS7003
#include "sensor_pms7003.h"
#endif
#if USE_REAL_CJMCU
#include "sensor_cjmcu4541.h"
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
        char payload[460];

        float pm25, pm10;
#if USE_REAL_PMS7003
        uint16_t pm25_u, pm10_u;
        if (pms7003_read(NULL, &pm25_u, &pm10_u) == ESP_OK) {
            pm25 = (float)pm25_u;
            pm10 = (float)pm10_u;
        } else {
            ESP_LOGW(TAG, "PMS7003 no responde o fallo lectura (PM simulados)");
            pm25 = random_float(5.0, 50.0);
            pm10 = random_float(10.0, 80.0);
        }
#else
        pm25 = random_float(5.0, 50.0);
        pm10 = random_float(10.0, 80.0);
#endif

        float co, no2, nh3;
#if USE_REAL_CJMCU
        if (cjmcu4541_read_measurement(&no2, &co, &nh3) != ESP_OK) {
            ESP_LOGW(TAG, "CJMCU-4541 no responde (co/no2/nh3 simulados)");
            co = random_float(0.1, 5.0);
            no2 = random_float(0.01, 0.2);
            nh3 = random_float(0.0, 2.0);
        }
#else
        co = random_float(0.1, 5.0);
        no2 = random_float(0.01, 0.2);
        nh3 = random_float(0.0, 2.0);
#endif

        uint16_t tvoc_ppb = 0;
#if USE_REAL_SGP30
        if (sgp30_get_latest(&tvoc_ppb, NULL) != ESP_OK) {
            ESP_LOGW(TAG, "SGP30 sin datos validos aun (tvoc=0)");
            tvoc_ppb = 0;
        }
#else
        tvoc_ppb = (uint16_t)(random_float(0.0, 400.0) + 0.5f);
#endif

#if USE_REAL_SCD40
        float co2, temp, hum;
        esp_err_t connected = scd40_is_connected();
        if (connected != ESP_OK) {
            ESP_LOGW(TAG, "SCD40 no responde (I2C)");
            snprintf(payload, sizeof(payload),
                     "{\"sensorId\":\"%s\",\"timestamp\":\"%s\",\"pm25\":%.2f,\"pm10\":%.2f,\"co\":%.2f,\"no2\":%.3f,\"nh3\":%.3f,\"tvoc\":%u}",
                     SENSOR_ID, get_timestamp(), pm25, pm10, co, no2, nh3, (unsigned)tvoc_ppb);
        } else {
            static int scd40_early_retries_left = 2;  /* Retry con delay 5.5 s solo en las 2 primeras veces que falte dato */
            static int scd40_consecutive_not_found = 0;  /* Para recuperación: reintentar start tras N ciclos sin datos */
            esp_err_t read_err = scd40_read_measurement(&co2, &temp, &hum);
            if (read_err == ESP_OK) {
                scd40_consecutive_not_found = 0;
                ESP_LOGI(TAG, "SCD40 conectado, lectura OK (CO2=%.0f ppm)", co2);
                snprintf(payload, sizeof(payload),
                         "{\"sensorId\":\"%s\",\"timestamp\":\"%s\",\"pm25\":%.2f,\"pm10\":%.2f,\"co\":%.2f,\"co2\":%.2f,\"no2\":%.3f,\"nh3\":%.3f,\"temp\":%.1f,\"hum\":%.1f,\"tvoc\":%u}",
                         SENSOR_ID, get_timestamp(), pm25, pm10, co, co2, no2, nh3, temp, hum, (unsigned)tvoc_ppb);
            } else {
                if (read_err == ESP_ERR_NOT_FOUND && scd40_early_retries_left > 0) {
                    vTaskDelay(pdMS_TO_TICKS(5500));
                    read_err = scd40_read_measurement(&co2, &temp, &hum);
                    if (read_err == ESP_OK) {
                        scd40_consecutive_not_found = 0;
                        ESP_LOGI(TAG, "SCD40 conectado, lectura OK tras espera (CO2=%.0f ppm)", co2);
                        snprintf(payload, sizeof(payload),
                                 "{\"sensorId\":\"%s\",\"timestamp\":\"%s\",\"pm25\":%.2f,\"pm10\":%.2f,\"co\":%.2f,\"co2\":%.2f,\"no2\":%.3f,\"nh3\":%.3f,\"temp\":%.1f,\"hum\":%.1f,\"tvoc\":%u}",
                                 SENSOR_ID, get_timestamp(), pm25, pm10, co, co2, no2, nh3, temp, hum, (unsigned)tvoc_ppb);
                    } else {
                        scd40_early_retries_left--;
                        if (read_err == ESP_ERR_NOT_FOUND) {
                            scd40_consecutive_not_found++;
                            ESP_LOGW(TAG, "SCD40: data_ready=0 (medición no lista aún o medición periódica no iniciada; comprobar en arranque si aparece 'Medición periódica iniciada')");
                            if (scd40_consecutive_not_found >= 4) {
                                scd40_stop_periodic_measurement();
                                vTaskDelay(pdMS_TO_TICKS(800));
                                if (scd40_start_periodic_measurement() == ESP_OK) {
                                    ESP_LOGW(TAG, "SCD40: recuperación: reintento de medición periódica");
                                }
                                scd40_consecutive_not_found = 0;
                            }
                        } else {
                            ESP_LOGW(TAG, "SCD40: error de lectura: %s", esp_err_to_name(read_err));
                        }
                        snprintf(payload, sizeof(payload),
                                 "{\"sensorId\":\"%s\",\"timestamp\":\"%s\",\"pm25\":%.2f,\"pm10\":%.2f,\"co\":%.2f,\"no2\":%.3f,\"nh3\":%.3f,\"tvoc\":%u}",
                                 SENSOR_ID, get_timestamp(), pm25, pm10, co, no2, nh3, (unsigned)tvoc_ppb);
                    }
                } else {
                    if (read_err == ESP_ERR_NOT_FOUND) {
                        scd40_consecutive_not_found++;
                        ESP_LOGW(TAG, "SCD40: data_ready=0 (medición no lista aún o medición periódica no iniciada; comprobar en arranque si aparece 'Medición periódica iniciada')");
                        if (scd40_consecutive_not_found >= 4) {
                            scd40_stop_periodic_measurement();
                            vTaskDelay(pdMS_TO_TICKS(800));
                            if (scd40_start_periodic_measurement() == ESP_OK) {
                                ESP_LOGW(TAG, "SCD40: recuperación: reintento de medición periódica");
                            }
                            scd40_consecutive_not_found = 0;
                        }
                    } else {
                        ESP_LOGW(TAG, "SCD40: error de lectura: %s", esp_err_to_name(read_err));
                    }
                    snprintf(payload, sizeof(payload),
                             "{\"sensorId\":\"%s\",\"timestamp\":\"%s\",\"pm25\":%.2f,\"pm10\":%.2f,\"co\":%.2f,\"no2\":%.3f,\"nh3\":%.3f,\"tvoc\":%u}",
                             SENSOR_ID, get_timestamp(), pm25, pm10, co, no2, nh3, (unsigned)tvoc_ppb);
                }
            }
        }
#else
        float co2 = random_float(400.0, 800.0);
        float temp = random_float(20.0, 35.0);
        float hum = random_float(30.0, 70.0);
        snprintf(payload, sizeof(payload),
                 "{\"sensorId\":\"%s\",\"timestamp\":\"%s\",\"pm25\":%.2f,\"pm10\":%.2f,\"co\":%.2f,\"co2\":%.2f,\"no2\":%.3f,\"nh3\":%.3f,\"temp\":%.1f,\"hum\":%.1f,\"tvoc\":%u}",
                 SENSOR_ID, get_timestamp(), pm25, pm10, co, co2, no2, nh3, temp, hum, (unsigned)tvoc_ppb);
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