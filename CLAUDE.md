# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

AB169D (中科蓝讯 Bluetrum) RISC-V MCU 语音控制智能灯固件。芯片型号 AB169D，RISC-V RV32IMAC @160MHz，512KB Flash + 56KB RAM，SOP8 封装。

关键词：**语音关键词识别 (KWS)**、智能灯控、PWM 调光、MIC 音频采集。

## Build & Flash

### Toolchain
- **IDE**: CodeBlocks (工程文件 `app/projects/zbit_asr/app-kws.cbp`)
- **Compiler**: `riscv32-elf-gcc` (march=rv32imac)
- **烧录工具**: AB16X Downloader V3.3.7 (授权码 `3CC9B441`, 私钥 `zbit_auth_private_test123`)
- **USB 驱动**: CP210x

### Build
CodeBlocks 中打开 `app-kws.cbp`，Build → Build 或按 Ctrl+F9。

编译流程（prebuild/postbuild 自动执行）：
1. `riscv32-elf-xmaker -b res.xm` — 生成资源文件
2. `riscv32-elf-xmaker -b xcfg.xm` — 生成配置
3. `riscv32-elf-objcopy -O binary app.rv32 app.bin` — 转二进制
4. `riscv32-elf-xmaker -b appxm.o` — 生成 .dcf/.prd

### Flash
使用 Downloader V3.3.7，选择 `Output/bin/app.prd` 烧录。
- 4 线烧录：PA2(UTX) + PA4(URX) + 3.3V + GND
- 波特率 1.5M
- 新芯片如报 KEY 校验错误，在 Downloader 中填入授权码 `3CC9B441`

### Output Files
- `Output/bin/app.prd` — 最终烧录固件
- `Output/bin/app.dcf` — 中间格式
- `Output/bin/app.rv32` — ELF 二进制
- `Output/bin/map.txt` — 链接映射表
- `Output/bin/header.bin` — 芯片配置头

## Architecture

### Board Variants (Two Boards)

| | 旧板（测试板） | 新板（产品板） |
|---|---|---|
| 原理图 | `reference/schematics/schematic-analysis-1784720854279.md` | `reference/schematics/schematic-analysis-1784770650881.md` |
| 功能 | 完整外设：8002功放+喇叭、按键、调试口、LED | 精简：RGB LED + 暖白灯组 + 冷白灯组 + MIC |
| 烧录/调试 | 有 J2 调试接口 | 无调试接口（需烧录好再焊） |
| 状态 | 功能已调通 ✅ | 待验证 |

**工作流**：旧板调好 → 取下芯片 → 焊到新板 → 验证。

### Project Structure

```
app/
├── platform/                # SDK 平台层（不修改或少修改）
│   ├── bsp/                     # 板级支持包 (BSP 驱动)
│   │   ├── bsp_asr.c/.h            # KWS 语音识别主处理（核心）
│   │   ├── bsp_audio.c             # MIC/音频通路配置
│   │   ├── bsp_dac.c               # DAC 驱动
│   │   ├── bsp_uart.c / bsp_i2c.c  # 通信外设
│   │   └── bsp_sdadc.c             # Σ-Δ ADC (MIC)
│   ├── modules/                 # 功能模块
│   │   └── zbit_kws/               # KWS 算法库
│   │       ├── zbitkws.h               # KWS API 头
│   │       ├── libkws_AB169_436_...a   # KWS 静态库 (V3.1.0, 17词)
│   │       ├── libkws_AB169_436_...c   # 关键词表 + 阈值
│   │       └── XYC-L124-...*           # 旧版算法库 (V3.0.0, 15词) 
│   └── header/                  # 芯片寄存器定义
│       ├── sfr.h                    # 特殊功能寄存器
│       └── io_def.h                 # GPIO 复用定义
├── projects/
│   └── zbit_asr/               # 项目工程
│       ├── app-kws.cbp             # CodeBlocks 工程文件
│       ├── config.h                # 系统配置（主配置文件）
│       ├── config.c                # 用户参数配置
│       ├── main.c                  # 入口
│       ├── ram.ld                  # 链接脚本
│       ├── led_yw.c / led_ctrl.h   # LED 控制逻辑（核心应用）
│       ├── functions/              # 功能模块
│       │   ├── func.c                 # 主状态机调度
│       │   ├── func_speaker.c         # Speaker 模式（含 KWS 激活）
│       │   └── func_lowpwr.c          # 电源管理/休眠
│       └── port/                  # 板级端口配置
│           ├── port_pwm.c            # PWM 初始化
│           └── port_key.c            # 按键配置
├── platform/header/config_define.h  # 系统常量定义
└── platform/header/config_extra.h    # 额外配置
```

