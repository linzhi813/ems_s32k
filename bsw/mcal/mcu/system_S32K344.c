/**
 * @file    system_S32K344.c
 * @brief   System initialization for NXP S32K344
 *
 * Clock tree configuration:
 *   FIRC (48 MHz) → PLL → 160 MHz CORE_CLK / AIPS_PLAT_CLK
 *
 * PLL formula: PLL_PHI = (FIRC / RDIV) × NDIV / ODIV2
 *              = (48 MHz / 3) × 20 / 2
 *              = 16 MHz × 20 / 2
 *              = 160 MHz
 *
 * Register documentation: S32K3XXRM, Chapters:
 *   - 55 (MC_CGM)  — Clock Generation Module
 *   - 54 (MC_ME)   — Mode Entry Module
 *   - 37-38        — PLL
 */

#include "system_S32K344.h"

/* ═══════════════════════════════════════════════════════════════
 * System core clock variable (referenced by RTD and application)
 * ═══════════════════════════════════════════════════════════════ */
uint32_t SystemCoreClock = 48000000u;  /* Default: FIRC 48 MHz before PLL init */

/* ═══════════════════════════════════════════════════════════════
 * S32K3 peripheral register definitions (minimal subset)
 * ═══════════════════════════════════════════════════════════════ */

/* ── MC_CGM (Clock Generation Module) ── */
#define MC_CGM_BASE             0x40274000u

typedef struct {
    volatile uint32_t PLL_CTRL;         /* 0x00: PLL Control (clock source select) */
    uint32_t _reserved0[3];
    volatile uint32_t DIV_UPD_TRIG;     /* 0x10: Divider update trigger           */
    uint32_t _reserved1[3];
    volatile uint32_t MUX0_CLK_CTL;     /* 0x20: MUX0 — CORE_CLK source          */
    uint32_t _reserved2[3];
    volatile uint32_t MUX0_DIV_CTL;     /* 0x30: MUX0 divider                    */
    uint32_t _reserved3[11];
    volatile uint32_t MUX2_CLK_CTL;     /* 0x60: MUX2 — AIPS_PLAT_CLK source    */
    uint32_t _reserved4[3];
    volatile uint32_t MUX2_DIV_CTL;     /* 0x70: MUX2 divider                    */
    uint32_t _reserved5[11];
    volatile uint32_t MUX6_CLK_CTL;     /* 0xA0: MUX6 — PLL_PHI0 source          */
    uint32_t _reserved6[3];
    volatile uint32_t MUX6_DIV_CTL;     /* 0xB0: MUX6 divider                    */
} MC_CGM_Type;

#define MC_CGM  ((MC_CGM_Type *)MC_CGM_BASE)

/* ── PLL registers ── */
#define PLL_BASE                0x4027C000u

typedef struct {
    volatile uint32_t CR;               /* 0x00: PLL Control Register            */
    volatile uint32_t SR;               /* 0x04: PLL Status Register             */
    volatile uint32_t DV;               /* 0x08: PLL Divider Register            */
    uint32_t _reserved0;
    volatile uint32_t FD;               /* 0x10: PLL Fractional Divider Register */
    uint32_t _reserved1[3];
    volatile uint32_t ODIV;             /* 0x20: PLL Output Divider Register     */
} PLL_Type;

#define PLL  ((PLL_Type *)PLL_BASE)

/* PLL_CR bit fields */
#define PLL_CR_CLKEN            (1u << 0)
#define PLL_CR_PLL_EN           (1u << 1)
#define PLL_CR_CLKSEL_MASK      (0x03u << 2)
#define PLL_CR_CLKSEL_FIRC      (0x01u << 2)    /* FIRC as PLL source            */

/* PLL_SR bit fields */
#define PLL_SR_LOCK             (1u << 0)

/* PLL_DV bit fields */
#define PLL_DV_RDIV_MASK        0x03u            /* Reference divider: 1,2,3,4    */
#define PLL_DV_RDIV_DIV3        0x02u            /* RDIV = 3 (FIRC 48MHz → 16MHz) */
#define PLL_DV_NDIV_MASK        0x3FF00u         /* Loop divider: 1–512           */
#define PLL_DV_NDIV_POS         8u

