# LLM 修改边界配置

本文件放在项目 `.ai/` 目录下，用于防止 LLM 修改不允许修改的位置。配套规则见 `.ai/LLM_RULES.md`。

---

## 执行原则

- 未显式允许修改的文件，默认禁止修改。
- 每次任务必须列出 `Allowed Files`。
- 每次修改文件前，必须先输出本次任务边界和最小验证方式，并等待用户确认。
- 如果任务需要修改禁止区域，必须单独请求用户确认。
- 已上板验证、生产验证、时序敏感代码默认只读。
- SDK 平台层（`app/platform/`）中的 BSP/库既有文件默认只读。

---

## 项目类型

裸机 / 通用项目（Bluetrum AB169D RISC-V SDK，CodeBlocks IDE）

### 绝对禁止修改

```text
app/projects/zbit_asr/s_start.S             (启动文件)
app/projects/zbit_asr/ram.ld                 (链接脚本)
app/platform/header/sfr.h                    (芯片寄存器定义)
app/platform/header/include.h               (全局包含链)
app/platform/libs/                           (SDK 预编译库文件)
app/platform/modules/zbit_kws/*.a            (KWS 算法静态库)
app/projects/zbit_asr/Output/bin/app.xm      (固件打包配置，含授权码)
app/projects/zbit_asr/Output/bin/header.bin  (芯片配置头)
```

### 默认允许修改

```text
app/projects/zbit_asr/                       (应用层代码目录)
app/projects/zbit_asr/led_yw.c               (LED 控制逻辑)
app/projects/zbit_asr/led_ctrl.h             (LED 控制头文件)
app/projects/zbit_asr/config.h               (系统配置主文件)
app/projects/zbit_asr/config.c               (用户参数配置)
app/projects/zbit_asr/functions/             (功能模块)
app/projects/zbit_asr/port/                  (板级端口配置)
app/projects/zbit_asr/message/               (消息处理)
app/projects/zbit_asr/plugin/                (插件模块)
app/projects/zbit_asr/display/               (显示模块)
tests/
docs/
```

### 需要用户单独确认后才能修改

```text
app/projects/zbit_asr/main.c                 (主入口)
app/platform/bsp/bsp_asr.c                   (KWS 核心，已验证代码)
app/platform/bsp/bsp_audio.c                 (音频通路配置)
app/projects/zbit_asr/functions/func_speaker.c   (Speaker 模式主循环)
app/projects/zbit_asr/functions/func.c            (主状态机调度)
app/projects/zbit_asr/functions/func_lowpwr.c     (电源管理/休眠)
app/platform/modules/zbit_kws/*.c            (算法库阈值/关键词文件)
app/projects/zbit_asr/app-kws.cbp            (CodeBlocks 工程文件)
```

### 已验证代码位置

```text
# 暂无正式标记。以下文件已上板验证：
# app/platform/bsp/bsp_asr.c — KWS 流水线已验证
# app/projects/zbit_asr/led_yw.c — LED PWM 控制已验证
```

### 主入口和主循环保护点

```text
app/projects/zbit_asr/main.c:
  - main(): sys_rst_init() / bsp_sys_init() / func_run()
  - 不删除/重排已有调用

app/projects/zbit_asr/functions/func.c:
  - func_run(): while(1) 主循环
  - func_process(): 每轮处理函数
  - 不删除/重排已有调用

app/projects/zbit_asr/functions/func_speaker.c:
  - func_speaker(): Speaker 模式主循环（含 KWS 激活）
  - 不删除/重排已有调用
```

### 测试代码允许放置位置

```text
tests/
app/projects/zbit_asr/test_*.c
```

### 本次任务边界

每次让 LLM 修改代码前，用户应填写：

```text
Allowed Files:
-

Need User Confirmation Before Touching:
-
```

如果用户没有填写，LLM 必须根据项目结构先提出建议边界和最小验证方式，并等待用户确认后再修改。
