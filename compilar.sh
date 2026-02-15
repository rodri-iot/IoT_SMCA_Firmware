#!/bin/bash
# Script para compilar el proyecto ESP32 Heltec V3

echo "🔧 Cargando entorno ESP-IDF..."
. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh

echo "📁 Cambiando al directorio del proyecto..."
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"

echo "🔨 Compilando proyecto..."
idf.py build

if [ $? -eq 0 ]; then
    echo ""
    echo "✅ Compilación exitosa!"
    echo "📦 Para flashear: idf.py flash"
    echo "📊 Para monitorear: idf.py monitor"
else
    echo ""
    echo "❌ Error en la compilación"
    exit 1
fi
