#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include "mqtt_client.h"

extern esp_mqtt_client_handle_t client;

esp_mqtt_client_handle_t mqtt_start(void);

#endif
