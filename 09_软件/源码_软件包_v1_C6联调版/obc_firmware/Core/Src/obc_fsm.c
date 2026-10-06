/**
 * PANO-3U - operating mode state machine
 */
#include "obc_fsm.h"
#include "main.h"
#include "payload_mgr.h"
#include "comm.h"
#include "log.h"

static SatMode_t mode_prev = MODE_SAFE;

void obc_set_mode(SatMode_t m)
{
    if (m == g_sat.mode) return;
    mode_prev = g_sat.mode;
    g_sat.mode = m;
    log_event(EVT_MODE_CHANGE, (uint32_t)m);
    /* mode entry actions */
    switch (m) {
    case MODE_SAFE:
        payload_power(false);
        break;
    case MODE_DOWNLINK:
        comm_downlink_start();
        break;
    case MODE_PAYLOAD:
        payload_power(true);
        break;
    default: break;
    }
}

void obc_fsm_init(void)
{
    /* first boot: detumble first */
    obc_set_mode(MODE_DETUMBLE);
}

/**
 * main state-machine loop (1Hz call from task_hk or standalone task)
 * transition logic:
 *   DETUMBLE --(detumble complete)--> IDLE
 *   IDLE --(scheduled entry due)--> PAYLOAD
 *   IDLE --(pass window)--> DOWNLINK
 *   PAYLOAD --(done)--> IDLE
 *   any --(low power/fault)--> SAFE --(ground command)--> IDLE
 */
void obc_fsm_step(void)
{
    switch (g_sat.mode) {
    case MODE_DETUMBLE:
        if (adcs_get_state() == ADCS_MISSION)
            obc_set_mode(MODE_IDLE);
        break;
    case MODE_IDLE:
        if (schedule_due()) {                     /* scheduled capture is due */
            obc_set_mode(MODE_PAYLOAD);
            payload_run_scheduled();
        } else if (comm_pass_predicted()) {       /* pass window */
            obc_set_mode(MODE_DOWNLINK);
        }
        break;
    case MODE_PAYLOAD:
        if (payload_done())
            obc_set_mode(MODE_IDLE);
        break;
    case MODE_DOWNLINK:
        if (comm_downlink_done())
            obc_set_mode(MODE_IDLE);
        break;
    case MODE_SAFE:
        /* recover only by ground command or by timeout after power recovers */
        if (g_sat.vbat_mv > 7000 && g_sat.uptime_s > 600)
            obc_set_mode(MODE_IDLE);
        break;
    default:
        obc_set_mode(MODE_SAFE);
    }
}
