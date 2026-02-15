# IoT SMCA Firmware - ESP32 V3 Heltec

Firmware para dispositivos ESP32-S3 Heltec V3.2 que envían datos de sensores ambientales por MQTTs al sistema SMCA.

## 📋 Descripción del Proyecto

Este proyecto implementa firmware para hasta 4 nodos ESP32-S3 Heltec que:
- **Fase 1**: Simulan datos de sensores ambientales (pm25, pm10, no2, co2, co, temperatura, humedad)
- **Fase 2**: Leen datos reales de sensores conectados físicamente
- Envían datos por MQTTs (TLS) al broker del sistema SMCA
- Utilizan certificados cliente para autenticación

## 🏗️ Arquitectura del Firmware

### Estructura Modular

```
esp32_heltec_v3/
├── main/
│   ├── main.c              # Punto de entrada, orquestación
│   ├── config.h            # Configuración centralizada (gitignored)
│   ├── config.h.example    # Plantilla para crear config.h
│   └── CMakeLists.txt
├── components/
│   ├── wifi_manager/       # Gestión de conexión WiFi
│   ├── spiffs_manager/     # Sistema de archivos para certificados
│   ├── smca_mqtt_client/   # Cliente MQTT con TLS
│   └── sensor_sim/         # Simulación de sensores (Fase 1)
│   └── sensor_driver/      # Drivers de sensores reales (Fase 2)
└── partitions.csv          # Tabla de particiones (SPIFFS para certs)
```

### Flujo de Ejecución

```
1. Inicialización
   ├── Montar SPIFFS
   ├── Cargar certificados TLS (CA, Client Cert, Client Key)
   └── Inicializar WiFi

2. Conexión
   ├── Conectar a WiFi
   ├── Sincronizar tiempo (SNTP) - Requerido para TLS
   └── Conectar a MQTT Broker (mqtts://)

3. Operación
   ├── Fase 1: Generar datos simulados
   ├── Fase 2: Leer sensores reales
   ├── Formatear JSON según protocolo SMCA
   └── Publicar en tópico: "iot/aire/lectura"
```

### Protocolo de Mensajes MQTT

El firmware envía mensajes JSON con el siguiente formato:

```json
{
  "sensorId": "esp32-air-003",
  "timestamp": "2026-01-25T10:30:00.000Z",
  "pm25": 25.5,
  "pm10": 45.2,
  "no2": 15.3,
  "co2": 450.0,
  "co": 1.2,
  "temperatura": 22.5,
  "humedad": 65.0
}
```

**Importante:**
- Solo se envía `sensorId` (el backend resuelve empresaId y dispositivoId)
- Los parámetros ambientales son opcionales (3-8 según el nodo)
- El timestamp es opcional pero recomendado

### Configuración Multi-Nodo

**config.h es la única fuente de configuración.** Todos los componentes obtienen sus valores desde este archivo.

1. **config.h**: Crear desde plantilla (`cp config.h.example config.h`) y editar:
   - SENSOR_ID, MQTT_CLIENT_ID, MQTT_BROKER_URI
   - WIFI_SSID, WIFI_PASS
2. **Certificados en SPIFFS**: 
   - `/spiffs/ca.crt` (CA del broker)
   - `/spiffs/client.crt` (Certificado del cliente)
   - `/spiffs/client.key` (Clave privada del cliente)

## 🔧 Requisitos

### ESP-IDF

**Repositorio oficial:** https://github.com/espressif/esp-idf

**Versión recomendada:** ESP-IDF v5.1 o superior (soporte completo para ESP32-S3)

**Instalación:**
```bash
# Clonar ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout v5.1
./install.sh esp32s3

# Configurar entorno
. ./export.sh
```

### Hardware

- **Placa:** Heltec WiFi LoRa 32 (V3.2)
- **MCU:** ESP32-S3FN8 (Xtensa 32-bit LX7 dual-core, 240 MHz)
- **LoRa:** SX1262 (no utilizado en este proyecto)
- **Flash:** 8MB SiP Flash
- **Memoria:** 512KB SRAM, 384KB ROM
- **Conectividad:** WiFi 802.11 b/g/n, Bluetooth 5 (LE)

### Características del ESP32-S3 Heltec V3.2

