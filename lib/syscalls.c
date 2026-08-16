/**
 * @file    syscalls.c
 * @brief   newlib-nano system call stubs for bare-metal S32K344
 *
 * Provides the minimal syscall set required by printf/scanf
 * on bare-metal ARM Cortex-M7:
 *
 *   - _write()   — printf / puts / fprintf output
 *   - _read()    — scanf / getchar input
 *   - _sbrk()    — heap for stdio internal buffering (malloc-based)
 *   - _isatty()  — stdio buffering decision (terminal detection)
 *   - _fstat()   — stream initialization (reports S_IFCHR)
 *
 * I/O routing (selectable via compile-time define):
 *   - USE_SEMIHOSTING  → ARM semihosting (debugger-connected I/O)
 *   - (default)        → _write / _read are weak stubs —
 *                        override in BSP with UART implementation
 *
 * Heap:
 *   _sbrk() uses linker symbols __heap_start__ and __heap_end__
 *   (defined in S32K344_flash.ld).  Stack collision detection
 *   via SP read prevents silent heap/stack corruption.
 */

#include "syscalls.h"

#include <errno.h>
#include <unistd.h>

/* ═══════════════════════════════════════════════════════════════
 *  _sbrk — heap extension for stdio buffers / malloc
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief  Extend the program break (heap) by @p incr bytes.
 *
 * On first call, initializes the internal heap pointer to
 * __heap_start__ (end of .bss in linker script).
 * Checks for collision with the current stack pointer before
 * allowing allocation to proceed.
 *
 * @param incr  Number of bytes to allocate (may be negative for free)
 * @return      Pointer to the start of the newly allocated region,
 *              or (void *)-1 if out of memory.
 */
void *_sbrk(ptrdiff_t incr)
{
    static char *heap_end = NULL;
    char *prev_heap_end;

    /* Initialize on first call */
    if (heap_end == NULL) {
        heap_end = &__heap_start__;
    }

    prev_heap_end = heap_end;

    /* Read current stack pointer for collision detection */
    char *sp;
    __asm__ volatile ("mov %0, sp" : "=r" (sp));

    /* Check for heap/stack overlap */
    if (heap_end + incr > sp) {
        errno = ENOMEM;
        return (void *)-1;
    }

    heap_end += incr;
    return (void *)prev_heap_end;
}


/* ═══════════════════════════════════════════════════════════════
 *  _write — output (used by printf, puts, fprintf, ...)
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief  Write @p nbyte bytes from @p buf to file descriptor @p fd.
 *
 * In the default (non-semihosting) configuration, this function is
 * declared WEAK so that BSP code can override it with a real UART
 * driver.  The stub implementation returns 0 bytes written.
 *
 * When USE_SEMIHOSTING is defined, output goes to the debugger
 * console via ARM semihosting SVC calls.
 *
 * @param fd     1 = stdout, 2 = stderr; all others ignored
 * @param buf    Data to write
 * @param nbyte  Number of bytes
 * @return       Number of bytes actually written
 */
#if defined(USE_SEMIHOSTING)
/* ── Semihosting-backed _write ── */
#include "semihosting.h"

int _write(int fd, const void *buf, size_t nbyte)
{
    (void)fd;
    sh_write((const char *)buf, (int)nbyte);
    return (int)nbyte;
}

#else
/* ── Weak stub — override in BSP ── */
__attribute__((weak))
int _write(int fd, const void *buf, size_t nbyte)
{
    (void)fd;
    (void)buf;
    (void)nbyte;
    /* Default: no output.  Override with UART driver in BSP. */
    return (int)nbyte;
}
#endif /* USE_SEMIHOSTING */


/* ═══════════════════════════════════════════════════════════════
 *  _read — input (used by scanf, getchar, fread(stdin), ...)
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief  Read up to @p nbyte bytes into @p buf from file descriptor @p fd.
 *
 * Default is a weak stub.  Override with UART driver in BSP.
 * With USE_SEMIHOSTING, reads from the debugger console.
 *
 * @param fd     0 = stdin; all others ignored
 * @param buf    Buffer to fill
 * @param nbyte  Maximum bytes to read
 * @return       Number of bytes read, 0 for EOF, -1 for error
 */
#if defined(USE_SEMIHOSTING)
int _read(int fd, void *buf, size_t nbyte)
{
    (void)fd;
    return sh_read((char *)buf, (int)nbyte);
}

#else
__attribute__((weak))
int _read(int fd, void *buf, size_t nbyte)
{
    (void)fd;
    (void)buf;
    (void)nbyte;
    /* Default: no input.  Override with UART driver in BSP. */
    return 0;  /* Return 0 = EOF; change to blocking read in BSP. */
}
#endif /* USE_SEMIHOSTING */


/* ═══════════════════════════════════════════════════════════════
 *  _isatty — terminal detection (stdio buffering decision)
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief  Check if file descriptor refers to a terminal.
 *
 * Returns 1 for stdin/stdout/stderr (fds 0-2), which tells
 * printf to use line-buffering on stdout.
 */
int _isatty(int fd)
{
    /* fd 0 = stdin, 1 = stdout, 2 = stderr */
    if (fd >= 0 && fd <= 2) {
        return 1;
    }
    errno = EBADF;
    return 0;
}


/* ═══════════════════════════════════════════════════════════════
 *  _fstat — file status (stream initialization)
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief  Get file status.
 *
 * Reports the descriptor as a character device so that stdio
 * initializes stdout/stdin correctly for printf/scanf.
 */
int _fstat(int fd, struct stat *st)
{
    (void)fd;
    st->st_mode = S_IFCHR;
    st->st_blksize = 0;
    st->st_size = 0;
    return 0;
}


/* ═══════════════════════════════════════════════════════════════
 *  _close / _lseek — stdio stream teardown/positioning stubs
 *
 *  Referenced by newlib's __sinit() (stream initialization) via
 *  _close_r/_lseek_r, so they are required for printf to link —
 *  even though bare-metal never opens real files.
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief  Close a file descriptor (stub — no filesystem).
 */
int _close(int fd)
{
    (void)fd;
    errno = EBADF;
    return -1;
}

/**
 * @brief  Reposition the file offset (stub — no filesystem).
 */
int _lseek(int fd, off_t offset, int whence)
{
    (void)fd;
    (void)offset;
    (void)whence;
    errno = EBADF;
    return -1;
}
