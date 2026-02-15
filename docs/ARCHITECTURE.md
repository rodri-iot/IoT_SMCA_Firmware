# 🏗️ Arquitectura del Firmware - ESP32-S3 Heltec V3

## 📐 Visión General

Este firmware implementa un sistema modular para nodos IoT que envían datos ambientales al sistema SMCA mediante MQTTs (TLS). El diseño permite escalar hasta 4 nodos con configuración mínima.

## 🎯 Objetivos del Diseño

1. **Modularidad**: Componentes independientes y reutilizables
2. **Configurabilidad**: Fácil cambio entre nodos mediante archivos de configuración
3. **Seguridad**: Autenticación mediante certificados TLS cliente
4. **Confiabilidad**: Manejo robusto de errores y reconexión automática
5. **Escalabilidad**: Preparado para Fase 2 (sensores reales)

## 📦 Estructura de Componentes

```
esp32_heltec_v3/
│
├── main/
│   ├── main.c                 # Orquestador principal
│   ├── config.h              # ⭐ Configuración centralizada (gitignored)
│   ├── config.h.example      # Plantilla para crear config.h
│   └── CMakeLists.txt
│
├── components/
│   │
│   ├── wifi_manager/         # 🔌 Gestión de WiFi
│   │   ├── wifi_manager.c
│   │   ├── wifi_manager.h
│   │   └── CMakeLists.txt
│   │
│   ├── spiffs_manager/       # 📁 Sistema de archivos
│   │   ├── spiffs_manager.c
│   │   ├── spiffs_manager.h
│   │   └── CMakeLists.txt
│   │
│   ├── smca_mqtt_client/     # 📡 Cliente MQTT con TLS
│   │   ├── smca_mqtt_client.c
│   │   ├── smca_mqtt_client.h
│   │   └── CMakeLists.txt
│   │
│   └── sensor_sim/           # 🌡️ Simulación de sensores (Fase 1)
│       ├── sensor_sim.c
│       ├── sensor_sim.h
│       └── CMakeLists.txt
│
└── partitions.csv            # Tabla de particiones (SPIFFS para certs)
```

## 🔄 Flujo de Ejecución

### 1. Inicialización (app_main)

```
┌─────────────────────────────────────────┐
│  app_main()                             │
│                                         │
│  1. Configurar logs                    │
│  2. Montar SPIFFS                      │
│  3. Cargar certificados TLS            │
│  4. Inicializar WiFi                   │
│  5. Esperar conexión WiFi              │
│  6. Sincronizar tiempo (SNTP)          │
│  7. Iniciar cliente MQTT               │
│  8. Iniciar simulación de sensores     │
│  9. Loop principal                     │
└─────────────────────────────────────────┘
```

### 2. Gestión de WiFi

```
wifi_manager
├── wifi_init_sta(ssid, password)
│   ├── Inicializar NVS
│   ├── Inicializar netif
│   ├── Crear event loop
│   ├── Configurar WiFi STA
│   └── Conectar
│
└── Eventos WiFi
    ├── WIFI_EVENT_STA_START
    ├── WIFI_EVENT_STA_CONNECTED
    ├── WIFI_EVENT_STA_GOT_IP  ✅ Listo
    └── WIFI_EVENT_STA_DISCONNECTED  🔄 Reintentar
```

### 3. Sistema de Archivos SPIFFS

```
spiffs_manager
├── spiffs_mount()
│   └── Montar partición SPIFFS en /spiffs
│
└── read_certificates()
    ├── Leer /spiffs/ca_crt.pem      → ca_crt
    ├── Leer /spiffs/client_crt.pem   → client_crt
    └── Leer /spiffs/client_key.pem   → client_key
```

### 4. Cliente MQTT con TLS

```
smca_mqtt_client
├── mqtt_start(broker_uri, client_id)
│   ├── Validar certificados
│   ├── Configurar cliente MQTT
│   │   ├── URI: mqtts://...
│   │   ├── Client ID
│   │   ├── CA Certificate
│   │   ├── Client Certificate
│   │   └── Client Key
│   ├── Registrar event handler
│   └── Iniciar cliente
│
└── mqtt_event_handler()
    ├── MQTT_EVENT_CONNECTED      ✅ Conectado
    ├── MQTT_EVENT_DISCONNECTED   🔌 Desconectado
    ├── MQTT_EVENT_ERROR          ❌ Error
    └── MQTT_EVENT_PUBLISHED     📤 Mensaje enviado
```

