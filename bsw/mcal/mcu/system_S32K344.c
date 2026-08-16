/**
 * @file    system_S32K344.c
 * @brief   System initialization for NXP S32K344
 *
 * Startup-time initialization only.  Clock tree strategy:
 *
 *   Boot default: FIRC 48 MHz (CORE_CLK = AIPS_PLAT_CLK = FIRC)
 *   SystemInit() does NOT switch the PLL — this matches the official
 *   NXP startup flow, where the clock tree is initialized later by
 *   the MCU driver (RTD Clock_Ip_Init) from main().
 *
 * Startup code (startup_s32k344.S) already handles, before calling
 * SystemInit():
 *   1. MSCM clock enable via MC_ME
 *   2. SWT0 watchdog disable
 *   3. TCM enable
 *   4. .data copy / .bss zero
 *   5. FPU enable
 *
 * Register documentation: S32K3XXRM (Rev.5) chapters:
 *   - 7  (MSCM)      base 0x40260000
 *   - 45 (MC_ME)     base 0x402DC000
 *   - 29 (PLL)       base 0x402E0000
 *   - 51 (MC_CGM)    base 0x402D8000
 */

#include "system_S32K344.h"
#include <stdint.h>

/* ═══════════════════════════════════════════════════════════════
 * System core clock variable (referenced by RTD and application)
 * ═══════════════════════════════════════════════════════════════ */
uint32_t SystemCoreClock = 48000000u;   /* FIRC 48 MHz — boot default */

/* ═══════════════════════════════════════════════════════════════
 * MSCM register offsets (base 0x40260000)
 * IRSPRC[240] at offset 880h — 16-bit array, step 2 bytes
 * (official RTD header: S32K311_MSCM.h, __IO uint16_t IRSPRC[240])
 * ═══════════════════════════════════════════════════════════════ */
#define MSCM_BASE                0x40260000u
#define MSCM_IRSPRC_OFFSET       0x880u  /* Interrupt steering array   */
#define MSCM_IRSPRC_COUNT        240u    /* IRSPRC0..IRSPRC239         */
#define MSCM_IRSPRC_M7_0_SHIFT   0u

/* ═══════════════════════════════════════════════════════════════
 * SCB (System Control Block, Cortex-M7) — cache control
 * ═══════════════════════════════════════════════════════════════ */
#define SCB_BASE                 0xE000E000u
#define SCB_ICACHE_CTRL          (*(volatile uint32_t *)(SCB_BASE + 0x010))
#define SCB_DCACHE_CTRL          (*(volatile uint32_t *)(SCB_BASE + 0x014))


/* ═══════════════════════════════════════════════════════════════
 * SystemInit
 *
 * Called by Reset_Handler before main().  Minimal, safe sequence:
 *   1. Route all 240 interrupt steering registers to CM7_0
 *      (mirrors official NXP SystemInit — MSCM.IRSPRC[i] |= coreMask)
 *   2. Enable I-Cache only
 *
 * D-Cache is intentionally left DISABLED during bring-up:
 *   - Debugger AHB reads bypass the D-cache, so Variables/Watch show
 *     stale values while the CPU executes from cache — confusing in
 *     the debugger (variables appear "frozen"/"run away").
 *   - Without MPU region configuration, cacheability attributes are
 *     defaulted by the core; enabling write-back D-cache over
 *     peripheral/ECC RAM requires careful cache maintenance.
 *   Re-enable together with MPU setup in the MCU driver phase.
 *
 *   3. SystemCoreClock stays 48 MHz (FIRC) until MCU driver init
 * ═══════════════════════════════════════════════════════════════ */
void SystemInit(void)
{
    /* IRSPRC is a 16-bit register array (step 2 bytes) */
    volatile uint16_t *irspc = (volatile uint16_t *)(MSCM_BASE + MSCM_IRSPRC_OFFSET);

    /* ── Step 1: Route interrupts to CM7_0 ── */
    for (uint32_t i = 0; i < MSCM_IRSPRC_COUNT; i++) {
        irspc[i] |= (uint16_t)(1u << MSCM_IRSPRC_M7_0_SHIFT);
    }

    /* ── Step 2: Enable I-Cache (D-Cache stays off — see header) ── */
    SCB_ICACHE_CTRL |= 1u;
    __asm__ volatile ("dsb; isb");
}


/* ═══════════════════════════════════════════════════════════════
 * SystemCoreClockUpdate
 *
 * Called after any runtime clock change to keep SystemCoreClock
 * synchronized with actual hardware state.
 * ═══════════════════════════════════════════════════════════════ */
void SystemCoreClockUpdate(void)
{
    /* Boot default is FIRC until PLL switch is implemented in the
     * MCU driver.  Re-evaluate when Clock_Ip_Init is integrated. */
    SystemCoreClock = 48000000u;
}
