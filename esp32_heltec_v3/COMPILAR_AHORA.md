# 🚀 Compilar el Proyecto - Instrucciones Inmediatas

## ✅ Todo Está Corregido

He corregido todos los problemas de código:
- ✅ MQTT configurado para ESP-IDF v5.1
- ✅ Dependencias actualizadas
- ✅ SNTP corregido
- ✅ SPIFFS configurado (comentado temporalmente)

## 🔨 Compilar Ahora

Ejecuta estos comandos en tu terminal:

```bash
# 1. Cargar entorno ESP-IDF
. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh

# 2. Ir al proyecto
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"

# 3. Configurar target (solo primera vez, o si cambias de target)
idf.py set-target esp32s3

# 4. Compilar
idf.py build
```

## 📋 Qué Esperar

### Primera Vez (set-target):
- Descargará componente MQTT automáticamente
- Configurará el proyecto para ESP32-S3
- Puede tardar 1-2 minutos

### Compilación:
- Compilará todos los componentes
- Generará el firmware
- **NO generará SPIFFS** (está comentado, se hará después)

### Salida Esperada:
```
Project build complete. To flash, run:
idf.py flash
```

## ⚠️ Si Hay Problemas de Permisos

Si ves errores de "Operation not permitted" al eliminar `build/`:

1. **Opción 1:** Ignorar y continuar (ESP-IDF manejará el build)
2. **Opción 2:** Cerrar Cursor y eliminar `build/` desde Finder
3. **Opción 3:** Usar `idf.py fullclean` (puede fallar si hay permisos)

## 📦 Después de Compilar

### 1. Copiar Certificados

```bash
# Copiar certificados del nodo-03 a spiffs/
cp /ruta/al/SMCA/mqtt/certs/ca/ca.crt spiffs/ca_crt.pem
cp /ruta/al/SMCA/mqtt/certs/clients/nodo03/client.crt spiffs/client_crt.pem
cp /ruta/al/SMCA/mqtt/certs/clients/nodo03/client.key spiffs/client_key.pem
```

### 2. Generar y Flashear SPIFFS

```bash
# Generar imagen SPIFFS
idf.py spiffsgen

# Flashear SPIFFS
idf.py spiffs-flash
```

### 3. Flashear Firmware

```bash
# Flashear firmware
idf.py flash

# O todo junto:
idf.py flash spiffs-flash
```

## ✅ Verificación

Después de compilar, verifica:

```bash
# Ver componentes descargados
ls -la managed_components/

# Deberías ver:
# espressif__mqtt/
```

## 🐛 Troubleshooting

### Error: "mqtt component not found"
- Verifica que `idf_component.yml` existe
- Verifica conexión a internet (necesaria para descargar componentes)

### Error: "SPIFFS failed"
- Normal, SPIFFS está comentado
- Se generará después con `idf.py spiffsgen`

### Error: "Permission denied" en build/
- Ignora y continúa
- O cierra Cursor y elimina manualmente
