/**
 * @file    semihosting.c
 * @brief   ARM Semihosting implementation for Cortex-M7
 *
 * Uses the SVC 0xAB instruction to invoke debugger-hosted
 * semihosting operations.  Works with J-Link, OpenOCD, and
 * Arm Development Studio debug probes.
 *
 * Reference:
 *   ARM IHI 0059 — Semihosting for AArch32 and AArch64
 */

#include "semihosting.h"

/* ═══════════════════════════════════════════════════════════════
 *  sh_call — generic semihosting request
 *
 *  On Cortex-M7 (Thumb mode), semihosting uses:
 *      SVC 0xAB
 *      (preceded by a BKPT 0xAB on some older implementations —
 *       not needed on modern debuggers)
 *
 *  Register convention:
 *      r0 → operation type (SH_SYS_*)
 *      r1 → pointer to parameter block
 *      r0 ← return value
 *
 *  The inline assembly preserves r2-r12 and lr.
 * ═══════════════════════════════════════════════════════════════ */
int sh_call(uint32_t op, void *args)
{
    register uint32_t r0 __asm__ ("r0") = op;
    register void     *r1 __asm__ ("r1") = args;
    register int       ret __asm__ ("r0");

    __asm__ volatile (
        "svc 0xAB\n"
        : "=r" (ret)
        : "r" (r0), "r" (r1)
        : "memory"
    );

    return ret;
}


/* ═══════════════════════════════════════════════════════════════
 *  sh_putc — write single character (SH_SYS_WRITEC)
 *
 *  This operation is special: it takes the character directly
 *  in r1 (not a pointer).  The SVC call writes one byte to
 *  the debugger console.
 * ═══════════════════════════════════════════════════════════════ */
void sh_putc(char c)
{
    /* SH_SYS_WRITEC uses a different calling convention:
     * r0 = 0x03 (operation code)
     * r1 = pointer to the character (or the char itself cast to ptr)
     *
     * The spec says: "The semihosting SVC will write the character
     * pointed to by R1 directly to the debugger console."
     *
     * Simplest approach: put the character on the stack and pass
     * a pointer to it.
     */
    sh_call(SH_SYS_WRITEC, &c);
}


/* ═══════════════════════════════════════════════════════════════
 *  sh_write0 — write null-terminated string (SH_SYS_WRITE0)
 * ═══════════════════════════════════════════════════════════════ */
void sh_write0(const char *s)
{
    sh_call(SH_SYS_WRITE0, (void *)s);
}


/* ═══════════════════════════════════════════════════════════════
 *  sh_write — write N bytes (SH_SYS_WRITE)
 *
 *  Parameter block (3 words):
 *      [0] = file handle (1 = stdout)
 *      [1] = pointer to data
 *      [2] = number of bytes to write
 *
 *  Returns 0 on success, or the number of bytes NOT written.
 * ═══════════════════════════════════════════════════════════════ */
int sh_write(const char *buf, int len)
{
    uint32_t block[3];
    block[0] = 1;               /* stdout */
    block[1] = (uint32_t)buf;
    block[2] = (uint32_t)len;

    int remaining = sh_call(SH_SYS_WRITE, block);
    return len - remaining;     /* Bytes actually written */
}


/* ═══════════════════════════════════════════════════════════════
 *  sh_read — read N bytes (SH_SYS_READ)
 *
 *  Parameter block (3 words):
 *      [0] = file handle (0 = stdin)
 *      [1] = pointer to buffer
 *      [2] = number of bytes to read
 *
 *  Returns 0 on success, or the number of bytes NOT read.
 *  If the return value equals len, no data was available.
 * ═══════════════════════════════════════════════════════════════ */
int sh_read(char *buf, int len)
{
    uint32_t block[3];
    block[0] = 0;               /* stdin */
    block[1] = (uint32_t)buf;
    block[2] = (uint32_t)len;

    int remaining = sh_call(SH_SYS_READ, block);
    return len - remaining;     /* Bytes actually read */
}


/* ═══════════════════════════════════════════════════════════════
 *  sh_getc — read single character (SH_SYS_READC)
 *
 *  Returns the character read, or -1 if no data available.
 * ═══════════════════════════════════════════════════════════════ */
int sh_getc(void)
{
    return sh_call(SH_SYS_READC, NULL);
}


/* ═══════════════════════════════════════════════════════════════
 *  sh_report_exception — notify debugger of exit
 *
 *  Uses SH_SYS_EXIT (0x18) with the ADP_Stopped_ApplicationExit
 *  reason code.  This tells the debugger the program has exited
 *  cleanly.
 *
 *  Parameter block (2 words):
 *      [0] = ADP_Stopped_ApplicationExit (0x20026)
 *      [1] = exit sub-code
 *
 *  This function does not return.
 * ═══════════════════════════════════════════════════════════════ */
void sh_report_exception(int status)
{
    uint32_t block[2];
    block[0] = 0x20026;         /* ADP_Stopped_ApplicationExit */
    block[1] = (uint32_t)status;

    sh_call(SH_SYS_EXIT, block);

    /* Should never reach here */
    while (1) {
        __asm__ volatile ("wfi");
    }
}
