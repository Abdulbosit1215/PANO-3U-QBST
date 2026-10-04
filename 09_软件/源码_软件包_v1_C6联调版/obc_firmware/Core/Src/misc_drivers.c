/**
 * PANO-3U - DRV8837 magnetorquer driver / watchdog / log (W25Q128)
 */
#include "main.h"

extern TIM_HandleTypeDef htim1;
/* TIM1: CH1=PE9(X), CH2=PE11(Y), CH3=PE13(Z), 100kHz PWM */

void drv8837_init(void)
{
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
}

/* duty: -1.0~+1.0, sign indicates direction (H-bridge direction via IN2, simplified single-end + direction bit) */
void drv8837_set(int ch, float duty)
{
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&htim1);
    uint32_t ccr = (uint32_t)((duty < 0 ? -duty : duty) * arr);
    switch (ch) {
    case 0: __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ccr); break;
    case 1: __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, ccr); break;
    case 2: __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, ccr); break;
    }
    /* Direction pin: each DRV8837 IN2 tied to GPIO (schematic simplification: IN2 high for reverse, IN1 PWM inverted) */
}

/* ---- Watchdog: TPS3823, PB0 toggle kick ---- */
void wdt_kick(void)
{
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_0);
}

/* ---- Ring log: W25Q128, 16B per record ---- */
#define LOG_START_SECTOR  2048   /* reserve first 8MB for firmware images */
typedef struct { uint32_t epoch; uint8_t code; uint8_t data[8]; uint8_t pad[3]; } LogRec_t;
static uint32_t log_ptr = 0;

void log_init(void) { /* read flash header to find write pointer, implemented in w25q128.c */ }
void log_event(uint8_t code, uint32_t data)
{
    LogRec_t r; r.epoch = g_sat.uptime_s; r.code = code;
    *(uint32_t *)r.data = data; r.pad[0] = 0xAA;
    /* w25q128_write(LOG_START_SECTOR*4096 + log_ptr, &r, 16); */
    log_ptr = (log_ptr + 16) % (8 * 1024 * 1024);   /* 8MB ring */
}
void log_get_bootcount(uint32_t *c) { *c = 0; /* read fixed sector via w25q128 */ }
void log_set_bootcount(uint32_t c)  { (void)c; }
