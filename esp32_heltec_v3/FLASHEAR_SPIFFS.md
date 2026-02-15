# 📤 Guía: Flashear SPIFFS con Certificados

## ✅ Problema Resuelto

El comando `idf.py spiffsgen` no existe en ESP-IDF v5.1. Ahora el proyecto está configurado para generar SPIFFS automáticamente durante el build.

## 🚀 Método 1: Automático (Recomendado)

### Paso 1: Copiar Certificados

Antes de compilar, copia los certificados al directorio `spiffs/`:

```bash
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"

# Copiar certificados (ajusta las rutas según tu proyecto SMCA)
cp /ruta/al/SMCA/mqtt/certs/ca/ca.crt spiffs/
cp /ruta/al/SMCA/mqtt/certs/clients/nodo03/client.crt spiffs/
cp /ruta/al/SMCA/mqtt/certs/clients/nodo03/client.key spiffs/

# Verificar que se copiaron
ls -lh spiffs/
```

**Debe mostrar:**
```
-rw-r--r--  ca.crt
-rw-r--r--  client.crt
-rw-r--r--  client.key
```

### Paso 2: Compilar (Genera SPIFFS Automáticamente)

```bash
# Cargar entorno ESP-IDF
. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh

# Compilar (genera SPIFFS automáticamente)
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"
idf.py build
```

La imagen SPIFFS se generará automáticamente en `build/spiffs.bin`.

### Paso 3: Flashear Todo

```bash
# Flashear firmware + SPIFFS en un solo comando
idf.py -p /dev/cu.usbserial-0001 flash
```

El flag `FLASH_IN_PROJECT` en `CMakeLists.txt` hace que SPIFFS se flashee automáticamente junto con el firmware.

## 🔧 Método 2: Manual (Si el automático falla)

### Usar el Script `generar_spiffs.sh`

```bash
# 1. Copiar certificados (igual que antes)
cp /ruta/certificados/*.crt spiffs/
cp /ruta/certificados/*.key spiffs/

# 2. Generar imagen SPIFFS manualmente
./generar_spiffs.sh

# 3. Flashear SPIFFS
idf.py -p /dev/cu.usbserial-0001 spiffs-flash
```

### O Flashear Manualmente con esptool

```bash
# Después de generar con el script
esptool.py -p /dev/cu.usbserial-0001 \
  --chip esp32s3 \
  write_flash \
  --flash_mode dio \
  --flash_freq 80m \
  --flash_size 2MB \
  0x110000 build/spiffs.bin
```

## 📋 Verificación

Después de flashear, monitorea los logs:

```bash
idf.py -p /dev/cu.usbserial-0001 monitor
```

**Debes ver:**
```
I (548) SPIFFS: SPIFFS montado: total: 896321, usado: XXXX
I (588) SPIFFS: ✅ ca.crt cargado (XXXX bytes)
I (618) SPIFFS: ✅ client.crt cargado (XXXX bytes)
I (658) SPIFFS: ✅ client.key cargado (XXXX bytes)
I (668) SPIFFS: 📄 ✅ Todos los certificados cargados correctamente
```

## 🐛 Solución de Problemas

### Error: "spiffs partition could not be found"

**Causa:** La tabla de particiones no incluye SPIFFS.

**Solución:** Verifica que `sdkconfig` tenga:
```
CONFIG_PARTITION_TABLE_CUSTOM=y
CONFIG_PARTITION_TABLE_FILENAME="partitions.csv"
```

Luego recompila:
```bash
rm -rf build
idf.py set-target esp32s3
idf.py build
```

### Error: "No se encontraron archivos .crt o .key"

**Causa:** Los certificados no están en `spiffs/`.

**Solución:** Copia los certificados antes de compilar o generar SPIFFS.

### Error: "SPIFFS montado pero certificados no encontrados"

**Causa:** SPIFFS está vacío o los archivos tienen nombres incorrectos.

**Solución:** 
1. Verifica los nombres: deben ser `ca.crt`, `client.crt`, `client.key`
2. Regenera SPIFFS con los certificados correctos
3. Reflashea SPIFFS

## 📝 Resumen de Comandos

```bash
# 1. Copiar certificados
cp /ruta/certificados/*.crt spiffs/
cp /ruta/certificados/*.key spiffs/

# 2. Compilar (genera SPIFFS automáticamente)
idf.py build

# 3. Flashear todo
idf.py -p /dev/cu.usbserial-0001 flash

# 4. Monitorear
idf.py -p /dev/cu.usbserial-0001 monitor
```

## ✅ Cambios Realizados

1. **CMakeLists.txt**: Habilitado `spiffs_create_partition_image` con `FLASH_IN_PROJECT`
2. **main.c**: Mejorado código de SNTP con múltiples servidores y mejor manejo de errores
3. **generar_spiffs.sh**: Script alternativo para generar SPIFFS manualmente si es necesario
