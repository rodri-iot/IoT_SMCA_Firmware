#!/bin/bash
# Script para configurar ESP-IDF existente para este proyecto

ESP_IDF_PATH="/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf"

echo "🔧 Configurando ESP-IDF existente para este proyecto"
echo ""

# Verificar que existe
if [ ! -d "$ESP_IDF_PATH" ]; then
    echo "❌ Error: No se encuentra ESP-IDF en: $ESP_IDF_PATH"
    exit 1
fi

echo "✅ ESP-IDF encontrado: $ESP_IDF_PATH"
echo ""

# Agregar a .zshrc si no existe
if ! grep -q "IDF_PATH.*IoT_SMCA_Firmware/esp-idf" ~/.zshrc 2>/dev/null; then
    echo "📝 Agregando configuración a ~/.zshrc..."
    cat >> ~/.zshrc << EOF

# ESP-IDF para proyecto V_2026 (compartido con otro proyecto)
export IDF_PATH="$ESP_IDF_PATH"
alias get_idf='. \$IDF_PATH/export.sh'
EOF
    echo "✅ Configuración agregada a ~/.zshrc"
else
    echo "ℹ️  Configuración ya existe en ~/.zshrc"
fi

echo ""
echo "📋 Para usar ESP-IDF en esta sesión:"
echo "   1. source ~/.zshrc"
echo "   2. get_idf"
echo "   O directamente:"
echo "   . $ESP_IDF_PATH/export.sh"
echo ""
echo "✅ Configuración completada"
