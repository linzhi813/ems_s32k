/**
 * @file    main.c
 * @brief   Application entry point — Engine Management System ECU
 *
 * This is the main entry point called by the startup code after
 * hardware initialization completes.
 *
 * Current status: UART console demo — every 10 ms sends the value of
 * g_counter_10ms to LPUART1 (PTC6 RX / PTC7 TX, 115200 8N1); the
 * commands "disable"/"enable" stop/resume the stream.  As APP modules
 * are developed (control, FaultManager, vios), their init functions
 * will be called from here.
 */

#include "mcu.h"
#include "Uart.h"
#include "Dio.h"
#include "Vios.h"
#include <stdio.h>
#include <string.h>

/* ── Cortex-M7 SysTick register definitions ── */
#define SYST_CSR        (*(volatile uint32_t *)0xE000E010u)
#define SYST_RVR        (*(volatile uint32_t *)0xE000E014u)
#define SYST_CVR        (*(volatile uint32_t *)0xE000E018u)

#define SYST_CSR_ENABLE     (1u << 0)
#define SYST_CSR_TICKINT    (1u << 1)
#define SYST_CSR_CLKSOURCE  (1u << 2)

/* ── 10 ms tick counter ── */
static volatile uint32_t g_counter_10ms;
static volatile uint8_t  g_flag_10ms;

/* ── UART console (LPUART1, PTC6 = RX / PTC7 = TX, 115200 8N1) ── */
static Uart_ConfigType g_uartConfig = { .baudRate = 115200u };
static bool            g_uartTxEnabled = true;   /* "disable"/"enable" commands */

/* ── Button/LED demo (SW1 on PTA7, LED3 on PTB14) ── */
static uint32_t g_ledPeriodMs;   /* last LED blink period reported by Vios */

/* ── RX command line buffer ── */
#define UART_CMD_LINE_MAX   16u
static char    g_rxLine[UART_CMD_LINE_MAX];
static uint8_t g_rxLineLen;
static bool    g_rxLineOverflow;   /* line longer than buffer — ignore it */

/**
 * @brief  Feed one received byte into the command parser
 *
 * Commands are recognized strictly on a whole-line basis: when CR or
 * LF arrives, the accumulated line must be EXACTLY "disable" or
 * "enable" to take effect.  A line that merely contains these words
 * (e.g. "disabled", "xxdisable") does nothing.
 */
static void Uart_ProcessRxByte(uint8_t ch)
{
    if ((ch == '\r') || (ch == '\n')) {
        /* Line complete — the entire line must equal a command word */
        if (!g_rxLineOverflow) {
            if (strcmp(g_rxLine, "disable") == 0) {
                g_uartTxEnabled = false;
                Uart_WriteString("TX disabled\r\n");
            } else if (strcmp(g_rxLine, "enable") == 0) {
                g_uartTxEnabled = true;
                Uart_WriteString("TX enabled\r\n");
            }
        }
        g_rxLineLen = 0u;
        g_rxLineOverflow = false;
        g_rxLine[0] = '\0';
        return;
    }

    if (g_rxLineOverflow) {
        return;                         /* keep ignoring until line end */
    }
    if (g_rxLineLen >= (UART_CMD_LINE_MAX - 1u)) {
        g_rxLineOverflow = true;        /* overlong line — ignore it */
        return;
    }
    g_rxLine[g_rxLineLen++] = (char)ch;
    g_rxLine[g_rxLineLen]   = '\0';
}

/**
 * @brief  SysTick interrupt handler — set 10 ms flag for main loop
 *
 * Overrides the weak SysTick_Handler in startup code.
 */
void SysTick_Handler(void)
{
    g_flag_10ms = 1u;
}

