/**
 * @file    Uart.h
 * @brief   MCAL UART Driver — register-level LPUART1 console driver
 *
 * Console UART on the S32K344 core board:
 *   - TX: PTC7   (LPUART1_TX, MSCR[71] SSS=2)
 *   - RX: PTC6   (LPUART1_RX, MSCR[70] IBE, IMCR[188] SSS=1)
 *   - Default: 115200 baud, 8 data bits, no parity, 1 stop bit
 *
 * RX is interrupt-driven (LPUART1 IRQ 142 → IRQ_142_Handler) into a
 * ring buffer; TX is blocking.  No RTD dependency — register access
 * only, in the same style as bsw/mcal/mcu.
 */

#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  UART driver configuration
 */
typedef struct {
    uint32_t baudRate;   /**< Baud rate in bps (e.g. 115200). 0 = default 115200 */
} Uart_ConfigType;

/**
 * @brief  Initialize LPUART1 (clock gate, pin mux, baud rate, RX interrupt)
 *
 * Steps:
 *   1. Enable LPUART1 module clock (MC_ME PRTN1_COFB2 block 75)
 *   2. Pin mux: PTC7 = LPUART1_TX out, PTC6 = LPUART1_RX in
 *   3. Module software reset, baud rate setup
 *      (module clock = AIPS_SLOW_CLK, read from MC_CGM MUX_0_DC_2)
 *   4. Enable RX + TX, enable RX interrupt in NVIC (IRQ 142)
 */
void Uart_Init(const Uart_ConfigType *config);

/**
 * @brief  Blocking transmit of a single byte
 */
void Uart_WriteByte(uint8_t byte);

/**
 * @brief  Blocking transmit of a NUL-terminated string
 */
void Uart_WriteString(const char *str);

/**
 * @brief  Non-blocking receive of a single byte
 * @return true if a byte was available and stored in *byte
 */
bool Uart_ReadByte(uint8_t *byte);

#ifdef __cplusplus
}
#endif

#endif /* UART_H */
