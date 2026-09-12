cmake_minimum_required(VERSION 3.20)

# ══════════════════════════════════════════════════════════════════
#  EMS S32K — Engine Management System for NXP S32K344
# ══════════════════════════════════════════════════════════════════
#
#  Build (run from project root):
#    cmake -S tools -B build -DCMAKE_TOOLCHAIN_FILE=tools/cmake/toolchain-arm-none-eabi.cmake
#    cmake --build build
#
#  Clean:
#    cmake --build build --target clean
#
#  Verbose:
#    cmake --build build -- VERBOSE=1
# ══════════════════════════════════════════════════════════════════

project(ems_s32k
    VERSION     0.1.0
    DESCRIPTION "Engine Management System — NXP S32K344 firmware"
    LANGUAGES   C ASM
)

# ── Project options ────────────────────────────────────────────────
option(USE_SEMIHOSTING  "Enable ARM semihosting for printf/scanf"  OFF)
option(BUILD_TESTS      "Build test executables"                   OFF)

# ── Root directory (one level above tools/) ────────────────────────
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH ROOT_DIR)

# ── Include directories ────────────────────────────────────────────
set(COMMON_INCLUDES
    ${ROOT_DIR}/bsw/mcal/mcu     # MCU driver (system_S32K344.h, mcu.h)
    ${ROOT_DIR}/bsw/mcal/uart    # UART driver (Uart.h)
    ${ROOT_DIR}/bsw/mcal/dio     # DIO driver (Dio.h)
    ${ROOT_DIR}/app/vios         # VIOS app module (Vios.h)
    ${ROOT_DIR}/lib              # syscalls.h, semihosting.h
    ${ROOT_DIR}/config           # FreeRTOSConfig.h, linker script
    ${ROOT_DIR}/bsw/services/os/FreeRTOS/include                    # FreeRTOS.h, task.h, ...
    ${ROOT_DIR}/bsw/services/os/FreeRTOS/portable/GCC/ARM_CM7/r0p1  # portmacro.h
)

# ── Compiler definitions ───────────────────────────────────────────
set(COMMON_DEFINES
    S32K344                             # Target MCU variant
    CORE_CM7                            # Cortex-M7 core
)

# ── Linker script ──────────────────────────────────────────────────
set(LINKER_SCRIPT "${ROOT_DIR}/config/S32K344_flash.ld")

# ══════════════════════════════════════════════════════════════════
#  Source files (paths relative to ROOT_DIR)
# ══════════════════════════════════════════════════════════════════

# ── Startup + system init ──────────────────────────────────────────
set(MCU_STARTUP_SOURCES
    ${ROOT_DIR}/bsw/mcal/mcu/startup_s32k344.S
    ${ROOT_DIR}/bsw/mcal/mcu/system_S32K344.c
)

# ── Embedded C library (syscalls) ──────────────────────────────────
set(SYSCALL_SOURCES
    ${ROOT_DIR}/lib/syscalls.c
)

# ── MCAL UART driver (LPUART1 console) ─────────────────────────────
set(UART_SOURCES
    ${ROOT_DIR}/bsw/mcal/uart/Uart.c
)

# ── MCAL DIO driver (SIUL2 GPIO) ───────────────────────────────────
set(DIO_SOURCES
    ${ROOT_DIR}/bsw/mcal/dio/Dio.c
)

# ── VIOS app module (button debounce + LED blink) ──────────────────
set(VIOS_SOURCES
    ${ROOT_DIR}/app/vios/Vios.c
)

# ── FreeRTOS kernel V11.3.0 (services layer — OS, core files only) ─
set(FREERTOS_SOURCES
    ${ROOT_DIR}/bsw/services/os/FreeRTOS/tasks.c
    ${ROOT_DIR}/bsw/services/os/FreeRTOS/queue.c
    ${ROOT_DIR}/bsw/services/os/FreeRTOS/list.c
    ${ROOT_DIR}/bsw/services/os/FreeRTOS/portable/GCC/ARM_CM7/r0p1/port.c
    ${ROOT_DIR}/bsw/services/os/FreeRTOS/portable/MemMang/heap_4.c
)

