# 1. Cargar entorno ESP-IDF
. /Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/IoT_SMCA_Firmware/esp-idf/export.sh

# 2. Verificar que funciona
idf.py --version

# 3. Ir al proyecto
cd "/Users/rodripinedo/DOCS/UBA_Master/Trabajo_Final-IoT/V_2026/IoT_SMCA_Firmware/esp32_heltec_v3"

# 4. Compilar
idf.py build

# 5. Flashear (primera vez)
idf.py -p /dev/cu.usbserial-0001 flash

# 6. Actualizar certificados
./generar_spiffs.sh
./flash_spiffs.sh /dev/cu.usbserial-0001