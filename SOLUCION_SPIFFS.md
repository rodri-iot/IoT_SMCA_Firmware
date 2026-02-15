# 🔧 Solución: Partición SPIFFS No Encontrada

## ❌ Problema

Al flashear el firmware, aparece el error:
```
E (461) SPIFFS: spiffs partition could not be found
E (471) SPIFFS: Fallo al montar SPIFFS (ESP_ERR_NOT_FOUND)
```

Y en los logs del bootloader solo aparecen 3 particiones:
- nvs
- phy_init  
- factory

**Falta la partición `spiffs`**

## ✅ Causa

El proyecto estaba usando la tabla de particiones por defecto (`SINGLE_APP`) en lugar de la tabla personalizada (`partitions.csv`) que incluye SPIFFS.

## 🔧 Solución Aplicada

1. **Actualizado `sdkconfig`** para usar tabla personalizada:
   - Cambiado `CONFIG_PARTITION_TABLE_SINGLE_APP=y` → `CONFIG_PARTITION_TABLE_CUSTOM=y`
   - Configurado `CONFIG_PARTITION_TABLE_FILENAME="partitions.csv"`

2. **Corregida zona horaria** en `config.h`:
   - Cambiado `TIMEZONE "BOT4"` → `TIMEZONE "ART3"` (Buenos Aires, UTC-3)

## 📋 Verificación

Después de recompilar, verifica que la partición SPIFFS aparece:

```bash
idf.py flash monitor
```

En los logs del bootloader deberías ver:
```
I (57) boot: ## Label            Usage          Type ST Offset   Length
I (64) boot:  0 nvs              WiFi data        01 02 00009000 00006000
I (72) boot:  1 phy_init         RF data          01 01 0000f000 00001000
I (79) boot:  2 factory          factory app      00 00 00010000 00100000
I (87) boot:  3 spiffs           SPIFFS           01 82 00110000 000f0000  ← DEBE APARECER
```

## 🚀 Próximos Pasos

1. **Recompilar:**
   ```bash
   idf.py build
   ```

2. **Flashear firmware:**
   ```bash
   idf.py flash
   ```

3. **Copiar certificados a spiffs/ y flashear SPIFFS:**
   ```bash
   # Copiar certificados
   cp /ruta/certificados/*.crt spiffs/
   cp /ruta/certificados/*.key spiffs/
   
   # Generar y flashear SPIFFS
   idf.py spiffsgen
   idf.py spiffs-flash
   ```

4. **Monitorear:**
   ```bash
   idf.py monitor
   ```

Ahora deberías ver:
- ✅ SPIFFS montado correctamente
- ✅ Certificados cargados
- ✅ Conexión MQTT exitosa