# ── Semihosting (optional) ─────────────────────────────────────────
set(SEMIHOSTING_SOURCES
    ${ROOT_DIR}/lib/semihosting.c
)

# ── Main application entry ─────────────────────────────────────────
set(APP_SOURCES
    ${ROOT_DIR}/app/main.c
)

# ══════════════════════════════════════════════════════════════════
#  Firmware target
# ══════════════════════════════════════════════════════════════════

add_executable(${PROJECT_NAME}
    ${MCU_STARTUP_SOURCES}
    ${SYSCALL_SOURCES}
    ${UART_SOURCES}
    ${DIO_SOURCES}
    ${VIOS_SOURCES}
    ${FREERTOS_SOURCES}
    ${APP_SOURCES}
)

if(USE_SEMIHOSTING)
    target_sources(${PROJECT_NAME} PRIVATE ${SEMIHOSTING_SOURCES})
    target_compile_definitions(${PROJECT_NAME} PRIVATE USE_SEMIHOSTING)
endif()

# ── Target properties ──────────────────────────────────────────────
set_target_properties(${PROJECT_NAME} PROPERTIES
    C_STANDARD                  11
    C_STANDARD_REQUIRED         ON
    C_EXTENSIONS                ON      # GNU extensions (asm, typeof, etc.)
    LINK_DEPENDS                ${LINKER_SCRIPT}
    SUFFIX                      ".elf"
)

# ── Linker script ──────────────────────────────────────────────────
target_link_options(${PROJECT_NAME} PRIVATE
    -T${LINKER_SCRIPT}
)

# ── Include directories ────────────────────────────────────────────
target_include_directories(${PROJECT_NAME} PRIVATE ${COMMON_INCLUDES})

# ── Compile definitions ────────────────────────────────────────────
target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMMON_DEFINES})

# ══════════════════════════════════════════════════════════════════
#  Post-build: generate .bin and .hex files for flashing
# ══════════════════════════════════════════════════════════════════

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary
        $<TARGET_FILE:${PROJECT_NAME}>
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex
        $<TARGET_FILE:${PROJECT_NAME}>
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.hex
    COMMAND ${CMAKE_SIZE} --format=berkeley
        $<TARGET_FILE:${PROJECT_NAME}>
    COMMENT "Generating .bin and .hex flash images"
)

# ══════════════════════════════════════════════════════════════════
#  Utility targets
# ══════════════════════════════════════════════════════════════════

# flash — program via J-Link (run from project root so the script's
# relative path `build/ems_s32k.bin` resolves correctly)
add_custom_target(flash
    COMMAND JLinkExe -device S32K344 -if SWD -speed 4000
        -CommanderScript ${ROOT_DIR}/tools/flash.jlink
    WORKING_DIRECTORY ${ROOT_DIR}
    DEPENDS ${PROJECT_NAME}
    COMMENT "Flashing ${PROJECT_NAME}.bin to S32K344 via J-Link"
)

# size — print detailed memory usage
add_custom_target(size
    COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${PROJECT_NAME}>
    COMMAND ${CMAKE_SIZE} --format=sysv   $<TARGET_FILE:${PROJECT_NAME}>
    DEPENDS ${PROJECT_NAME}
    COMMENT "Memory footprint:"
)

# disasm — generate disassembly listing
add_custom_target(disasm
    COMMAND ${CMAKE_OBJDUMP} -d -S $<TARGET_FILE:${PROJECT_NAME}>
        > ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.disasm
    DEPENDS ${PROJECT_NAME}
    COMMENT "Generating disassembly listing"
)

# ══════════════════════════════════════════════════════════════════
#  Tests (future)
# ══════════════════════════════════════════════════════════════════

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(${ROOT_DIR}/test)
endif()
