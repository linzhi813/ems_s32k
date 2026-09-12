/**
 * @file    Uart.c
 * @brief   MCAL UART Driver — register-level LPUART1 console driver
 *
 * Console UART for the S32K344 core board (no RTD, register access only):
 *   - TX: PTC7   (MSCR[71]: OBE=1, SSS=2 → LPUART1_TX)
 *   - RX: PTC6   (MSCR[70]: IBE=1, pull-up; IMCR[188] SSS=1 → LPUART1_RX)
 *   - Clock: LPUART1_CLK = AIPS_SLOW_CLK = SCS_CLK / (MUX_0_DC_2.DIV + 1)
 *            (24 MHz at boot: FIRC 48 MHz / 2 — verified on hardware),
 *            gated by MC_ME PRTN1_COFB2 block 75
 *   - Baud: 115200 8N1 — baud = src / (OSR × SBR), OSR = 16, SBR = 13
 *   - RX: interrupt-driven (IRQ 142 → IRQ_142_Handler) ring buffer
 *   - TX: blocking (waits TDRE)
 *
 * Register values verified against:
 *   - RTD PAL headers  S32K344_LPUART.h / S32K344_SIUL2.h / S32K344_MC_ME.h
 *   - S32CT device DB  eclipse/mcu_data/.../S32K344_172MQFP/signal_configuration.xml
 *   - SVD              config/S32K344.svd (LPUART_1 @ 0x4032C000, IRQ 142)
 */

#include "Uart.h"
#include "mcu.h"          /* SIUL2_BASE, MC_ME_BASE, Mcu_GetCoreClockHz() */
#include <stddef.h>       /* NULL */

/* ═══════════════════════════════════════════════════════════════
 * LPUART1 register offsets (base 0x4032C000, S32K3xxRM ch.71)
 * ═══════════════════════════════════════════════════════════════ */
#define LPUART1_BASE             0x4032C000u
#define LPUART1_GLOBAL           (*(volatile uint32_t *)(LPUART1_BASE + 0x08u))
#define LPUART1_BAUD             (*(volatile uint32_t *)(LPUART1_BASE + 0x10u))
#define LPUART1_STAT             (*(volatile uint32_t *)(LPUART1_BASE + 0x14u))
#define LPUART1_CTRL             (*(volatile uint32_t *)(LPUART1_BASE + 0x18u))
#define LPUART1_DATA             (*(volatile uint32_t *)(LPUART1_BASE + 0x1Cu))

#define LPUART_GLOBAL_RST        (1u << 1)     /* software module reset      */

#define LPUART_STAT_RDRF         (1u << 21)    /* receive data ready         */
#define LPUART_STAT_TDRE         (1u << 23)    /* transmit data reg empty    */
#define LPUART_STAT_OR           (1u << 19)    /* receive overrun (w1c)      */

#define LPUART_CTRL_RE           (1u << 18)    /* receiver enable            */
#define LPUART_CTRL_TE           (1u << 19)    /* transmitter enable         */
#define LPUART_CTRL_RIE          (1u << 21)    /* RDRF interrupt enable      */

#define LPUART_BAUD_OSR_16       (15u << 24)   /* OSR field 0xF = 16×        */
#define LPUART_BAUD_SBR(x)       ((uint32_t)(x) & 0x1FFFu)

/* ═══════════════════════════════════════════════════════════════
 * MC_ME clock gate for LPUART1 — PRTN1_COFB2 block 75
 * (same unlock sequence as the MSCM clock in startup code)
 * ═══════════════════════════════════════════════════════════════ */
#define MC_ME_PRTN1_COFB2_STAT   (*(volatile uint32_t *)(MC_ME_BASE + 0x318u))
#define MC_ME_PRTN1_COFB2_CLKEN  (*(volatile uint32_t *)(MC_ME_BASE + 0x338u))
#define MC_ME_PRTN1_PUPD         (*(volatile uint32_t *)(MC_ME_BASE + 0x304u))
#define MC_ME_CTL_KEY            (*(volatile uint32_t *)(MC_ME_BASE + 0x000u))
#define MC_ME_COFB2_BLOCK75      (1u << 11)    /* LPUART_1 clock             */

/* ═══════════════════════════════════════════════════════════════
 * SIUL2 pin mux (base from mcu.h)
 *   MSCR array @ 0x240, IMCR array @ 0xA40 — index = GPIO number
 * ═══════════════════════════════════════════════════════════════ */
#define SIUL2_MSCR(n)            (*(volatile uint32_t *)(SIUL2_BASE + 0x240u + 4u * (n)))
#define SIUL2_IMCR(n)            (*(volatile uint32_t *)(SIUL2_BASE + 0xA40u + 4u * (n)))

