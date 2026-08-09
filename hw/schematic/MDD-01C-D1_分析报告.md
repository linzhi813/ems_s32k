# MDD-01C-D1 — 发动机控制器 ECU 原理图 综合分析

| 属性 | 内容 |
|------|------|
| **文件性质** | PCB 电路原理图 (Schematic)，版本 V0 |
| **日期** | 2025-12-25 |
| **总页数** | 11 页（Sheet 1–11），图纸尺寸混合 A3/Custom |
| **用途** | 基于 NXP S32K344 的 6 缸发动机管理系统 ECU 硬件设计 |

---

## Sheet 1：电源管理

| 器件 | 功能 |
|------|------|
| **TLE7368E** | Infineon PMIC，多路输出电源管理芯片 |
| Q1 **NVATS5A114PLZT4G** | 反接保护 MOSFET，60V/60A |
| L1 **15μH** + D1 **MURS120T3G** | Buck 降压电路 + 续流二极管 |
| U2/U3 **NCV317MABSTT3G** | 两路可调 LDO，输出 **12V_OUT** 和 **7.3V_OUT** |
| Q4 **BCP53-16** | 电池开关晶体管 (UBAT_SW) |

### 电源轨

| 电源 | 用途 |
|------|------|
| VCC5V | 微控制器主供电 |
| VCC1.5V | S32K3 核心电压 (VDD_HV) |
| VCC1.1V | 内核低电压 |
| V_V_5VAPP1 / V_V_5VAPP2 | 传感器 5V 参考电源 |
| 12V_OUT / 7.3V_OUT | 外部执行器供电 |
| UBAT_SW | MCU 控制的电池开关 |

---

## Sheet 2：主控 MCU — NXP S32K344EHT1MPBST

- **封装**：172 引脚（推测为 BGA/MAXQFP）
- **晶振**：16MHz 外部晶振 (ABM3-16.000MHZ-D2Y-T)
- **JTAG**：标准 20 针接口 (CON-20)，含 TRST/TDI/TDO/TMS/TCK/RTCK/RESET
- **调试串口**：LPUART14 (RX/TX 引出至 J3)
- **EEPROM**：M95128-DW (SPI 接口, TSSOP-8, U5)

### SPI 分配

| SPI 模块 | 连接目标 | 信号 |
|----------|---------|------|
| LPSPI0 | 板级 SPI | B_D_SCK/MISO/MOSI/PCS3/PCS4 |
| LPSPI1 | PT2000 喷油芯片 | B_C_SCK/MISO/MOSI |
| LPSPI3 | 其他驱动芯片 | B_B_SCK/MISO/MOSI/PCS0 |

### 其他外设

| 外设 | 用途 |
|------|------|
| CAN0/CAN1 | 两组 CAN-FD |
| eMIOS × 多路 | 喷油、点火、PWM 控制 |
| ADC × 多路 | 传感器、电流反馈、电压监控 |

---

## Sheet 3：模拟传感器信号调理

采用 **KP214N2611** (DSOF-8) 压力传感器接口 + **MPXHZ6115A6U** 环境压力传感器。

### 传感器信号列表

| 信号 | 传感器 | 备注 |
|------|--------|------|
| I_A_APP1/2 | 加速踏板位置 | 双路冗余 |
| I_A_TPS1/2 | 节气门位置 | 双路冗余 |
| I_A_MAP1/2 | 进气歧管绝对压力 | 双路冗余 |
| I_A_RAILPS/RAILPS2 | 共轨燃油压力 | 双路冗余 |
| I_A_BPS | 大气压力 | |
| I_A_OPS | 机油压力 | |
| I_A_PTO | 涡轮增压压力 | |
| I_R_CTS | 冷却液温度 | NTC 电阻分压 |
| I_R_IATS | 进气温度 | NTC 电阻分压 |
| I_R_FTS | 燃油温度 | NTC 电阻分压 |
| I_R_OTS | 机油温度 | NTC 电阻分压 |
| I_R_ETS | 排气温度 | NTC 电阻分压 |

每个传感器输入均配有 **10nF+100nF 滤波**、100K 偏置电阻、1.5K 分压网络，且有 TP 测试点引出。

---

## Sheet 4：数字/开关量输入

处理 12 路开关信号，统一 UBAT_SW 上拉、150K 串联限流、100nF 去耦：

