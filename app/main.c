/**
 * @file    main.c
 * @brief   Application entry point — Engine Management System ECU
 *
 * This is the main entry point called by the startup code after
 * hardware initialization completes.
 *
 * Current status: FreeRTOS console demo —
 *   - Task_UartConsole (10 ms): increments g_counter_10ms and sends it
 *     to LPUART1 (PTC6 RX / PTC7 TX, 115200 8N1); the commands
 *     "disable"/"enable" stop/resume the stream.
 *   - Task_ButtonLed (50 ms): SW1 debounce + LED3 blink (500 ms ↔ 2 s).
 *   - SysTick (1 ms, 48 MHz FIRC) is the FreeRTOS tick source.
 * As APP modules are developed (control, FaultManager, vios), their
 * tasks will be created from here.
 */

#include "mcu.h"
#include "Uart.h"
#include "Dio.h"
#include "Vios.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <string.h>

/* ── 10 ms tick counter (single writer: Task_UartConsole) ── */
static uint32_t g_counter_10ms;

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
 * @brief  Send a whole line atomically with respect to task preemption
 *
 * The 10 ms task (priority 2) can preempt the 50 ms task (priority 1)
 * in the middle of a multi-byte write and interleave its own line.
 * Suspending the scheduler for the duration of the write keeps lines
 * intact.  (taskENTER_CRITICAL is deliberately avoided — it would also
 * mask the UART RX ISR and SysTick for ~2 ms per line.)
 */
static void Console_WriteLine(const char *line)
{
    vTaskSuspendAll();
    Uart_WriteString(line);
    (void)xTaskResumeAll();
}

/**
 * @brief  10 ms task — counter accumulation + UART TX/RX communication
 *
 * vTaskDelayUntil keeps the period jitter-free even though the
 * blocking TX (~2 ms per line) shifts execution within each period.
 */
static void Task_UartConsole(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    (void)pvParameters;

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));

        g_counter_10ms++;

        if (g_uartTxEnabled) {
            char line[32];
            int  len = snprintf(line, sizeof(line),
                                "g_counter_10ms = %lu\r\n",
                                (unsigned long)g_counter_10ms);
            for (int i = 0; i < len; i++) {
                Uart_WriteByte((uint8_t)line[i]);
            }
        }

        /* ── UART receive: "disable" stops the 10 ms stream,
         *    "enable" resumes it (same task → no race) ── */
        uint8_t ch;
        while (Uart_ReadByte(&ch)) {
            Uart_ProcessRxByte(ch);
        }
    }
}

/**
 * @brief  50 ms task — SW1 button debounce + LED blink
 */
static void Task_ButtonLed(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    (void)pvParameters;

    for (;;) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));

        uint32_t period = Vios_Main50ms();
        if (period != g_ledPeriodMs) {
            g_ledPeriodMs = period;
            char line[48];
            int  len = snprintf(line, sizeof(line),
                                "SW1 pressed -> LED blink %lu ms\r\n",
                                (unsigned long)period);
            if (len >= (int)sizeof(line)) {
                len = (int)sizeof(line) - 1u;   /* truncation — keep NUL */
            }
            line[len] = '\0';
            Console_WriteLine(line);
        }
    }
}

/* ── FreeRTOS hooks ── */

/**
 * @brief  Stack overflow hook (configCHECK_FOR_STACK_OVERFLOW = 2)
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    Uart_WriteString("STACK OVERFLOW: ");
    Uart_WriteString(pcTaskName);
    Uart_WriteString("\r\n");
    for (;;) {
    }
}

/**
 * @brief  Main application entry point.
 *
 * Hardware is fully initialized by the time we reach here:
 *   - FPU enabled (full access CP10/CP11)
 *   - I-Cache enabled (D-Cache intentionally off)
 *   - Clock tree: FIRC @ 48 MHz
 *   - .data copied, .bss zeroed
 *
 * @return  Never returns
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

    /* ── Create the two application tasks ── */
    if (xTaskCreate(Task_UartConsole, "uart10ms",
                    configMINIMAL_STACK_SIZE, NULL, 2, NULL) != pdPASS) {
        Uart_WriteString("FATAL: uart task create failed\r\n");
        for (;;) {
        }
    }
    if (xTaskCreate(Task_ButtonLed, "btn50ms",
                    configMINIMAL_STACK_SIZE, NULL, 1, NULL) != pdPASS) {
        Uart_WriteString("FATAL: button/LED task create failed\r\n");
        for (;;) {
        }
    }

    /* ── Hand control to the kernel; never returns ── */
    vTaskStartScheduler();

    for (;;) {   /* unreachable */
    }
}
