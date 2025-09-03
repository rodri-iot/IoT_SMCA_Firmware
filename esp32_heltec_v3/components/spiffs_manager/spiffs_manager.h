#ifndef SPIFFS_MANAGER_H
#define SPIFFS_MANAGER_H

#include "esp_err.h"

extern char *ca_crt;
extern char *client_crt;
extern char *client_key;


void spiffs_mount();
void spiffs_unmount();
esp_err_t spiffs_read_file(const char *path, char **buffer);

void read_certificates(void);


#endif
