# Resource Checklist

Use this checklist to decide what a complete chip development resource package should contain. Not every chip has every item; mark important missing items in the final risk section instead of inventing them.

## 输出分区

最终 HTML 报告使用 clean technical dashboard 风格，并包含三个中文表格：

- 硬件开发资料
- 软件开发资料
- 烧录编程 / 量产测试资料

同一份资料可以出现在多个表格中，但备注要说明它在该阶段的用途。聊天回复只给简短摘要和 HTML 文件链接，不要重复粘贴完整表格。

## MCU/SoC 必需资料

| 类别 | 常见名称 | 用途 |
|---|---|---|
| 数据手册 | datasheet, product specification | 电气限制、引脚、封装、存储大小、订货信息 |
| 参考手册 | reference manual, technical reference manual, user manual | 外设寄存器、时钟树、复位行为、总线架构 |
| 编程手册 | programming manual, CPU core manual, debug manual | 内核异常、Flash 编程、调试行为 |
| SDK/HAL/BSP | SDK, HAL, LL, BSP, firmware library | 启动代码、外设驱动、中间件、板级支持 |
| 例程 | examples, demos, sample code | 可参考的工程结构和外设初始化 |
| 工具链支持 | Keil pack, IAR support, GCC/CMake, PlatformIO, IDE extension | 编译、烧录、调试、IDE 配置 |
| 烧录/调试支持 | flash algorithm, programmer, OpenOCD/J-Link scripts, CMSIS-DAP notes | 下载和调试可靠性 |
| 勘误表 | errata, silicon limitations | 芯片已知问题和 workaround |

## 硬件开发资料

- 产品页
- 数据手册
- 硬件设计指南
- 参考原理图
- 开发板手册
- PCB/layout 指南
- 引脚复用表
- CAD 文件、封装、3D 模型

## 软件开发资料

- 参考手册 / 用户手册
- 编程手册
- SDK/HAL/BSP
- 例程
- 中间件
- SVD/寄存器元数据
- 应用笔记
- 启动文件和链接脚本

## 烧录编程 / 量产测试资料

- 烧录工具
- Flash 编程手册
- Bootloader / ISP / DFU 文档
- 调试器/下载器说明
- 安全启动、读保护、加密说明
- 产测工具、批量烧录工具、校准文档

## 无线/模组补充

- RF 硬件设计指南
- 天线设计/layout 指南
- 认证/法规说明
- 主机接口协议或 AT 命令手册
- 协议栈和共存说明
- 产测、射频校准、MAC/address 编程文档

## 外设芯片补充

- 寄存器表或命令集
- 总线时序和事务图
- 参考驱动或 Linux 驱动
- 评估板文件
- 校准流程和 NVM 说明
- 中断/告警行为和故障状态定义

## 最小下载集

对于典型 MCU，最少应有：

1. 数据手册。
2. 参考手册或用户手册。
3. 勘误表。
4. SDK/HAL/BSP。
5. 至少一个可运行例程。
6. 开发板原理图或最小系统参考设计。
7. 烧录/调试说明。