/* PLL_ODIV bit fields */
#define PLL_ODIV_ODIV2_MASK     0x3F000u         /* Output divider 2              */
#define PLL_ODIV_ODIV2_POS      12u
#define PLL_ODIV_ODIV2_DIV2     0x1000u          /* ODIV2 = 2                     */

/* ── MC_ME (Mode Entry) ── */
#define MC_ME_BASE              0x40270000u

typedef struct {
    uint32_t _reserved0[2];
    volatile uint32_t CTRL_KEY;         /* 0x08: Control Key register            */
    uint32_t _reserved1[15];
    volatile uint32_t RUN_PC0;          /* 0x48: RUN mode peripheral config 0    */
    volatile uint32_t RUN_PC1;          /* 0x4C: RUN mode peripheral config 1    */
} MC_ME_Type;

#define MC_ME  ((MC_ME_Type *)MC_ME_BASE)

/* MC_ME_CTRL_KEY magic to unlock registers */
#define MC_ME_CTRL_KEY_UNLOCK    0x00005AF0u
#define MC_ME_CTRL_KEY_LOCK      0x0000A50Fu

/* ── MSCM (Misc System Control Module) ── */
#define MSCM_BASE               0x40290000u
#define MSCM_CPxCFG0_OFFSET     0xB8u
#define MSCM_CPxCFG0_CLKEN      (1u << 0)

/* ── SCB (System Control Block, Cortex-M7) ── */
#define SCB_BASE                0xE000E000u

typedef struct {
    uint32_t _reserved0[4];
    volatile uint32_t ICACHE_CTRL;      /* 0x010: I-Cache control    */
    volatile uint32_t DCACHE_CTRL;      /* 0x014: D-Cache control    */
} SCB_CACHE_Type;

#define SCB_CACHE ((SCB_CACHE_Type *)(SCB_BASE + 0x010))


/* ═══════════════════════════════════════════════════════════════
 * Static helper: short delay loop for PLL lock wait
 * ═══════════════════════════════════════════════════════════════ */
static void _delay_loops(uint32_t loops)
{
    while (loops--) {
        __asm__ volatile ("nop");
    }
}


/* ═══════════════════════════════════════════════════════════════
 * SystemInit
 *
 * Full initialization sequence executed before main():
 *   1. Enable MSCM clock (required for RTD critical sections)
 *   2. Enable I-Cache and D-Cache
 *   3. Configure PLL for 160 MHz
 *   4. Wait for PLL lock
 *   5. Switch CORE_CLK and AIPS_PLAT_CLK to PLL
 *   6. Update SystemCoreClock
 * ═══════════════════════════════════════════════════════════════ */
