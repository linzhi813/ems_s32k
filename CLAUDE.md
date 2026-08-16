# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

发动机控制器基础软件开发项目 — a 6-cylinder engine management system (EMS) ECU firmware running on the **NXP S32K344** (ARM Cortex-M7, 160 MHz). Follows **AUTOSAR layered architecture**: Application → BSW (Services/HAL/MCAL/CDD) → Hardware.


## Project Structure

```
ems_s32k/
├── app/                          # 应用层
│   ├── control/                  #   发动机控制算法 (空燃比、点火正时、怠速控制)
│   ├── FaultManager/             #   故障管理器 (故障码管理、监测策略)
│   └── vios/                     #   车辆 IO 系统 (传感器信号处理、执行器控制)
├── bsw/                          # 基础软件层 (Basic Software)
│   ├── cdd/                      #   复杂驱动 (特殊外设驱动)
│   ├── hal/                      #   硬件抽象层 (Hardware Abstraction Layer)
│   │   ├── can_tp/               #     CAN 传输层
│   │   ├── io_hw_ab/             #     I/O 硬件抽象
│   │   └── nvm/                  #     NVM 抽象
│   ├── mcal/                     #   微控制器抽象层 (MCAL)
│   │   ├── adc/                  #     ADC 驱动
│   │   ├── can/                  #     CAN 驱动
│   │   ├── dio/                  #     DIO 驱动
│   │   ├── fls/                  #     Flash 驱动
│   │   ├── gpt/                  #     GPT 定时器驱动
│   │   ├── mcu/                  #     MCU 驱动
│   │   ├── pwm/                  #     PWM 驱动
│   │   ├── spi/                  #     SPI 驱动
│   │   └── wdg/                  #     WDG 看门狗驱动
│   └── services/                 #   服务层
│       ├── com/                  #     通信服务
│       └── os/                   #     操作系统
├── build/                        # 构建输出
├── config/                       # 配置文件
├── doc/                          # 文档与参考手册
├── hw/                           # 硬件相关
│   ├── bom/                      #   物料清单
│   ├── pinout/                   #   引脚分配
│   └── schematic/                #   原理图
├── lib/                          # 外部库
├── test/                         # 测试
│   ├── integration/              #   集成测试
│   └── unit/                     #   单元测试
├── tools/                        # 构建系统 (CMakeLists.txt) + 工具链 + 烧录脚本
│   ├── cmake/
│   │   └── toolchain-arm-none-eabi.cmake
│   └── flash.jlink
```

The project is at **v0.1.0** — startup code, build system, and embedded C library are in place. Application modules are in skeleton form.

## Development Environment

- **MCU:** NXP S32K344EHT1MPBST (172-pin, Cortex-M7, 160 MHz)
- **Compiler:** GNU ARM Embedded Toolchain 10-2020-q4-major (`arm-none-eabi-gcc`)
- **Build:** CMake 3.20+ with Ninja or MinGW Makefiles
- **IDE:** VS Code with Cortex-Debug extension (S32 Design Studio optional)
- **RTD:** NXP S32K3 Real-Time Driver (RTD) — future MCAL provider
- **Debugger:** J-Link (SWD/JTAG), LPUART14 debug serial
- **C Library:** newlib-nano (`--specs=nano.specs`)

## Build

Use CMake presets (defined in `tools/CMakePresets.json`) — this is the path VS Code CMake Tools uses:

```bash
# Configure + build Debug (from tools/ — preset file location)
cd tools && cmake --preset arm-debug && cmake --build --preset arm-debug

# Release
cd tools && cmake --preset arm-release && cmake --build --preset arm-release

# Flash via J-Link (from project root; script paths are root-relative)
cmake --build build --target flash

# Memory usage / disassembly
cmake --build build --target size
cmake --build build --target disasm
```

Build outputs in `build/`: `ems_s32k.elf`, `.bin`, `.hex`, `.map`

### Optional flags

```bash
-DUSE_SEMIHOSTING=ON   # Route printf/scanf through ARM semihosting (debugger console)
-DBUILD_TESTS=ON        # Build tests (future)
```

### Debug (VS Code, verified working)

- Press **F5** with "S32K344 Debug (J-Link)" configuration
- Requires Cortex-Debug extension + J-Link probe connected via SWD
- `serverpath` in launch.json points to `C:/Program Files/SEGGER/JLink/JLinkGDBServerCL.exe` (Cortex-Debug does NOT use `jlinkPath` to find the GDB server)
- SVD file at `config/S32K344.svd` enables the Peripherals register view

## Architecture

### AUTOSAR Layer Stack

```
┌─────────────────────────────────┐
│  app/                           │  ← Application (ASW)
│  ├── control/     Engine control algorithms (AFR, ignition, idle)
│  ├── FaultManager/  DTC management, monitoring strategies
│  └── vios/        Vehicle I/O system (sensors, actuators)
├─────────────────────────────────┤
│  bsw/services/   OS, COM stack  │  ← Services
├─────────────────────────────────┤
│  bsw/hal/        CanTp, IoHwAb, │  ← ECU Abstraction
│                  NvM            │
├─────────────────────────────────┤
│  bsw/cdd/        Custom drivers │  ← Complex Drivers
│                  (PT2000, etc.) │
├─────────────────────────────────┤
│  bsw/mcal/       Adc, Can, Dio, │  ← MCAL (NXP RTD)
│                  Fls, Gpt, Mcu, │
│                  Pwm, Spi, Wdg  │
└─────────────────────────────────┘
```