### Key Source Files

| File | Purpose |
|------|---------|
| `bsp_asr.c` | **KWS 核心**：SDADC 中断回调 → `kws_load_input()` → `kws_classify()` → `led_ctrl()` |
| `led_yw.c` | **LED 状态机**：开/关、调光(5级)、调色(黄/白/黄白)、定时(10/30/60min) |
| `config.h` | **项目配置总开关**：ASR_RECOG_EN、时钟、外设使能、Flash 分区 |
| `func_speaker.c` | Speaker 模式主循环、音频通路初始化 |
| `port_pwm.c` | TIMER2 PWM 初始化（25KHz，4通道） |
| `ram.ld` | 内存布局：sram0+sram1 分区、com/bank 机制 |
| `app.xm` | **固件打包配置**：授权码 setauth、weight 资源、flash 分区 |

### KWS Pipeline

```
MIC (PA0) → SDADC DMA (16kHz) → kws_load_input() → kws_classify()
    ↑每48ms触发一次                              ↓
    asr_sdadc_process()                    top(命令ID) + prob(置信度)
                                                ↓
                                         prob ≥ KW_TRG_MODE[top]?
                                            ↓           ↓
                                         执行        忽略
                                      led_ctrl(top)
```

- **17 个关键词**（V3.1.0 新库）：小爱小爱(唤醒)、开灯、关灯、回来了、出去了、睡觉了、改变颜色、变颜色、换颜色、亮一点、暗一点、暖色光、白色光、自然光、中等亮度、最大亮度、最小亮度
- **唤醒词机制**：`NUM_WW=16` 表示 index 0 "小爱小爱" 为唤醒词，`WW_TRG_MODE` 用于唤醒判定
- **每帧** 384 采样点 (128×3)，帧间隔 ~24ms
- 每 48ms 一次完整识别循环（SDADC DMA 中断频率）

### Product Board GPIO Map (新板)

| MCU Pin | 网络名 | 驱动对象 |
|---------|--------|---------|
| Pin1 | VDD | 电源输入 (USB 5V→2.2Ω→VDD) |
| Pin2 | ? | 未标注 |
| Pin3 | VIO | IO 电平参考 |
| Pin4 | GREEN | RGB 绿色 LED（直接 GPIO 驱动，无串联电阻） |
| Pin5 | RED/W | RGB 红色 / Q2 驱动白色灯组 |
| Pin6 | BLUE/Y | RGB 蓝色 / Q1 驱动暖色灯组 |
| Pin7 | GND | 地 |
| Pin8 | MIC | 麦克风输入 (ADC) |

## Reference Materials

`reference/` 目录下存放了驱动开发需参考的资料：

| 路径 | 内容 |
|------|------|
| `reference/datasheets/` | 8002 功放数据手册（FM/NS/8002A/AiP 多个品牌） |
| `reference/schematics/` | 新旧板原理图分析报告 |
| `AB169C_AB169D DataSheet.pdf` | 主控芯片数据手册 |
| `AB16X_SDK开发手册(1).pdf` | SDK 开发手册（94页，含 API、硬件设计参考） |

## Important Notes

- **led_ctrl(top)** 当前为注释状态，新编译固件需启用
- 新板 RGB LED **无串联限流电阻**，GPIO 源电流有限，PWM 占空比不可设为 100%
- KWS 库使用 int16_t 类型（V3.1.0），旧版为 int32_t
- 授权码 `3CC9B441` + 私钥 `zbit_auth_private_test123` 在 `app.xm` 中通过 `setauth()` 配置
- weight model 在 flash 中通过 `setuserbin()` 定义地址
