# 📦 Guía de Instalación - ESP-IDF para ESP32-S3 Heltec V3

Esta guía te ayudará a instalar y configurar ESP-IDF para desarrollar firmware para el ESP32-S3 Heltec V3.2.

## 🔧 Requisitos Previos

### Sistema Operativo

- **macOS** (recomendado: 10.15 o superior)
- **Linux** (Ubuntu 20.04+, Debian 10+, Fedora 30+)
- **Windows** (Windows 10/11 con WSL2 o MSYS2)

### Herramientas Necesarias

- Git
- Python 3.8 o superior
- CMake 3.16 o superior
- Compilador de C/C++

## 📥 Instalación de ESP-IDF

### Paso 1: Clonar ESP-IDF

```bash
# Crear directorio para ESP-IDF (recomendado: ~/esp)
mkdir -p ~/esp
cd ~/esp

# Clonar repositorio oficial de ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
```

### Paso 2: Seleccionar Versión

Para ESP32-S3, recomendamos **ESP-IDF v5.1** o superior (soporte completo y estable):

```bash
# Cambiar a la versión v5.1
git checkout v5.1
git submodule update --init --recursive
```

**Versiones compatibles:**
- ✅ ESP-IDF v5.1.x (recomendado)
- ✅ ESP-IDF v5.2.x
- ✅ ESP-IDF v5.3.x
- ⚠️ ESP-IDF v4.4.x (soporte limitado para ESP32-S3)

### Paso 3: Instalar Herramientas

```bash
# Instalar herramientas para ESP32-S3
./install.sh esp32s3
```

Esto instalará:
- Compilador Xtensa para ESP32-S3
- OpenOCD (debugger)
- Esptool (flasher)
- Otras herramientas necesarias

**Tiempo estimado:** 10-20 minutos (depende de la conexión)

### Paso 4: Configurar Entorno

**macOS / Linux:**

```bash
# Agregar al ~/.zshrc o ~/.bashrc
alias get_idf='. $HOME/esp/esp-idf/export.sh'

# O ejecutar manualmente cada vez:
. ~/esp/esp-idf/export.sh
```

**Windows (PowerShell):**

```powershell
# Ejecutar en cada sesión:
$HOME/esp/esp-idf/export.ps1
```

### Paso 5: Verificar Instalación

```bash
# Verificar que ESP-IDF esté configurado
idf.py --version

# Verificar herramientas
xtensa-esp32s3-elf-gcc --version
```

## 🚀 Configuración del Proyecto

### Paso 1: Configurar Variable de Entorno

```bash
# Asegurarse de que IDF_PATH esté configurado
export IDF_PATH=$HOME/esp/esp-idf

# Verificar
echo $IDF_PATH
```

### Paso 2: Configurar el Proyecto

```bash
cd /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3

# Configurar target (ESP32-S3)
idf.py set-target esp32s3

# Abrir menú de configuración
idf.py menuconfig
```

### Paso 3: Configuraciones Importantes en menuconfig

1. **Partition Table:**
   - `Partition Table` → `Custom partition table CSV`
   - Archivo: `partitions.csv`

2. **SPIFFS:**
   - `Component config` → `SPIFFS Configuration`
   - Habilitar SPIFFS

3. **WiFi:**
   - `Component config` → `Wi-Fi`
   - Habilitar WiFi

4. **MQTT:**
   - `Component config` → `MQTT Configuration`
   - Habilitar MQTT

5. **TLS:**
   - `Component config` → `mbedTLS`
   - Habilitar soporte TLS

## 📁 Preparar Certificados TLS

### Paso 1: Copiar Certificados

Los certificados deben copiarse desde tu proyecto SMCA:

```bash
# Desde el proyecto SMCA, copiar certificados del nodo-03
# Origen: mqtt/certs/clients/nodo03/
# Destino: esp32_heltec_v3/spiffs/

mkdir -p esp32_heltec_v3/spiffs
cp /ruta/al/proyecto/SMCA/mqtt/certs/ca/ca.crt esp32_heltec_v3/spiffs/ca_crt.pem
cp /ruta/al/proyecto/SMCA/mqtt/certs/clients/nodo03/client.crt esp32_heltec_v3/spiffs/client_crt.pem
cp /ruta/al/proyecto/SMCA/mqtt/certs/clients/nodo03/client.key esp32_heltec_v3/spiffs/client_key.pem
```

### Paso 2: Verificar Certificados