| 信号 | 来源 |
|------|------|
| I_S_EXHST | 排气制动开关 |
| I_S_BREAK | 制动踏板 |
| I_S_CES | 巡航使能 |
| I_S_BRKRED | 制动冗余 |
| I_S_DIAG | 诊断请求 |
| I_S_CLUTCH | 离合器 |
| I_S_AC | 空调请求 |
| I_S_ACPR | A/C 压力 |
| I_S_RMTEN | 远程使能 |
| I_S_NGEAR | 空档 |
| I_S_BOTCLUTCH | 离合器底部 |
| I_S_START | 启动请求 |
| I_S_WFS | 水燃油分离 |
| I_S_LOGIC | 逻辑电源状态（含 T15 点火输入，经 D10 二极管隔离） |

---

## Sheet 5：曲轴/凸轮轴/车速接口

这是曲轴和凸轮轴位置传感器处理的**关键部分**：

| 通道 | 传感器 | 处理芯片 |
|------|--------|----------|
| **I_F_CRSPOS/CRSNEG** | 曲轴位置传感器（磁阻/VR） | MAX9924UAUB+T (U8, MSOP-10) |
| **I_F_CAMPOS/CAMNEG** | 凸轮轴位置传感器 | MAX9924UAUB+T (U9) |
| **I_F_VSS** | 车速传感器（频率输入） | LM2903AVQDRQ1 比较器 (U10A) |
| **I_F_FAN** | 风扇速度反馈（PWM 输入） | LM2903AVQDRQ1 比较器 (U10B) |

MAX9924 是专业的 VR 传感器接口芯片，支持自适应阈值和零交叉检测，输出信号经 BSS138LT1G MOSFET 驱动后送入 MCU。

---

## Sheet 6 & 7：喷油与预驱 — PT2000 (Bank A/B)

这是整个 ECU 设计**最核心的部分**，使用两颗 **NXP PT2000** (LQFP80) 可编程电磁阀控制器：

### PT2000 功能

- 7 路高边预驱 (S_HS/B_HS) + 8 路低边预驱 (G_LS/D_LS)
- 每路含诊断功能（开路/短路/过流检测）
- 通过 SPI (LPSPI1) 与 MCU 通信
- 诊断标志位输出 (FLAG0–3)，中断输出 (IRQB)

### 功率级拓扑

| 驱动轴 | 高边 MOSFET | 低边 MOSFET |
|--------|------------|------------|
| Bank A (缸1-3) | Q46A/B, Q8A/B 等 **BUK9K29-100E** 双 N-MOS | Q60A/B, Q58A/B 等 **BUK9K29-100E** |
| Bank B (缸4-6) | Q62A/B, Q64A/B 等 **BUK9K29-100E** | Q65A/B, Q66A/B, Q67A/B, Q68A/B |

### 充电泵

D25/D39 **FFD10UP20S** 二极管 + L3/L4 15μH 电感 (SRP1513CA-150M) → 产生 **UBOOST_A/B** 升压电压

### 电流反馈

每路低边 MOSFET 源极串联 **R015 (15mΩ)** 采样电阻，经 RC 滤波后送入 PT2000 VSENSE 引脚实现闭环电流控制。

### 保护器件

| 器件 | 用途 |
|------|------|
| BAS16HT1G | V_V_MR 瞬态保护 |
| MURS120T3G | Boost 整流（超快恢复） |
| RBQ20BM100AFHTL | 高边续流（肖特基共阴） |
| IPD50N10S3L16 | 低压保护泄放 MOSFET |

---

## Sheet 8：高边驱动 & EGR/H 桥

| 芯片 | 功能 | 通道 |
|------|------|------|
| **BTS50085-1TMA** (U20) | 主继电器驱动, TO-220-7, 5mΩ | 1 路 40A HSD |
| **BTT6020-1EKA** (U13) | 大电流 HSD, DSO-14, 7A | 1 路，带电流诊断 (IS pin) |
| **TLE82092SAAUMA1** (U21) | SPI 控制 H 桥驱动器 | EGR PWM (2 路) + 方向控制 |
| **CJ945** 多路 Lowside | 低边输出阵列 | 17 路低边驱动 |

---

## Sheet 9：多路低边 & 执行器控制

| 器件 | 信号 | 功能 |
|------|------|------|
| U15 **多路 LSD** | 18 路执行器输出 | 风扇、电磁阀、继电器、指示灯等 |
| **U16 INA193AIDBVR** | I_A_VCV | 电流检测放大器 (增益 20，测量 VCV 电流) |
| Q69 **NCV8402ASTT1G** | O_S_MRLY | 主继电器控制 (自保护 NMOS) |

### U15 控制的 18 路输出

