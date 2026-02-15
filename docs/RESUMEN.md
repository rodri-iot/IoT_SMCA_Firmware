# 📋 Resumen del Proyecto - Firmware ESP32-S3 Heltec V3

## ✅ Estado Actual del Proyecto

### Completado

1. ✅ **Arquitectura modular** implementada
2. ✅ **Código limpiado** y corregido
3. ✅ **Formato MQTT** alineado con scripts de simulación
4. ✅ **Configuración multi-nodo** preparada (nodo-03: esp32-air-003)
5. ✅ **Documentación completa** creada
6. ✅ **Particiones** configuradas para ESP32-S3

### Pendiente (Fase 2)

- ⏳ Drivers de sensores reales
- ⏳ Calibración de sensores
- ⏳ Validación de datos de sensores

## 🎯 Repositorio ESP-IDF Requerido

**Repositorio oficial:** https://github.com/espressif/esp-idf

**Versión recomendada:** ESP-IDF v5.1 o superior

**Instalación:**
```bash
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout v5.1
./install.sh esp32s3
. ./export.sh
```

Ver `INSTALL.md` para guía completa.

## 📁 Estructura del Proyecto

```
IoT_SMCA_Firmware/
├── README.md              # Documentación principal
├── ARCHITECTURE.md        # Arquitectura detallada
├── INSTALL.md             # Guía de instalación ESP-IDF
├── SETUP_NODO.md          # Guía rápida configuración nodo
├── RESUMEN.md             # Este archivo
│
├── esp32_heltec_v3/       # ⭐ Proyecto principal
│   ├── main/
│   │   ├── main.c         # Orquestador
│   │   ├── config.h       # ⭐ Configuración centralizada (gitignored)
│   │   └── config.h.example
│   ├── components/
│   │   ├── wifi_manager/  # Gestión WiFi
│   │   ├── spiffs_manager/# Sistema de archivos
│   │   ├── smca_mqtt_client/ # Cliente MQTT TLS
│   │   └── sensor_sim/    # Simulación sensores
│   └── partitions.csv     # Tabla de particiones
│
└── scripts_test/          # Scripts de simulación Node.js
    ├── simulate-esp32.js
    └── simulate-esp32-02.js
```

## 🔧 Configuración Actual (Nodo-03)

### Archivos Clave

1. **`esp32_heltec_v3/main/config.h`** (crear desde config.h.example)
   - Configuración centralizada: SENSOR_ID, MQTT_CLIENT_ID, MQTT_BROKER_URI, WIFI_SSID, WIFI_PASS

2. **`esp32_heltec_v3/spiffs/`**
   - Certificados TLS (ca_crt.pem, client_crt.pem, client_key.pem)

## 📡 Protocolo de Comunicación

### Tópico MQTT
```
iot/aire/lectura
```

### Formato de Mensaje
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

**Parámetros simulados (Fase 1):**
- pm25, pm10, no2, co2, co, temperatura, humedad

## 🚀 Próximos Pasos

### Inmediatos

1. **Instalar ESP-IDF v5.1+**
   ```bash
   # Ver INSTALL.md
   ```

2. **Configurar nodo-03**
   ```bash
   # Ver SETUP_NODO.md
   # - cp config.h.example config.h
   # - Editar config.h
   # - Copiar certificados
   ```

3. **Compilar y flashear**
   ```bash
   cd esp32_heltec_v3
   idf.py set-target esp32s3
   idf.py build
   idf.py flash spiffs-flash
   ```

4. **Verificar funcionamiento**
   ```bash
   idf.py monitor
   # Verificar logs de conexión y envío de datos
   ```

### Fase 2 (Futuro)

1. Implementar drivers de sensores I2C/SPI
2. Agregar calibración de sensores
3. Implementar validación de datos
4. Agregar manejo de errores de sensores

## 📚 Documentación Disponible

- **README.md**: Visión general y documentación completa
- **ARCHITECTURE.md**: Arquitectura detallada del sistema
- **INSTALL.md**: Guía de instalación ESP-IDF paso a paso
- **SETUP_NODO.md**: Guía rápida para configurar un nodo

## 🔍 Características del ESP32-S3 Heltec V3.2

- **MCU:** ESP32-S3FN8 (Xtensa LX7 dual-core, 240 MHz)
- **Flash:** 8MB SiP Flash
- **RAM:** 512KB SRAM, 384KB ROM
- **Conectividad:** WiFi 802.11 b/g/n, Bluetooth 5 (LE)
- **USB:** Type-C para programación y alimentación
- **Batería:** Soporte LiPo 3.7V con gestión integrada
- **Display:** OLED 0.96" 128x64 (opcional)

## ⚠️ Notas Importantes

1. **Certificados TLS:** Deben ser específicos para cada nodo
2. **sensorId:** Debe coincidir con el registrado en la BD del sistema SMCA
3. **Broker MQTT:** Debe estar accesible desde la red WiFi del nodo
4. **Tiempo:** SNTP es necesario para validar certificados TLS

## 🐛 Troubleshooting Rápido

| Problema | Solución |
|----------|----------|
| Certificados no cargados | Verificar que existan en `spiffs/` |
| WiFi no conecta | Verificar credenciales en `config.h` |
| MQTT falla | Verificar URI del broker y certificados |
| No se ven datos en SMCA | Verificar `sensorId` en BD y logs del broker |

## 📞 Soporte

- Ver documentación en `README.md`
- Ver guías específicas en `INSTALL.md` y `SETUP_NODO.md`
- Ver arquitectura en `ARCHITECTURE.md`
