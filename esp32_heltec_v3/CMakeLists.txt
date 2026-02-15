# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.16)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(esp32_heltec_v3)

# Crear imagen SPIFFS desde el directorio spiffs
# Nota: Los certificados TLS deben copiarse a spiffs/ antes de compilar
# 
# IMPORTANTE: Copiar certificados antes de compilar:
# cp /ruta/certificados/ca.crt spiffs/
# cp /ruta/certificados/client.crt spiffs/
# cp /ruta/certificados/client.key spiffs/
#
# La imagen SPIFFS se generará automáticamente durante el build
# y se flasheará junto con el firmware al ejecutar: idf.py flash
spiffs_create_partition_image(spiffs ${CMAKE_CURRENT_SOURCE_DIR}/spiffs FLASH_IN_PROJECT)