### Key Hardware Interfaces (from schematic)

| Peripheral | MCU Instance | Connected To |
|---|---|---|
| CAN-FD #1 | CAN0 (PTB0/PTB1) | TJA1051T-3 transceiver |
| CAN-FD #2 | CAN1 (PTE4/PTE5) | TJA1051T-3 transceiver |
| SPI (board) | LPSPI0 | Multi-channel LSD (U15), EEPROM (M95128-DW) |
| SPI (injection) | LPSPI1 | 2× PT2000 programmable solenoid controllers (Bank A/B) |
| SPI (drivers) | LPSPI3 | TLE82092 H-bridge (EGR), other driver ICs |
| eMIOS | Multiple channels | Injection timing, ignition timing, PWM outputs |
| ADC | Multiple channels | MAP, TPS, APP, rail pressure, temperatures, current sense |
| Debug UART | LPUART14 | Console output |

### Injection Control (Critical Path)

Two NXP **PT2000** chips drive 6 injectors via SPI (LPSPI1):
- **Bank A** (cylinders 1–3) and **Bank B** (cylinders 4–6)
- Closed-loop current control via 15 mΩ sense resistors feeding VSENSE pins
- Boost voltage (UBOOST_A/B) generated by diode + inductor charge pump
- Power stage uses BUK9K29-100E dual N-MOSFETs for high-side and low-side

### Signal Naming Convention

```
I_{type}_{signal}    Input signal    e.g., I_A_MAP1 (analog MAP sensor)
O_{type}_{signal}    Output signal   e.g., O_F_FAN (frequency output, fan)
```

Types: `A` = Analog, `S` = Switch/Digital, `F` = Frequency/PWM, `R` = Resistive (NTC), `P` = Power, `T` = Timer/PWM

### Power Architecture

TLE7368E PMIC provides: VCC5V (MCU), VCC1.5V (VDD_HV), VCC1.1V (core). Two NCV317MAB LDOs generate 12V_OUT and 7.3V_OUT for external actuators. Sensor reference: V_V_5VAPP1/2 (5V).

## Key Documents

| Document | Path | Content |
|---|---|---|
| Schematic | `hw/schematic/mdd-01c-d220251225.pdf` | 11-page ECU schematic V0 |
| Schematic Analysis | `hw/schematic/MDD-01C-D1_分析报告.md` | Chinese-language detailed analysis of all 10 sheets |
| S32K3xx Data Sheet | `doc/S32K3xxDS.pdf` | NXP data sheet |
| S32K3xx Reference Manual | `doc/S32K3XXRM.pdf` | NXP reference manual (~49 MB) |
| RTD Training (Pins/Clocks) | `doc/06_S32K3xx_Pins_and_Clocks_with_RTD_Training.pdf` | Pin mux and clock config |
| RTD Training (Timers) | `doc/08_S32K3xx_PIT_and_STM_with_RTD_Training.pdf` | PIT and STM usage |
| RTD Training (CAN) | `doc/18_S32K3xx_Communication_Modules_FlexCAN_with_RTD_Training.pdf` | FlexCAN with RTD |

## Project Conventions

- **Language:** C (primary for BSW/MCAL/APP), possible limited C++ for application
- **AUTOSAR naming:** Module names follow AUTOSAR abbreviations (Adc, Can, Dio, Gpt, Pwm, Spi, Wdg, NvM, CanTp, IoHwAb)
- **APP modules** use PascalCase (FaultManager) or lowercase (control, vios)
- **Chinese documentation:** README and schematic analysis are written in Chinese
- **Encoding:** UTF-8

## Implementation Status & Roadmap

- [x] 1. Startup code + clock tree (`bsw/mcal/mcu/startup_s32k344.S`, `system_S32K344.c`) — **verified on hardware** (IVT boot via SBAF, SWT0 disabled, TCM enabled, SRAM ECC init, FIRC 48MHz)
- [x] 2. CMake toolchain + linker script (`tools/CMakeLists.txt`, `tools/cmake/`, `tools/CMakePresets.json`, `config/S32K344_flash.ld`)
- [x] 3. Embedded C library (`lib/syscalls.c`, `lib/semihosting.c`)
- [x] 4. VS Code debug environment (`.vscode/launch.json`, `tasks.json`) — **verified working** with J-Link SWD
- [ ] 5. Integrate NXP S32K3 RTD into `lib/` (RTD installed at `C:/NXP/S32DS.3.6.1/S32DS/software/PlatformSDK_S32K3/RTD/`)
- [ ] 6. MCAL drivers: Dio → Spi → Adc → Gpt → Pwm → Can → Fls → Wdg
- [ ] 7. HAL: IoHwAb, CanTp, NvM
- [ ] 8. CDD: PT2000 injection driver (SPI-based, most complex)
- [ ] 9. Services: OS, COM stack
- [ ] 10. APP: vios → control → FaultManager

### Verified Hardware Details

- IVT must place CM7_0 start address (vector table, VTOR-aligned 2048) at offset 0x0C — SBAF reads it there
- MSCM IRSPRC is a **16-bit** array (offset 0x880, 240 entries) — 32-bit access causes bus fault
- SWT0 unlock sequence: 0xC520/0xD928 to service reg (0x40270010), then CR = 0xFF000040
- J-Link flash workflow: `cmake --build build --target flash` (erase → loadbin → verify → reset → go)
