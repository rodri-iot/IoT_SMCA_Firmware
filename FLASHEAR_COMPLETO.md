# 📤 Guía Completa: Flashear Firmware + SPIFFS

## ✅ Problemas Corregidos

1. **Tabla de particiones:** Ahora usa `partitions.csv` con SPIFFS incluido
2. **Zona horaria:** Corregida a `ART3` (Buenos Aires, UTC-3)

## 🚀 Pasos para Flashear

### Paso 1: Cargar Entorno ESP-IDF

```bash
. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh
```

### Paso 2: Ir al Proyecto

```bash
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"
```

### Paso 3: Compilar (si no lo hiciste)

```bash
idf.py build
```

### Paso 4: Identificar Puerto Serial

```bash
ls /dev/cu.usbserial-* /dev/tty.usbserial-* 2>/dev/null
```

**O conecta el ESP32 y ejecuta:**
```bash
idf.py flash
# Te mostrará los puertos disponibles si hay varios
```

### Paso 5: Flashear Firmware

```bash
# Reemplaza /dev/cu.usbserial-0001 con tu puerto
idf.py -p /dev/cu.usbserial-0001 flash
```

**O si solo hay un puerto:**
```bash
idf.py flash
```

### Paso 6: Copiar Certificados a SPIFFS

**IMPORTANTE:** Los certificados deben estar en formato `.crt` y `.key`:

```bash
# Desde tu proyecto SMCA, copiar certificados
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

### Paso 7: Generar Imagen SPIFFS

```bash
idf.py spiffsgen
```

### Paso 8: Flashear SPIFFS

```bash
idf.py -p /dev/cu.usbserial-0001 spiffs-flash
```

### Paso 9: Monitorear Logs

```bash
idf.py -p /dev/cu.usbserial-0001 monitor
```

**O todo junto:**
```bash
idf.py -p /dev/cu.usbserial-0001 flash spiffs-flash monitor
```

## ✅ Verificación en Logs

Después de flashear, en los logs del bootloader deberías ver:

```
I (57) boot: ## Label            Usage          Type ST Offset   Length
I (64) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (72) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (79) boot:  2 factory          factory app      00 00 00010000 00100000
I (87) boot:  3 spiffs           SPIFFS           01 82 00110000 000f0000  ← DEBE APARECER
```

Y en los logs de la aplicación:

```
I (461) SPIFFS: 🔧 Montando SPIFFS...
I (471) SPIFFS: SPIFFS montado: total: 983040, usado: XXXX
I (481) SPIFFS: ✅ ca.crt cargado (XXXX bytes)
I (491) SPIFFS: ✅ client.crt cargado (XXXX bytes)
I (501) SPIFFS: ✅ client.key cargado (XXXX bytes)
I (511) SPIFFS: 📄 ✅ Todos los certificados cargados correctamente
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

### Error: "No se pudo sincronizar el tiempo"

**Causa:** Problemas de red o firewall bloqueando NTP.

**Solución:** 
- Verifica conexión WiFi
- Verifica que `pool.ntp.org` sea accesible
- El timeout de SNTP no impide que MQTT funcione (solo afecta timestamps)

### Error: "Certificados no cargados"

**Causa:** SPIFFS no flasheado o certificados no copiados.

**Solución:**
1. Verifica que los certificados estén en `spiffs/`
2. Genera imagen SPIFFS: `idf.py spiffsgen`
3. Flashea SPIFFS: `idf.py spiffs-flash`

## 📝 Comandos Rápidos (Todo en Uno)

```bash
# 1. Cargar entorno
. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh

# 2. Ir al proyecto
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"

# 3. Compilar
idf.py build

# 4. Flashear firmware
idf.py -p /dev/cu.usbserial-0001 flash

# 5. (Después de copiar certificados) Generar y flashear SPIFFS
idf.py spiffsgen
idf.py -p /dev/cu.usbserial-0001 spiffs-flash

# 6. Monitorear
idf.py -p /dev/cu.usbserial-0001 monitor
```

## 🎯 Resumen

1. ✅ **Tabla de particiones corregida** - SPIFFS ahora está incluido
2. ✅ **Zona horaria corregida** - ART3 (Buenos Aires)
3. ✅ **Certificados configurados** - Lee `.crt` y `.key`
4. ✅ **WiFi configurado** - Rodrigo(2.4G)

**Próximo paso:** Copiar certificados y flashear SPIFFS.
