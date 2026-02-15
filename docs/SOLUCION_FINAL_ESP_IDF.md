# 🔧 Solución Final: Instalar ESP-IDF v5.1

## Problema Detectado

Hay restricciones de permisos que impiden clonar con git directamente. Usaremos el **instalador oficial de ESP-IDF** que maneja esto automáticamente.

## Solución: Instalador Oficial

### Paso 1: Ejecutar Instalador

El instalador oficial descarga ESP-IDF sin usar git directamente:

```bash
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp"

# Descargar instalador
curl -L https://raw.githubusercontent.com/espressif/esp-idf/master/install.sh -o install.sh

# Dar permisos de ejecución
chmod +x install.sh

# Ejecutar instalador para ESP32-S3
# Esto descargará ESP-IDF v5.1 automáticamente
./install.sh esp32s3
```

**Nota:** El instalador puede tardar 10-20 minutos dependiendo de tu conexión.

### Paso 2: Verificar Instalación

```bash
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp/esp-idf"

# Cargar entorno
. ./export.sh

# Verificar
idf.py --version
```

### Paso 3: Configurar Permanente

Agrega a tu `~/.zshrc`:

```bash
# ESP-IDF para proyecto V_2026
export IDF_PATH="/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp/esp-idf"
alias get_idf='. $IDF_PATH/export.sh'
```

Luego:
```bash
source ~/.zshrc
```

## Alternativa: Usar Script Automático

He creado un script que intenta múltiples métodos:

```bash
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware"
./instalar_esp_idf.sh
```

## Si el Instalador También Falla

Si incluso el instalador tiene problemas, puede ser:

1. **Antivirus/Seguridad:** Verifica si algún software de seguridad está bloqueando
2. **Permisos de usuario:** Verifica que tengas permisos de escritura
3. **Espacio en disco:** Verifica que tengas suficiente espacio (ESP-IDF necesita ~2GB)

### Verificar Espacio

```bash
df -h "/Users/rodripinedo/DOCS"
```

### Verificar Permisos

```bash
ls -ld "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp"
```

## Solución Temporal: Usar Copia Existente

Si necesitas trabajar inmediatamente mientras se resuelve el problema:

```bash
# Usar la copia existente temporalmente
export IDF_PATH="/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf"
. $IDF_PATH/export.sh
```

Esto te permitirá compilar mientras resuelves el problema de instalación.
