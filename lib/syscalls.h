/**
 * @file    syscalls.h
 * @brief   newlib-nano system call stubs — public interface
 *
 * Provides declarations for the minimal syscall set required by
 * printf/scanf on bare-metal ARM Cortex-M targets:
 *   _write, _read, _sbrk, _isatty, _fstat
 *
 * Compiler flags:
 *   -DUSE_SEMIHOSTING   → route _write/_read through ARM semihosting
 *   (default)           → _write/_read are stubs; override in BSP
 */

#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <sys/stat.h>
#include <sys/types.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ─────────────────────────────────────────────────────────────
 * Heap boundary symbols (defined in linker script)
 * ───────────────────────────────────────────────────────────── */
extern char __heap_start__;
extern char __heap_end__;

/* ─────────────────────────────────────────────────────────────
 * System call prototypes (newlib expects these exact names)
 * ───────────────────────────────────────────────────────────── */

/**
 * @brief  Write to a file descriptor (used by printf, puts, etc.)
 * @param  fd     File descriptor (1 = stdout, 2 = stderr)
 * @param  buf    Buffer to write
 * @param  nbyte  Number of bytes to write
 * @return Number of bytes actually written, or -1 on error
 */
int _write(int fd, const void *buf, size_t nbyte);

/**
 * @brief  Read from a file descriptor (used by scanf, getchar, etc.)
 * @param  fd     File descriptor (0 = stdin)
 * @param  buf    Buffer to read into
 * @param  nbyte  Maximum number of bytes to read
 * @return Number of bytes actually read, 0 = EOF, or -1 on error
 */
int _read(int fd, void *buf, size_t nbyte);

/**
 * @brief  Extend the program heap (used by malloc, calloc, realloc)
 * @param  incr  Number of bytes to add to heap
 * @return Pointer to start of new heap region, or (void*)-1 on failure
 */
void *_sbrk(ptrdiff_t incr);

/**
 * @brief  Check if fd is a terminal
 * @return 1 if fd is stdin/stdout/stderr, 0 otherwise
 */
int _isatty(int fd);

/**
 * @brief  Get file status
 * @return 0 on success, reporting fd as S_IFCHR (character device)
 */
int _fstat(int fd, struct stat *st);

/**
 * @brief  Close a file descriptor (stub — required by __sinit)
 * @return -1 with errno = EBADF
 */
int _close(int fd);

/**
 * @brief  Reposition the file offset (stub — required by __sinit)
 * @return -1 with errno = EBADF
 */
int _lseek(int fd, off_t offset, int whence);

#ifdef __cplusplus
}
#endif

#endif /* SYSCALLS_H */