void SystemInit(void)
{
    uint32_t reg;

    /* ── Step 1: Enable MSCM clock ──
     * MSCM must be clocked before any peripheral access because
     * RTD critical section functions read MSCM registers to
     * determine which core they run on.
     */
    *(volatile uint32_t *)(MSCM_BASE + MSCM_CPxCFG0_OFFSET) |= MSCM_CPxCFG0_CLKEN;

    /* ── Step 2: Enable I-Cache ── */
    SCB_CACHE->ICACHE_CTRL |= (1u << 0);   /* ICACHE_EN */
    __asm__ volatile ("dsb; isb");

    /* ── Step 3: Enable D-Cache ── */
    SCB_CACHE->DCACHE_CTRL |= (1u << 0);   /* DCACHE_EN */
    __asm__ volatile ("dsb; isb");

    /* ── Step 4: Configure PLL ──
     *
     * Source: FIRC (48 MHz)
     * RDIV   = 3  →  48 MHz / 3  = 16 MHz (VCO input)
     * NDIV   = 20 → 16 MHz × 20  = 320 MHz (VCO)
     * ODIV2  = 2  → 320 MHz / 2  = 160 MHz (PLL_PHI0 output)
     */

    /* 4a. Select FIRC as PLL source */
    reg  = PLL->CR;
    reg &= ~PLL_CR_CLKSEL_MASK;
    reg |= PLL_CR_CLKSEL_FIRC;
    PLL->CR = reg;

    /* 4b. Set reference divider RDIV = 3 */
    reg  = PLL->DV;
    reg &= ~PLL_DV_RDIV_MASK;
    reg |= PLL_DV_RDIV_DIV3;
    PLL->DV = reg;

    /* 4c. Set loop divider NDIV = 20 */
    reg  = PLL->DV;
    reg &= ~PLL_DV_NDIV_MASK;
    reg |= (20u << PLL_DV_NDIV_POS);
    PLL->DV = reg;

    /* 4d. Set output divider ODIV2 = 2 */
    reg  = PLL->ODIV;
    reg &= ~PLL_ODIV_ODIV2_MASK;
    reg |= PLL_ODIV_ODIV2_DIV2;
    PLL->ODIV = reg;

    /* 4e. Set PLL_PHI clock frequency in fractional divider (FD) */
    /* For integer mode, set FD = NDIV = 20 */
    PLL->FD = 20u;

    /* 4f. Enable PLL */
    PLL->CR |= PLL_CR_CLKEN;
    PLL->CR |= PLL_CR_PLL_EN;

    /* 4g. Wait for PLL lock (with timeout safety) */
    for (uint32_t timeout = 0; timeout < 1000000u; timeout++) {
        if (PLL->SR & PLL_SR_LOCK) {
            break;
        }
    }

    /* ── Step 5: Unlock MC_ME for clock source switching ── */
    MC_ME->CTRL_KEY = MC_ME_CTRL_KEY_UNLOCK;

    /* 5a. Select PLL_PHI0 as MUX0 (CORE_CLK) source */
    MC_CGM->MUX0_CLK_CTL = 0x02u;       /* Select input #2 (PLL_PHI0) */
    MC_CGM->MUX0_DIV_CTL = 1u;          /* Divider = 1 (no division)  */
    MC_CGM->DIV_UPD_TRIG  = 0xA5A50001u;/* Trigger divider update     */
    _delay_loops(1000);

    /* 5b. Select PLL_PHI0 as MUX2 (AIPS_PLAT_CLK) source */
    MC_CGM->MUX2_CLK_CTL = 0x02u;       /* Select input #2 (PLL_PHI0) */
    MC_CGM->MUX2_DIV_CTL = 1u;          /* Divider = 1 (no division)  */
    MC_CGM->DIV_UPD_TRIG  = 0xA5A50002u;/* Trigger divider update     */
    _delay_loops(1000);

    /* ── Step 6: Re-lock MC_ME ── */
    MC_ME->CTRL_KEY = MC_ME_CTRL_KEY_LOCK;

    /* ── Step 7: Update global ── */
    SystemCoreClock = 160000000u;
}


/* ═══════════════════════════════════════════════════════════════
 * SystemCoreClockUpdate
 *
 * Called after any runtime clock change to keep SystemCoreClock
 * synchronized with actual hardware state.
 * ═══════════════════════════════════════════════════════════════ */
void SystemCoreClockUpdate(void)
{
    /* Default: read back from PLL configuration.
     * For now, return the known 160 MHz value since we don't
     * support dynamic clock switching at runtime.
     */
    uint32_t pll_src = (PLL->CR & PLL_CR_CLKSEL_MASK) >> 2;
    uint32_t rdiv    = (PLL->DV & PLL_DV_RDIV_MASK) + 1;
    uint32_t ndiv    = (PLL->DV & PLL_DV_NDIV_MASK) >> PLL_DV_NDIV_POS;
    uint32_t odiv2   = ((PLL->ODIV & PLL_ODIV_ODIV2_MASK) >> PLL_ODIV_ODIV2_POS) + 1;

    uint32_t src_freq;
    switch (pll_src) {
        case 0:  src_freq = 0;          break;  /* No source (shouldn't happen) */
        case 1:  src_freq = 48000000u;  break;  /* FIRC: 48 MHz                */
        case 2:  src_freq = 0;          break;  /* FXOSC: not configured       */
        case 3:  src_freq = 0;          break;  /* Reserved                    */
        default: src_freq = 0;          break;
    }

    if (src_freq != 0 && rdiv != 0 && odiv2 != 0) {
        SystemCoreClock = (src_freq / rdiv) * ndiv / odiv2;
    }
}
