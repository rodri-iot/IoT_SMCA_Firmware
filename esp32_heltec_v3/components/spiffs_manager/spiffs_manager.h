#ifndef SPIFFS_MANAGER_H
#define SPIFFS_MANAGER_H

#include "esp_err.h"

esp_err_t spiffs_mount(void);
const char* get_cert_path_ca(void);
const char* get_cert_path_client_crt(void);
const char* get_cert_path_client_key(void);

#endif
