/**
 * @file    startup_s32k344.S
 * @brief   Startup code for NXP S32K344 (ARM Cortex-M7)
 *
 * Contents:
 *   - Image Vector Table (IVT) — required by S32K3 boot ROM
 *   - ARM vector table (16 system exceptions + 256 device IRQ slots)
 *   - Reset_Handler: stack init, data copy, BSS zero, FPU/cache, SystemInit, main
 *   - Default_Handler: weak default for all unused vectors
 *
 * Toolchain: GNU ARM Embedded (arm-none-eabi-gcc)
 * Syntax:    Unified Assembly Language (UAL) with GNU assembler directives
 */

    .syntax unified
    .cpu    cortex-m7
    .fpu    fpv5-sp-d16
    .thumb

/* ═══════════════════════════════════════════════════════════════
 * Global symbols
 * ═══════════════════════════════════════════════════════════════ */
    .global __stack_top__
    .global Reset_Handler
    .global Default_Handler

    .global NMI_Handler
    .global HardFault_Handler
    .global MemManage_Handler
    .global BusFault_Handler
    .global UsageFault_Handler
    .global SVC_Handler
    .global DebugMon_Handler
    .global PendSV_Handler
    .global SysTick_Handler

    /* Weak aliases for all device IRQ handlers → Default_Handler */
    .weak   IRQ_000_Handler
    .weak   IRQ_001_Handler
    .weak   IRQ_002_Handler
    .weak   IRQ_003_Handler
    .weak   IRQ_004_Handler
    .weak   IRQ_005_Handler
    .weak   IRQ_006_Handler
    .weak   IRQ_007_Handler
    .weak   IRQ_008_Handler
    .weak   IRQ_009_Handler
    .weak   IRQ_010_Handler
    .weak   IRQ_011_Handler
    .weak   IRQ_012_Handler
    .weak   IRQ_013_Handler
    .weak   IRQ_014_Handler
    .weak   IRQ_015_Handler
    .weak   IRQ_016_Handler
    .weak   IRQ_017_Handler
    .weak   IRQ_018_Handler
    .weak   IRQ_019_Handler
    .weak   IRQ_020_Handler
    .weak   IRQ_021_Handler
    .weak   IRQ_022_Handler
    .weak   IRQ_023_Handler
    .weak   IRQ_024_Handler
    .weak   IRQ_025_Handler
    .weak   IRQ_026_Handler
    .weak   IRQ_027_Handler
    .weak   IRQ_028_Handler
    .weak   IRQ_029_Handler
    .weak   IRQ_030_Handler
    .weak   IRQ_031_Handler
    .weak   IRQ_032_Handler
    .weak   IRQ_033_Handler
    .weak   IRQ_034_Handler
    .weak   IRQ_035_Handler
    .weak   IRQ_036_Handler
    .weak   IRQ_037_Handler
    .weak   IRQ_038_Handler
    .weak   IRQ_039_Handler
    .weak   IRQ_040_Handler
    .weak   IRQ_041_Handler
    .weak   IRQ_042_Handler
    .weak   IRQ_043_Handler
    .weak   IRQ_044_Handler
    .weak   IRQ_045_Handler
    .weak   IRQ_046_Handler
    .weak   IRQ_047_Handler
    .weak   IRQ_048_Handler
    .weak   IRQ_049_Handler
    .weak   IRQ_050_Handler
    .weak   IRQ_051_Handler
    .weak   IRQ_052_Handler
    .weak   IRQ_053_Handler
    .weak   IRQ_054_Handler
    .weak   IRQ_055_Handler
    .weak   IRQ_056_Handler
    .weak   IRQ_057_Handler
    .weak   IRQ_058_Handler
    .weak   IRQ_059_Handler
    .weak   IRQ_060_Handler
    .weak   IRQ_061_Handler
    .weak   IRQ_062_Handler
    .weak   IRQ_063_Handler
    .weak   IRQ_064_Handler
    .weak   IRQ_065_Handler
    .weak   IRQ_066_Handler
    .weak   IRQ_067_Handler
    .weak   IRQ_068_Handler
    .weak   IRQ_069_Handler
    .weak   IRQ_070_Handler
    .weak   IRQ_071_Handler
    .weak   IRQ_072_Handler
    .weak   IRQ_073_Handler
    .weak   IRQ_074_Handler
    .weak   IRQ_075_Handler
    .weak   IRQ_076_Handler
    .weak   IRQ_077_Handler
    .weak   IRQ_078_Handler
    .weak   IRQ_079_Handler
    .weak   IRQ_080_Handler
    .weak   IRQ_081_Handler
    .weak   IRQ_082_Handler
    .weak   IRQ_083_Handler
    .weak   IRQ_084_Handler
    .weak   IRQ_085_Handler
    .weak   IRQ_086_Handler
    .weak   IRQ_087_Handler
    .weak   IRQ_088_Handler
    .weak   IRQ_089_Handler
    .weak   IRQ_090_Handler
    .weak   IRQ_091_Handler
    .weak   IRQ_092_Handler
    .weak   IRQ_093_Handler
    .weak   IRQ_094_Handler
    .weak   IRQ_095_Handler
    .weak   IRQ_096_Handler
    .weak   IRQ_097_Handler
    .weak   IRQ_098_Handler
    .weak   IRQ_099_Handler
    .weak   IRQ_100_Handler
    .weak   IRQ_101_Handler
    .weak   IRQ_102_Handler
    .weak   IRQ_103_Handler
    .weak   IRQ_104_Handler
    .weak   IRQ_105_Handler
    .weak   IRQ_106_Handler
    .weak   IRQ_107_Handler
    .weak   IRQ_108_Handler
    .weak   IRQ_109_Handler
    .weak   IRQ_110_Handler
    .weak   IRQ_111_Handler
    .weak   IRQ_112_Handler
    .weak   IRQ_113_Handler
    .weak   IRQ_114_Handler
    .weak   IRQ_115_Handler
    .weak   IRQ_116_Handler
    .weak   IRQ_117_Handler
    .weak   IRQ_118_Handler
    .weak   IRQ_119_Handler
    .weak   IRQ_120_Handler
    .weak   IRQ_121_Handler
    .weak   IRQ_122_Handler
    .weak   IRQ_123_Handler
    .weak   IRQ_124_Handler
    .weak   IRQ_125_Handler
    .weak   IRQ_126_Handler
    .weak   IRQ_127_Handler
    .weak   IRQ_128_Handler
    .weak   IRQ_129_Handler
    .weak   IRQ_130_Handler
    .weak   IRQ_131_Handler
    .weak   IRQ_132_Handler
    .weak   IRQ_133_Handler
    .weak   IRQ_134_Handler
    .weak   IRQ_135_Handler
    .weak   IRQ_136_Handler
    .weak   IRQ_137_Handler
    .weak   IRQ_138_Handler
    .weak   IRQ_139_Handler
    .weak   IRQ_140_Handler
    .weak   IRQ_141_Handler
    .weak   IRQ_142_Handler
    .weak   IRQ_143_Handler
    .weak   IRQ_144_Handler
    .weak   IRQ_145_Handler
    .weak   IRQ_146_Handler
    .weak   IRQ_147_Handler
    .weak   IRQ_148_Handler
    .weak   IRQ_149_Handler
    .weak   IRQ_150_Handler
    .weak   IRQ_151_Handler
    .weak   IRQ_152_Handler
    .weak   IRQ_153_Handler
    .weak   IRQ_154_Handler
    .weak   IRQ_155_Handler
    .weak   IRQ_156_Handler
    .weak   IRQ_157_Handler
    .weak   IRQ_158_Handler
    .weak   IRQ_159_Handler
    .weak   IRQ_160_Handler
    .weak   IRQ_161_Handler
    .weak   IRQ_162_Handler
    .weak   IRQ_163_Handler
    .weak   IRQ_164_Handler
    .weak   IRQ_165_Handler
    .weak   IRQ_166_Handler
    .weak   IRQ_167_Handler
    .weak   IRQ_168_Handler
    .weak   IRQ_169_Handler
    .weak   IRQ_170_Handler
    .weak   IRQ_171_Handler
    .weak   IRQ_172_Handler
    .weak   IRQ_173_Handler
    .weak   IRQ_174_Handler
    .weak   IRQ_175_Handler
    .weak   IRQ_176_Handler
    .weak   IRQ_177_Handler
    .weak   IRQ_178_Handler
    .weak   IRQ_179_Handler
    .weak   IRQ_180_Handler
    .weak   IRQ_181_Handler
    .weak   IRQ_182_Handler
    .weak   IRQ_183_Handler
    .weak   IRQ_184_Handler
    .weak   IRQ_185_Handler
    .weak   IRQ_186_Handler
    .weak   IRQ_187_Handler
    .weak   IRQ_188_Handler
    .weak   IRQ_189_Handler
    .weak   IRQ_190_Handler
    .weak   IRQ_191_Handler
    .weak   IRQ_192_Handler
    .weak   IRQ_193_Handler
    .weak   IRQ_194_Handler
    .weak   IRQ_195_Handler
    .weak   IRQ_196_Handler
    .weak   IRQ_197_Handler
    .weak   IRQ_198_Handler
    .weak   IRQ_199_Handler
    .weak   IRQ_200_Handler
    .weak   IRQ_201_Handler
    .weak   IRQ_202_Handler
    .weak   IRQ_203_Handler
    .weak   IRQ_204_Handler
    .weak   IRQ_205_Handler
    .weak   IRQ_206_Handler
    .weak   IRQ_207_Handler
    .weak   IRQ_208_Handler
    .weak   IRQ_209_Handler
    .weak   IRQ_210_Handler
    .weak   IRQ_211_Handler
    .weak   IRQ_212_Handler
    .weak   IRQ_213_Handler
    .weak   IRQ_214_Handler
    .weak   IRQ_215_Handler
    .weak   IRQ_216_Handler
    .weak   IRQ_217_Handler
    .weak   IRQ_218_Handler
    .weak   IRQ_219_Handler
    .weak   IRQ_220_Handler
    .weak   IRQ_221_Handler
    .weak   IRQ_222_Handler
    .weak   IRQ_223_Handler
    .weak   IRQ_224_Handler
    .weak   IRQ_225_Handler
    .weak   IRQ_226_Handler
    .weak   IRQ_227_Handler
    .weak   IRQ_228_Handler
    .weak   IRQ_229_Handler
    .weak   IRQ_230_Handler
    .weak   IRQ_231_Handler
    .weak   IRQ_232_Handler
    .weak   IRQ_233_Handler
    .weak   IRQ_234_Handler
    .weak   IRQ_235_Handler
    .weak   IRQ_236_Handler
    .weak   IRQ_237_Handler
    .weak   IRQ_238_Handler
    .weak   IRQ_239_Handler
    .weak   IRQ_240_Handler
    .weak   IRQ_241_Handler
    .weak   IRQ_242_Handler
    .weak   IRQ_243_Handler
    .weak   IRQ_244_Handler
    .weak   IRQ_245_Handler
    .weak   IRQ_246_Handler
    .weak   IRQ_247_Handler
    .weak   IRQ_248_Handler
    .weak   IRQ_249_Handler
    .weak   IRQ_250_Handler
    .weak   IRQ_251_Handler
    .weak   IRQ_252_Handler
    .weak   IRQ_253_Handler
    .weak   IRQ_254_Handler
    .weak   IRQ_255_Handler


