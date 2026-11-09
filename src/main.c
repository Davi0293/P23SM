/*
 * ps2_xmb_combo.sprx - VSH plugin for PS3 HEN + webMAN MOD
 *
 * Hold START + SELECT on a pad (e.g. a PS2 controller behind a USB adapter,
 * which has no PS button) to open the in-game XMB menu, as if the PS button
 * had been pressed.
 *
 * How the PS button is sent: webMAN MOD has a virtual pad. This plugin asks
 * it to press PS by writing the web command "/pad.ps3?ps" into
 * /dev_hdd0/tmp/wm_request (webMAN's file-polling command interface).
 * No sockets are needed, so the plugin stays small and safe to load in VSH.
 *
 * NOTE: webMAN MOD also uses SELECT+START (play time) and SELECT (VSH menu)
 * as pad shortcuts. Disable those in /setup.ps3 (XMB/In-Game PAD SHORTCUTS)
 * or they will fire together with this combo.
 */

#include <stdint.h>
#include <string.h>

#include <io/pad.h>
#include <sys/thread.h>
#include <sys/file.h>

/* ---- configuration ---------------------------------------------------- */
#define POLL_INTERVAL_US   50000      /* 50 ms between pad polls            */
#define HOLD_TIME_US       400000     /* combo must be held this long       */
#define COOLDOWN_US        1500000    /* ignore the combo after a trigger   */
#define BOOT_DELAY_US      20000000   /* let VSH/webMAN finish booting      */
#define MAX_PORTS          7

/* Digital buttons, first byte group (CELL_PAD_BTN_OFFSET_DIGITAL1 = index 2) */
#define BTN_SELECT_MASK    0x0001
#define BTN_START_MASK     0x0008
#define DIGITAL1_INDEX     2

#define WM_REQUEST_FILE    "/dev_hdd0/tmp/wm_request"
#define WM_PS_COMMAND      "/pad.ps3?ps"

/* ---- state ------------------------------------------------------------ */
static sys_ppu_thread_t g_thread;
static volatile int     g_running = 0;

/* ---- helpers ---------------------------------------------------------- */
static int send_ps_button(void)
{
    int fd = -1;
    u64 written = 0;
    const char cmd[] = WM_PS_COMMAND;

    if (sysFsOpen(WM_REQUEST_FILE, SYS_O_WRONLY | SYS_O_CREAT | SYS_O_TRUNC,
                  &fd, NULL, 0) != 0)
        return -1;

    sysFsWrite(fd, cmd, sizeof(cmd) - 1, &written);
    sysFsClose(fd);
    return 0;
}

static int combo_pressed(void)
{
    padInfo2 info;
    padData  data;
    int i;

    memset(&info, 0, sizeof(info));
    if (ioPadGetInfo2(&info) != 0)
        return 0;

    for (i = 0; i < MAX_PORTS; i++) {
        if (!info.port_status[i])
            continue;

        memset(&data, 0, sizeof(data));
        if (ioPadGetData(i, &data) != 0 || data.len <= DIGITAL1_INDEX)
            continue;

        /* Mask both bits and require both to be set on the same pad */
        {
            uint16_t b = data.button[DIGITAL1_INDEX];
            if ((b & BTN_SELECT_MASK) && (b & BTN_START_MASK))
                return 1;
        }
    }
    return 0;
}

static void poll_thread(void *arg)
{
    int held_us = 0;
    (void)arg;

    sysUsleep(BOOT_DELAY_US);
    ioPadInit(MAX_PORTS);   /* harmless if VSH already initialised it */

    while (g_running) {
        if (combo_pressed()) {
            held_us += POLL_INTERVAL_US;
            if (held_us >= HOLD_TIME_US) {
                send_ps_button();
                held_us = 0;
                sysUsleep(COOLDOWN_US);
            }
        } else {
            held_us = 0;
        }
        sysUsleep(POLL_INTERVAL_US);
    }

    sysThreadExit(0);
}

/* ---- PRX entry points ------------------------------------------------- */
int module_start(size_t args, void *argp)
{
    (void)args; (void)argp;
    g_running = 1;
    sysThreadCreate(&g_thread, poll_thread, NULL, 3000, 0x4000,
                    THREAD_JOINABLE, "ps2_xmb_combo");
    return 0;
}

int module_stop(size_t args, void *argp)
{
    u64 retval;
    (void)args; (void)argp;
    g_running = 0;
    sysThreadJoin(g_thread, &retval);
    return 0;
}
