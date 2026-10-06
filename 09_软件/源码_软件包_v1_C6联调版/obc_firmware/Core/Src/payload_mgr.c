/**
 * PANO-3U - payload manager: CM4 protocol interaction + capture scheduling
 */
#include "payload_mgr.h"
#include "main.h"
#include "link_proto.h"
#include "log.h"
#include "comm.h"
#include <string.h>

/* ---- Capture schedule table (max 32 entries, stored in RTC backup domain) ---- */
typedef struct {
    uint32_t epoch;      /* execution time */
    uint8_t  cmd;        /* 0x03=shoot 0x04=video 0x06=timelapse */
    uint16_t param1;     /* burst count / duration */
    uint16_t param2;     /* interval */
} SchedEntry_t;

static SchedEntry_t sched[32];
static int sched_count = 0;
static bool pl_busy = false;
static uint32_t pl_last_rsp = 0;

/* ---- Link: frame transmit ---- */
static void pl_send(uint8_t cmd, const uint8_t *pl, uint8_t len)
{
    uint8_t frame[280];
    uint16_t n = link_encode(frame, cmd, pl, len);
    HAL_UART_Transmit(&huart2, frame, n, 100);
}

/* ---- Frame receive callback (ISR -> decoder -> here) ---- */
static void on_pl_frame(uint8_t cmd, const uint8_t *pl, uint8_t len)
{
    pl_last_rsp = g_sat.uptime_s;
    switch (cmd) {
    case RSP_STATUS:
        if (len >= 12) {
            g_sat.pl_mode    = pl[0];
            g_sat.pl_err     = pl[len - 7];   /* last_err field offset */
            g_sat.pl_online  = true;
            if (pl[0] == 0) pl_busy = false;  /* IDLE */
        }
        break;
    case RSP_FPREP: {
        /* CM4 prepared file: enqueue for downlink */
        if (len >= 12) {
            uint32_t fid; uint16_t total;
            memcpy(&fid, &pl[0], 4); memcpy(&total, &pl[4], 2);
            comm_queue_file(fid, total);
        }
        break; }
    case RSP_ACK:
    default:
        break;
    }
}

void payload_mgr_init(void)
{
    payload_power(true);           /* power payload on by default */
    sched_count = 0;
}

/* 1Hz polling: heartbeat + schedule check */
void payload_mgr_step(void)
{
    static uint32_t last_ping = 0;
    if (g_sat.uptime_s - last_ping >= 1) {
        pl_send(CMD_PING, NULL, 0);
        last_ping = g_sat.uptime_s;
    }
    /* link health: 60s without response -> restart payload power */
    if (g_sat.uptime_s - pl_last_rsp > 60 && g_sat.mode == MODE_PAYLOAD) {
        payload_power(false);
        HAL_Delay(500);
        payload_power(true);
        log_event(EVT_ERROR, ERR_LINK_TIMEOUT);
        pl_last_rsp = g_sat.uptime_s;
    }
}

/* ---- Schedule dispatch ---- */
int schedule_add(uint32_t epoch, uint8_t cmd, uint16_t p1, uint16_t p2)
{
    if (sched_count >= 32) return -1;
    sched[sched_count] = (SchedEntry_t){epoch, cmd, p1, p2};
    sched_count++;
    return 0;
}

bool schedule_due(void)
{
    if (sched_count == 0 || pl_busy) return false;
    uint32_t now = rtc_epoch();
    for (int i = 0; i < sched_count; i++)
        if (sched[i].epoch <= now) return true;
    return false;
}

void payload_run_scheduled(void)
{
    uint32_t now = rtc_epoch();
    for (int i = 0; i < sched_count; i++) {
        if (sched[i].epoch <= now) {
            uint8_t pl[4];
            switch (sched[i].cmd) {
            case 0x03:  /* shoot */
                pl[0] = (uint8_t)sched[i].param1;
                pl[1] = sched[i].param2 & 0xFF; pl[2] = sched[i].param2 >> 8;
                pl_send(CMD_SHOOT, pl, 3);
                log_event(EVT_SHOOT, sched[i].param1);
                break;
            case 0x04:  /* video */
                pl[0] = sched[i].param1 & 0xFF; pl[1] = sched[i].param1 >> 8;
                pl[2] = 30;
                pl_send(CMD_VID_START, pl, 3);
                log_event(EVT_VIDEO, sched[i].param1);
                break;
            }
            /* remove executed entry */
            memmove(&sched[i], &sched[i + 1], (sched_count - i - 1) * sizeof(SchedEntry_t));
            sched_count--;
            i--;
            pl_busy = true;
        }
    }
}

bool payload_done(void) { return !pl_busy; }

/* for comm.c chunk reads: request from CM4 over UART */
uint16_t payload_read_chunk(uint32_t fid, uint16_t chunk_no, uint8_t *out)
{
    /* request-response: actually async, simplified here as sync wait (200ms timeout) */
    uint8_t req[6];
    memcpy(req, &fid, 4); memcpy(req + 4, &chunk_no, 2);
    pl_send(0x0B, req, 6);     /* CMD_FILE_CHUNK (protocol extension) */
    /* wait for RSP_CHUNK... actual implementation uses semaphore */
    return 0;   /* placeholder: full implementation in link_proto.c async state machine */
}

void task_payload(void *arg)
{
    for (;;) {
        payload_mgr_step();
        obc_fsm_step();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
