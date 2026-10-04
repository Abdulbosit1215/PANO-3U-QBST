/**
 * PANO-3U OBC - global types and shared definitions
 */
#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

/* ---- Operating modes ---- */
typedef enum {
    MODE_SAFE = 0,       /* Safe mode: minimum power, wait for ground command */
    MODE_DETUMBLE,       /* Detumble: post-deployment B-dot */
    MODE_IDLE,           /* Idle: nominal telemetry */
    MODE_PAYLOAD,        /* Payload imaging active */
    MODE_DOWNLINK,       /* Downlink active */
    MODE_DEPLOY,         /* Antenna deployment sequence */
} SatMode_t;

/* ---- Error codes (aligned with protocol spec §3) ---- */
typedef enum {
    ERR_NONE = 0, ERR_CAM0, ERR_CAM1, ERR_DISK_FULL, ERR_DISK_IO,
    ERR_ENCODER, ERR_TEMP, ERR_PARAM, ERR_LINK_TIMEOUT,
    ERR_LOW_POWER, ERR_COMM, ERR_WATCHDOG_RESET,
} ErrCode_t;

/* ---- Log event codes ---- */
typedef enum {
    EVT_BOOT = 0x01, EVT_MODE_CHANGE, EVT_LOW_POWER, EVT_DEPLOY,
    EVT_SHOOT, EVT_VIDEO, EVT_DOWNLINK_START, EVT_DOWNLINK_DONE,
    EVT_CMD_RX, EVT_ERROR,
} EvtCode_t;

/* ---- Global spacecraft state ---- */
typedef struct {
    SatMode_t mode;
    uint32_t  uptime_s;
    uint32_t  boot_count;
    uint8_t   last_error;
    /* ADCS */
    float mag[3];        /* magnetic field uT */
    float gyro[3];       /* angular rate deg/s */
    float mag_prev[3];
    /* power */
    uint16_t vbat_mv;
    int16_t  ibat_ma;
    uint16_t v5_mv, i5_ma;
    /* temperatures x6: battery0/1, camera0/1, OBC board, EPS board */
    int8_t   temp[6];
    /* cached payload status (CM4 STATUS frame) */
    uint8_t  pl_mode, pl_err;
    uint16_t pl_free_mb, pl_files;
    bool     pl_online;
} SatState_t;

extern SatState_t g_sat;

/* module interfaces */
void MX_GPIO_Init(void); void MX_I2C1_Init(void); void MX_I2C2_Init(void);
void MX_SPI2_Init(void); void MX_USART1_Init(void); void MX_USART2_Init(void);
void MX_TIM1_PWM_Init(void); void MX_ADC_Init(void);
void task_hk(void *); void task_adcs(void *); void task_comm(void *); void task_payload(void *);
void power_guard(void); void heater_force(bool on); void antenna_deploy(void);
void thermal_update(void);

/* payload power switch: PAYLOAD_EN -> EPS TPS2553 -> CM4 5V */
static inline void payload_power(bool on) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
static inline void heater_force(bool on) {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, on ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

#endif
