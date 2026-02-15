# 📁 Gestión de SPIFFS Independiente del Firmware

## ✅ Respuesta Rápida

**SÍ, puedes gestionar SPIFFS independientemente del firmware.**

Puedes actualizar certificados **SIN recompilar** y **SIN flashear el firmware completo**. Solo necesitas:
1. Generar nueva imagen SPIFFS con los certificados actualizados
2. Flashear solo la partición SPIFFS

---

## 🔄 Flujo de Trabajo

### Primera Vez (Flashear Todo)

```bash
# 1. Compilar firmware (con o sin certificados en spiffs/)
idf.py build

# 2. Flashear firmware + SPIFFS
idf.py -p /dev/cu.usbserial-0001 flash
```

### Actualizar Solo Certificados (Sin Recompilar)

```bash
# 1. Copiar nuevos certificados
cp /ruta/nuevos/certificados/ca.crt spiffs/
cp /ruta/nuevos/certificados/client.crt spiffs/
cp /ruta/nuevos/certificados/client.key spiffs/

# 2. Generar nueva imagen SPIFFS
./generar_spiffs.sh
# O manualmente:
python3 $IDF_PATH/components/spiffs/spiffsgen.py 983040 spiffs/ build/spiffs.bin

# 3. Flashear SOLO SPIFFS (sin tocar el firmware)
./flash_spiffs.sh /dev/cu.usbserial-0001
```

**¡Listo!** El firmware sigue igual, solo se actualizaron los certificados.

---

## 📋 Métodos para Flashear SPIFFS

### Método 1: Usando el Script `flash_spiffs.sh` (Recomendado)

```bash
# Requiere que la imagen SPIFFS esté en build/spiffs.bin
./flash_spiffs.sh /dev/cu.usbserial-0001
```

**Ventajas:**
- Automático, detecta la partición SPIFFS
- Verifica que la imagen exista
- Más seguro y fácil de usar

### Método 2: Usando `esptool.py` Directamente

```bash
# Flashear directamente a la dirección de la partición SPIFFS
esptool.py -p /dev/cu.usbserial-0001 \
  --chip esp32s3 \
  write_flash \
  --flash_mode dio \
  --flash_freq 80m \
  --flash_size 2MB \
  0x110000 build/spiffs.bin
```

**Ventajas:**
- Más control
- Útil si `idf.py spiffs-flash` no funciona

### Método 3: Usando el Script `generar_spiffs.sh`

```bash
# El script genera la imagen y te da el comando para flashear
./generar_spiffs.sh
# Luego ejecuta el comando que muestra
```

---

## 🔍 Verificar Partición SPIFFS

Para verificar que SPIFFS se flasheó correctamente:

```bash
# Monitorear logs
idf.py -p /dev/cu.usbserial-0001 monitor
```

**Debes ver:**
```
I (548) SPIFFS: SPIFFS montado: total: 896321, usado: XXXX
I (588) SPIFFS: ✅ ca.crt cargado (XXXX bytes)
I (618) SPIFFS: ✅ client.crt cargado (XXXX bytes)
I (658) SPIFFS: ✅ client.key cargado (XXXX bytes)
```

---

## ⚠️ Consideraciones Importantes

### 1. Tamaño de la Imagen SPIFFS

- **Tamaño máximo:** 960KB (0xF0000 bytes)
- **Tamaño usado:** Depende de tus certificados
- **Verificar:** `ls -lh build/spiffs.bin`

### 2. Formato de Archivos

Los certificados deben tener los nombres exactos:
- `ca.crt` (no `ca_crt.pem`)
- `client.crt` (no `client_crt.pem`)
- `client.key` (no `client_key.pem`)

### 3. Reinicio del Dispositivo

Después de flashear SPIFFS, el dispositivo debe reiniciarse para que los nuevos certificados se carguen. Esto sucede automáticamente al usar `idf.py spiffs-flash`.

### 4. Compatibilidad con Firmware

- ✅ Puedes actualizar SPIFFS con cualquier versión del firmware
- ✅ No necesitas recompilar si solo cambias certificados
- ✅ El firmware lee los certificados desde SPIFFS al iniciar

---

## 🚀 Ejemplo Completo: Actualizar Certificados

```bash
# 1. Ir al proyecto
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"

# 2. Cargar entorno ESP-IDF
. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh

# 3. Copiar nuevos certificados
cp /ruta/nuevos/certificados/ca.crt spiffs/
cp /ruta/nuevos/certificados/client.crt spiffs/
cp /ruta/nuevos/certificados/client.key spiffs/

# 4. Verificar que se copiaron
ls -lh spiffs/

# 5. Generar imagen SPIFFS
./generar_spiffs.sh

# 6. Flashear solo SPIFFS
idf.py -p /dev/cu.usbserial-0001 spiffs-flash

# 7. Monitorear para verificar
idf.py -p /dev/cu.usbserial-0001 monitor
```

**Tiempo total:** ~30 segundos (sin recompilar)

---

## 📊 Comparación: Con vs Sin Recompilación

| Acción | Con Recompilación | Sin Recompilación |
|--------|-------------------|-------------------|
| **Tiempo** | 2-5 minutos | 30 segundos |
| **Flashea firmware** | ✅ Sí | ❌ No |
| **Flashea SPIFFS** | ✅ Sí | ✅ Solo SPIFFS |
| **Reinicia dispositivo** | ✅ Sí | ✅ Sí |
| **Útil para** | Cambios en código | Solo certificados |

---

## 🎯 Resumen

✅ **SÍ puedes gestionar SPIFFS independientemente**
- Actualiza certificados sin recompilar
- Flashea solo la partición SPIFFS
- El firmware no se toca
- Proceso rápido (~30 segundos)

**Comando clave:**
```bash
./flash_spiffs.sh /dev/cu.usbserial-0001
```
