# LLM 修改边界配置

本文件放在项目 `.ai/` 目录下，由用户在项目接入 LLM 辅助开发前填写，用于防止 LLM 修改不允许修改的位置。配套规则见 `.ai/LLM_RULES.md`。

---

> **使用说明（LLM 生成时阅读）：**
> 以下内容为模板。`{{...}}` 标记处需要替换为项目实际路径。
> `LLM` 在初始化时应自动分析项目结构并填入实际路径，而非保留示例。
> 根据项目类型选择对应模板章节，删除不适用的。

---

## 执行原则

- 未显式允许修改的文件，默认禁止修改。
- 每次任务必须列出 `Allowed Files`。
- 每次修改文件前，必须先输出本次任务边界和最小验证方式，并等待用户确认。
- 如果任务需要修改禁止区域，必须单独请求用户确认。
- 用户确认必须具体到文件或目录，不能泛化成整个工程可改。
- 已上板验证、生产验证、时序敏感代码默认只读。
- HAL/CMSIS/厂商 SDK 既有文件默认只读；仅允许按确认边界新增缺失的同版本官方库文件。

---

## STM32CubeMX 项目模板

当检测到 `.ioc` 文件时使用此模板。

### 绝对禁止修改

```text
{{startup 文件，如 startup_stm32f103xe.s}}
{{链接脚本，如 STM32F103XX_FLASH.ld}}
{{HAL 驱动目录中的既有文件，如 Drivers/STM32F1xx_HAL_Driver/}}
{{CMSIS 目录中的既有文件，如 Drivers/CMSIS/}}
{{CubeMX CMake/构建文件，如 cmake/stm32cubemx/CMakeLists.txt}}
{{工具链文件，如 cmake/gcc-arm-none-eabi.cmake}}
{{系统文件，如 Core/Src/system_stm32f1xx.c, sysmem.c, syscalls.c}}
{{.ioc 文件}}
```

### 默认允许修改

```text
{{应用源文件目录，如 Core/Src/}}       (仅 USER CODE 区域)
{{应用头文件目录，如 Core/Inc/}}       (仅 USER CODE 区域)
{{应用源文件目录}}/(新增用户文件)        (不受 USER CODE 限制)
{{根 CMakeLists.txt}}                  (添加源文件和包含路径)
.ai/STM32_RESOURCE_TABLE.md            (添加或调整 STM32F1 外设时必须更新)
app/
tests/
docs/
```

### 需要用户单独确认后才能修改

```text
{{main.c}}                  (非 USER CODE 区域，如 SystemClock_Config)
{{gpio.c 等外设初始化文件}}   (非 USER CODE 区域，如 MX_GPIO_Init)
{{中断处理文件，如 stm32f1xx_it.c}}
{{HAL MSP 文件，如 stm32f1xx_hal_msp.c}}
{{HAL 配置头文件，如 stm32f1xx_hal_conf.h}}
{{缺失的 HAL/CMSIS 官方库文件}} (仅允许从同版本官方包复制新增；禁止修改既有文件)
CMakePresets.json
```

### HAL/CMSIS 缺失库文件补齐规则

CubeMX 或厂商工程缺少 HAL/CMSIS 官方库文件时，允许在用户确认后补齐。规则：

- 只能新增缺失文件，不能编辑、覆盖、格式化或重排 HAL/CMSIS 目录中的既有文件。
- 新增文件必须来自与工程匹配的官方包版本；禁止手写、拼接或从未知来源复制。
- 本轮 `Allowed Files` 必须逐项列出新增目标路径、来源包名、来源版本和源文件路径。
- 复制后只做构建验证和必要的包含路径/源文件列表检查；需要改变 HAL/CMSIS 行为时，改走用户代码封装、工程配置或重新生成代码。

### STM32F1 HAL / STM32F1 CubeMX 专用边界

```text
*_template.c                         (禁止添加、引用或编译)
{{main.c}}                            (仅 USER CODE 区域，且只添加函数调用)
{{stm32f1xx_hal_msp.c}}               (仅经用户确认后修改 MSP 初始化)
{{stm32f1xx_it.c}}                    (仅经用户确认后修改 IRQHandler)
{{stm32f1xx_hal_conf.h}}              (仅经用户确认后修改 HAL 模块开关)
{{gpio.c/usart.c/spi.c 等生成文件}}    (仅 USER CODE 区域；区域外改动应走 CubeMX)
{{.ai/STM32_RESOURCE_TABLE.md}}        (添加、删除、调整外设后必须更新)
```

