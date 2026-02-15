#!/bin/bash
# Script para generar imagen SPIFFS manualmente
# Úsalo si spiffs_create_partition_image no funciona en el build

set -e

# Colores para output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}🔧 Generador de Imagen SPIFFS${NC}"
echo ""

# Verificar que ESP-IDF esté cargado
if ! command -v python3 &> /dev/null; then
    echo -e "${RED}❌ Error: Python3 no encontrado${NC}"
    exit 1
fi

# Directorio del proyecto
PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SPIFFS_DIR="${PROJECT_DIR}/spiffs"
BUILD_DIR="${PROJECT_DIR}/build"

# Verificar que existe el directorio spiffs
if [ ! -d "$SPIFFS_DIR" ]; then
    echo -e "${RED}❌ Error: Directorio spiffs/ no encontrado${NC}"
    exit 1
fi

# Verificar que hay archivos en spiffs
if [ -z "$(ls -A $SPIFFS_DIR/*.crt $SPIFFS_DIR/*.key 2>/dev/null)" ]; then
    echo -e "${YELLOW}⚠️  Advertencia: No se encontraron archivos .crt o .key en spiffs/${NC}"
    echo "   Asegúrate de copiar los certificados antes de generar la imagen."
    echo ""
fi

# Tamaño de la partición SPIFFS (0xF0000 = 960KB en hex)
SPIFFS_SIZE=983040  # 960KB en bytes

# Buscar spiffsgen.py en ESP-IDF
if [ -z "$IDF_PATH" ]; then
    echo -e "${YELLOW}⚠️  IDF_PATH no está configurado. Intentando detectar...${NC}"
    # Intentar detectar desde la ruta común
    if [ -f "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/components/spiffs/spiffsgen.py" ]; then
        SPIFFSGEN="/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/components/spiffs/spiffsgen.py"
    else
        echo -e "${RED}❌ Error: No se puede encontrar spiffsgen.py${NC}"
        echo "   Carga el entorno ESP-IDF primero: . /ruta/esp-idf/export.sh"
        exit 1
    fi
else
    SPIFFSGEN="${IDF_PATH}/components/spiffs/spiffsgen.py"
fi

if [ ! -f "$SPIFFSGEN" ]; then
    echo -e "${RED}❌ Error: spiffsgen.py no encontrado en ${SPIFFSGEN}${NC}"
    exit 1
fi

# Crear directorio build si no existe
mkdir -p "$BUILD_DIR"

# Nombre del archivo de salida
OUTPUT_FILE="${BUILD_DIR}/spiffs.bin"

echo -e "${GREEN}📁 Directorio SPIFFS:${NC} $SPIFFS_DIR"
echo -e "${GREEN}📦 Archivo de salida:${NC} $OUTPUT_FILE"
echo -e "${GREEN}📏 Tamaño de partición:${NC} ${SPIFFS_SIZE} bytes (960KB)"
echo ""

# Listar archivos que se incluirán
echo -e "${GREEN}📄 Archivos en spiffs/:${NC}"
ls -lh "$SPIFFS_DIR" | grep -E "\.(crt|key|txt)$" || echo "   (ninguno encontrado)"
echo ""

# Generar imagen SPIFFS
echo -e "${GREEN}🔨 Generando imagen SPIFFS...${NC}"
python3 "$SPIFFSGEN" "$SPIFFS_SIZE" "$SPIFFS_DIR" "$OUTPUT_FILE"

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}✅ Imagen SPIFFS generada exitosamente!${NC}"
    echo -e "${GREEN}📦 Archivo:${NC} $OUTPUT_FILE"
    ls -lh "$OUTPUT_FILE"
    echo ""
    echo -e "${YELLOW}📤 Para flashear SPIFFS, ejecuta:${NC}"
    echo "   idf.py -p /dev/cu.usbserial-XXXX spiffs-flash"
    echo ""
    echo -e "${YELLOW}   O flashea manualmente con esptool:${NC}"
    echo "   esptool.py -p /dev/cu.usbserial-XXXX write_flash 0x110000 $OUTPUT_FILE"
else
    echo ""
    echo -e "${RED}❌ Error al generar imagen SPIFFS${NC}"
    exit 1
fi