```bash
ls -lh esp32_heltec_v3/spiffs/
# Debe mostrar:
# - ca_crt.pem
# - client_crt.pem
# - client_key.pem
```

## 🔨 Compilación

### Compilar el Proyecto

```bash
cd esp32_heltec_v3
idf.py build
```

**Salida esperada:**
```
Project build complete. To flash, run:
idf.py flash
```

### Generar Imagen SPIFFS

```bash
# Generar imagen SPIFFS con los certificados
idf.py spiffsgen

# O manualmente:
mkspiffs -c spiffs -b 4096 -p 256 -s 0xF0000 spiffs_image.bin spiffs/
```

## 📤 Flasheo

### Conectar el Dispositivo

1. Conectar ESP32-S3 Heltec V3.2 por USB Type-C
2. Identificar el puerto:

```bash
# macOS
ls /dev/cu.usbserial-*  # o /dev/cu.usbmodem*

# Linux
ls /dev/ttyUSB*  # o /dev/ttyACM*

# Windows
# Verificar en Administrador de Dispositivos
```

### Flashear Firmware

```bash
# Flashear firmware + particiones
idf.py -p /dev/cu.usbserial-XXXX flash

# Flashear SPIFFS con certificados
idf.py -p /dev/cu.usbserial-XXXX spiffs-flash
```

**Nota:** Reemplazar `/dev/cu.usbserial-XXXX` con tu puerto serial.

### Flashear Todo en un Paso

```bash
# Flashear firmware y SPIFFS
idf.py -p /dev/cu.usbserial-XXXX flash spiffs-flash
```

## 📊 Monitoreo

### Ver Logs en Tiempo Real

```bash
idf.py -p /dev/cu.usbserial-XXXX monitor
```

**Atajos útiles:**
- `Ctrl+]` - Salir del monitor
- `Ctrl+T` → `Ctrl+H` - Ver ayuda

### Monitoreo con Filtros

```bash
# Solo ver logs de un componente
idf.py monitor --print-filter="*:I"  # Solo INFO
idf.py monitor --print-filter="MQTT:I SENSOR_SIM:I"  # Componentes específicos
```

## 🐛 Troubleshooting

### Error: "IDF_PATH not set"

```bash
# Solución: Configurar entorno
. ~/esp/esp-idf/export.sh
export IDF_PATH=$HOME/esp/esp-idf
```

### Error: "No se encuentra el puerto serial"

**macOS:**
```bash
# Instalar drivers CP2102 (USB-to-Serial)
# Descargar desde: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
```

**Linux:**
```bash
# Agregar usuario al grupo dialout
sudo usermod -a -G dialout $USER
# Reiniciar sesión
```

### Error: "Permission denied" al flashear

**Linux:**
```bash
sudo chmod 666 /dev/ttyUSB0  # Reemplazar con tu puerto
# O agregar reglas udev permanentes
```

### Error: "SPIFFS mount failed"

- Verificar que los certificados existan en `spiffs/`
- Verificar tamaño de partición SPIFFS en `partitions.csv`
- Verificar que SPIFFS se haya flasheado correctamente

### Error: "MQTT connection failed"

- Verificar certificados TLS
- Verificar que el broker esté accesible desde la red
- Verificar configuración de WiFi
- Verificar sincronización de tiempo (SNTP)

## 📚 Recursos Adicionales

- [Documentación Oficial ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [ESP32-S3 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)
- [Heltec ESP32 V3 Docs](https://docs.heltec.cn/)
- [Guía de Particiones ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-guides/partition-tables.html)
- [MQTT Client API](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/protocols/mqtt.html)

## ✅ Checklist de Instalación

- [ ] ESP-IDF v5.1+ instalado
- [ ] Herramientas para ESP32-S3 instaladas
- [ ] Entorno configurado (export.sh ejecutado)
- [ ] Proyecto configurado (`idf.py set-target esp32s3`)
- [ ] Certificados copiados a `spiffs/`
- [ ] Proyecto compila sin errores
- [ ] Dispositivo conectado y reconocido
- [ ] Firmware flasheado correctamente
- [ ] SPIFFS flasheado con certificados
- [ ] Logs visibles en monitor

## 🎯 Próximos Pasos

1. Crear `config.h` desde plantilla: `cp config.h.example config.h`
2. Editar `config.h` con datos del nodo (SENSOR_ID, MQTT_BROKER_URI, WIFI_SSID, WIFI_PASS)
3. Compilar y flashear
4. Verificar conexión MQTT y envío de datos
5. Monitorear logs para confirmar funcionamiento
