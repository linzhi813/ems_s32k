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
 *
 * Verified against S32K3xxRM Rev.5 memory map chapters:
 *   MSCM    0x40260000 (ch.7)      MC_RGM  0x4028C000
 *   SWT_0   0x40270000 (ch.65)     SIUL2   0x40290000
 *   STM_0   0x40274000             WKPU    0x402B4000 (ch.47)
 *   MC_ME   0x402DC000 (ch.45)     MC_CGM  0x402D8000
 *   PLL     0x402E0000 (ch.29)     eMIOS_0 0x40088000 (ch.62)
 * ───────────────────────────────────────────────────────────── */

/* MSCM (misc system control module) */
#define MSCM_BASE                0x40260000u

/* SWT_0 (internal software watchdog 0) */
#define SWT0_BASE                0x40270000u

/* STM_0 (system timer module 0) */
#define STM0_BASE                0x40274000u

/* MC_RGM (reset generation module) */
#define MC_RGM_BASE              0x4028C000u

/* SIUL2 (pin mux / GPIO) */
#define SIUL2_BASE               0x40290000u

/* WKPU (wakeup unit) */
#define WKPU_BASE                0x402B4000u

/* MC_ME (mode entry module) */
#define MC_ME_BASE               0x402DC000u

/* MC_CGM (clock generation module) */
#define MC_CGM_BASE              0x402D8000u

/* PLL */
#define PLL_BASE                 0x402E0000u

/* eMIOS_0 */
#define EMIOS0_BASE              0x40088000u

/* ─────────────────────────────────────────────────────────────
 * Clock frequencies
 * ───────────────────────────────────────────────────────────── */

#define FIRC_FREQ_HZ             48000000u   /* Fast Internal RC (boot default) */
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

/**
 * @brief  Wait for interrupt — low-power idle
 *
 * Executes WFI: the core sleeps until the next interrupt.
 * Use in the main idle loop instead of busy-wait polling.
 */
static inline void Mcu_WaitForInterrupt(void)
{
    __asm__ volatile ("wfi");
}

#ifdef __cplusplus
}
#endif

#endif /* MCU_H */