/**
 * @brief  Main application entry point.
 *
 * Hardware is fully initialized by the time we reach here:
 *   - FPU enabled (full access CP10/CP11)
 *   - I-Cache and D-Cache enabled
 *   - Clock tree: PLL @ 160 MHz (CORE_CLK / AIPS_PLAT_CLK)
 *   - .data copied, .bss zeroed
 *
 * @return  Never returns (bare-metal infinite loop)
 */
int main(void)
{
    /* ── Variable storage space for startup verification ── */
    volatile uint32_t sys_clk = Mcu_GetCoreClockHz();
    (void)sys_clk;  /* Available for debugger inspection */

    /* ── UART console: LPUART1 @ PTC6(RX)/PTC7(TX), 115200 8N1 ── */
    Uart_Init(&g_uartConfig);
    Uart_WriteString("EMS S32K UART console ready\r\n");

    /* ── Button/LED demo: SW1 on PTA7, LED3 on PTB14 ── */
    Vios_Init();
    g_ledPeriodMs = VIOS_LED_PERIOD_500MS;
    Uart_WriteString("Button/LED demo: LED blinks 500 ms; press SW1 (PTA7) to switch 500 ms <-> 2 s\r\n");
    if (Dio_ReadChannel(DIO_CH_BUTTON_SW1) == STD_LOW) {
        Uart_WriteString("BTN idle = LOW (OK)\r\n");
    } else {
        Uart_WriteString("BTN idle = HIGH (pressed? wiring check needed)\r\n");
    }

    /* ── Configure SysTick for 10 ms interval ──
     * Reload = (CORE_CLK / 100) - 1
     * CORE_CLK is FIRC 48 MHz until MCU driver switches to PLL.
     */
    SYST_RVR = Mcu_GetCoreClockHz() / 100u - 1u;
    SYST_CVR = 0u;
    SYST_CSR = SYST_CSR_ENABLE | SYST_CSR_TICKINT | SYST_CSR_CLKSOURCE;

    /* ── Main control loop ── */
    for (;;) {
        /*
         * TODO: Add application tasks as they are implemented:
         *
         *   vios_Init();          // Initialize sensor/actuator I/O
         *   control_Init();       // Initialize engine control algorithms
         *   faultMgr_Init();      // Initialize diagnostic fault manager
         *
         *   for (;;) {
         *       vios_ReadSensors();        // Acquire all sensor data
         *       control_Run();             // Run control algorithms
         *       faultMgr_Monitor();        // Check fault conditions
         *       vios_WriteActuators();     // Update actuator outputs
         *   }
         */

        if (g_flag_10ms) {
            g_flag_10ms = 0u;
            g_counter_10ms++;

            /* ── Button debounce + LED blink task ── */
            {
                uint32_t period = Vios_Main10ms();
                if (period != g_ledPeriodMs) {
                    g_ledPeriodMs = period;
                    char line[32];
                    int  len = snprintf(line, sizeof(line),
                                        "SW1 pressed -> LED blink %lu ms\r\n",
                                        (unsigned long)period);
                    for (int i = 0; i < len; i++) {
                        Uart_WriteByte((uint8_t)line[i]);
                    }
                }
            }

            if (g_uartTxEnabled) {
                char line[32];
                int  len = snprintf(line, sizeof(line),
                                    "g_counter_10ms = %lu\r\n",
                                    (unsigned long)g_counter_10ms);
                for (int i = 0; i < len; i++) {
                    Uart_WriteByte((uint8_t)line[i]);
                }
            }
        }

        /* ── UART receive: "disable" stops the 10 ms stream,
         *    "enable" resumes it ── */
        {
            uint8_t ch;
            while (Uart_ReadByte(&ch)) {
                Uart_ProcessRxByte(ch);
            }
        }

        /* Idle until the next interrupt (low-power).  The 10 ms
         * SysTick wakes the core; busy-waiting instead would spin
         * the whole loop within a single source line, which makes
         * GDB "step" never complete (it steps until the line
         * number changes). */
        Mcu_WaitForInterrupt();
    }
}
