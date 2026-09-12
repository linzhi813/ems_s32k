/**
 * @file    FreeRTOSConfig.h
 * @brief   FreeRTOS V11.3.0 configuration — EMS S32K344 (Cortex-M7 r0p1, GCC)
 *
 * Clock:   FIRC 48 MHz (SystemCoreClock) — SysTick tick = 1 ms
 * Tasks:   Task_UartConsole (10 ms, prio 2), Task_ButtonLed (50 ms, prio 1), idle
 * IRQ:     PendSV/SysTick = 0xFF (hard-coded in port.c xPortStartScheduler),
 *          LPUART1 IRQ142 = 0x40 (configMAX_SYSCALL_INTERRUPT_PRIORITY,
 *          masked during kernel critical sections)
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* ── Scheduling ─────────────────────────────────────────────────── */
#define configUSE_PREEMPTION                     1
#define configUSE_TIME_SLICING                   1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION  0
#define configUSE_TICKLESS_IDLE                  0
#define configIDLE_SHOULD_YIELD                  1
#define configMAX_PRIORITIES                     8

/* ── Tick: SysTick from core clock, 1 ms period ───────────────────
 * port.c derives configSYSTICK_CLOCK_HZ from configCPU_CLOCK_HZ.   */
#define configCPU_CLOCK_HZ                       ( 48000000UL )
#define configTICK_RATE_HZ                       ( ( TickType_t ) 1000 )
#define configUSE_16_BIT_TICKS                   0

/* ── Memory: 32 KB heap_4 pool (TCBs + 2 task stacks + idle) ────── */
#define configTOTAL_HEAP_SIZE                    ( ( size_t ) ( 32 * 1024 ) )
#define configMINIMAL_STACK_SIZE                 ( ( uint16_t ) 512 )
#define configMAX_TASK_NAME_LEN                  16
#define configCHECK_FOR_STACK_OVERFLOW           2
#define configSUPPORT_STATIC_ALLOCATION          0
#define configSUPPORT_DYNAMIC_ALLOCATION         1
#define configUSE_NEWLIB_REENTRANT               0

/* ── Optional modules — not vendored in this project, keep off ──── */
#define configUSE_MUTEXES                        0
#define configUSE_RECURSIVE_MUTEXES              0
#define configUSE_COUNTING_SEMAPHORES            0
#define configUSE_QUEUE_SETS                     0
#define configQUEUE_REGISTRY_SIZE                0
#define configUSE_TIMERS                         0
#define configUSE_CO_ROUTINES                    0
#define configUSE_TASK_NOTIFICATIONS             1
#define configUSE_TRACE_FACILITY                 0
#define configUSE_STATS_FORMATTING_FUNCTIONS     0
#define configENABLE_BACKWARD_COMPATIBILITY      0

/* ── Hooks ────────────────────────────────────────────────────────
 * No idle hook: once the scheduler runs, the kernel's idle task takes
 * over the spare cycles as-is. */
#define configUSE_IDLE_HOOK                      0
#define configUSE_TICK_HOOK                      0
#define configUSE_MALLOC_FAILED_HOOK             0

/* ── Cortex-M7 NVIC priorities (S32K344 implements 4 bits) ────────
 * PRIGROUP stays at reset value 0 → all implemented bits preempt.
 * Critical sections raise BASEPRI to 0x40, masking every IRQ with
 * priority >= 0x40 — including the UART ISR (set to 0x40 in Uart.c)
 * and SysTick/PendSV (0xFF, set by the port).  Any ISR above 0x40
 * must not call FromISR APIs — none do today.                      */
#define configPRIO_BITS                          4
#define configKERNEL_INTERRUPT_PRIORITY          ( 0xF0u )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY     ( 0x40u )

/* ── configASSERT: freeze with interrupts masked + BKPT trap ──────
 * With a debugger attached the BKPT stops here; without one it
 * escalates into the existing HardFault_Handler (stacked-PC capture). */
#define configASSERT( x )                         \
    do {                                          \
        if( ( x ) == 0 ) {                        \
            taskDISABLE_INTERRUPTS();             \
            for( ; ; ) { __asm volatile( "bkpt #0" ); } \
        }                                         \
    } while( 0 )

/* ── API inclusion gates (V11 still gates these) ────────────────── */
#define INCLUDE_vTaskDelay                       1
#define INCLUDE_vTaskDelayUntil                  1
#define INCLUDE_uxTaskGetStackHighWaterMark      1

/* ── Override the weak handlers in startup_s32k344.S with the
 *    strong definitions in port.c ────────────────────────────────── */
#define vPortSVCHandler         SVC_Handler
#define xPortPendSVHandler      PendSV_Handler
#define xPortSysTickHandler     SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