### 5. Simulación de Sensores (Fase 1)

```
sensor_sim
└── sensor_simulation_task()
    └── Loop infinito:
        ├── Generar valores simulados
        │   ├── pm25: 5-100 µg/m³
        │   ├── pm10: 10-150 µg/m³
        │   ├── no2: 10-200 ppb
        │   ├── co2: 400-1200 ppm
        │   ├── co: 0.5-10 ppm
        │   ├── temperatura: 18-28 °C
        │   └── humedad: 30-80 %
        │
        ├── Generar timestamp ISO 8601
        ├── Formatear JSON
        ├── Publicar en MQTT
        └── Esperar intervalo (5s)
```

## 📡 Protocolo de Comunicación

### Formato de Mensaje MQTT

**Tópico:** `iot/aire/lectura`

**Payload (JSON):**
```json
{
  "sensorId": "esp32-air-003",
  "timestamp": "2026-01-25T10:30:00.123Z",
  "pm25": 25.5,
  "pm10": 45.2,
  "no2": 15.3,
  "co2": 450.0,
  "co": 1.2,
  "temperatura": 22.5,
  "humedad": 65.0
}
```

**Características:**
- ✅ Solo `sensorId` es requerido
- ✅ Parámetros ambientales son opcionales (3-8 según nodo)
- ✅ `timestamp` es opcional pero recomendado
- ✅ El backend resuelve `empresaId` y `dispositivoId` automáticamente

### QoS y Confiabilidad

- **QoS Level:** 1 (At least once delivery)
- **Retain:** No
- **Clean Session:** Sí (por defecto)

## 🔐 Seguridad

### Autenticación TLS

1. **Certificado CA**: Valida la identidad del broker
2. **Certificado Cliente**: Autentica el dispositivo
3. **Clave Privada**: Firma de mensajes

### Almacenamiento de Certificados

- **Ubicación:** Partición SPIFFS (`/spiffs/`)
- **Tamaño:** ~960KB disponible
- **Formato:** PEM (Privacy-Enhanced Mail)

### Flujo de Autenticación

```
1. ESP32 carga certificados desde SPIFFS
2. ESP32 se conecta a broker (mqtts://)
3. Broker valida certificado cliente
4. ESP32 valida certificado CA del broker
5. Establecimiento de conexión TLS segura
6. Autenticación mutua completada ✅
```

## ⚙️ Configuración Multi-Nodo

### Configuración Centralizada

**`config.h` es la única fuente de configuración.** Todos los componentes (main, sensor_sim, wifi_manager, smca_mqtt_client) obtienen sus valores desde este archivo.

- **config.h.example**: Plantilla versionada con placeholders (sin datos reales)
- **config.h**: Archivo real con credenciales (gitignored, cada desarrollador crea el suyo)

#### Crear config.h (primera vez o nuevo clone)

```bash
cd esp32_heltec_v3/main
cp config.h.example config.h
# Editar config.h con valores reales
```

#### Contenido de `main/config.h`

```c
// Nodo
#define SENSOR_ID "esp32-air-003"        // ID del sensor
#define MQTT_CLIENT_ID "esp32-air-003"   // ID del cliente MQTT
#define MQTT_BROKER_URI "mqtts://..."    // URI del broker
#define MQTT_TOPIC "iot/aire/lectura"     // Tópico MQTT
#define SEND_INTERVAL_MS 5000             // Intervalo de envío

// WiFi (unificado en config.h)
#define WIFI_SSID "tu-red-wifi"
#define WIFI_PASS "tu-password"
```

#### Certificados en SPIFFS

```
/spiffs/
├── ca.crt          # CA del broker
├── client.crt      # Certificado del nodo
└── client.key      # Clave privada del nodo
```

### Proceso de Configuración por Nodo

**Para nodo-03 (esp32-air-003):**

1. Crear/editar `config.h`:
   ```bash
   cp config.h.example config.h
   ```
   ```c
   #define SENSOR_ID "esp32-air-003"
   #define MQTT_CLIENT_ID "esp32-air-003"
   #define WIFI_SSID "tu-red"
   #define WIFI_PASS "tu-pass"
   #define MQTT_BROKER_URI "mqtts://192.168.1.X:8883"
   ```

2. Copiar certificados:
   ```bash
   cp mqtt/certs/clients/nodo03/* spiffs/
   ```

3. Compilar y flashear:
   ```bash
   idf.py build flash spiffs-flash
   ```