/* ═══════════════════════════════════════════════════════════════
 * S32K3 Image Vector Table (IVT)
 *
 * The S32K3 boot ROM (SBAF) searches flash for a valid IVT header.
 * This MUST be the first 256 bytes of the flash image.
 *
 * Layout verified against NXP official startup_cm7.s
 * (.boot_header section) — S32K3_RTD_3_0_0:
 *   00h  IVT marker          = 0x5AA55AA5
 *   04h  Boot configuration  = CM7_0_ENABLE=1, non-secure boot
 *   08h  Reserved
 *   0Ch  CM7_0 start address = vector table (must honor VTOR
 *        alignment — power of 2 ≥ table size)
 *   10h  Reserved
 *   14h  CM7_1 start address (not used, S32K344 single core)
 *   18h  Reserved
 *   1Ch  CM7_2 start address (not used)
 *   20h  Reserved
 *   24h  XRDC configuration pointer
 *   28h  Lifecycle configuration pointer
 *   2Ch.. Reserved (pad to 256 bytes)
 * ═══════════════════════════════════════════════════════════════ */
    .section .ivt_header, "a", %progbits
    .align  8

IVT:
    .word   0x5AA55AA5          /* 00h: IVT marker                     */
    .word   0x00000001          /* 04h: Boot config — CM7_0_ENABLE=1,  */
                                /*      BOOT_SEQ=0 (non-secure boot)   */
    .word   0x00000000          /* 08h: Reserved                       */
    .word   Vectors             /* 0Ch: CM7_0 start = vector table     */
    .word   0x00000000          /* 10h: Reserved                       */
    .word   0x00000000          /* 14h: CM7_1 start (not used)         */
    .word   0x00000000          /* 18h: Reserved                       */
    .word   0x00000000          /* 1Ch: CM7_2 start (not used)         */
    .word   0x00000000          /* 20h: Reserved                       */
    .word   0x00000000          /* 24h: XRDC configuration pointer     */
    .word   0x00000000          /* 28h: Lifecycle configuration ptr    */
    .word   0x00000000          /* 2Ch: Reserved                       */
    .org    IVT + 0x100         /* 30h..: Reserved — pad to 256 bytes  */