`O_F_FAN`, `O_F_ENGN`, `O_T_FANL`, `O_S_ERRLGT`, `O_S_WFLGT`, `O_S_FLHT`, `O_S_A`, `O_S_OBD`, `O_P_FPUMP_RELAY`, `O_T_EXHSTBRK`, `O_S_EXHSTBRKLPT`, `O_T_ClntDsp`, `O_S_FANH`, `O_T_VCV_LSD`, `O_S_STRT`, `O_S_GLWLP`, `O_S_GLWRLY`, `O_F_V`

MCU 通过 LPSPI0 的 **B_D_SCK/MISO/MOSI/PCS3** 与 U15 通信。

---

## Sheet 10：CAN 通信

**双通道 CAN-FD**，使用两颗 NXP **TJA1051T-3-1J** (SO8NB)：

| 通道 | TX/RX | 连接到 MCU |
|------|-------|------------|
| CAN1 (CAN0) | B_D_CAN1TX/M, B_D_CAN1RX/M | PTB0/PTB1 |
| CAN2 (CAN1) | B_D_CAN2TX/M, B_D_CAN2RX/M | PTE4/PTE5 |

每路 CAN 配备：
- 共模扼流圈 **51μH** (Fchip4532)
- ESD 保护 **MMBZ33VALT1G** 双向 TVS
- 终端匹配 **61.9Ω** + 滤波电容 **4.7nF**

---

## Sheet 11：连接器引脚分配

J1A–J1E + CON154 定义了完整的线束接口，涵盖所有喷油、点火、传感器、CAN 总线、电源的对外连接。

---

## 关键器件清单

| 类别 | 型号 | 封装 | 数量 |
|------|------|------|------|
| MCU | S32K344EHT1MPBST | 172-pin | 1 |
| PMIC | TLE7368E | — | 1 |
| 喷油控制 | PT2000 | LQFP80 | 2 |
| CAN 收发 | TJA1051T-3-1J | SO8NB | 2 |
| VR 接口 | MAX9924UAUB+T | MSOP-10 | 2 |
| 比较器 | LM2903AVQDRQ1 | SOIC-8 | 2 |
| 压力传感 | KP214N2611 | DSOF-8 | 1 |
| 压力传感 | MPXHZ6115A6U | — | 1 |
| HSD | BTS50085-1TMA | TO-220-7 | 1 |
| HSD | BTT6020-1EKA | DSO-14 | 1 |
| H 桥 | TLE82092SAAUMA1 | — | 1 |
| 电流检测 | INA193AIDBVR | SOT23-5 | 1 |
| 功率 MOS | BUK9K29-100E | SOT1205 | 50+ |
| LDO | NCV317MABSTT3G | SOT-223 | 2 |
| EEPROM | M95128-DW | TSSOP-8 | 1 |
| 主继电器 | NCV8402ASTT1G | SOT-223 | 1 |

---

## 架构总图

```
                    ┌─────────────────────────────────┐
    VBAT ──────────┤ TLE7368E PMIC                   │
                   │  ├→ VCC5V                       │
                   │  ├→ VCC1.5V / VCC1.1V           │
                   │  ├→ 12V_OUT / 7.3V_OUT           │
                   │  └→ V_V_5VAPP1/2 (Sensor 5V)    │
                   └─────────────────────────────────┘
                                     │
              ┌──────────────────────┼──────────────────────┐
              │                      │                      │
    ┌─────────▼─────────┐  ┌────────▼────────┐  ┌──────────▼──────────┐
    │ S32K344 MCU       │  │ CAN ×2          │  │ Analog/Digital I/O  │
    │ (Cortex-M7 @160M) │  │ TJA1051T-3      │  │ KP214N2611 + RC网络 │
    │ 172-pin           │  │                  │  │ 12路开关量输入       │
    └──┬──────┬──────┬──┘  └─────────────────┘  └─────────────────────┘
       │      │      │
       SPI1   SPI3   SPI0
  ┌────▼──┐ ┌─▼────┐ ┌▼───────────────────┐
  │PT2000 │ │PT2000│ │TLE82092 + CJ945 +  │
  │Bank A │ │Bank B│ │BTS50085 + BTT6020  │
  │缸1-3  │ │缸4-6 │ │EGR/Throttle/LSD×18 │
  │喷油器 │ │喷油器│ │                     │
  └───────┘ └──────┘ └────────────────────┘
       │      │
  ┌────▼──────▼────┐
  │ BUK9K29-100E×N │  ← N-MOS 功率阵列
  │ (50+ MOSFETs)  │
  └────────────────┘
```

---

> **总结**：这是一个 **6 缸高压共轨柴油机/缸内直喷汽油机 ECU** 的完整硬件设计，采用 PT2000 作为电磁阀（喷油器）可编程控制器，具备精确的电流闭环控制和全面的故障诊断能力。
