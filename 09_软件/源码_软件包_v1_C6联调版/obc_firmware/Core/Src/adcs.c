/**
 * PANO-3U - attitude control (magnetic-only)
 * Strategy:
 *   Phase 1 DETUMBLE: B-dot detumble, target |ω| < 2 deg/s
 *   Phase 2 IDLE: keep slow spin 1~2 deg/s (panorama mission does not require precise pointing)
 * Actuator: 3-axis magnetorquer (DRV8837 PWM drive, max magnetic dipole 0.2 Am²/axis)
 * Sensors: QMC5883L magnetometer (primary), MPU9250 gyroscope (secondary)
 */
#include "adcs.h"
#include "main.h"
#include "qmc5883l.h"
#include "mpu9250.h"
#include "drv8837.h"

#define BDOT_GAIN      0.5f     /* dipole gain Am² per (uT/s) */
#define MAG_DIP_MAX    0.2f     /* per-axis max dipole Am² */
#define DETUMBLE_TGT   2.0f     /* target angular rate deg/s */
#define DT             0.1f     /* control period 10Hz */

static adcs_state_t state = ADCS_INIT;
static float mtq_duty[3] = {0, 0, 0};

void adcs_init(void)
{
    qmc5883l_init();
    mpu9250_init();
    drv8837_init();
    state = ADCS_DETUMBLE;
}

/* sensor readout (called by task_hk at 1Hz) */
void adcs_read_sensors(void)
{
    qmc5883l_read(g_sat.mag);
    mpu9250_read_gyro(g_sat.gyro);
}

/**
 * B-dot algorithm: m = -K * dB/dt
 * Physical meaning: produce dipole opposing magnetic-field rate change to dissipate rotational energy
 */
static void bdot_step(void)
{
    float db[3];
    for (int i = 0; i < 3; i++) {
        db[i] = (g_sat.mag[i] - g_sat.mag_prev[i]) / DT;
        g_sat.mag_prev[i] = g_sat.mag[i];
        /* dipole command = -K * dB/dt, with saturation */
        float m = -BDOT_GAIN * db[i];
        if (m >  MAG_DIP_MAX) m =  MAG_DIP_MAX;
        if (m < -MAG_DIP_MAX) m = -MAG_DIP_MAX;
        mtq_duty[i] = m / MAG_DIP_MAX;   /* -1..+1, sign indicates current direction */
    }
}

/* detumble completion check: all three angular rates below threshold */
static bool detumbled(void)
{
    for (int i = 0; i < 3; i++)
        if (g_sat.gyro[i] > DETUMBLE_TGT || g_sat.gyro[i] < -DETUMBLE_TGT)
            return false;
    return true;
}

void task_adcs(void *arg)
{
    TickType_t last = xTaskGetTickCount();
    for (;;) {
        switch (state) {
        case ADCS_DETUMBLE:
            bdot_step();
            if (detumbled()) {
                state = ADCS_MISSION;    /* enter mission state: magnetorquer only compensates disturbances */
                for (int i = 0; i < 3; i++) mtq_duty[i] = 0;
            }
            break;
        case ADCS_MISSION:
            /* mission state: weak B-dot keeps slow spin to prevent disturbance accumulation */
            bdot_step();
            for (int i = 0; i < 3; i++) mtq_duty[i] *= 0.3f;
            break;
        default:
            state = ADCS_DETUMBLE;
            break;
        }
        /* output to DRV8837: duty<0 means reverse */
        for (int i = 0; i < 3; i++)
            drv8837_set(i, mtq_duty[i]);
        vTaskDelayUntil(&last, pdMS_TO_TICKS((TickType_t)(DT * 1000)));
    }
}

adcs_state_t adcs_get_state(void) { return state; }
