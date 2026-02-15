#!/bin/bash
# Script para flashear solo la partición SPIFFS
# Útil para actualizar certificados sin recompilar el firmware

set -e

# Colores
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${GREEN}📤 Flasheando Partición SPIFFS${NC}"
echo ""

# Verificar que ESP-IDF esté cargado
if [ -z "$IDF_PATH" ]; then
    echo -e "${YELLOW}⚠️  Cargando entorno ESP-IDF...${NC}"
    . /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh
fi

# Directorio del proyecto
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SPIFFS_IMAGE="${PROJECT_DIR}/build/spiffs.bin"

# Verificar que existe la imagen SPIFFS
if [ ! -f "$SPIFFS_IMAGE" ]; then
    echo -e "${RED}❌ Error: No se encontró build/spiffs.bin${NC}"
    echo ""
    echo "Genera la imagen SPIFFS primero:"
    echo "  ./generar_spiffs.sh"
    exit 1
fi

# Detectar puerto serial
PORT="${1:-/dev/cu.usbserial-0001}"

if [ ! -e "$PORT" ]; then
    echo -e "${YELLOW}⚠️  Puerto $PORT no encontrado${NC}"
    echo ""
    echo "Puertos disponibles:"
    ls /dev/cu.usbserial-* /dev/tty.usbserial-* 2>/dev/null || echo "  (ninguno encontrado)"
    echo ""
    echo "Uso: $0 [puerto]"
    echo "Ejemplo: $0 /dev/cu.usbserial-0001"
    exit 1
fi

echo -e "${GREEN}📦 Imagen SPIFFS:${NC} $SPIFFS_IMAGE"
echo -e "${GREEN}📏 Tamaño:${NC} $(ls -lh "$SPIFFS_IMAGE" | awk '{print $5}')"
echo -e "${GREEN}🔌 Puerto:${NC} $PORT"
echo ""

# Dirección de la partición SPIFFS (0x110000 según partitions.csv)
SPIFFS_OFFSET=0x110000

echo -e "${GREEN}🔨 Flasheando SPIFFS a $SPIFFS_OFFSET...${NC}"
echo ""

# Flashear usando esptool
python3 "$IDF_PATH/components/esptool_py/esptool/esptool.py" \
  --chip esp32s3 \
  -p "$PORT" \
  -b 460800 \
  write_flash \
  --flash_mode dio \
  --flash_freq 80m \
  --flash_size 2MB \
  "$SPIFFS_OFFSET" "$SPIFFS_IMAGE"

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}✅ SPIFFS flasheado exitosamente!${NC}"
    echo ""
    echo -e "${YELLOW}📊 Para verificar, monitorea los logs:${NC}"
    echo "   idf.py -p $PORT monitor"
else
    echo ""
    echo -e "${RED}❌ Error al flashear SPIFFS${NC}"
    exit 1
fi