STM32F1 外设资源规则：

- `main.c` 不写 GPIO、DMA、NVIC、AFIO remap、外设时钟或 HAL 参数配置。
- 外设相关 GPIO 复用、外设时钟、DMA、NVIC、AFIO remap 优先集中在 `stm32f1xx_hal_msp.c` 的 `HAL_xxx_MspInit()` 中；普通 GPIO 输入输出初始化可保留在 `MX_GPIO_Init()`。
- 所有 `IRQHandler` 放在 `stm32f1xx_it.c`，并调用对应 `HAL_xxx_IRQHandler()`。
- 不随意修改 `SystemClock_Config()`、SWD/JTAG 配置、HSE 引脚和 AFIO/SWJ remap。
- 每个外设只能选择一种 remap，不能同时保留默认映射、partial remap、full remap 多套配置。
- 使用 DMA 时，`MX_DMA_Init()` 必须早于使用 DMA 的外设 `MX_xxx_Init()` 调用。

### CubeMX USER CODE 说明

CubeMX 生成的文件中有 `USER CODE BEGIN xxx` / `USER CODE END xxx` 标记。规则：

- LLM 只能在 `USER CODE BEGIN/END` 区域内修改代码。
- 区域外的修改在下次 CubeMX 生成时会被覆盖。
- 如需修改区域外的内容（如新增外设、改引脚、改时钟），引导用户在 CubeMX 中操作并重新生成。
- CubeMX 重新生成后，检查本文件中的路径是否需要更新。

### 主入口和主循环保护点

```text
{{main.c}}: HAL_Init()
{{main.c}}: SystemClock_Config()
{{main.c}}: MX_GPIO_Init()
{{main.c}}: while (1) { ... }
```

规则：

- 不删除已有调用。
- 不重排已有调用。
- 测试代码只允许添加带注释的函数调用。
- 调试时只允许临时屏蔽，并备注原因、日期和恢复条件。

---

## ESP-IDF 项目模板

当检测到 `sdkconfig` 文件时使用此模板。

### 绝对禁止修改

```text
sdkconfig
sdkconfig.defaults
components/esp_common/
components/esp_system/
components/freertos/
components/hal/
components/soc/
tools/
```

### 默认允许修改

```text
main/
components/{{项目自定义组件}}/
tests/
docs/
CMakeLists.txt              (根目录，添加组件和源文件)
```

### 需要用户单独确认后才能修改

```text
Kconfig.projbuild
sdkconfig.defaults
main/main.c                 (app_main 函数结构)
```

---

## 裸机 / 通用项目模板

无 IDE 标识文件时使用此模板。

### 绝对禁止修改

```text
{{startup 文件}}
{{链接脚本}}
{{厂商 SDK/HAL 目录中的既有文件}}
{{板级支持包目录}}
```

### 默认允许修改

```text
{{应用代码目录}}
app/
tests/
docs/
Makefile / CMakeLists.txt   (仅添加源文件部分)
```

### 需要用户单独确认后才能修改

```text
{{main.c}}
{{中断处理文件}}
{{外设驱动文件}}
{{缺失的厂商 SDK/HAL/CMSIS 官方库文件}} (仅允许从同版本官方包复制新增；禁止修改既有文件)
```

---

## 通用章节（所有项目类型适用）

### 已验证代码位置

```text
# 暂无。随着上板验证逐步添加。
# 示例：
# {{drivers/sensor.c}} — VERIFIED_ON_HARDWARE: {{YYYY-MM-DD}}
```

保护标记：

```text
VERIFIED_ON_HARDWARE
DO_NOT_TOUCH_TIMING
LOGIC_ANALYZER_VERIFIED
OSCILLOSCOPE_VERIFIED
PRODUCTION_VERIFIED
```

### 测试代码允许放置位置

```text
tests/
{{应用源文件目录}}/test_*.c
```

### 生成代码允许放置位置

```text
{{应用源文件目录}}
{{应用头文件目录}}
app/
```

### 本次任务边界

每次让 LLM 修改代码前，用户应填写：

```text
Allowed Files:
-

Forbidden Files / Areas:
-

Protected Verified Code:
-

Temporary Debug Allowed:
- yes/no

Need User Confirmation Before Touching:
-
```

如果用户没有填写，LLM 必须根据项目结构先提出建议边界和最小验证方式，并等待用户确认后再修改。
