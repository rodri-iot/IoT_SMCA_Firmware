# 📁 Certificados TLS para SPIFFS

## Archivos Requeridos

Copia los siguientes archivos a este directorio (`spiffs/`):

1. **ca.crt** - Certificado CA del broker MQTT
2. **client.crt** - Certificado del cliente (nodo-03)
3. **client.key** - Clave privada del cliente (nodo-03)

## Origen de los Certificados

Los certificados deben generarse desde tu proyecto SMCA:
- CA: `mqtt/certs/ca/ca.crt`
- Cliente: `mqtt/certs/clients/nodo03/client.crt`
- Clave: `mqtt/certs/clients/nodo03/client.key`

## Copiar Certificados

```bash
# Desde el directorio del proyecto SMCA
cp mqtt/certs/ca/ca.crt esp32_heltec_v3/spiffs/
cp mqtt/certs/clients/nodo03/client.crt esp32_heltec_v3/spiffs/
cp mqtt/certs/clients/nodo03/client.key esp32_heltec_v3/spiffs/
```

## Verificar Archivos

```bash
ls -lh esp32_heltec_v3/spiffs/
# Debe mostrar:
# - ca.crt
# - client.crt
# - client.key
```

## Generar y Flashear SPIFFS

Después de copiar los certificados:

```bash
cd esp32_heltec_v3
idf.py spiffsgen
idf.py spiffs-flash
```
