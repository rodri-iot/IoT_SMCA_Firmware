# SCD40 en Heltec WiFi LoRa 32 V3

## Pines I2C (conector J3)

| Señal | GPIO | Conector |
|-------|------|----------|
| SDA   | 45   | J3 Pin 6 |
| SCL   | 46   | J3 Pin 5 |

La configuración se define en `esp32_heltec_v3/main/config.h` (`I2C_SDA_GPIO`, `I2C_SCL_GPIO`). Alimentación del SCD40: 3.3 V (J3 P2/3) y GND (J3 P1).

## Troubleshooting

Si en el monitor aparece **"Start periodic falló: ESP_FAIL"** o **"no se pudo iniciar medición periódica"**:

1. El firmware envía el comando **Stop Periodic Measurement**, espera 500 ms y luego **Start Periodic Measurement** para forzar al SCD40 a modo idle antes de iniciar. Asegúrate de tener la versión que incluye esta secuencia.
2. Comprueba el **cableado** (SDA/SCL en J3 según la tabla anterior) y que el SCD40 esté bien alimentado a **3.3 V**.
3. Si el error persiste, prueba con otro módulo SCD40 o revisa pull-ups y longitud de cables (I2C a 50 kHz en el firmware para mayor margen).
