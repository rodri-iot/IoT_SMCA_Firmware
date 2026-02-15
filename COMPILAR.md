# 🔨 Guía de Compilación - ESP32 Heltec V3

## 📋 Requisitos Previos

- ESP-IDF v5.1+ instalado y configurado
- Python 3.8+ instalado
- Certificados TLS (opcional para compilar, requeridos para flashear)

## 🚀 Pasos para Compilar

### Paso 1: Abrir Terminal

Abre tu terminal (Terminal.app o iTerm2 en macOS).

### Paso 2: Cargar Entorno ESP-IDF

```bash
# Cargar el entorno ESP-IDF
. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh
```

**Nota:** El punto (`.`) al inicio es importante. Esto carga las variables de entorno necesarias.

**Verificación:** Deberías ver un mensaje como:
```
Done! You can now compile ESP-IDF projects.
```

### Paso 3: Ir al Directorio del Proyecto

```bash
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"
```

### Paso 4: Configurar Target (Solo Primera Vez)

```bash
idf.py set-target esp32s3
```

**Nota:** Solo necesitas hacer esto la primera vez, o si cambias de target (ESP32, ESP32-S3, etc.).

**Salida esperada:**
```
Set Target to: esp32s3, new sdkconfig will be created.
-- Configuring done
-- Generating done
-- Build files have been written to: ...
```

### Paso 5: Compilar el Proyecto

```bash
idf.py build
```

**Salida esperada:**
```
Project build complete. To flash, run:
idf.py flash
```

## ✅ Verificación de Compilación Exitosa

Si la compilación fue exitosa, verás:
- ✅ `Project build complete`
- ✅ Archivos generados en `build/`:
  - `build/esp32_heltec_v3.bin` (firmware)
  - `build/bootloader/bootloader.bin`
  - `build/partition_table/partition-table.bin`

## 🔍 Comandos Útiles

### Limpiar Build (si hay problemas)

```bash
idf.py fullclean
```

### Ver Información del Proyecto

```bash
idf.py show_efuse_table
idf.py size
idf.py size-components
```

### Ver Configuración

```bash
idf.py menuconfig
```

## 🐛 Solución de Problemas

### Error: "command not found: idf.py"

**Solución:** Asegúrate de haber cargado el entorno ESP-IDF:
```bash
. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh
```

### Error: "Failed to resolve component 'mqtt'"

**Solución:** Verifica que `idf_component.yml` existe y tiene la dependencia:
```bash
cat idf_component.yml
```

### Error: "build directory corrupt"

**Solución:** Elimina y regenera el build:
```bash
rm -rf build
idf.py set-target esp32s3
idf.py build
```

## 📦 Después de Compilar

### Flashear Firmware

```bash
# Conectar ESP32 por USB y ejecutar:
idf.py flash

# O especificar el puerto manualmente:
idf.py -p /dev/cu.usbserial-* flash
```

### Monitorear Logs

```bash
idf.py monitor

# O combinar flash + monitor:
idf.py flash monitor
```

### Flashear SPIFFS (después de copiar certificados)

```bash
# 1. Copiar certificados a spiffs/
cp /ruta/certificados/*.crt spiffs/
cp /ruta/certificados/*.key spiffs/

# 2. Generar imagen SPIFFS
idf.py spiffsgen

# 3. Flashear SPIFFS
idf.py spiffs-flash
```

## 📝 Script Rápido (Todo en Uno)

Puedes crear un script para automatizar:

```bash
#!/bin/bash
# compilar.sh

# Cargar ESP-IDF
. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh

# Ir al proyecto
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"

# Compilar
idf.py build
```

Hacer ejecutable:
```bash
chmod +x compilar.sh
./compilar.sh
```

## 🎯 Resumen Rápido

```bash
# 1. Cargar entorno
. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh

# 2. Ir al proyecto
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"

# 3. Compilar
idf.py build
```

¡Listo! 🎉
