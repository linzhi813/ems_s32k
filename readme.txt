发动机控制器基础软件开发项目
================================

项目基于 NXP S32K3 平台，遵循 AUTOSAR 分层架构。

项目结构
--------

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
└── tools/                        # 开发工具与脚本

开发环境
--------
- MCU: NXP S32K344 (ARM Cortex-M7)
- IDE: S32 Design Studio / VS Code
- RTD: S32K3 Real-Time Driver
- 编译器: GCC ARM Embedded

版本历史
--------
v0.1.0 - 2026-08-09  初始项目骨架搭建