/* ═══════════════════════════════════════════════════════════════
 * ARM Cortex-M7 Vector Table
 *
 * Must be aligned to power-of-two boundary (linker handles this).
 * 16 system exception vectors + 256 device IRQ slots.
 * ═══════════════════════════════════════════════════════════════ */
    .section .vectors, "a", %progbits
    .align  11                  /* 2^11 = 2048-byte alignment
                                 * (272 vectors × 4 B = 1088 B table,
                                 *  VTOR needs power-of-2 ≥ table) */

Vectors:
    .word   __stack_top__       /* 0x000: Initial stack pointer     */
    .word   Reset_Handler       /* 0x004: Reset                     */
    .word   NMI_Handler         /* 0x008: Non-Maskable Interrupt    */
    .word   HardFault_Handler   /* 0x00C: Hard Fault                */
    .word   MemManage_Handler   /* 0x010: Memory Management Fault   */
    .word   BusFault_Handler    /* 0x014: Bus Fault                 */
    .word   UsageFault_Handler  /* 0x018: Usage Fault               */
    .word   0                   /* 0x01C: Reserved                  */
    .word   0                   /* 0x020: Reserved                  */
    .word   0                   /* 0x024: Reserved                  */
    .word   0                   /* 0x028: Reserved                  */
    .word   SVC_Handler         /* 0x02C: Supervisor Call           */
    .word   DebugMon_Handler    /* 0x030: Debug Monitor             */
    .word   0                   /* 0x034: Reserved                  */
    .word   PendSV_Handler      /* 0x038: Pendable Service          */
    .word   SysTick_Handler     /* 0x03C: System Tick Timer         */

    /* ── Device-specific interrupts (IRQ 0 – 255) ── */
    /* Each maps to a weak alias; override in application code.   */
    /* Rename these to the actual peripheral IRQ names once the   */
    /* S32K3 RTD IRQ enumeration is available.                   */
    .word   IRQ_000_Handler     /* IRQ   0 */
    .word   IRQ_001_Handler     /* IRQ   1 */
    .word   IRQ_002_Handler     /* IRQ   2 */
    .word   IRQ_003_Handler     /* IRQ   3 */
    .word   IRQ_004_Handler     /* IRQ   4 */
    .word   IRQ_005_Handler     /* IRQ   5 */
    .word   IRQ_006_Handler     /* IRQ   6 */
    .word   IRQ_007_Handler     /* IRQ   7 */
    .word   IRQ_008_Handler     /* IRQ   8 */
    .word   IRQ_009_Handler     /* IRQ   9 */
    .word   IRQ_010_Handler     /* IRQ  10 */
    .word   IRQ_011_Handler     /* IRQ  11 */
    .word   IRQ_012_Handler     /* IRQ  12 */
    .word   IRQ_013_Handler     /* IRQ  13 */
    .word   IRQ_014_Handler     /* IRQ  14 */
    .word   IRQ_015_Handler     /* IRQ  15 */
    .word   IRQ_016_Handler     /* IRQ  16 */
    .word   IRQ_017_Handler     /* IRQ  17 */
    .word   IRQ_018_Handler     /* IRQ  18 */
    .word   IRQ_019_Handler     /* IRQ  19 */
    .word   IRQ_020_Handler     /* IRQ  20 */
    .word   IRQ_021_Handler     /* IRQ  21 */
    .word   IRQ_022_Handler     /* IRQ  22 */
    .word   IRQ_023_Handler     /* IRQ  23 */
    .word   IRQ_024_Handler     /* IRQ  24 */
    .word   IRQ_025_Handler     /* IRQ  25 */
    .word   IRQ_026_Handler     /* IRQ  26 */
    .word   IRQ_027_Handler     /* IRQ  27 */
    .word   IRQ_028_Handler     /* IRQ  28 */
    .word   IRQ_029_Handler     /* IRQ  29 */
    .word   IRQ_030_Handler     /* IRQ  30 */
    .word   IRQ_031_Handler     /* IRQ  31 */
    .word   IRQ_032_Handler     /* IRQ  32 */
    .word   IRQ_033_Handler     /* IRQ  33 */
    .word   IRQ_034_Handler     /* IRQ  34 */
    .word   IRQ_035_Handler     /* IRQ  35 */
    .word   IRQ_036_Handler     /* IRQ  36 */
    .word   IRQ_037_Handler     /* IRQ  37 */
    .word   IRQ_038_Handler     /* IRQ  38 */
    .word   IRQ_039_Handler     /* IRQ  39 */
    .word   IRQ_040_Handler     /* IRQ  40 */
    .word   IRQ_041_Handler     /* IRQ  41 */
    .word   IRQ_042_Handler     /* IRQ  42 */
    .word   IRQ_043_Handler     /* IRQ  43 */
    .word   IRQ_044_Handler     /* IRQ  44 */
    .word   IRQ_045_Handler     /* IRQ  45 */
    .word   IRQ_046_Handler     /* IRQ  46 */
    .word   IRQ_047_Handler     /* IRQ  47 */
    .word   IRQ_048_Handler     /* IRQ  48 */
    .word   IRQ_049_Handler     /* IRQ  49 */
    .word   IRQ_050_Handler     /* IRQ  50 */
    .word   IRQ_051_Handler     /* IRQ  51 */
    .word   IRQ_052_Handler     /* IRQ  52 */
    .word   IRQ_053_Handler     /* IRQ  53 */
    .word   IRQ_054_Handler     /* IRQ  54 */
    .word   IRQ_055_Handler     /* IRQ  55 */
    .word   IRQ_056_Handler     /* IRQ  56 */
    .word   IRQ_057_Handler     /* IRQ  57 */
    .word   IRQ_058_Handler     /* IRQ  58 */
    .word   IRQ_059_Handler     /* IRQ  59 */
    .word   IRQ_060_Handler     /* IRQ  60 */
    .word   IRQ_061_Handler     /* IRQ  61 */
    .word   IRQ_062_Handler     /* IRQ  62 */
    .word   IRQ_063_Handler     /* IRQ  63 */
    .word   IRQ_064_Handler     /* IRQ  64 */
    .word   IRQ_065_Handler     /* IRQ  65 */
    .word   IRQ_066_Handler     /* IRQ  66 */
    .word   IRQ_067_Handler     /* IRQ  67 */
    .word   IRQ_068_Handler     /* IRQ  68 */
    .word   IRQ_069_Handler     /* IRQ  69 */
    .word   IRQ_070_Handler     /* IRQ  70 */
    .word   IRQ_071_Handler     /* IRQ  71 */
    .word   IRQ_072_Handler     /* IRQ  72 */
    .word   IRQ_073_Handler     /* IRQ  73 */
    .word   IRQ_074_Handler     /* IRQ  74 */
    .word   IRQ_075_Handler     /* IRQ  75 */
    .word   IRQ_076_Handler     /* IRQ  76 */
    .word   IRQ_077_Handler     /* IRQ  77 */
    .word   IRQ_078_Handler     /* IRQ  78 */
    .word   IRQ_079_Handler     /* IRQ  79 */
    .word   IRQ_080_Handler     /* IRQ  80 */
    .word   IRQ_081_Handler     /* IRQ  81 */
    .word   IRQ_082_Handler     /* IRQ  82 */
    .word   IRQ_083_Handler     /* IRQ  83 */
    .word   IRQ_084_Handler     /* IRQ  84 */
    .word   IRQ_085_Handler     /* IRQ  85 */
    .word   IRQ_086_Handler     /* IRQ  86 */
    .word   IRQ_087_Handler     /* IRQ  87 */
    .word   IRQ_088_Handler     /* IRQ  88 */
    .word   IRQ_089_Handler     /* IRQ  89 */
    .word   IRQ_090_Handler     /* IRQ  90 */
    .word   IRQ_091_Handler     /* IRQ  91 */
    .word   IRQ_092_Handler     /* IRQ  92 */
    .word   IRQ_093_Handler     /* IRQ  93 */
    .word   IRQ_094_Handler     /* IRQ  94 */
    .word   IRQ_095_Handler     /* IRQ  95 */
    .word   IRQ_096_Handler     /* IRQ  96 */
    .word   IRQ_097_Handler     /* IRQ  97 */
    .word   IRQ_098_Handler     /* IRQ  98 */
    .word   IRQ_099_Handler     /* IRQ  99 */
    .word   IRQ_100_Handler     /* IRQ 100 */
    .word   IRQ_101_Handler     /* IRQ 101 */
    .word   IRQ_102_Handler     /* IRQ 102 */
    .word   IRQ_103_Handler     /* IRQ 103 */
    .word   IRQ_104_Handler     /* IRQ 104 */
    .word   IRQ_105_Handler     /* IRQ 105 */
    .word   IRQ_106_Handler     /* IRQ 106 */
    .word   IRQ_107_Handler     /* IRQ 107 */
    .word   IRQ_108_Handler     /* IRQ 108 */
    .word   IRQ_109_Handler     /* IRQ 109 */
    .word   IRQ_110_Handler     /* IRQ 110 */
    .word   IRQ_111_Handler     /* IRQ 111 */
    .word   IRQ_112_Handler     /* IRQ 112 */
    .word   IRQ_113_Handler     /* IRQ 113 */
    .word   IRQ_114_Handler     /* IRQ 114 */
    .word   IRQ_115_Handler     /* IRQ 115 */
    .word   IRQ_116_Handler     /* IRQ 116 */
    .word   IRQ_117_Handler     /* IRQ 117 */
    .word   IRQ_118_Handler     /* IRQ 118 */
    .word   IRQ_119_Handler     /* IRQ 119 */
    .word   IRQ_120_Handler     /* IRQ 120 */
    .word   IRQ_121_Handler     /* IRQ 121 */
    .word   IRQ_122_Handler     /* IRQ 122 */
    .word   IRQ_123_Handler     /* IRQ 123 */
    .word   IRQ_124_Handler     /* IRQ 124 */
    .word   IRQ_125_Handler     /* IRQ 125 */
    .word   IRQ_126_Handler     /* IRQ 126 */
    .word   IRQ_127_Handler     /* IRQ 127 */
    .word   IRQ_128_Handler     /* IRQ 128 */
    .word   IRQ_129_Handler     /* IRQ 129 */
    .word   IRQ_130_Handler     /* IRQ 130 */
    .word   IRQ_131_Handler     /* IRQ 131 */
    .word   IRQ_132_Handler     /* IRQ 132 */
    .word   IRQ_133_Handler     /* IRQ 133 */
    .word   IRQ_134_Handler     /* IRQ 134 */
    .word   IRQ_135_Handler     /* IRQ 135 */
    .word   IRQ_136_Handler     /* IRQ 136 */
    .word   IRQ_137_Handler     /* IRQ 137 */
    .word   IRQ_138_Handler     /* IRQ 138 */
    .word   IRQ_139_Handler     /* IRQ 139 */
    .word   IRQ_140_Handler     /* IRQ 140 */
    .word   IRQ_141_Handler     /* IRQ 141 */
    .word   IRQ_142_Handler     /* IRQ 142 */
    .word   IRQ_143_Handler     /* IRQ 143 */
    .word   IRQ_144_Handler     /* IRQ 144 */
    .word   IRQ_145_Handler     /* IRQ 145 */
    .word   IRQ_146_Handler     /* IRQ 146 */
    .word   IRQ_147_Handler     /* IRQ 147 */
    .word   IRQ_148_Handler     /* IRQ 148 */
    .word   IRQ_149_Handler     /* IRQ 149 */
    .word   IRQ_150_Handler     /* IRQ 150 */
    .word   IRQ_151_Handler     /* IRQ 151 */
    .word   IRQ_152_Handler     /* IRQ 152 */
    .word   IRQ_153_Handler     /* IRQ 153 */
    .word   IRQ_154_Handler     /* IRQ 154 */
    .word   IRQ_155_Handler     /* IRQ 155 */
    .word   IRQ_156_Handler     /* IRQ 156 */
    .word   IRQ_157_Handler     /* IRQ 157 */
    .word   IRQ_158_Handler     /* IRQ 158 */
    .word   IRQ_159_Handler     /* IRQ 159 */
    .word   IRQ_160_Handler     /* IRQ 160 */
    .word   IRQ_161_Handler     /* IRQ 161 */
    .word   IRQ_162_Handler     /* IRQ 162 */
    .word   IRQ_163_Handler     /* IRQ 163 */
    .word   IRQ_164_Handler     /* IRQ 164 */
    .word   IRQ_165_Handler     /* IRQ 165 */
    .word   IRQ_166_Handler     /* IRQ 166 */
    .word   IRQ_167_Handler     /* IRQ 167 */
    .word   IRQ_168_Handler     /* IRQ 168 */
    .word   IRQ_169_Handler     /* IRQ 169 */
    .word   IRQ_170_Handler     /* IRQ 170 */
    .word   IRQ_171_Handler     /* IRQ 171 */
    .word   IRQ_172_Handler     /* IRQ 172 */
    .word   IRQ_173_Handler     /* IRQ 173 */
    .word   IRQ_174_Handler     /* IRQ 174 */
    .word   IRQ_175_Handler     /* IRQ 175 */
    .word   IRQ_176_Handler     /* IRQ 176 */
    .word   IRQ_177_Handler     /* IRQ 177 */
    .word   IRQ_178_Handler     /* IRQ 178 */
    .word   IRQ_179_Handler     /* IRQ 179 */
    .word   IRQ_180_Handler     /* IRQ 180 */
    .word   IRQ_181_Handler     /* IRQ 181 */
    .word   IRQ_182_Handler     /* IRQ 182 */
    .word   IRQ_183_Handler     /* IRQ 183 */
    .word   IRQ_184_Handler     /* IRQ 184 */
    .word   IRQ_185_Handler     /* IRQ 185 */
    .word   IRQ_186_Handler     /* IRQ 186 */
    .word   IRQ_187_Handler     /* IRQ 187 */
    .word   IRQ_188_Handler     /* IRQ 188 */
    .word   IRQ_189_Handler     /* IRQ 189 */
    .word   IRQ_190_Handler     /* IRQ 190 */
    .word   IRQ_191_Handler     /* IRQ 191 */
    .word   IRQ_192_Handler     /* IRQ 192 */
    .word   IRQ_193_Handler     /* IRQ 193 */
    .word   IRQ_194_Handler     /* IRQ 194 */
    .word   IRQ_195_Handler     /* IRQ 195 */
    .word   IRQ_196_Handler     /* IRQ 196 */
    .word   IRQ_197_Handler     /* IRQ 197 */
    .word   IRQ_198_Handler     /* IRQ 198 */
    .word   IRQ_199_Handler     /* IRQ 199 */
    .word   IRQ_200_Handler     /* IRQ 200 */
    .word   IRQ_201_Handler     /* IRQ 201 */
    .word   IRQ_202_Handler     /* IRQ 202 */
    .word   IRQ_203_Handler     /* IRQ 203 */
    .word   IRQ_204_Handler     /* IRQ 204 */
    .word   IRQ_205_Handler     /* IRQ 205 */
    .word   IRQ_206_Handler     /* IRQ 206 */
    .word   IRQ_207_Handler     /* IRQ 207 */
    .word   IRQ_208_Handler     /* IRQ 208 */
    .word   IRQ_209_Handler     /* IRQ 209 */
    .word   IRQ_210_Handler     /* IRQ 210 */
    .word   IRQ_211_Handler     /* IRQ 211 */
    .word   IRQ_212_Handler     /* IRQ 212 */
    .word   IRQ_213_Handler     /* IRQ 213 */
    .word   IRQ_214_Handler     /* IRQ 214 */
    .word   IRQ_215_Handler     /* IRQ 215 */
    .word   IRQ_216_Handler     /* IRQ 216 */
    .word   IRQ_217_Handler     /* IRQ 217 */
    .word   IRQ_218_Handler     /* IRQ 218 */
    .word   IRQ_219_Handler     /* IRQ 219 */
    .word   IRQ_220_Handler     /* IRQ 220 */
    .word   IRQ_221_Handler     /* IRQ 221 */
    .word   IRQ_222_Handler     /* IRQ 222 */
    .word   IRQ_223_Handler     /* IRQ 223 */
    .word   IRQ_224_Handler     /* IRQ 224 */
    .word   IRQ_225_Handler     /* IRQ 225 */
    .word   IRQ_226_Handler     /* IRQ 226 */
    .word   IRQ_227_Handler     /* IRQ 227 */
    .word   IRQ_228_Handler     /* IRQ 228 */
    .word   IRQ_229_Handler     /* IRQ 229 */
    .word   IRQ_230_Handler     /* IRQ 230 */
    .word   IRQ_231_Handler     /* IRQ 231 */
    .word   IRQ_232_Handler     /* IRQ 232 */
    .word   IRQ_233_Handler     /* IRQ 233 */
    .word   IRQ_234_Handler     /* IRQ 234 */
    .word   IRQ_235_Handler     /* IRQ 235 */
    .word   IRQ_236_Handler     /* IRQ 236 */
    .word   IRQ_237_Handler     /* IRQ 237 */
    .word   IRQ_238_Handler     /* IRQ 238 */
    .word   IRQ_239_Handler     /* IRQ 239 */
    .word   IRQ_240_Handler     /* IRQ 240 */
    .word   IRQ_241_Handler     /* IRQ 241 */
    .word   IRQ_242_Handler     /* IRQ 242 */
    .word   IRQ_243_Handler     /* IRQ 243 */
    .word   IRQ_244_Handler     /* IRQ 244 */
    .word   IRQ_245_Handler     /* IRQ 245 */
    .word   IRQ_246_Handler     /* IRQ 246 */
    .word   IRQ_247_Handler     /* IRQ 247 */
    .word   IRQ_248_Handler     /* IRQ 248 */
    .word   IRQ_249_Handler     /* IRQ 249 */
    .word   IRQ_250_Handler     /* IRQ 250 */
    .word   IRQ_251_Handler     /* IRQ 251 */
    .word   IRQ_252_Handler     /* IRQ 252 */
    .word   IRQ_253_Handler     /* IRQ 253 */
    .word   IRQ_254_Handler     /* IRQ 254 */
    .word   IRQ_255_Handler     /* IRQ 255 */


