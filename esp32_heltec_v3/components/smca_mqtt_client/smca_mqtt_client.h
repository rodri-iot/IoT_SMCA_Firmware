#ifndef SMCA_MQTT_CLIENT_H
#define SMCA_MQTT_CLIENT_H

#include "mqtt_client.h"

esp_mqtt_client_handle_t mqtt_start(const char *broker_uri, const char *client_id);

#endif
