#include "sensor_sim.h"
#include "esp_system.h"

static float simular(float min, float max) {
    return min + (esp_random() % (int)((max - min) * 100)) / 100.0f;
}

float simular_pm25(void)         { return simular(5.0, 150.0); }
float simular_pm10(void)         { return simular(10.0, 200.0); }
float simular_co2(void)          { return simular(400.0, 2000.0); }
float simular_no2(void)          { return simular(0.01, 0.20); }
float simular_co(void)           { return simular(0.1, 10.0); }
float simular_voc(void)          { return simular(0.1, 1.5); }
float simular_temperatura(void)  { return simular(15.0, 35.0); }
float simular_humedad(void)      { return simular(30.0, 90.0); }