/* ═══════════════════════════════════════════════════════════════
 * Reset Handler
 *
 * Called on power-up / reset. Performs:
 *   1. Copy initialized data (.data) from FLASH to SRAM
 *   2. Zero-fill .bss section
 *   3. Initialize ITCM/DTCM (copy fast code/data from FLASH)
 *   4. Call constructors / init_array
 *   5. Call SystemInit() (clock tree, cache, FPU)
 *   6. Call main()
 *   7. If main() returns, loop forever
 * ═══════════════════════════════════════════════════════════════ */
    .section .text.Reset_Handler, "ax", %progbits
    .type   Reset_Handler, %function
    .align  2

    /* Ensure Thumb bit set on symbol (Cortex-M7 executes Thumb
     * only; official NXP code writes Reset_Handler+1 in the
     * vector table for the same reason) */
    .thumb_func
Reset_Handler:
    /* ── Disable interrupts during initialization ── */
    cpsid   i

    /* ═══════════════════════════════════════════════════════════
     * MSCM clock enable via MC_ME
     *
     * Sequence verified against official NXP startup_cm7.s
     * (S32K3_RTD_3_0_0).  MSCM must be clocked before any
     * peripheral access — RTD critical sections read MSCM
     * registers to determine the running core.
     * ═══════════════════════════════════════════════════════════ */
