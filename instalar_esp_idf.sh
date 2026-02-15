#!/bin/bash
# Script para instalar ESP-IDF v5.1 limpiamente

set -e

PROJECT_DIR="/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware"
ESP_IDF_DIR="$PROJECT_DIR/esp/esp-idf"
TEMP_DIR="$HOME/esp-idf-temp-install"

echo "🚀 Instalando ESP-IDF v5.1 para proyecto SMCA"
echo ""

# Método 1: Intentar clonar en home y mover
echo "📥 Método 1: Clonando en directorio temporal ($TEMP_DIR)..."
mkdir -p "$(dirname $TEMP_DIR)"
cd "$(dirname $TEMP_DIR)"

if git clone --recursive --branch v5.1 https://github.com/espressif/esp-idf.git "$(basename $TEMP_DIR)" 2>/dev/null; then
    echo "✅ Clonado exitosamente"
    echo "📦 Moviendo a ubicación final..."
    mkdir -p "$PROJECT_DIR/esp"
    mv "$TEMP_DIR" "$ESP_IDF_DIR"
    echo "✅ ESP-IDF instalado en: $ESP_IDF_DIR"
else
    echo "⚠️  Método 1 falló, intentando método alternativo..."
    
    # Método 2: Usar instalador oficial
    echo "📥 Método 2: Descargando instalador oficial..."
    cd "$PROJECT_DIR/esp"
    
    # Descargar script de instalación
    if command -v curl &> /dev/null; then
        curl -L https://raw.githubusercontent.com/espressif/esp-idf/master/install.sh -o install.sh
    elif command -v wget &> /dev/null; then
        wget https://raw.githubusercontent.com/espressif/esp-idf/master/install.sh
    else
        echo "❌ Error: Se necesita curl o wget"
        exit 1
    fi
    
    chmod +x install.sh
    
    echo "🔧 Ejecutando instalador (esto puede tardar varios minutos)..."
    echo "   Esto descargará ESP-IDF v5.1 automáticamente"
    ./install.sh esp32s3
    
    if [ -d "esp-idf" ]; then
        echo "✅ Instalación completada"
    else
        echo "❌ La instalación no creó el directorio esperado"
        echo "💡 Verifica los logs arriba para más información"
        exit 1
    fi
fi

echo ""
echo "✅ ESP-IDF instalado en: $ESP_IDF_DIR"
echo ""
echo "📝 Próximos pasos:"
echo "   1. cd $ESP_IDF_DIR"
echo "   2. . ./export.sh"
echo "   3. idf.py --version"
echo ""
echo "   O agrega a ~/.zshrc:"
echo "   export IDF_PATH=\"$ESP_IDF_DIR\""
echo "   alias get_idf='. \$IDF_PATH/export.sh'"
