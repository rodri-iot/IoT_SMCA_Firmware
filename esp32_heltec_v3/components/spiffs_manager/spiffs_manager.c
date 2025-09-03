#include "spiffs_manager.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include <stdlib.h>
#include <string.h>
#include "esp_err.h"


static const char *TAG = "SPIFFS";

char *ca_crt = NULL;
char *client_crt = NULL;
char *client_key = NULL;

void spiffs_mount() {
    ESP_LOGI(TAG, "🔧 Montando SPIFFS..."); // Log para verificar
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Fallo al montar SPIFFS (%s)", esp_err_to_name(ret));
        return;
    }

    size_t total = 0, used = 0;
    esp_spiffs_info(NULL, &total, &used);
    ESP_LOGI(TAG, "SPIFFS montado: total: %d, usado: %d", total, used);
}

void spiffs_unmount() {
    esp_vfs_spiffs_unregister(NULL);
    ESP_LOGI(TAG, "SPIFFS desmontado");
}

esp_err_t spiffs_read_file(const char *path, char **buffer) {
    FILE *f = fopen(path, "r");
    if (!f) {
        ESP_LOGE(TAG, "Fallo al abrir el archivo: %s", path);
        return ESP_FAIL;
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    *buffer = malloc(size + 1);
    if (!*buffer) {
        ESP_LOGE(TAG, "Memory allocation failed for %s", path);
        fclose(f);
        return ESP_ERR_NO_MEM;
    }

    fread(*buffer, 1, size, f);
    (*buffer)[size] = '\0';  // Null-terminate

    fclose(f);
    ESP_LOGI(TAG, "Read file %s (%d bytes)", path, (int)size);
    return ESP_OK;
}

void read_certificates() {
    esp_err_t ret;
    ret = spiffs_read_file("/spiffs/ca_crt.pem", &ca_crt);
    if (ret != ESP_OK || !ca_crt) {
        ESP_LOGE(TAG, "❌ Error al leer ca_crt.pem: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "✅ ca_crt.pem cargado (%d bytes)", strlen(ca_crt));
    }
    ret = spiffs_read_file("/spiffs/client_crt.pem", &client_crt);
    if (ret != ESP_OK || !client_crt) {
        ESP_LOGE(TAG, "❌ Error al leer client_crt.pem: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "✅ client_crt.pem cargado (%d bytes)", strlen(client_crt));
    }
    ret = spiffs_read_file("/spiffs/client_key.pem", &client_key);
    if (ret != ESP_OK || !client_key) {
        ESP_LOGE(TAG, "❌ Error al leer client_key.pem: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "✅ client_key.pem cargado (%d bytes)", strlen(client_key));
    }
    if (ca_crt && client_crt && client_key) {
        ESP_LOGI(TAG, "📄 Certificados cargados correctamente");
    } else {
        ESP_LOGE(TAG, "❌ Error al leer los certificados");
    }
}

/*
void read_certificates() {
    if (spiffs_read_file("/spiffs/ca_crt.pem", &ca_crt) != ESP_OK) {
        ESP_LOGE(TAG, "❌ Error al leer ca_crt.pem");
    }
    if (spiffs_read_file("/spiffs/client_crt.pem", &client_crt) != ESP_OK) {
        ESP_LOGE(TAG, "❌ Error al leer client_crt.pem");
    }
    if (spiffs_read_file("/spiffs/client_key.pem", &client_key) != ESP_OK) {
        ESP_LOGE(TAG, "❌ Error al leer client_key.pem");
    }

    if (ca_crt && client_crt && client_key) {
        ESP_LOGI(TAG, "📄 Certificados cargados correctamente");
    } else {
        ESP_LOGE(TAG, "❌ Error al leer los certificados");
    }
}
*/