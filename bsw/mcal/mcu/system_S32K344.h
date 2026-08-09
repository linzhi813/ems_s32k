/**
 * @file    system_S32K344.h
 * @brief   System initialization for NXP S32K344 — clock tree, cache, FPU
 *
 * Called by startup code before main().
 * Configures FIRC → PLL_PHI0 → 160MHz core / platform clock.
 */

#ifndef SYSTEM_S32K344_H
#define SYSTEM_S32K344_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  System clock initialization
 *
 * Sequence:
 *   1. Enable MSCM clock gating (required first on S32K3)
 *   2. Configure PLL for 160MHz (FIRC 48MHz / 3 → × 20 → / 2 = 160MHz)
 *   3. Switch system clock to PLL
 *   4. Update SystemCoreClock variable
 */
void SystemInit(void);

/**
 * @brief  Update SystemCoreClock variable after clock changes
 */
void SystemCoreClockUpdate(void);

/** Current system (core/platform) clock frequency in Hz */
extern uint32_t SystemCoreClock;

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_S32K344_H */