InitMSCMClock:
    ldr     r0, =0x402DC310        /* MCME_PRTN1_COFB0_STAT      */
    ldr     r1, [r0]
    ldr     r2, =(1 << 24)         /* MCME_MSCM_REQ              */
    ands    r1, r1, r2
    bne     MSCMClockEnabled

    ldr     r0, =0x402DC330        /* MCME_PRTN1_COFB0_CLKEN     */
    ldr     r1, [r0]
    orrs    r1, r1, r2
    str     r1, [r0]

    ldr     r0, =0x402DC304        /* MCME_PRTN1_PUPD            */
    ldr     r1, [r0]
    orrs    r1, r1, #1
    str     r1, [r0]

    ldr     r0, =0x402DC000        /* MCME_CTL_KEY               */
    ldr     r1, =0x5AF0
    str     r1, [r0]
    ldr     r1, =0xA50F
    str     r1, [r0]

MSCMClockEnabled:
    /* Wait for clock enable to take effect */
    ldr     r0, =0x402DC310        /* MCME_PRTN1_COFB0_STAT      */
    ldr     r2, =(1 << 24)
WaitMSCMClock:
    ldr     r1, [r0]
    ands    r1, r1, r2
    beq     WaitMSCMClock

    /* ═══════════════════════════════════════════════════════════
     * Disable SWT0 (internal watchdog 0)
     *
     * SWT_0 base 0x40270000.  Sequence from official startup:
     *   1. Unlock service register (0x40270010): 0xC520, 0xD928
     *   2. Write control register (0x40270000) = 0xFF000040
     * ═══════════════════════════════════════════════════════════ */
