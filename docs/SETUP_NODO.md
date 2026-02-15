# 🔧 Guía Rápida: Configurar Nodo-03 (esp32-air-003)

Esta guía te ayudará a configurar el firmware para el nodo-03 con ID `esp32-air-003`.

## 📋 Checklist Pre-Flasheo

- [ ] ESP-IDF v5.1+ instalado y configurado
- [ ] Certificados del nodo-03 disponibles
- [ ] Credenciales WiFi conocidas
- [ ] IP/hostname del broker MQTT conocido

## ⚙️ Paso 1: Crear config.h (primera vez)

Si es la primera vez o clonaste el repo, crear `config.h` desde la plantilla:

```bash
cd esp32_heltec_v3/main
cp config.h.example config.h
```

**Nota:** `config.h` está en `.gitignore` (no se versiona credenciales reales).

## ⚙️ Paso 2: Configurar Identificación del Nodo y WiFi

Editar `esp32_heltec_v3/main/config.h`:

```c
// ID del sensor (debe coincidir con el registrado en la BD)
#define SENSOR_ID "esp32-air-003"

// ID del cliente MQTT (generalmente igual a SENSOR_ID)
#define MQTT_CLIENT_ID "esp32-air-003"

// URI del broker MQTT (ajustar según tu red)
#define MQTT_BROKER_URI "mqtts://192.168.0.13:8883"

// Credenciales WiFi (unificado en config.h)
#define WIFI_SSID "tu-red-wifi"
#define WIFI_PASS "tu-password"
```

## 🔐 Paso 3: Copiar Certificados

Los certificados deben estar en `esp32_heltec_v3/spiffs/`:

```bash
cd esp32_heltec_v3

# Crear directorio si no existe
mkdir -p spiffs

# Copiar certificados desde tu proyecto SMCA
# Ajustar rutas según tu estructura
cp /ruta/al/SMCA/mqtt/certs/ca/ca.crt spiffs/ca_crt.pem
cp /ruta/al/SMCA/mqtt/certs/clients/nodo03/client.crt spiffs/client_crt.pem
cp /ruta/al/SMCA/mqtt/certs/clients/nodo03/client.key spiffs/client_key.pem

# Verificar
ls -lh spiffs/
```

**Resultado esperado:**
```
-rw-r--r--  1 user  staff   1.2K ca_crt.pem
-rw-r--r--  1 user  staff   1.5K client_crt.pem
-rw-r--r--  1 user  staff   1.8K client_key.pem
```

## 🔨 Paso 4: Compilar

```bash
cd esp32_heltec_v3

# Configurar target (solo la primera vez)
idf.py set-target esp32s3

# Compilar
idf.py build
```

**Verificar salida:**
```
Project build complete. To flash, run:
idf.py flash
```

## 📤 Paso 5: Flashear

```bash
# Identificar puerto serial
# macOS: ls /dev/cu.usbserial-*
# Linux: ls /dev/ttyUSB*

# Flashear firmware + SPIFFS
idf.py -p /dev/cu.usbserial-XXXX flash spiffs-flash
```

## 📊 Paso 6: Monitorear

```bash
# Ver logs en tiempo real
idf.py -p /dev/cu.usbserial-XXXX monitor
```

**Logs esperados:**
```
I (1234) MAIN: 🚀 Iniciando SMCA - Heltec V3.2
I (1235) MAIN: 📡 Nodo: esp32-air-003 (sensorId: esp32-air-003)
I (1236) SPIFFS: ✅ ca_crt.pem cargado (1234 bytes)
I (1237) SPIFFS: ✅ client_crt.pem cargado (1567 bytes)
I (1238) SPIFFS: ✅ client_key.pem cargado (1678 bytes)
I (2000) WiFi: 📶 Conectando a WiFi: tu-red-wifi
I (5000) WiFi: ✅ Conectado a WiFi
I (6000) SNTP: ✅ Tiempo sincronizado
I (7000) MQTT: ✅ Conectado al broker MQTT
I (8000) SENSOR_SIM: 🚀 Iniciando simulación de sensores
I (13000) SENSOR_SIM: 📤 [1] esp32-air-003 → PM2.5: 25.5, PM10: 45.2, CO2: 450
```

## ✅ Verificación

1. **WiFi conectado:** Ver log `✅ Conectado a WiFi`
2. **Tiempo sincronizado:** Ver log `✅ Tiempo sincronizado`
3. **MQTT conectado:** Ver log `✅ Conectado al broker MQTT`
4. **Datos enviados:** Ver logs periódicos `📤 [N] esp32-air-003 → ...`

## 🐛 Troubleshooting

### Error: "Certificados no cargados"

**Solución:**
```bash
# Verificar que los certificados existan
ls -lh esp32_heltec_v3/spiffs/

# Verificar nombres exactos (case-sensitive)
# Deben ser: ca_crt.pem, client_crt.pem, client_key.pem
```

### Error: "WiFi no conecta"

**Solución:**
- Verificar credenciales en `config.h` (WIFI_SSID, WIFI_PASS)
- Verificar que la red esté en rango
- Verificar que la red permita conexiones

### Error: "MQTT connection failed"

**Solución:**
- Verificar que el broker esté accesible desde la red
- Verificar URI en `config.h` (IP correcta, puerto 8883)
- Verificar certificados (deben ser del nodo-03)
- Verificar que el broker esté ejecutándose

### No se ven datos en el sistema SMCA

**Solución:**
- Verificar que `sensorId` coincida con el registrado en la BD
- Verificar logs del broker MQTT
- Verificar que el backend esté procesando mensajes
- Verificar tópico MQTT: debe ser `iot/aire/lectura`

## 🔄 Configurar Otros Nodos

Para configurar nodo-01, nodo-02, o nodo-04:

1. Cambiar `SENSOR_ID` y `MQTT_CLIENT_ID` en `config.h`
2. Copiar certificados correspondientes (nodo01, nodo02, nodo04)
3. Recompilar y flashear

**Ejemplo para nodo-01:**
```c
#define SENSOR_ID "esp32-air-001"
#define MQTT_CLIENT_ID "esp32-air-001"
```

```bash
cp /ruta/al/SMCA/mqtt/certs/clients/nodo01/client.crt spiffs/client_crt.pem
cp /ruta/al/SMCA/mqtt/certs/clients/nodo01/client.key spiffs/client_key.pem
```

## 📚 Referencias

- Ver `README.md` para documentación completa
- Ver `ARCHITECTURE.md` para detalles de arquitectura
- Ver `INSTALL.md` para instalación de ESP-IDF