## 📊 Gestión de Memoria

### Particiones (partitions.csv)

```
┌─────────────┬──────────┬─────────┬──────────┐
│ Partición   │ Tipo     │ Offset  │ Tamaño   │
├─────────────┼──────────┼─────────┼──────────┤
│ nvs         │ data     │ 0x9000  │ 24 KB    │
│ phy_init    │ data     │ 0xF000  │ 4 KB     │
│ factory     │ app      │ 0x10000 │ 1 MB     │
│ spiffs      │ data     │ 0x110000│ 960 KB   │
└─────────────┴──────────┴─────────┴──────────┘
```

### Uso de Memoria

- **Flash:** ~1MB (aplicación) + 960KB (SPIFFS)
- **RAM:** ~512KB SRAM disponible
- **Stack:** 8KB por tarea (sensor_sim)

## 🔄 Manejo de Errores

### Estrategias de Recuperación

1. **WiFi Desconectado:**
   - Reintento automático cada 5 segundos
   - Log de eventos para diagnóstico

2. **MQTT Desconectado:**
   - Reconexión automática por ESP-IDF
   - Buffer de mensajes (si está habilitado)

3. **Certificados Faltantes:**
   - Error crítico al inicio
   - Log detallado del problema

4. **SNTP Fallido:**
   - Timeout después de 40 intentos
   - Continuar sin timestamp (no crítico)

## 🚀 Fase 2: Sensores Reales

### Arquitectura Futura

```
components/
├── sensor_driver/          # Nuevo componente
│   ├── i2c_bus.c          # Bus I2C compartido
│   ├── sensor_pm25.c      # Sensor PM2.5/PM10
│   ├── sensor_gas.c       # Sensores de gas (NO2, CO, CO2)
│   ├── sensor_dht.c       # Temperatura/Humedad
│   └── sensor_manager.c   # Orquestador de sensores
│
└── sensor_sim/            # Mantener para testing
```

### Transición Fase 1 → Fase 2

1. Mantener `sensor_sim` para testing
2. Crear `sensor_driver` para sensores reales
3. Seleccionar modo en `config.h`:
   ```c
   #define SENSOR_MODE_SIM 0
   #define SENSOR_MODE_REAL 1
   #define SENSOR_MODE SENSOR_MODE_REAL
   ```

## 📈 Escalabilidad

### Soporte para 4 Nodos

Cada nodo requiere:
- Configuración única en `config.h`
- Certificados únicos en SPIFFS
- Mismo código base (compilación única)

### Proceso de Deployment

```
Para cada nodo (01-04):
1. Editar config.h con datos del nodo
2. Copiar certificados correspondientes
3. Compilar proyecto
4. Flashear firmware + SPIFFS
5. Verificar funcionamiento
```

## 🔍 Debugging y Monitoreo

### Logs por Componente

- `MAIN`: Orquestación general
- `WiFi`: Eventos de conexión WiFi
- `SPIFFS`: Operaciones de archivos
- `MQTT`: Eventos MQTT y TLS
- `SENSOR_SIM`: Generación de datos

### Niveles de Log

Configurables en `config.h`:
- `ESP_LOG_ERROR`: Solo errores
- `ESP_LOG_WARN`: Advertencias y errores
- `ESP_LOG_INFO`: Información general (recomendado)
- `ESP_LOG_DEBUG`: Debug detallado

## 📚 Referencias Técnicas

### ESP32-S3 Heltec V3.2

- **MCU:** ESP32-S3FN8 (Xtensa LX7 dual-core, 240 MHz)
- **Flash:** 8MB SiP Flash
- **RAM:** 512KB SRAM, 384KB ROM
- **GPIO:** 36 pines disponibles
- **Comunicación:** WiFi 802.11 b/g/n, Bluetooth 5 (LE)

### ESP-IDF v5.1

- **MQTT Client:** `esp_mqtt_client.h`
- **TLS:** `esp_tls.h` (mbedTLS)
- **SPIFFS:** `esp_spiffs.h`
- **WiFi:** `esp_wifi.h`
- **SNTP:** `esp_netif_sntp.h`

## ✅ Checklist de Arquitectura

- [x] Componentes modulares e independientes
- [x] Configuración centralizada
- [x] Manejo robusto de errores
- [x] Protocolo MQTT estándar
- [x] Seguridad TLS implementada
- [x] Preparado para múltiples nodos
- [x] Documentación completa
- [ ] Fase 2: Drivers de sensores (pendiente)