#define MSCR_SSS_MASK            0x7u
#define MSCR_PUS                 (1u << 11)    /* pull select: 1 = up       */
#define MSCR_PUE                 (1u << 13)    /* pull enable               */
#define MSCR_IBE                 (1u << 19)    /* input buffer enable       */
#define MSCR_OBE                 (1u << 21)    /* output buffer enable      */

#define PIN_PTC6                 70u           /* GPIO[70] → LPUART1_RX     */
#define PIN_PTC7                 71u           /* GPIO[71] → LPUART1_TX     */
#define LPUART1_RX_IMCR_INDEX    188u          /* SIUL2 IMCR for LPUART1_RX */
#define LPUART1_RX_IMCR_SSS      1u            /* selects PTC6 as RX source */

/* ═══════════════════════════════════════════════════════════════
 * NVIC — LPUART1 = IRQ 142 (vector slot 158 = IRQ_142_Handler)
 * ═══════════════════════════════════════════════════════════════ */
#define NVIC_ISER_BASE           0xE000E100u
#define NVIC_IPR_BASE            0xE000E400u   /* byte array, one entry per IRQ */
#define LPUART1_IRQN             142u
#define LPUART1_IRQ_PRIORITY     0x40u         /* configMAX_SYSCALL_INTERRUPT_PRIORITY */

/* ═══════════════════════════════════════════════════════════════
 * LPUART1 clock source (MC_CGM from mcu.h):
 *   LPUART1_CLK = AIPS_SLOW_CLK = SCS_CLK / (MUX_0_DC_2.DIV + 1)
 *   SCS_CLK = FIRC 48 MHz (PLL is not used in this project).
 *   Hardware-verified boot default: DIV = 1 → 24 MHz.
 * ═══════════════════════════════════════════════════════════════ */
#define MC_CGM_MUX_0_DC_2            (*(volatile uint32_t *)(MC_CGM_BASE + 0x310u))
#define MC_CGM_MUX_0_DC_2_DE         (1u << 31)
#define MC_CGM_MUX_0_DC_2_DIV_MASK   0x70000u
#define MC_CGM_MUX_0_DC_2_DIV_SHIFT  16u

/**
 * @brief  Current LPUART1 module clock frequency (AIPS_SLOW_CLK)
 */
static uint32_t Uart_GetModuleClockHz(void)
{
    uint32_t div = 1u;
    uint32_t dc2 = MC_CGM_MUX_0_DC_2;

    if ((dc2 & MC_CGM_MUX_0_DC_2_DE) != 0u) {
        div = ((dc2 & MC_CGM_MUX_0_DC_2_DIV_MASK) >> MC_CGM_MUX_0_DC_2_DIV_SHIFT) + 1u;
    }
    return FIRC_FREQ_HZ / div;
}

/* ═══════════════════════════════════════════════════════════════
 * RX ring buffer (filled by IRQ_142_Handler, drained by Uart_ReadByte)
 *
 * 256 bytes: the FreeRTOS console task drains the ring every 10 ms,
 * during which ~115 bytes can arrive at 115200 baud — a 64-byte ring
 * would overflow.
 * ═══════════════════════════════════════════════════════════════ */
#define UART_RX_RING_SIZE        256u
#define UART_RX_RING_MASK        (UART_RX_RING_SIZE - 1u)
static volatile uint8_t  s_rxRing[UART_RX_RING_SIZE];
static volatile uint32_t s_rxHead;      /* ISR writes here  */
static volatile uint32_t s_rxTail;      /* main reads here  */

/* ═══════════════════════════════════════════════════════════════
 * Static helpers
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief  Enable the LPUART1 module clock (MC_ME partition 1 COFB2 block 75)
 *
 * Same flow as the MSCM clock enable in startup_s32k344.S:
 *   CLKEN bit → PRTN1_PUPD → key sequence → wait for STAT bit.
 */
static void Uart_EnableModuleClock(void)
{
    if ((MC_ME_PRTN1_COFB2_STAT & MC_ME_COFB2_BLOCK75) != 0u) {
        return;                         /* already running */
    }
    MC_ME_PRTN1_COFB2_CLKEN |= MC_ME_COFB2_BLOCK75;
    MC_ME_PRTN1_PUPD        |= 1u;
    MC_ME_CTL_KEY            = 0x5AF0u;
    MC_ME_CTL_KEY            = 0xA50Fu;
    while ((MC_ME_PRTN1_COFB2_STAT & MC_ME_COFB2_BLOCK75) == 0u) {
        /* wait for the clock to start */
    }
}

/* ═══════════════════════════════════════════════════════════════
 * Public API
 * ═══════════════════════════════════════════════════════════════ */

