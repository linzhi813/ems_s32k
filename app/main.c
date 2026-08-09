/**
 * @file    main.c
 * @brief   Application entry point — Engine Management System ECU
 *
 * This is the main entry point called by the startup code after
 * hardware initialization completes.
 *
 * Current status: minimal skeleton.  As APP modules are developed
 * (control, FaultManager, vios), their init functions will be
 * called from here.
 */

#include "mcu.h"

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

    /* ── Configure SysTick for 10 ms interval ──
     * CORE_CLK = 160 MHz → 10 ms = 1,600,000 cycles
     * Reload = 1,600,000 - 1 = 1,599,999
     */
    SYST_RVR = 1599999u;
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
        }
    }
}
