#include "spiffs_manager.h"
#include "esp_log.h"
#include "esp_spiffs.h"

static const char *TAG = "SPIFFS_MGR";

esp_err_t spiffs_mount(void) {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "📦 SPIFFS montado correctamente");
    } else {
        ESP_LOGE(TAG, "❌ Error montando SPIFFS (%s)", esp_err_to_name(ret));
    }
    return ret;
}

const char* get_cert_path_ca(void)        { return "/spiffs/ca_cert.pem"; }
const char* get_cert_path_client_crt(void){ return "/spiffs/client_cert.pem"; }
const char* get_cert_path_client_key(void){ return "/spiffs/client_key.pem"; }
