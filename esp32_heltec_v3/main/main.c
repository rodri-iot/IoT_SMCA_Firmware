#include <stdio.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif_sntp.h"
#include "config.h"
#include "wifi_manager.h"
#include "smca_mqtt_client.h"
#include "spiffs_manager.h"
#include "sensor_sim.h"
#include "i2c_bus.h"
#if USE_REAL_SCD40
#include "sensor_scd40.h"
#endif

static void sync_time_init_and_wait(void);

void app_main(void) {
    ESP_LOGI("MAIN", "🚀 Iniciando SMCA - Heltec V3.2");
    ESP_LOGI("MAIN", "📡 Nodo: %s (sensorId: %s)", MQTT_CLIENT_ID, SENSOR_ID);

    // Configurar niveles de log
    esp_log_level_set("esp-tls", LOG_LEVEL_TLS);
    esp_log_level_set("esp_mqtt_client", LOG_LEVEL_MQTT);
    esp_log_level_set("esp-tls-mbedtls", ESP_LOG_WARN);
    esp_log_level_set("WiFi", LOG_LEVEL_WIFI);

    // 1. Montar SPIFFS y cargar certificados
    ESP_LOGI("MAIN", "📁 Montando SPIFFS...");
    spiffs_mount();
    read_certificates();

#if USE_REAL_SCD40
    // 1b. Inicializar I2C y SCD40 (Fase 1 - sensores reales)
    ESP_LOGI("MAIN", "🔌 Inicializando I2C (SDA=%d, SCL=%d)...", I2C_SDA_GPIO, I2C_SCL_GPIO);
    esp_err_t err = i2c_bus_init(I2C_SDA_GPIO, I2C_SCL_GPIO, (i2c_port_t)I2C_PORT_NUM);
    if (err != ESP_OK) {
        ESP_LOGE("MAIN", "❌ Fallo I2C: %s", esp_err_to_name(err));
    } else if (scd40_init() != ESP_OK) {
        ESP_LOGW("MAIN", "⚠️ SCD40 no detectado; se usarán valores simulados para CO2/temp/hum");
    } else {
        (void)scd40_stop_periodic_measurement();
        vTaskDelay(pdMS_TO_TICKS(500));
        vTaskDelay(pdMS_TO_TICKS(100));
        esp_err_t start_ret = scd40_start_periodic_measurement();
        if (start_ret == ESP_OK) {
            ESP_LOGI("MAIN", "✅ SCD40 listo (primera lectura en ~5 s)");
        } else {
            ESP_LOGW("MAIN", "SCD40: no se pudo iniciar medición periódica: %s", esp_err_to_name(start_ret));
        }
    }
#endif

    // 2. Inicializar WiFi
    ESP_LOGI("MAIN", "📶 Conectando a WiFi: %s", WIFI_SSID);
    wifi_init_sta(WIFI_SSID, WIFI_PASS);

    // Esperar conexión WiFi (mínimo 5 segundos)
    vTaskDelay(pdMS_TO_TICKS(5000));

    // 3. Sincronizar tiempo (requerido para TLS)
    ESP_LOGI("MAIN", "⏰ Sincronizando tiempo...");
    sync_time_init_and_wait();

    // 4. Iniciar cliente MQTT
    ESP_LOGI("MAIN", "🔌 Conectando a MQTT broker: %s", MQTT_BROKER_URI);
    esp_mqtt_client_handle_t mqtt_client = mqtt_start(MQTT_BROKER_URI, MQTT_CLIENT_ID);
    
    if (mqtt_client == NULL) {
        ESP_LOGE("MAIN", "❌ No se pudo iniciar cliente MQTT");
        return;
    }

    // Esperar conexión MQTT
    vTaskDelay(pdMS_TO_TICKS(3000));

    // 5. Iniciar simulación de sensores
    ESP_LOGI("MAIN", "🌡️ Iniciando simulación de sensores...");
    start_sensor_simulation(mqtt_client);

    ESP_LOGI("MAIN", "✅ Sistema iniciado correctamente");

    // Loop principal
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
        ESP_LOGD("MAIN", "💓 Sistema operativo");
    }
}

static void sync_time_init_and_wait(void) {
    // Configurar zona horaria (opcional, para logs locales)
    setenv("TZ", TIMEZONE, 1);
    tzset();

    // Configurar SNTP con múltiples servidores para mayor confiabilidad
    // Usamos 2 servidores (máximo recomendado para evitar problemas)
    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG_MULTIPLE(2,
        ESP_SNTP_SERVER_LIST("pool.ntp.org", "time.nist.gov")
    );
    config.sync_cb = NULL;  // Sin callback, verificaremos manualmente
    
    esp_err_t ret = esp_netif_sntp_init(&config);
    if (ret != ESP_OK) {
        ESP_LOGE("SNTP", "❌ Error al inicializar SNTP: %s", esp_err_to_name(ret));
        return;
    }

    // Esperar hasta que SNTP tenga tiempo válido (tm_year >= 125 ~ año >= 2025)
    // Aumentamos el tiempo de espera y verificamos más frecuentemente
    const int max_retries = 60;  // 60 intentos = 30 segundos
    bool time_synced = false;
    
    for (int i = 0; i < max_retries; i++) {
        time_t now = 0;
        struct tm timeinfo = {0};
        time(&now);
        localtime_r(&now, &timeinfo);
        
        // Verificar si el tiempo es válido (año >= 2025)
        if (timeinfo.tm_year >= 125) {
            char strftime_buf[64];
            strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
            ESP_LOGI("SNTP", "✅ Tiempo sincronizado: %s", strftime_buf);
            time_synced = true;
            break;
        }
        
        // Log cada 5 intentos para no saturar
        if ((i + 1) % 5 == 0 || i < 5) {
            ESP_LOGI("SNTP", "⏳ Esperando sincronización... (%d/%d)", i + 1, max_retries);
        }
        
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    
    if (!time_synced) {
        ESP_LOGW("SNTP", "⚠️ Timeout: No se pudo sincronizar el tiempo. Continuando sin sincronización.");
        ESP_LOGW("SNTP", "⚠️ MQTT puede funcionar, pero los timestamps pueden ser incorrectos.");
    }
}