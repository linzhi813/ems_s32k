/**
 * @file    Dio.h
 * @brief   MCAL Dio Driver — digital I/O (GPIO) API for S32K344
 *
 * Register-level SIUL2 GPIO driver (no RTD), AUTOSAR Dio API subset:
 *   - Dio_Init()          pin mux / pad configuration
 *   - Dio_ReadChannel()   read pad input level
 *   - Dio_WriteChannel()  drive pad output level
 *
 * Board pins used by the button/LED demo:
 *   - PTA7  (GPIO[7])  SW1 user button — input with internal
 *                      pull-down, active-high (pressed = HIGH)
 *   - PTB14 (GPIO[46]) LED3 green — output, active-high (HIGH lights
 *                      the LED: PTB14 — LED3 — R7 2.2k — GND)
 *
 * The Dio API works with raw pad levels (STD_HIGH / STD_LOW); the
 * active-low mapping for button/LED lives in the application layer.
 */

#ifndef DIO_H
#define DIO_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ─────────────────────────────────────────────────────────────
 * Types
 * ───────────────────────────────────────────────────────────── */

/** Channel ID = SIUL2 GPIO pad number (0..159) */
typedef uint16_t Dio_ChannelType;

/** Pad output level */
typedef uint8_t Dio_LevelType;

#define STD_LOW   0x0u
#define STD_HIGH  0x1u

/* ─────────────────────────────────────────────────────────────
 * Board channel definitions
 * ───────────────────────────────────────────────────────────── */

#define DIO_CH_BUTTON_SW1  7u    /* PTA7  — user button input     */
#define DIO_CH_LED1        46u   /* PTB14 — LED1 output           */

/** Configuration (reserved for future port-based config) */
typedef struct
{
    uint8_t reserved;
} Dio_ConfigType;

/* ─────────────────────────────────────────────────────────────
 * Public API
 * ───────────────────────────────────────────────────────────── */

/**
 * @brief  Initialize the Dio driver
 *
 * Configures the SIUL2 pin mux:
 *   - PTA7:  input buffer + internal pull-down (idle = LOW)
 *   - PTB14: output buffer + input buffer (read-back capable)
 *
 * @param config  Configuration pointer (may be NULL — defaults used)
 */
void Dio_Init(const Dio_ConfigType *config);

/**
 * @brief  Read the input level of a channel (pad)
 *
 * @param channel  GPIO pad number
 * @return  STD_HIGH (1) or STD_LOW (0)
 */
Dio_LevelType Dio_ReadChannel(Dio_ChannelType channel);

/**
 * @brief  Set the output level of a channel (pad)
 *
 * @param channel  GPIO pad number
 * @param level    STD_HIGH / STD_LOW
 */
void Dio_WriteChannel(Dio_ChannelType channel, Dio_LevelType level);

#ifdef __cplusplus
}
#endif

#endif /* DIO_H */
