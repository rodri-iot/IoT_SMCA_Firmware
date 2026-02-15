# 🔧 Solución: Problemas de Permisos con Submódulos ESP-IDF

## Problema

Al intentar actualizar submódulos, aparecen errores de "Operation not permitted" al eliminar archivos en `components/openthread/`.

## Solución: Continuar sin OpenThread

**Importante:** OpenThread NO es necesario para ESP32-S3 con WiFi y MQTT. Puedes continuar con la instalación ignorando este submódulo.

### Opción 1: Continuar con la Instalación (Recomendado)

```bash
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp/esp-idf"

# Instalar herramientas para ESP32-S3 (ignorará openthread)
./install.sh esp32s3

# Configurar entorno
. ./export.sh

# Verificar instalación
idf.py --version
```

El script `install.sh` instalará todas las herramientas necesarias para ESP32-S3, y el error de openthread no afectará tu desarrollo.

### Opción 2: Usar sudo (Si realmente necesitas limpiar)

```bash
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp/esp-idf"

# Eliminar con sudo (solo si es absolutamente necesario)
sudo rm -rf components/openthread

# Recrear submódulos
git submodule update --init --recursive
```

**⚠️ Advertencia:** Usar sudo puede cambiar permisos de archivos. Solo úsalo si es necesario.

### Opción 3: Cambiar Permisos (Alternativa)

```bash
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp/esp-idf"

# Cambiar permisos del directorio
sudo chmod -R u+w components/openthread

# Luego eliminar
rm -rf components/openthread

# Recrear submódulos
git submodule update --init --recursive
```

## Verificación

Después de instalar ESP-IDF, verifica que todo funcione:

```bash
# Verificar que ESP-IDF esté configurado
idf.py --version

# Verificar herramientas
xtensa-esp32s3-elf-gcc --version

# Probar compilación en tu proyecto
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"
idf.py set-target esp32s3
idf.py build
```

## Nota Importante

El error de openthread **NO afecta** el desarrollo con ESP32-S3 para:
- ✅ WiFi
- ✅ MQTT
- ✅ TLS/SSL
- ✅ SPIFFS
- ✅ Sensores I2C/SPI

OpenThread solo es necesario si planeas usar protocolos Thread (mesh networking), que no es tu caso.

## Próximos Pasos

1. Ejecuta `./install.sh esp32s3` (ignorará el error de openthread)
2. Configura el entorno: `. ./export.sh`
3. Continúa con la configuración de tu proyecto según `SETUP_NODO.md`
