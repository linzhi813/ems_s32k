# Toolchain file for NXP S32K344 (ARM Cortex-M7, bare-metal)
#
# Usage:
#   cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake -G "Unix Makefiles"
#   cmake --build build
#
# Prerequisites:
#   GNU ARM Embedded Toolchain (arm-none-eabi-gcc)
#   https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain

cmake_minimum_required(VERSION 3.20)

# ── Skip compiler sanity check (cross-compiler can't run host binaries) ──
# MUST be set BEFORE any compiler-related _INIT variables
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# ── Target triple ──────────────────────────────────────────────────
set(CMAKE_SYSTEM_NAME       Generic)
set(CMAKE_SYSTEM_PROCESSOR  arm)

set(TRIPLE                  arm-none-eabi)
set(TOOLCHAIN_PREFIX        ${TRIPLE}-)

# ── Compiler settings ──────────────────────────────────────────────
set(CMAKE_C_COMPILER        ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER      ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_ASM_COMPILER      ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_AR                ${TOOLCHAIN_PREFIX}gcc-ar)
set(CMAKE_OBJCOPY           ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_OBJDUMP           ${TOOLCHAIN_PREFIX}objdump)
set(CMAKE_SIZE              ${TOOLCHAIN_PREFIX}size)
set(CMAKE_GDB               ${TOOLCHAIN_PREFIX}gdb)

# ══════════════════════════════════════════════════════════════════
#  Compiler flags
#
#  IMPORTANT: Use space-separated STRINGS, not CMake lists (;).
#  Semicolons leak through to the shell in Unix Makefiles / MSYS2,
#  causing "command not found" errors for every flag after the first.
# ══════════════════════════════════════════════════════════════════

# ── MCU flags (Cortex-M7 + FPv5-SP-D16) ──────────────────────────
set(CPU_FLAGS "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-sp-d16 -mfloat-abi=hard")

# ── Common warning + optimization flags ──────────────────────────
set(COMMON_FLAGS "${CPU_FLAGS} -ffunction-sections -fdata-sections -fno-common -fno-strict-aliasing -fshort-enums -fomit-frame-pointer -Wall -Wextra")

# ── C compiler flags ──────────────────────────────────────────────
set(CMAKE_C_FLAGS_INIT   "${COMMON_FLAGS} -std=gnu11 -ffreestanding -fno-builtin")

# ── C++ compiler flags ────────────────────────────────────────────
set(CMAKE_CXX_FLAGS_INIT "${COMMON_FLAGS} -std=gnu++17 -ffreestanding -fno-builtin -fno-rtti -fno-exceptions")

# ── Assembler flags ───────────────────────────────────────────────
set(CMAKE_ASM_FLAGS_INIT "${CPU_FLAGS} -x assembler-with-cpp")

# ── Linker flags ──────────────────────────────────────────────────
# Linker script path is set in CMakeLists.txt via target_link_options().
set(CMAKE_EXE_LINKER_FLAGS_INIT "${CPU_FLAGS} -Wl,--gc-sections -Wl,--print-memory-usage -Wl,-Map=${CMAKE_PROJECT_NAME}.map --specs=nano.specs -lc -lm")

# ── Debug build (default) ─────────────────────────────────────────
set(CMAKE_C_FLAGS_DEBUG_INIT   "-Og -g3 -gdwarf-3")
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Og -g3 -gdwarf-3")
set(CMAKE_ASM_FLAGS_DEBUG_INIT "-g3 -gdwarf-3")

# ── Release build ─────────────────────────────────────────────────
set(CMAKE_C_FLAGS_RELEASE_INIT   "-Os -g0 -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-Os -g0 -DNDEBUG")
set(CMAKE_ASM_FLAGS_RELEASE_INIT "")

# ── Find tools rule (don't search host system paths) ──────────────
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
