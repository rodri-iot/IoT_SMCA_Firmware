# 📋 Paso a Paso: Compilar Proyecto ESP32

## ✅ Paso 1: Verificar Versiones

### 1.1 Verificar Python

```bash
python3 --version
```

**Salida esperada:** `Python 3.8.x` o superior (debe ser 3.8+)

### 1.2 Verificar que ESP-IDF existe

```bash
ls -la /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh
```

**Salida esperada:** Debe mostrar el archivo (no error "No such file")

### 1.3 Verificar versión de ESP-IDF (opcional)

```bash
cd /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf
git describe --tags
```

**Salida esperada:** `v5.1.x` o similar

## 🔧 Paso 2: Cargar Entorno ESP-IDF

**⚠️ IMPORTANTE:** Este paso es necesario cada vez que abres una terminal nueva.

```bash
. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh
```

**Nota:** El punto (`.`) al inicio es **obligatorio**. Es la forma de ejecutar un script en el shell actual.

**Salida esperada:**
```
Detecting the Python interpreter
Checking "python3" ...
Python 3.12.3
"python3" has been detected
...
Done! You can now compile ESP-IDF projects.
Go to the project directory and run:
  idf.py build
```

### 2.1 Verificar que idf.py está disponible

```bash
which idf.py
```

**Salida esperada:** Debe mostrar una ruta (no "command not found")

```bash
idf.py --version
```

**Salida esperada:** `ESP-IDF v5.1.2` o similar

## 📁 Paso 3: Ir al Directorio del Proyecto

```bash
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"
```

**Verificar que estás en el lugar correcto:**
```bash
pwd
ls -la
```

**Debes ver:** `CMakeLists.txt`, `main/`, `components/`, etc.

## 🎯 Paso 4: Configurar Target (Solo Primera Vez)

Si es la primera vez o cambiaste de target:

```bash
idf.py set-target esp32s3
```

**Salida esperada:**
```
Set Target to: esp32s3, new sdkconfig will be created.
Running cmake in directory ...
-- Configuring done
-- Generating done
-- Build files have been written to: ...
```

**Nota:** Si ya configuraste el target antes, puedes saltar este paso.

## 🔨 Paso 5: Compilar

```bash
idf.py build
```

**Salida esperada (al final):**
```
Project build complete. To flash, run:
idf.py flash
```

**Tiempo estimado:** 2-5 minutos (primera vez puede tardar más)

## ✅ Paso 6: Verificar Compilación Exitosa

```bash
ls -lh build/esp32_heltec_v3.bin
```

**Salida esperada:** Debe mostrar el archivo `.bin` con su tamaño.

## 🐛 Solución de Problemas

### Error: "command not found: idf.py"

**Causa:** No cargaste el entorno ESP-IDF.

**Solución:**
```bash
. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh
```

### Error: "No such file or directory: export.sh"

**Causa:** La ruta de ESP-IDF es incorrecta.

**Solución:** Verifica la ruta:
```bash
find ~ -name "export.sh" -path "*/esp-idf/*" 2>/dev/null
```

### Error: "Python version not supported"

**Causa:** Python muy antiguo o muy nuevo.

**Solución:** Verifica la versión:
```bash
python3 --version
# Debe ser 3.8, 3.9, 3.10, 3.11 o 3.12
```

## 📝 Comandos Completos (Copia y Pega)

```bash
# 1. Cargar entorno ESP-IDF
. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh

# 2. Verificar que funciona
idf.py --version

# 3. Ir al proyecto
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"

# 4. Configurar target (solo primera vez)
idf.py set-target esp32s3

# 5. Compilar
idf.py build
```

## 🎯 Resumen Rápido

Cada vez que abres una terminal nueva:

```bash
# 1. Cargar ESP-IDF (OBLIGATORIO)
. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh

# 2. Ir al proyecto
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"

# 3. Compilar
idf.py build
```

## 💡 Tip: Agregar al .zshrc (Opcional)

Para no tener que cargar el entorno manualmente cada vez, puedes agregar al final de `~/.zshrc`:

```bash
# ESP-IDF
alias get_idf='. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh'
```

Luego ejecuta:
```bash
source ~/.zshrc
```

Y podrás usar:
```bash
get_idf
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"
idf.py build
```
