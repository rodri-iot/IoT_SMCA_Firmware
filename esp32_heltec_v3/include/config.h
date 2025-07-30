#ifndef CONFIG_H
#define CONFIG_H

extern const char* WIFI_SSID;
extern const char* WIFI_PASS;

extern const char* MQTT_BROKER_URI;
extern const char* MQTT_CLIENT_ID;
extern const char* MQTT_USERNAME;
extern const char* MQTT_PASSWORD;
extern const char* MQTT_TOPIC_PUB;

#define MQTT_QOS 1

#define CERT_CA_PATH       "/spiffs/ca_cert.pem"
#define CERT_CLIENT_CRT    "/spiffs/client_cert.pem"
#define CERT_CLIENT_KEY    "/spiffs/client_key.pem"

#endif
