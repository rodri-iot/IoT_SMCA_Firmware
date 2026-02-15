# 📥 Instalación Limpia de ESP-IDF v5.1

## Problema Detectado

Hay problemas de permisos al clonar directamente con git. Usaremos el método oficial de instalación de ESP-IDF que maneja mejor estos casos.

## Método Recomendado: Instalador Oficial

### Paso 1: Instalar Python y Herramientas

```bash
# Verificar Python (debe ser 3.8+)
python3 --version

# Instalar pip si no está instalado
python3 -m ensurepip --upgrade
```

### Paso 2: Usar el Instalador de ESP-IDF

El método oficial usa un script Python que descarga y configura todo automáticamente:

```bash
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware"

# Crear directorio para ESP-IDF
mkdir -p esp
cd esp

# Descargar instalador
python3 -m venv venv
source venv/bin/activate

# Instalar esptool (herramienta oficial)
pip install esptool

# O usar el método manual: descargar y ejecutar instalador
# wget https://raw.githubusercontent.com/espressif/esp-idf/master/install.sh
# chmod +x install.sh
# ./install.sh esp32s3
```

### Paso 3: Método Alternativo - Clonar Manualmente con Permisos

Si el problema persiste, intenta con sudo (solo si es necesario):

```bash
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp"

# Clonar con permisos explícitos
sudo git clone --recursive --branch v5.1 https://github.com/espressif/esp-idf.git esp-idf

# Cambiar propietario a tu usuario
sudo chown -R $(whoami) esp-idf

# Continuar con instalación normal
cd esp-idf
./install.sh esp32s3
```

### Paso 4: Verificar Instalación

```bash
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp/esp-idf"
. ./export.sh
idf.py --version
```

## Solución Rápida: Usar Homebrew (macOS)

Si tienes Homebrew instalado:

```bash
# Instalar ESP-IDF vía Homebrew
brew install esp-idf

# Configurar para este proyecto
export IDF_PATH="/opt/homebrew/opt/esp-idf"  # o /usr/local/opt/esp-idf en Intel
. $IDF_PATH/export.sh
```

## Verificar Problemas de Permisos

```bash
# Verificar atributos extendidos (macOS)
xattr -l "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp"

# Si hay atributos problemáticos, eliminarlos:
xattr -c "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp"
```

## Configuración Permanente

Una vez instalado, agrega a `~/.zshrc`:

```bash
# ESP-IDF para proyecto V_2026
export IDF_PATH="/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp/esp-idf"
alias get_idf='. $IDF_PATH/export.sh'
```
