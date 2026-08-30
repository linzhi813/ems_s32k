/**
 * @file    Vios.c
 * @brief   Vehicle I/O System — button debounce & LED blink logic
 *
 * Hardware (core board V1.0):
 *   - SW1 user button on PTA7 — externally pulled up to 5 V,
 *     internal pull-down enabled by Dio.  Pressed = pad HIGH.
 *   - LED3 green on PTB14 — PTB14 — LED3 — R7 2.2k — GND;
 *     pad HIGH sources current → LED on (active-high).
 *
 * Timing:
 *   - Called every 10 ms from the SysTick-driven main loop.
 *   - Debounce: 6 consecutive equal samples = 60 ms (≥ 50 ms) on
 *     BOTH edges — press must be stable before the event fires, and
 *     the release must be stable before the next press is accepted.
 *     One press → one event, contact bounce cannot multi-trigger.
 *   - Blink: toggle LED at half period — 500 ms mode toggles every
 *     250 ms (25 ticks), 2 s mode every 1 s (100 ticks).
 */

#include "Vios.h"
#include "Dio.h"
#include <stddef.h>       /* NULL */

/* ── Active levels (core board wiring) ── */
#define BTN_PRESSED_LEVEL   STD_HIGH    /* pressed → 5 V on pad       */
#define LED_ON_LEVEL        STD_HIGH    /* pad sources current → on   */

/* ── Debounce ── */
#define BTN_DEBOUNCE_SAMPLES  6u       /* 6 × 10 ms = 60 ms ≥ 50 ms   */

/* ── Button debounce state machine ── */
typedef enum
{
    BTN_RELEASED,          /* idle — no press in progress            */
    BTN_PRESS_PENDING,     /* level low seen, still confirming       */
    BTN_PRESSED,           /* press confirmed, event pending         */
    BTN_RELEASE_PENDING,   /* level high seen, confirming release    */
} BtnStateType;

static BtnStateType s_btnState;
static uint8_t      s_btnSampleCnt;
static bool         s_pressEvent;

/* ── LED blink ── */
static uint32_t s_ledPeriodMs;      /* 100 (power-up) or 1000        */
static uint32_t s_ledBlinkTick;     /* 10 ms ticks since last toggle */
static bool     s_ledOn;

void Vios_Init(void)
{
    Dio_Init(NULL);

    s_btnState     = BTN_RELEASED;
    s_btnSampleCnt = 0u;
    s_pressEvent   = false;

    s_ledPeriodMs  = VIOS_LED_PERIOD_500MS;
    s_ledBlinkTick = 0u;
    s_ledOn        = true;                 /* start with LED on      */
    Dio_WriteChannel(DIO_CH_LED1, LED_ON_LEVEL);
}

uint32_t Vios_Main10ms(void)
{
    bool pressed = (Dio_ReadChannel(DIO_CH_BUTTON_SW1) == BTN_PRESSED_LEVEL);

    /* ── Button debounce state machine ── */
    switch (s_btnState) {

    case BTN_RELEASED:
        if (pressed) {
            s_btnState     = BTN_PRESS_PENDING;
            s_btnSampleCnt = 1u;
        }
        break;

    case BTN_PRESS_PENDING:
        if (pressed) {
            if (++s_btnSampleCnt >= BTN_DEBOUNCE_SAMPLES) {
                s_btnState   = BTN_PRESSED;
                s_pressEvent = true;      /* one event per press    */
            }
        } else {
            s_btnState = BTN_RELEASED;    /* bounce — restart       */
        }
        break;

    case BTN_PRESSED:
        if (!pressed) {
            s_btnState     = BTN_RELEASE_PENDING;
            s_btnSampleCnt = 1u;
        }
        break;

    case BTN_RELEASE_PENDING:
        if (!pressed) {
            if (++s_btnSampleCnt >= BTN_DEBOUNCE_SAMPLES) {
                s_btnState = BTN_RELEASED;
            }
        } else {
            s_btnState = BTN_PRESSED;     /* bounce — still held    */
        }
        break;
    }

    /* ── Confirmed press → toggle blink period ── */
    if (s_pressEvent) {
        s_pressEvent = false;
        s_ledPeriodMs = (s_ledPeriodMs == VIOS_LED_PERIOD_500MS)
                        ? VIOS_LED_PERIOD_2000MS
                        : VIOS_LED_PERIOD_500MS;
        s_ledBlinkTick = 0u;              /* restart blink phase    */
    }

    /* ── LED blink: toggle at half period ── */
    s_ledBlinkTick++;
    if (s_ledBlinkTick >= (s_ledPeriodMs / 20u)) {
        s_ledBlinkTick = 0u;
        s_ledOn        = !s_ledOn;
        Dio_WriteChannel(DIO_CH_LED1, s_ledOn ? LED_ON_LEVEL : STD_LOW);
    }

    return s_ledPeriodMs;
}
