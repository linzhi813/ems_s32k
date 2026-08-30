/**
 * @file    Vios.h
 * @brief   Vehicle I/O System — button detection & LED indicator
 *
 * First VIOS modules on the core board:
 *   - Button SW1 (PTA7):  polled input with 60 ms debounce (≥ 50 ms
 *     requirement), one press event per complete press/release cycle
 *   - LED3 (PTB14):       blink indicator, period toggles between
 *     500 ms and 2 s on each confirmed button press
 *
 * Vios_Main10ms() must be called every 10 ms from the main loop
 * (driven by the SysTick 10 ms flag).
 */

#ifndef VIOS_H
#define VIOS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ─────────────────────────────────────────────────────────────
 * LED blink periods
 * ───────────────────────────────────────────────────────────── */

#define VIOS_LED_PERIOD_500MS    500u   /* power-up default        */
#define VIOS_LED_PERIOD_2000MS   2000u  /* after one button press  */

/**
 * @brief  Initialize button and LED I/O (Dio pin config, LED on)
 */
void Vios_Init(void);

/**
 * @brief  Main 10 ms task — debounce button, blink LED
 *
 * - Samples the button and runs the debounce state machine
 *   (press and release edges both filtered, 6 × 10 ms = 60 ms).
 * - A confirmed press toggles the LED blink period 100 ms ↔ 1 s.
 * - Advances the LED blink timer; toggles the LED at each half
 *   period.
 *
 * @return  Current LED blink period in ms (100 or 1000), so the
 *          caller can detect a mode change.
 */
uint32_t Vios_Main10ms(void);

#ifdef __cplusplus
}
#endif

#endif /* VIOS_H */