void Uart_Init(const Uart_ConfigType *config)
{
    uint32_t sbr;

    if (config == NULL) {
        return;
    }

    /* ── 1. Enable the LPUART1 clock gate ── */
    Uart_EnableModuleClock();

    /* ── 2. Pin mux (SSS values from the S32CT device database) ──
     * PTC6: LPUART1_RX input — input buffer + pull-up (idle = high),
     *       IMCR[188] = 1 routes PTC6 into the LPUART1 RX data path.
     * PTC7: LPUART1_TX output — output buffer, SSS = 2. */
    SIUL2_MSCR(PIN_PTC6) = (SIUL2_MSCR(PIN_PTC6) & ~(MSCR_SSS_MASK | MSCR_OBE))
                           | MSCR_IBE | MSCR_PUE | MSCR_PUS;
    SIUL2_MSCR(PIN_PTC7) = (SIUL2_MSCR(PIN_PTC7) & ~(MSCR_SSS_MASK | MSCR_IBE))
                           | MSCR_OBE | 2u;
    SIUL2_IMCR(LPUART1_RX_IMCR_INDEX) = LPUART1_RX_IMCR_SSS;

    /* ── 3. Module software reset ── */
    LPUART1_GLOBAL |=  LPUART_GLOBAL_RST;
    LPUART1_GLOBAL &= ~LPUART_GLOBAL_RST;

    /* ── 4. Baud rate ──
     * LPUART1_CLK = AIPS_SLOW_CLK = 24 MHz (FIRC / 2 at boot).
     * baud = src / (OSR × SBR); OSR = 16, SBR = 13 → 115384 bps (+0.16 %). */
    sbr = Uart_GetModuleClockHz() / (16u * config->baudRate);
    if (sbr < 1u) {
        sbr = 1u;
    } else if (sbr > 0x1FFFu) {
        sbr = 0x1FFFu;
    }
    LPUART1_BAUD = LPUART_BAUD_OSR_16 | LPUART_BAUD_SBR(sbr);

    /* ── 5. 8N1, receiver + transmitter on (RIE armed after NVIC) ── */
    LPUART1_CTRL = LPUART_CTRL_RE | LPUART_CTRL_TE;

    /* ── 6. Reset ring buffer, enable LPUART1 in NVIC, arm RX IRQ ── */
    s_rxHead = 0u;
    s_rxTail = 0u;
    /* FreeRTOS: priority 0x40 = configMAX_SYSCALL_INTERRUPT_PRIORITY —
     * maskable during kernel critical sections (BASEPRI).  The reset
     * default of 0 would put the ISR above BASEPRI and it could never
     * be masked.  IRQ_142_Handler calls no FromISR API, so 0x40 is safe. */
    *(volatile uint8_t *)(NVIC_IPR_BASE + LPUART1_IRQN) = LPUART1_IRQ_PRIORITY;
    *(volatile uint32_t *)(NVIC_ISER_BASE + 4u * (LPUART1_IRQN / 32u)) =
        (1u << (LPUART1_IRQN % 32u));
    LPUART1_CTRL |= LPUART_CTRL_RIE;
}

void Uart_WriteByte(uint8_t byte)
{
    while ((LPUART1_STAT & LPUART_STAT_TDRE) == 0u) {
        /* wait until the transmit data register is empty */
    }
    LPUART1_DATA = byte;
}

void Uart_WriteString(const char *str)
{
    while (*str != '\0') {
        Uart_WriteByte((uint8_t)*str);
        str++;
    }
}

bool Uart_ReadByte(uint8_t *byte)
{
    if (s_rxHead == s_rxTail) {
        return false;                   /* buffer empty */
    }
    *byte    = s_rxRing[s_rxTail];
    s_rxTail = (s_rxTail + 1u) & UART_RX_RING_MASK;
    return true;
}

/* ═══════════════════════════════════════════════════════════════
 * LPUART1 receive interrupt — IRQ 142, vector slot 158
 *
 * Overrides the weak IRQ_142_Handler alias in startup_s32k344.S.
 * Drains RDRF into the ring buffer and clears overrun (w1c).
 * ═══════════════════════════════════════════════════════════════ */
void IRQ_142_Handler(void)
{
    while ((LPUART1_STAT & LPUART_STAT_RDRF) != 0u) {
        uint32_t next = (s_rxHead + 1u) & UART_RX_RING_MASK;
        if (next != s_rxTail) {
            s_rxRing[s_rxHead] = (uint8_t)LPUART1_DATA;
            s_rxHead = next;
        } else {
            (void)LPUART1_DATA;         /* ring full — discard to clear RDRF */
        }
    }
    if ((LPUART1_STAT & LPUART_STAT_OR) != 0u) {
        LPUART1_STAT |= LPUART_STAT_OR; /* overrun — write 1 to clear */
    }
}
