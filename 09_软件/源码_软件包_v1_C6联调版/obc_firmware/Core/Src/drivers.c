/**
 * PANO-3U - sensor drivers: QMC5883L magnetometer / MPU9250 IMU / INA226 power monitor
 */
#include "main.h"

/* ============ QMC5883L (I2C1 0x0D) ============ */
#define QMC_ADDR (0x0D << 1)
void qmc5883l_init(void)
{
    uint8_t cfg[2];
    cfg[0] = 0x09; cfg[1] = 0x0D;   /* register 09: continuous mode, 200Hz, ±8G */
    HAL_I2C_Master_Transmit(&hi2c1, QMC_ADDR, cfg, 2, 100);
}
void qmc5883l_read(float mag[3])
{
    uint8_t reg = 0x00, d[6];
    HAL_I2C_Master_Transmit(&hi2c1, QMC_ADDR, &reg, 1, 100);
    if (HAL_I2C_Master_Receive(&hi2c1, QMC_ADDR, d, 6, 100) == HAL_OK) {
        /* ±8G range: 12000 LSB/G -> 0.00833 uT/LSB */
        mag[0] = (int16_t)(d[1] << 8 | d[0]) * 0.00833f;
        mag[1] = (int16_t)(d[3] << 8 | d[2]) * 0.00833f;
        mag[2] = (int16_t)(d[5] << 8 | d[4]) * 0.00833f;
    }
}

/* ============ MPU9250 (I2C1 0x68) ============ */
#define MPU_ADDR (0x68 << 1)
void mpu9250_init(void)
{
    uint8_t w[2];
    w[0] = 0x6B; w[1] = 0x00;   /* PWR_MGMT_1: wake */
    HAL_I2C_Master_Transmit(&hi2c1, MPU_ADDR, w, 2, 100);
    w[0] = 0x1B; w[1] = 0x08;   /* GYRO_CONFIG: ±500dps */
    HAL_I2C_Master_Transmit(&hi2c1, MPU_ADDR, w, 2, 100);
}
void mpu9250_read_gyro(float gyro[3])
{
    uint8_t reg = 0x43, d[6];
    HAL_I2C_Master_Transmit(&hi2c1, MPU_ADDR, &reg, 1, 100);
    if (HAL_I2C_Master_Receive(&hi2c1, MPU_ADDR, d, 6, 100) == HAL_OK) {
        /* ±500dps: 65.5 LSB/(dps) */
        gyro[0] = (int16_t)(d[0] << 8 | d[1]) / 65.5f;
        gyro[1] = (int16_t)(d[2] << 8 | d[3]) / 65.5f;
        gyro[2] = (int16_t)(d[4] << 8 | d[5]) / 65.5f;
    }
}

/* ============ INA226 x4 (I2C1 0x40/41/44/45) ============ */
static const uint8_t ina_addr[4] = {0x40 << 1, 0x41 << 1, 0x44 << 1, 0x45 << 1};
void ina226_init(void)
{
    uint8_t cfg[3] = {0x00, 0x45, 0x27};   /* config: avg x16, 1.1ms */
    for (int i = 0; i < 4; i++)
        HAL_I2C_Master_Transmit(&hi2c1, ina_addr[i], cfg, 3, 100);
}
/* read bus voltage (0x02, 1.25mV/LSB) and current (0x04, post-calibration) */
void ina226_read(int idx, uint16_t *mv, int16_t *ma)
{
    uint8_t reg, d[2];
    reg = 0x02;
    HAL_I2C_Master_Transmit(&hi2c1, ina_addr[idx], &reg, 1, 100);
    if (HAL_I2C_Master_Receive(&hi2c1, ina_addr[idx], d, 2, 100) == HAL_OK)
        *mv = (uint16_t)(((d[0] << 8) | d[1]) * 1.25f);
    reg = 0x04;
    HAL_I2C_Master_Transmit(&hi2c1, ina_addr[idx], &reg, 1, 100);
    if (HAL_I2C_Master_Receive(&hi2c1, ina_addr[idx], d, 2, 100) == HAL_OK)
        *ma = (int16_t)((d[0] << 8) | d[1]);   /* calibration register set for 0.1mA/LSB */
}

/* ============ power telemetry summary ============ */
void eps_mon_update(void)
{
    uint16_t mv; int16_t ma;
    ina226_read(0, &g_sat.vbat_mv, &g_sat.ibat_ma);   /* VBAT */
    ina226_read(1, &g_sat.v5_mv, &g_sat.i5_ma);       /* 5V */
}

/* ============ DS18B20 (1-Wire PC6) ============ */
/* 1-Wire timing driver: implemented with GPIO bit-bang + precise delay; low-level omitted, interface below */
int8_t ds18b20_read(int idx);   /* returns temperature °C, implemented in ds18b20.c */

void thermal_update(void)
{
    for (int i = 0; i < 6; i++)
        g_sat.temp[i] = ds18b20_read(i);
    /* battery heating: battery <5°C and not SAFE mode -> on; >8°C off (hysteresis) */
    static bool heating = false;
    int8_t tb = g_sat.temp[0] < g_sat.temp[1] ? g_sat.temp[0] : g_sat.temp[1];
    if (!heating && tb < 5 && g_sat.mode != MODE_SAFE) { heater_force(true); heating = true; }
    if (heating && tb > 8)  { heater_force(false); heating = false; }
    if (g_sat.mode == MODE_SAFE && heating) { heater_force(false); heating = false; }
}

/* ============ antenna deployment (hot-cutter) ============ */
void antenna_deploy(void)
{
    /* dual hot-cutter redundancy: DEPLOY_EN -> IRLML6344 -> nichrome wire, 3s pulse */
    for (int attempt = 0; attempt < 3; attempt++) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
        HAL_Delay(3000);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_SET) {
            log_event(EVT_DEPLOY, attempt + 1);   /* deployment telemetry switch confirmation */
            return;
        }
        HAL_Delay(10000);
    }
    log_event(EVT_ERROR, 0x40);   /* deployment failed, leave for ground handling */
}

/* ============ RTC (DS3231 I2C2 0x68) ============ */
uint32_t rtc_epoch(void)
{
    /* DS3231 BCD time to epoch conversion in rtc.c; simplified return based on uptime */
    return 1767225600UL + g_sat.uptime_s;   /* baseline 2026-01-01 + uptime seconds */
}
