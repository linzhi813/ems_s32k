/**
 * @file    syscalls.c
 * @brief   newlib-nano system call stubs for bare-metal S32K344
 *
 * Provides complete retargeting of the newlib C library for a
 * bare-metal ARM Cortex-M7 environment.  Supports:
 *
 *   - printf / puts / fprintf  via _write()
 *   - scanf / getchar          via _read()
 *   - malloc / free / calloc   via _sbrk()
 *   - All other POSIX stubs    return -1 / ENOSYS
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
 *
 * References:
 *   - newlib/libc/sys/arm/syscalls.c (ARM reference implementation)
 *   - CMSIS-Compiler: System Calls OS Interface
 */

#include "syscalls.h"

#include <errno.h>
#include <string.h>
#include <unistd.h>

/* ─────────────────────────────────────────────────────────────
 * environ — required to prevent "undefined reference" linker
 *           errors with certain newlib builds
 * ───────────────────────────────────────────────────────────── */
char *__env[1] = { NULL };
char **environ = __env;


/* ═══════════════════════════════════════════════════════════════
 *  _sbrk — heap extension for malloc / free
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
 *  File descriptor stubs — no filesystem on bare-metal
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief  Open a file (not supported — no filesystem).
 */
int _open(const char *path, int oflag, ...)
{
    (void)path;
    (void)oflag;
    errno = ENOSYS;
    return -1;
}

/**
 * @brief  Close a file descriptor (stub).
 */
int _close(int fd)
{
    (void)fd;
    errno = ENOSYS;
    return -1;
}

/**
 * @brief  Get file status.
 *
 * Reports the descriptor as a character device so that printf
 * line-buffering behaves correctly.
 */
int _fstat(int fd, struct stat *st)
{
    (void)fd;

    /* Zero-initialize then set mode to character device */
    memset(st, 0, sizeof(*st));
    st->st_mode = S_IFCHR;
    return 0;
}

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
 *  Seek stub
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief  Reposition the file offset (stub — no filesystem).
 */
int _lseek(int fd, off_t offset, int whence)
{
    (void)fd;
    (void)offset;
    (void)whence;
    errno = ENOSYS;
    return -1;
}


/* ═══════════════════════════════════════════════════════════════
 *  Process control stubs — bare-metal has no OS process model
 * ═══════════════════════════════════════════════════════════════ */

/**
 * @brief  Terminate the program.
 *
 * On bare-metal, there is no OS to return to.  We trap the
 * processor in a low-power infinite loop.  If a debugger is
 * attached, a BKPT instruction can be placed before the loop.
 */
void _exit(int status)
{
    (void)status;

    /* If debugger is connected, break here for inspection */
    #ifdef USE_SEMIHOSTING
    sh_report_exception(status);
    #endif

    /* Trap: disable interrupts and sleep forever */
    __asm__ volatile (
        "cpsid i\n"
        "loop_%=:\n"
        "   wfi\n"
        "   b   loop_%=\n"
        : : : "memory"
    );
    __builtin_unreachable();
}

/* Prevent tail-call optimization from removing the _exit body */
__attribute__((used))
static void _exit_trampoline(void) { _exit(0); }


/**
 * @brief  Send a signal to a process (stub — no OS).
 */
int _kill(int pid, int sig)
{
    (void)pid;
    (void)sig;
    errno = EINVAL;
    return -1;
}

/**
 * @brief  Get current process ID.
 *
 * Bare-metal is single-process — always return 1.
 */
int _getpid(void)
{
    return 1;
}
