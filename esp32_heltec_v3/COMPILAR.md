# 🚀 Compilar el Proyecto - Pasos Finales

## Problema Resuelto

He creado el archivo `idf_component.yml` que declara la dependencia de MQTT. El Component Manager descargará automáticamente el componente cuando compiles.

## Pasos para Compilar

### 1. Limpiar Build Anterior

```bash
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"
rm -rf build managed_components
```

### 2. Configurar Target

```bash
idf.py set-target esp32s3
```

**Nota:** La primera vez, el Component Manager descargará el componente MQTT automáticamente. Esto puede tardar unos minutos.

### 3. Compilar

```bash
idf.py build
```

## ¿Qué Esperar?

### Primera Compilación

1. **Descarga de Componentes:**
   ```
   -- Component manager will download: espressif__mqtt
   ```

2. **Compilación:**
   ```
   [1/XXX] Building C object...
   ...
   Project build complete.
   ```

### Si Hay Errores

Si ves errores relacionados con MQTT:

1. **Verificar conexión a internet** (necesaria para descargar componentes)

2. **Verificar archivo idf_component.yml:**
   ```bash
   cat idf_component.yml
   ```

3. **Limpiar y reintentar:**
   ```bash
   rm -rf build managed_components
   idf.py fullclean
   idf.py set-target esp32s3
   idf.py build
   ```

## Verificación

Después de compilar exitosamente:

```bash
# Ver componentes descargados
ls -la managed_components/

# Deberías ver algo como:
# espressif__mqtt/
```

## Explicación del Cambio

### Antes (ESP-IDF v4.x)
- MQTT estaba en `components/mqtt/esp_mqtt`
- Se incluía automáticamente

### Ahora (ESP-IDF v5.1+)
- MQTT se maneja vía Component Manager
- Se declara en `idf_component.yml`
- Se descarga automáticamente a `managed_components/`

### Ventajas
- ✅ Componentes más actualizados
- ✅ Mejor gestión de versiones
- ✅ Fácil actualización
- ✅ Componentes de terceros más fácilmente

## Próximos Pasos

Una vez compilado exitosamente:

1. **Flashear:**
   ```bash
   idf.py flash
   ```

2. **Flashear SPIFFS (certificados):**
   ```bash
   idf.py spiffs-flash
   ```

3. **Monitorear:**
   ```bash
   idf.py monitor
   ```
