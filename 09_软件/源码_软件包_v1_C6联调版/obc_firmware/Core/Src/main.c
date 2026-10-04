/**
 * PANO-3U OBC firmware - main program
 * MCU: STM32F405RGT6 @168MHz, HAL library
 * Tasks: telemetry collection / mode management / attitude control (B-dot) / payload scheduling / UHF comms
 * RTOS: FreeRTOS
 */
#include "main.h"
#include "obc_fsm.h"
#include "adcs.h"
#include "comm.h"
#include "eps_mon.h"
#include "payload_mgr.h"
#include "watchdog.h"
#include "log.h"

/* HAL handles (CubeMX-generated sections omitted, declared here) */
I2C_HandleTypeDef  hi2c1, hi2c2;
SPI_HandleTypeDef  hspi2;
UART_HandleTypeDef huart1;   /* comm board */
UART_HandleTypeDef huart2;   /* CM4 payload */
TIM_HandleTypeDef  htim1;    /* PWM: magnetorquer XYZ */
WDT_Handle_t       hwdt;

/* global state */
SatState_t g_sat = {
    .mode = MODE_SAFE,
    .uptime_s = 0,
    .boot_count = 0,
    .last_error = ERR_NONE,
    .mag = {0}, .gyro = {0},
    .vbat_mv = 0, .ibat_ma = 0,
    .temp = {25,25,25,25,25,25},
};

/* FreeRTOS tasks */
static TaskHandle_t h_task_hk, h_task_adcs, h_task_comm, h_task_pl;

void SystemClock_Config(void);  /* CubeMX-generated: HSE 8M -> PLL -> 168M */

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* peripheral init */
    MX_GPIO_Init();       /* includes separation-switch / antenna-status GPIO */
    MX_I2C1_Init();       /* INA226/IMU/magnetometer */
    MX_I2C2_Init();       /* DS3231 RTC */
    MX_SPI2_Init();       /* W25Q128 */
    MX_USART1_Init();     /* comm board 9600 */
    MX_USART2_Init();     /* CM4 115200 */
    MX_TIM1_PWM_Init();   /* magnetorquer driver */
    MX_ADC_Init();        /* backup voltage sampling */

    log_init();           /* W25Q128 ring log */
    uint32_t boot = 0;
    log_get_bootcount(&boot);
    g_sat.boot_count = boot + 1;
    log_set_bootcount(g_sat.boot_count);
    log_event(EVT_BOOT, g_sat.boot_count);

    /* post-deployment delay: CDS requires 30 min before RF transmission/mechanism deployment */
    if (g_sat.boot_count <= 2) {   /* only during early post-deployment phase */
        HAL_Delay(30U * 60U * 1000U);
        antenna_deploy();          /* hot-cutter antenna release */
    }

    obc_fsm_init();
    adcs_init();
    comm_init();
    payload_mgr_init();

    /* task creation: priority high->low = comm > ADCS > housekeeping > payload */
    xTaskCreate(task_comm,    "comm", 1024, NULL, 4, &h_task_comm);
    xTaskCreate(task_adcs,    "adcs",  768, NULL, 3, &h_task_adcs);
    xTaskCreate(task_hk,      "hk",   1024, NULL, 2, &h_task_hk);
    xTaskCreate(task_payload, "pl",   1536, NULL, 1, &h_task_pl);

    vTaskStartScheduler();
    while (1);   /* should not reach here */
}

/**
 * housekeeping task: 1Hz telemetry collection + watchdog kick + battery protection
 */
void task_hk(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    for (;;) {
        eps_mon_update();        /* INA226 x4 -> g_sat.vbat_mv, etc. */
        adcs_read_sensors();     /* IMU + magnetometer */
        thermal_update();        /* DS18B20 x6 -> heater control */
        power_guard();           /* low voltage -> SAFE mode */

        wdt_kick();              /* kick TPS3823 (PA0 toggle->actual dedicated GPIO) */

        if ((g_sat.uptime_s % 30) == 0)
            comm_queue_beacon(); /* 30s beacon */

        g_sat.uptime_s++;
        vTaskDelayUntil(&last, pdMS_TO_TICKS(1000));
    }
}

/* low-battery protection: VBAT<6.6V -> SAFE mode, payload off */
void power_guard(void)
{
    if (g_sat.vbat_mv > 0 && g_sat.vbat_mv < 6600 && g_sat.mode != MODE_SAFE) {
        log_event(EVT_LOW_POWER, g_sat.vbat_mv);
        payload_power(false);    /* deassert PAYLOAD_EN */
        heater_force(false);
        obc_set_mode(MODE_SAFE);
    }
}
