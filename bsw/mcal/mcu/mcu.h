/**
 * @file    mcu.h
 * @brief   MCU Driver — top-level include for S32K344 initialization
 *
 * This is the entry point for all MCU-level functionality:
 * clock configuration, power modes, cache control, and system init.
 */

#ifndef MCU_H
#define MCU_H

#include <stdint.h>
#include <stdbool.h>
#include "system_S32K344.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ─────────────────────────────────────────────────────────────
 * S32K344 memory map (key peripheral base addresses)
 * ───────────────────────────────────────────────────────────── */

/* AIPS-Lite peripheral slots */
#define AIPS0_BASE               0x40000000u
#define AIPS1_BASE               0x40200000u

/* SIUL2 (pin mux / GPIO) */
#define SIUL2_BASE               0x402D0000u

/* WKPU (wakeup unit) */
#define WKPU_BASE                0x402B0000u

/* MSCM (misc system control module) */
#define MSCM_BASE                0x40290000u

/* MC_CGM (clock generation module) */
#define MC_CGM_BASE              0x40274000u

/* MC_ME (mode entry module) */
#define MC_ME_BASE               0x40270000u

/* PLL */
#define PLL_BASE                 0x4027C000u

/* WDOG / SWT */
#define SWT0_BASE                0x402A0000u

/* STM */
#define STM0_BASE                0x402D8000u

/* ─────────────────────────────────────────────────────────────
 * Clock frequencies (after system clock init)
 * ───────────────────────────────────────────────────────────── */

#define SYS_CLK_FREQ_HZ          160000000u  /* CORE_CLK / AIPS_PLAT_CLK */
#define FIRC_FREQ_HZ             48000000u   /* Fast Internal RC */
#define SIRC_FREQ_HZ             32000u      /* Slow Internal RC */

/* ─────────────────────────────────────────────────────────────
 * Public API
 * ───────────────────────────────────────────────────────────── */

/**
 * @brief  Initialize the MCU subsystem
 *
 * Performs complete early initialization:
 *   1. Disable watchdog (SWT0)
 *   2. Enable FPU (CP10/CP11 full access)
 *   3. Enable I-Cache and D-Cache
 *   4. Initialize clock tree (FIRC → PLL → 160MHz)
 *   5. Enable MSCM clock (required before any RTD/peripheral use)
 */
void Mcu_Init(void);

/**
 * @brief  Get the current core clock frequency in Hz
 */
static inline uint32_t Mcu_GetCoreClockHz(void)
{
    return SystemCoreClock;
}

/**
 * @brief  Delay for approximately the given number of microseconds
 * @note   Uses a simple busy-wait loop; not cycle-accurate.
 */
void Mcu_DelayUs(uint32_t us);

/**
 * @brief  Trigger a system reset
 */
void Mcu_Reset(void) __attribute__((noreturn));

#ifdef __cplusplus
}
#endif

#endif /* MCU_H */
