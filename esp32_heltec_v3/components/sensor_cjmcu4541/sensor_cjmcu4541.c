/**
 * @file sensor_cjmcu4541.c
 * @brief Driver CJMCU-4541 / MiCS-4514 - ADC analogico, NO2/CO/NH3
 *
 * La placa CJMCU-4541 (version analogica) expone pines RED y NOX con voltajes
 * proporcionales a la resistencia de cada elemento sensor. Se usa el ADC1 del
 * ESP32-S3 para leer los voltajes y las formulas DFRobot_MICS para convertir
 * el ratio Rs/R0 a concentraciones en ppm.
 */

#include "sensor_cjmcu4541.h"
#include "config.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdbool.h>

static const char *TAG = "CJMCU4541";

#define VCC_MV  3300

static adc_oneshot_unit_handle_t s_adc_handle = NULL;
static adc_cali_handle_t s_cali_handle = NULL;

static adc_channel_t s_red_channel;
static adc_channel_t s_nox_channel;

static float s_r0_factor_red = 0.0f;
static float s_r0_factor_nox = 0.0f;
static bool s_calibrated = false;

static float get_no2_ppm(float rs_r0_ox) {
    if (rs_r0_ox < 1.1f) return 0.0f;
    float no2 = (rs_r0_ox - 0.045f) / 6.13f;
    if (no2 < 0.1f) return 0.0f;
    if (no2 > 10.0f) return 10.0f;
    return no2;
}

static float get_co_ppm(float rs_r0_red) {
    if (rs_r0_red > 0.425f) return 0.0f;
    float co = (0.425f - rs_r0_red) / 0.000405f;
    if (co > 1000.0f) return 1000.0f;
    if (co < 1.0f) return 0.0f;
    return co;
}

static float get_nh3_ppm(float rs_r0_red) {
    if (rs_r0_red > 0.8f) return 0.0f;
    float nh3 = (0.8f - rs_r0_red) / 0.0015f;
    if (nh3 < 1.0f) return 0.0f;
    if (nh3 > 500.0f) return 500.0f;
    return nh3;
}

static esp_err_t gpio_to_adc_channel(int gpio, adc_channel_t *channel) {
    /* ESP32-S3 ADC1: GPIO1=CH0, GPIO2=CH1, ... GPIO10=CH9 */
    if (gpio < 1 || gpio > 10) return ESP_ERR_INVALID_ARG;
    *channel = (adc_channel_t)(gpio - 1);
    return ESP_OK;
}

esp_err_t cjmcu4541_init(void) {
    esp_err_t ret;

    ret = gpio_to_adc_channel(CJMCU_RED_GPIO, &s_red_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO %d no es un canal ADC1 valido", CJMCU_RED_GPIO);
        return ret;
    }
    ret = gpio_to_adc_channel(CJMCU_NOX_GPIO, &s_nox_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "GPIO %d no es un canal ADC1 valido", CJMCU_NOX_GPIO);
        return ret;
    }

    adc_oneshot_unit_init_cfg_t unit_cfg = { .unit_id = ADC_UNIT_1 };
    ret = adc_oneshot_new_unit(&unit_cfg, &s_adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "adc_oneshot_new_unit: %s", esp_err_to_name(ret));
        return ret;
    }

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ret = adc_oneshot_config_channel(s_adc_handle, s_red_channel, &chan_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "config RED channel: %s", esp_err_to_name(ret));
        return ret;
    }
    ret = adc_oneshot_config_channel(s_adc_handle, s_nox_channel, &chan_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "config NOX channel: %s", esp_err_to_name(ret));
        return ret;
    }

    adc_cali_curve_fitting_config_t cali_cfg = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_cfg, &s_cali_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Calibracion ADC no disponible (%s), se usara conversion lineal", esp_err_to_name(ret));
        s_cali_handle = NULL;
    }

    s_calibrated = false;
    s_r0_factor_red = 0.0f;
    s_r0_factor_nox = 0.0f;

    ESP_LOGI(TAG, "CJMCU-4541 (MiCS-4514) ADC init OK (RED=GPIO%d CH%d, NOX=GPIO%d CH%d)",
             CJMCU_RED_GPIO, s_red_channel, CJMCU_NOX_GPIO, s_nox_channel);
    return ESP_OK;
}

static esp_err_t read_channel_mv(adc_channel_t channel, int *out_mv) {
    int raw;
    esp_err_t ret = adc_oneshot_read(s_adc_handle, channel, &raw);
    if (ret != ESP_OK) return ret;

    if (s_cali_handle) {
        return adc_cali_raw_to_voltage(s_cali_handle, raw, out_mv);
    }
    /* Conversion lineal como fallback (ADC_ATTEN_DB_11 ~ 0-3100 mV efectivos) */
    *out_mv = (int)((float)raw / 4095.0f * 3100.0f);
    return ESP_OK;
}

esp_err_t cjmcu4541_is_connected(void) {
    if (s_adc_handle == NULL) return ESP_ERR_INVALID_STATE;
    int red_mv, nox_mv;
    esp_err_t ret = read_channel_mv(s_red_channel, &red_mv);
    if (ret != ESP_OK) return ret;
    ret = read_channel_mv(s_nox_channel, &nox_mv);
    if (ret != ESP_OK) return ret;
    /* Si ambos canales estan saturados en 0 o al maximo, el sensor no esta conectado */
    if ((red_mv < 50 && nox_mv < 50) || (red_mv > 3050 && nox_mv > 3050)) {
        return ESP_ERR_NOT_FOUND;
    }
    return ESP_OK;
}

esp_err_t cjmcu4541_read_measurement(float *no2_ppm, float *co_ppm, float *nh3_ppm) {
    if (s_adc_handle == NULL) return ESP_ERR_INVALID_STATE;

    int red_mv, nox_mv;
    esp_err_t ret = read_channel_mv(s_red_channel, &red_mv);
    if (ret != ESP_OK) return ret;
    ret = read_channel_mv(s_nox_channel, &nox_mv);
    if (ret != ESP_OK) return ret;

    /* Proteccion contra division por cero */
    if (red_mv < 1) red_mv = 1;
    if (nox_mv < 1) nox_mv = 1;

    /* Rs_factor = (VCC - Vout) / Vout, proporcional a Rs del sensor */
    float factor_red = (float)(VCC_MV - red_mv) / (float)red_mv;
    float factor_nox = (float)(VCC_MV - nox_mv) / (float)nox_mv;

    if (!s_calibrated) {
        s_r0_factor_red = factor_red;
        s_r0_factor_nox = factor_nox;
        if (s_r0_factor_red < 0.001f) s_r0_factor_red = 0.001f;
        if (s_r0_factor_nox < 0.001f) s_r0_factor_nox = 0.001f;
        s_calibrated = true;
        ESP_LOGI(TAG, "Baseline: RED=%d mV (factor=%.3f), NOX=%d mV (factor=%.3f)",
                 red_mv, s_r0_factor_red, nox_mv, s_r0_factor_nox);
        if (no2_ppm) *no2_ppm = 0.0f;
        if (co_ppm)  *co_ppm  = 0.0f;
        if (nh3_ppm) *nh3_ppm = 0.0f;
        return ESP_OK;
    }

    float rs_r0_red = factor_red / s_r0_factor_red;
    float rs_r0_nox = factor_nox / s_r0_factor_nox;

    if (no2_ppm) *no2_ppm = get_no2_ppm(rs_r0_nox);
    if (co_ppm)  *co_ppm  = get_co_ppm(rs_r0_red);
    if (nh3_ppm) *nh3_ppm = get_nh3_ppm(rs_r0_red);

    return ESP_OK;
}
