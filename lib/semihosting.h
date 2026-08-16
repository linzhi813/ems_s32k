/**
 * @file    semihosting.h
 * @brief   ARM Semihosting interface — debugger console I/O
 *
 * ARM semihosting lets bare-metal firmware communicate with a
 * connected debugger (J-Link, OpenOCD, Arm DS) via SVC 0xAB
 * calls.  This is a lightweight way to get printf/scanf working
 * before UART drivers are implemented.
 *
 * Semihosting spec: ARM "Semihosting for AArch32 and AArch64"
 * (IHI 0059, latest version)
 *
 * Usage:
 *   #define USE_SEMIHOSTING  before including this header, or
 *   compile with -DUSE_SEMIHOSTING to route syscalls _write/
 *   _read through semihosting.
 */

#ifndef SEMIHOSTING_H
#define SEMIHOSTING_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ─────────────────────────────────────────────────────────────
 * Semihosting operation codes
 * ───────────────────────────────────────────────────────────── */
#define SH_SYS_OPEN             0x01
#define SH_SYS_CLOSE            0x02
#define SH_SYS_WRITEC           0x03   /* Write single char        */
#define SH_SYS_WRITE0           0x04   /* Write null-terminated str*/
#define SH_SYS_WRITE            0x05   /* Write N bytes            */
#define SH_SYS_READ             0x06   /* Read N bytes             */
#define SH_SYS_READC            0x07   /* Read single char         */
#define SH_SYS_ISERROR          0x08
#define SH_SYS_ISTTY            0x09
#define SH_SYS_SEEK             0x0A
#define SH_SYS_FLEN             0x0C
#define SH_SYS_TMPNAM           0x0D
#define SH_SYS_REMOVE           0x0E
#define SH_SYS_RENAME           0x0F
#define SH_SYS_CLOCK            0x10
#define SH_SYS_TIME             0x11
#define SH_SYS_SYSTEM           0x12
#define SH_SYS_ERRNO            0x13
#define SH_SYS_GET_CMDLINE      0x15
#define SH_SYS_HEAPINFO         0x16
#define SH_SYS_EXIT             0x18
#define SH_SYS_EXIT_EXTENDED    0x20

/* ─────────────────────────────────────────────────────────────
 * Public API
 * ───────────────────────────────────────────────────────────── */

/**
 * @brief  Low-level semihosting call.
 *
 * All semihosting operations use this single SVC mechanism.
 * r0 = operation code, r1 = pointer to parameter block.
 *
 * @param op    Semihosting operation code (SH_SYS_*)
 * @param args  Pointer to argument block
 * @return      Operation-specific return value in r0
 */
int sh_call(uint32_t op, void *args);

/**
 * @brief  Write a single character to the debugger console.
 * @param c  Character to write
 */
void sh_putc(char c);

/**
 * @brief  Write a null-terminated string to the debugger console.
 * @param s  String to write (must be null-terminated)
 */
void sh_write0(const char *s);

/**
 * @brief  Write @p len bytes to the debugger console.
 * @param buf  Buffer to write
 * @param len  Number of bytes
 * @return     Number of bytes written (should equal len)
 */
int sh_write(const char *buf, int len);

/**
 * @brief  Read @p len bytes from the debugger console.
 * @param buf  Buffer to fill
 * @param len  Maximum bytes to read
 * @return     Number of bytes actually read (0 = EOF/nothing)
 */
int sh_read(char *buf, int len);

/**
 * @brief  Read a single character from the debugger console.
 * @return  Character read, or -1 if no data available
 */
int sh_getc(void);

#ifdef __cplusplus
}
#endif

#endif /* SEMIHOSTING_H */