DisableSWT0:
    ldr     r0, =0x40270010
    ldr     r1, =0xC520
    str     r1, [r0]
    ldr     r1, =0xD928
    str     r1, [r0]
    ldr     r0, =0x40270000
    ldr     r1, =0xFF000040
    str     r1, [r0]

    /* ═══════════════════════════════════════════════════════════
     * Enable ITCM and DTCM (disable RETEN, set EN)
     *   CM7_ITCMCR = 0xE000EF90, CM7_DTCMCR = 0xE000EF94
     * ═══════════════════════════════════════════════════════════ */
EnableTCM:
    ldr     r1, =0xE000EF94        /* DTCMCR                    */
    ldr     r0, [r1]
    bic     r0, r0, #0x4           /* clear RETEN               */
    orr     r0, r0, #0x1           /* set EN                    */
    str     r0, [r1]
    ldr     r1, =0xE000EF90        /* ITCMCR                    */
    ldr     r0, [r1]
    bic     r0, r0, #0x4
    orr     r0, r0, #0x1
    str     r0, [r1]

    /* ═══════════════════════════════════════════════════════════
     * SRAM ECC initialization
     *
     * S32K3 SRAM is ECC-protected: reading a location whose ECC
     * has never been written faults.  Zero-fill the whole SRAM
     * (256 KB) with 64-bit stores before any use — including
     * the stack, which lives at the top of SRAM.
     * ═══════════════════════════════════════════════════════════ */
InitSRAMECC:
    ldr     r1, =__sram_start__    /* 0x20400000                */
    ldr     r2, =__sram_size__     /* 256 KB                    */
    subs    r2, r2, #8
SRAMLoop:
    movs    r0, #0
    movs    r3, #0
    stm     r1!, {r0, r3}
    subs    r2, r2, #8
    bge     SRAMLoop

    /* ── Copy .data section from FLASH to SRAM ──
     *    Source:      __etext (in FLASH)
     *    Destination: __data_start__ (in SRAM)
     *    Length:      __data_end__ - __data_start__
     */
    ldr     r0, =__etext
    ldr     r1, =__data_start__
    ldr     r2, =__data_end__
    subs    r2, r2, r1            /* r2 = data section length */
    beq     2f                    /* skip if zero-length */
1:
    ldrb    r3, [r0], #1
    strb    r3, [r1], #1
    subs    r2, r2, #1
    bne     1b
2:

    /* ── Zero-fill .bss section ── */
    ldr     r0, =__bss_start__
    ldr     r1, =__bss_end__
    subs    r1, r1, r0            /* r1 = bss section length */
    beq     2f                    /* skip if zero-length */
    movs    r2, #0
1:
    strb    r2, [r0], #1
    subs    r1, r1, #1
    bne     1b
2:

    /* ── Copy ITCM code from FLASH if present ── */
    ldr     r0, =__itcm_start__
    ldr     r1, =__itcm_load_start__
    ldr     r2, =__itcm_load_end__
    subs    r2, r2, r1
    beq     2f
1:
    ldrb    r3, [r1], #1
    strb    r3, [r0], #1
    subs    r2, r2, #1
    bne     1b
2:

    /* ── Copy DTCM data from FLASH if present ── */
    ldr     r0, =__dtcm_start__
    ldr     r1, =__dtcm_load_start__
    ldr     r2, =__dtcm_load_end__
    subs    r2, r2, r1
    beq     2f
1:
    ldrb    r3, [r1], #1
    strb    r3, [r0], #1
    subs    r2, r2, #1
    bne     1b
2:

    /* ── Enable FPU (CP10 and CP11 full access) ──
     *    Cortex-M7 CPACR is at address 0xE000ED88
     *    Set CP10[23:22] = 0b11 and CP11[21:20] = 0b11
     */
    ldr     r0, =0xE000ED88
    ldr     r1, [r0]
    orr     r1, r1, #(0xF << 20)
    str     r1, [r0]
    dsb
    isb

    /* ── Call SystemInit() — clock tree, cache ── */
    bl      SystemInit

    /* ── Re-enable interrupts ── */
    cpsie   i

    /* ── Call static constructors ── */
    bl      __libc_init_array

    /* ── Call main() ── */
    bl      main

    /* ── If main() returns, loop forever ── */
    b       .

    .size   Reset_Handler, . - Reset_Handler


/* ═══════════════════════════════════════════════════════════════
 * Default Exception / IRQ Handlers
 *
 * All handlers are defined WEAK so application code can override.
 * Default behavior: infinite loop (trap).
 * HardFault_Handler saves stacked PC for debugging.
 * ═══════════════════════════════════════════════════════════════ */

    .section .text.Default_Handler, "ax", %progbits
    .thumb_func
    .type   Default_Handler, %function
Default_Handler:
    b       .                       /* Infinite loop — trap */
    .size   Default_Handler, . - Default_Handler

    /* Macro to declare a weak handler alias to Default_Handler */
    .macro  def_irq_handler  handler_name
    .thumb_func
    .weak   \handler_name
    .type   \handler_name, %function
\handler_name:
    b       Default_Handler
    .size   \handler_name, . - \handler_name
    .endm

    /* Special: HardFault_Handler — save stacked PC for debug */
    .section .text.HardFault_Handler, "ax", %progbits
    .thumb_func
    .type   HardFault_Handler, %function
