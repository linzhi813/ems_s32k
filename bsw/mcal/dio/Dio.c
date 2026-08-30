/**
 * @file    Dio.c
 * @brief   MCAL Dio Driver — register-level SIUL2 GPIO for S32K344
 *
 * No RTD, direct register access.  SIUL2 is always clocked (no MC_ME
 * clock gate needed).
 *
 * SIUL2 register layout (S32K3xxRM ch.63; verified against
 * config/S32K344.svd and RTD header S32K344_SIUL2.h):
 *   MSCR[0..159]  32-bit  @ 0x240 + 4*n                    pin mux / pad config
 *   GPDO[0..159]  byte    @ 0x1300 + 4*(n/4) + (3 - n%4)   pad data out
 *   GPDI[0..159]  byte    @ 0x1500 + 4*(n/4) + (3 - n%4)   pad data in
 *
 * MSCR bit positions (match the hardware-verified LPUART1 pin mux in
 * bsw/mcal/uart/Uart.c):
 *   SSS[2:0] = source signal select, 0 = GPIO
 *   PUS[11]  = pull select      (1 = pull-up)
 *   PUE[13]  = pull enable
 *   IBE[19]  = input buffer enable
 *   OBE[21]  = output buffer enable
 *
 * Pin usage (core board V1.0):
 *   PTA7  = SW1 user button: externally pulled up to 5 V,
 *           active-high (pressed = HIGH) — internal pull-down enabled
 *           so the pin reads LOW while the button is released
 *   PTB14 = LED3 (green): PTB14 — LED3 — R7 2.2k — GND,
 *           active-high (pad HIGH sources current → LED on)
 */

#include "Dio.h"
#include "mcu.h"          /* SIUL2_BASE */

/* ═══════════════════════════════════════════════════════════════
 * SIUL2 register access macros (index = GPIO pad number)
 * ═══════════════════════════════════════════════════════════════ */
#define SIUL2_MSCR(n)   (*(volatile uint32_t *)(SIUL2_BASE + 0x240u + 4u * (n)))
#define SIUL2_GPDO(n)   (*(volatile uint8_t  *)(SIUL2_BASE + 0x1300u + 4u * ((n) / 4u) + (3u - ((n) % 4u))))
#define SIUL2_GPDI(n)   (*(volatile uint8_t  *)(SIUL2_BASE + 0x1500u + 4u * ((n) / 4u) + (3u - ((n) % 4u))))

#define MSCR_SSS_MASK   0x7u
#define MSCR_PUS        (1u << 11)    /* pull select: 1 = up       */
#define MSCR_PUE        (1u << 13)    /* pull enable               */
#define MSCR_IBE        (1u << 19)    /* input buffer enable       */
#define MSCR_OBE        (1u << 21)    /* output buffer enable      */

/* ═══════════════════════════════════════════════════════════════
 * Public API
 * ═══════════════════════════════════════════════════════════════ */

void Dio_Init(const Dio_ConfigType *config)
{
    (void)config;                       /* reserved for future use */

    /* ── PTA7 (GPIO[7]): button input ──
     * IBE + internal pull-down (PUS = 0 selects down, PUE enables);
     * SSS = 0 (GPIO).  OBE stays 0. */
    SIUL2_MSCR(DIO_CH_BUTTON_SW1) =
        (SIUL2_MSCR(DIO_CH_BUTTON_SW1) & ~(MSCR_SSS_MASK | MSCR_OBE | MSCR_PUS))
        | MSCR_IBE | MSCR_PUE;

    /* ── PTB14 (GPIO[46]): LED output ──
     * OBE + IBE (read-back of driven level); SSS = 0 (GPIO).
     * Push-pull (ODE = 0), default slew rate. */
    SIUL2_MSCR(DIO_CH_LED1) =
        (SIUL2_MSCR(DIO_CH_LED1) & ~MSCR_SSS_MASK)
        | MSCR_OBE | MSCR_IBE;

    /* LED off at boot until the application starts the blink task
     * (active-high: pad LOW = off) */
    Dio_WriteChannel(DIO_CH_LED1, STD_LOW);
}

Dio_LevelType Dio_ReadChannel(Dio_ChannelType channel)
{
    return (SIUL2_GPDI(channel) & 1u) ? STD_HIGH : STD_LOW;
}

void Dio_WriteChannel(Dio_ChannelType channel, Dio_LevelType level)
{
    SIUL2_GPDO(channel) = (level == STD_HIGH) ? 1u : 0u;
}