- USB Type-C para programación y alimentación
- Interfaz de batería LiPo (3.7V) con gestión integrada
- Display OLED 0.96" 128x64 (opcional para debug)
- Antena WiFi/Bluetooth integrada (2.4GHz)
- 36 pines GPIO disponibles
- Protección ESD y regulación de voltaje integrada

## 📦 Estructura de Componentes

### 1. wifi_manager
- Inicializa WiFi en modo Station (STA)
- Maneja eventos de conexión/desconexión
- Reintentos automáticos

### 2. spiffs_manager
- Monta sistema de archivos SPIFFS
- Lee certificados TLS desde partición SPIFFS
- Almacena certificados en memoria para uso por MQTT

### 3. smca_mqtt_client
- Cliente MQTT con soporte TLS (mqtts://)
- Autenticación mediante certificados cliente
- Manejo de eventos MQTT (conexión, desconexión, errores)
- Publicación con QoS 1

### 4. sensor_sim (Fase 1)
- Genera valores simulados para parámetros ambientales
- Rangos realistas según tipo de sensor
- Intervalo configurable de envío

### 5. sensor_driver (Fase 2 - Futuro)
- Drivers para sensores I2C/SPI/UART
- Lectura de sensores reales
- Calibración y validación de datos

## 🔐 Certificados TLS

Los certificados deben estar en la partición SPIFFS:

```
/spiffs/
├── ca_crt.pem          # Certificado CA del broker
├── client_crt.pem      # Certificado del cliente (nodo-03)
└── client_key.pem      # Clave privada del cliente
```

**Para nodo-03 (esp32-air-003):**
- Los certificados deben generarse con el script del proyecto SMCA
- Copiar desde: `mqtt/certs/clients/nodo03/`
- Subir a SPIFFS antes de flashear

## 🚀 Compilación y Flasheo

### Configurar proyecto

```bash
cd esp32_heltec_v3
idf.py set-target esp32s3
idf.py menuconfig
```

**Configuraciones importantes:**
- Partition Table: Custom (usar `partitions.csv`)
- SPIFFS: Habilitado
- WiFi: Habilitado
- MQTT: Habilitado
- TLS: Habilitado

### Compilar

```bash
idf.py build
```

### Flashear

```bash
# Flashear firmware + particiones
idf.py flash

# Flashear SPIFFS con certificados
idf.py spiffsgen
idf.py spiffs-flash
```

### Monitoreo

```bash
idf.py monitor
```

## 📝 Configuración por Nodo

### Nodo-03 (esp32-air-003)

1. **Crear config.h** (primera vez): `cp config.h.example config.h`

2. **Editar config.h** (configuración centralizada):
```c
#define SENSOR_ID "esp32-air-003"
#define MQTT_CLIENT_ID "esp32-air-003"
#define MQTT_BROKER_URI "mqtts://tu-broker:8883"
#define WIFI_SSID "tu-red-wifi"
#define WIFI_PASS "tu-password"
```

Todos los componentes (main, sensor_sim, smca_mqtt_client) obtienen la configuración desde config.h.

## 🔄 Fases de Desarrollo

### Fase 1: Simulación (Actual)
- ✅ WiFi Manager
- ✅ SPIFFS Manager
- ✅ MQTT Client con TLS
- ✅ Simulación de sensores
- ✅ Formato de mensajes SMCA

### Fase 2: Sensores Reales
- ⏳ Drivers I2C para sensores
- ⏳ Calibración de sensores
- ⏳ Validación de datos
- ⏳ Manejo de errores de sensores

## 🐛 Troubleshooting

### Error: "Certificados no cargados"
- Verificar que SPIFFS esté montado correctamente
- Verificar que los certificados existan en `/spiffs/`
- Verificar tamaño de partición SPIFFS

### Error: "Fallo al sincronizar el tiempo"
- Verificar conexión WiFi
- Verificar acceso a servidor NTP
- Aumentar timeout en `sync_time_init_and_wait()`

### Error: "MQTT connection failed"
- Verificar certificados TLS
- Verificar que el broker esté accesible
- Verificar configuración de red

## 📚 Referencias

- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [Heltec ESP32 V3 Documentation](https://docs.heltec.cn/)
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [MQTT Client ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/protocols/mqtt.html)

## 📄 Licencia

Ver archivo LICENSE