HardFault_Handler:
    /* On entry to HardFault, the processor pushes {r0-r3, r12, LR, PC, xPSR}
     * onto the stack. Read the stacked PC at offset 24 (0x18) bytes above SP. */
    tst     lr, #4              /* Check EXC_RETURN bit 2 to determine MSP vs PSP */
    ite     eq
    mrseq   r0, msp
    mrsne   r0, psp
    ldr     r1, [r0, #24]       /* Stacked PC */
    b       .                   /* Trap here; inspect r1 = faulting PC */
    .size   HardFault_Handler, . - HardFault_Handler

    /* ── Other system exception handlers ── */
    def_irq_handler  NMI_Handler
    def_irq_handler  MemManage_Handler
    def_irq_handler  BusFault_Handler
    def_irq_handler  UsageFault_Handler
    def_irq_handler  SVC_Handler
    def_irq_handler  DebugMon_Handler
    def_irq_handler  PendSV_Handler
    def_irq_handler  SysTick_Handler

    /* ── Device IRQ handlers (IRQ 0 – 255) ── */
    def_irq_handler  IRQ_000_Handler
    def_irq_handler  IRQ_001_Handler
    def_irq_handler  IRQ_002_Handler
    def_irq_handler  IRQ_003_Handler
    def_irq_handler  IRQ_004_Handler
    def_irq_handler  IRQ_005_Handler
    def_irq_handler  IRQ_006_Handler
    def_irq_handler  IRQ_007_Handler
    def_irq_handler  IRQ_008_Handler
    def_irq_handler  IRQ_009_Handler
    def_irq_handler  IRQ_010_Handler
    def_irq_handler  IRQ_011_Handler
    def_irq_handler  IRQ_012_Handler
    def_irq_handler  IRQ_013_Handler
    def_irq_handler  IRQ_014_Handler
    def_irq_handler  IRQ_015_Handler
    def_irq_handler  IRQ_016_Handler
    def_irq_handler  IRQ_017_Handler
    def_irq_handler  IRQ_018_Handler
    def_irq_handler  IRQ_019_Handler
    def_irq_handler  IRQ_020_Handler
    def_irq_handler  IRQ_021_Handler
    def_irq_handler  IRQ_022_Handler
    def_irq_handler  IRQ_023_Handler
    def_irq_handler  IRQ_024_Handler
    def_irq_handler  IRQ_025_Handler
    def_irq_handler  IRQ_026_Handler
    def_irq_handler  IRQ_027_Handler
    def_irq_handler  IRQ_028_Handler
    def_irq_handler  IRQ_029_Handler
    def_irq_handler  IRQ_030_Handler
    def_irq_handler  IRQ_031_Handler
    def_irq_handler  IRQ_032_Handler
    def_irq_handler  IRQ_033_Handler
    def_irq_handler  IRQ_034_Handler
    def_irq_handler  IRQ_035_Handler
    def_irq_handler  IRQ_036_Handler
    def_irq_handler  IRQ_037_Handler
    def_irq_handler  IRQ_038_Handler
    def_irq_handler  IRQ_039_Handler
    def_irq_handler  IRQ_040_Handler
    def_irq_handler  IRQ_041_Handler
    def_irq_handler  IRQ_042_Handler
    def_irq_handler  IRQ_043_Handler
    def_irq_handler  IRQ_044_Handler
    def_irq_handler  IRQ_045_Handler
    def_irq_handler  IRQ_046_Handler
    def_irq_handler  IRQ_047_Handler
    def_irq_handler  IRQ_048_Handler
    def_irq_handler  IRQ_049_Handler
    def_irq_handler  IRQ_050_Handler
    def_irq_handler  IRQ_051_Handler
    def_irq_handler  IRQ_052_Handler
    def_irq_handler  IRQ_053_Handler
    def_irq_handler  IRQ_054_Handler
    def_irq_handler  IRQ_055_Handler
    def_irq_handler  IRQ_056_Handler
    def_irq_handler  IRQ_057_Handler
    def_irq_handler  IRQ_058_Handler
    def_irq_handler  IRQ_059_Handler
    def_irq_handler  IRQ_060_Handler
    def_irq_handler  IRQ_061_Handler
    def_irq_handler  IRQ_062_Handler
    def_irq_handler  IRQ_063_Handler
    def_irq_handler  IRQ_064_Handler
    def_irq_handler  IRQ_065_Handler
    def_irq_handler  IRQ_066_Handler
    def_irq_handler  IRQ_067_Handler
    def_irq_handler  IRQ_068_Handler
    def_irq_handler  IRQ_069_Handler
    def_irq_handler  IRQ_070_Handler
    def_irq_handler  IRQ_071_Handler
    def_irq_handler  IRQ_072_Handler
    def_irq_handler  IRQ_073_Handler
    def_irq_handler  IRQ_074_Handler
    def_irq_handler  IRQ_075_Handler
    def_irq_handler  IRQ_076_Handler
    def_irq_handler  IRQ_077_Handler
    def_irq_handler  IRQ_078_Handler
    def_irq_handler  IRQ_079_Handler
    def_irq_handler  IRQ_080_Handler
    def_irq_handler  IRQ_081_Handler
    def_irq_handler  IRQ_082_Handler
    def_irq_handler  IRQ_083_Handler
    def_irq_handler  IRQ_084_Handler
    def_irq_handler  IRQ_085_Handler
    def_irq_handler  IRQ_086_Handler
    def_irq_handler  IRQ_087_Handler
    def_irq_handler  IRQ_088_Handler
    def_irq_handler  IRQ_089_Handler
    def_irq_handler  IRQ_090_Handler
    def_irq_handler  IRQ_091_Handler
    def_irq_handler  IRQ_092_Handler
    def_irq_handler  IRQ_093_Handler
    def_irq_handler  IRQ_094_Handler
    def_irq_handler  IRQ_095_Handler
    def_irq_handler  IRQ_096_Handler
    def_irq_handler  IRQ_097_Handler
    def_irq_handler  IRQ_098_Handler
    def_irq_handler  IRQ_099_Handler
    def_irq_handler  IRQ_100_Handler
    def_irq_handler  IRQ_101_Handler
    def_irq_handler  IRQ_102_Handler
    def_irq_handler  IRQ_103_Handler
    def_irq_handler  IRQ_104_Handler
    def_irq_handler  IRQ_105_Handler
    def_irq_handler  IRQ_106_Handler
    def_irq_handler  IRQ_107_Handler
    def_irq_handler  IRQ_108_Handler
    def_irq_handler  IRQ_109_Handler
    def_irq_handler  IRQ_110_Handler
    def_irq_handler  IRQ_111_Handler
    def_irq_handler  IRQ_112_Handler
    def_irq_handler  IRQ_113_Handler
    def_irq_handler  IRQ_114_Handler
    def_irq_handler  IRQ_115_Handler
    def_irq_handler  IRQ_116_Handler
    def_irq_handler  IRQ_117_Handler
    def_irq_handler  IRQ_118_Handler
    def_irq_handler  IRQ_119_Handler
    def_irq_handler  IRQ_120_Handler
    def_irq_handler  IRQ_121_Handler
    def_irq_handler  IRQ_122_Handler
    def_irq_handler  IRQ_123_Handler
    def_irq_handler  IRQ_124_Handler
    def_irq_handler  IRQ_125_Handler
    def_irq_handler  IRQ_126_Handler
    def_irq_handler  IRQ_127_Handler
    def_irq_handler  IRQ_128_Handler
    def_irq_handler  IRQ_129_Handler
    def_irq_handler  IRQ_130_Handler
    def_irq_handler  IRQ_131_Handler
    def_irq_handler  IRQ_132_Handler
    def_irq_handler  IRQ_133_Handler
    def_irq_handler  IRQ_134_Handler
    def_irq_handler  IRQ_135_Handler
    def_irq_handler  IRQ_136_Handler
    def_irq_handler  IRQ_137_Handler
    def_irq_handler  IRQ_138_Handler
    def_irq_handler  IRQ_139_Handler
    def_irq_handler  IRQ_140_Handler
    def_irq_handler  IRQ_141_Handler
    def_irq_handler  IRQ_142_Handler
    def_irq_handler  IRQ_143_Handler
    def_irq_handler  IRQ_144_Handler
    def_irq_handler  IRQ_145_Handler
    def_irq_handler  IRQ_146_Handler
    def_irq_handler  IRQ_147_Handler
    def_irq_handler  IRQ_148_Handler
    def_irq_handler  IRQ_149_Handler
    def_irq_handler  IRQ_150_Handler
    def_irq_handler  IRQ_151_Handler
    def_irq_handler  IRQ_152_Handler
    def_irq_handler  IRQ_153_Handler
    def_irq_handler  IRQ_154_Handler
    def_irq_handler  IRQ_155_Handler
    def_irq_handler  IRQ_156_Handler
    def_irq_handler  IRQ_157_Handler
    def_irq_handler  IRQ_158_Handler
    def_irq_handler  IRQ_159_Handler
    def_irq_handler  IRQ_160_Handler
    def_irq_handler  IRQ_161_Handler
    def_irq_handler  IRQ_162_Handler
    def_irq_handler  IRQ_163_Handler
    def_irq_handler  IRQ_164_Handler
    def_irq_handler  IRQ_165_Handler
    def_irq_handler  IRQ_166_Handler
    def_irq_handler  IRQ_167_Handler
    def_irq_handler  IRQ_168_Handler
    def_irq_handler  IRQ_169_Handler
    def_irq_handler  IRQ_170_Handler
    def_irq_handler  IRQ_171_Handler
    def_irq_handler  IRQ_172_Handler
    def_irq_handler  IRQ_173_Handler
    def_irq_handler  IRQ_174_Handler
    def_irq_handler  IRQ_175_Handler
    def_irq_handler  IRQ_176_Handler
    def_irq_handler  IRQ_177_Handler
    def_irq_handler  IRQ_178_Handler
    def_irq_handler  IRQ_179_Handler
    def_irq_handler  IRQ_180_Handler
    def_irq_handler  IRQ_181_Handler
    def_irq_handler  IRQ_182_Handler
    def_irq_handler  IRQ_183_Handler
    def_irq_handler  IRQ_184_Handler
    def_irq_handler  IRQ_185_Handler
    def_irq_handler  IRQ_186_Handler
    def_irq_handler  IRQ_187_Handler
    def_irq_handler  IRQ_188_Handler
    def_irq_handler  IRQ_189_Handler
    def_irq_handler  IRQ_190_Handler
    def_irq_handler  IRQ_191_Handler
    def_irq_handler  IRQ_192_Handler
    def_irq_handler  IRQ_193_Handler
    def_irq_handler  IRQ_194_Handler
    def_irq_handler  IRQ_195_Handler
    def_irq_handler  IRQ_196_Handler
    def_irq_handler  IRQ_197_Handler
    def_irq_handler  IRQ_198_Handler
    def_irq_handler  IRQ_199_Handler
    def_irq_handler  IRQ_200_Handler
    def_irq_handler  IRQ_201_Handler
    def_irq_handler  IRQ_202_Handler
    def_irq_handler  IRQ_203_Handler
    def_irq_handler  IRQ_204_Handler
    def_irq_handler  IRQ_205_Handler
    def_irq_handler  IRQ_206_Handler
    def_irq_handler  IRQ_207_Handler
    def_irq_handler  IRQ_208_Handler
    def_irq_handler  IRQ_209_Handler
    def_irq_handler  IRQ_210_Handler
    def_irq_handler  IRQ_211_Handler
    def_irq_handler  IRQ_212_Handler
    def_irq_handler  IRQ_213_Handler
    def_irq_handler  IRQ_214_Handler
    def_irq_handler  IRQ_215_Handler
    def_irq_handler  IRQ_216_Handler
    def_irq_handler  IRQ_217_Handler
    def_irq_handler  IRQ_218_Handler
    def_irq_handler  IRQ_219_Handler
    def_irq_handler  IRQ_220_Handler
    def_irq_handler  IRQ_221_Handler
    def_irq_handler  IRQ_222_Handler
    def_irq_handler  IRQ_223_Handler
    def_irq_handler  IRQ_224_Handler
    def_irq_handler  IRQ_225_Handler
    def_irq_handler  IRQ_226_Handler
    def_irq_handler  IRQ_227_Handler
    def_irq_handler  IRQ_228_Handler
    def_irq_handler  IRQ_229_Handler
    def_irq_handler  IRQ_230_Handler
    def_irq_handler  IRQ_231_Handler
    def_irq_handler  IRQ_232_Handler
    def_irq_handler  IRQ_233_Handler
    def_irq_handler  IRQ_234_Handler
    def_irq_handler  IRQ_235_Handler
    def_irq_handler  IRQ_236_Handler
    def_irq_handler  IRQ_237_Handler
    def_irq_handler  IRQ_238_Handler
    def_irq_handler  IRQ_239_Handler
    def_irq_handler  IRQ_240_Handler
    def_irq_handler  IRQ_241_Handler
    def_irq_handler  IRQ_242_Handler
    def_irq_handler  IRQ_243_Handler
    def_irq_handler  IRQ_244_Handler
    def_irq_handler  IRQ_245_Handler
    def_irq_handler  IRQ_246_Handler
    def_irq_handler  IRQ_247_Handler
    def_irq_handler  IRQ_248_Handler
    def_irq_handler  IRQ_249_Handler
    def_irq_handler  IRQ_250_Handler
    def_irq_handler  IRQ_251_Handler
    def_irq_handler  IRQ_252_Handler
    def_irq_handler  IRQ_253_Handler
    def_irq_handler  IRQ_254_Handler
    def_irq_handler  IRQ_255_Handler

    .end
