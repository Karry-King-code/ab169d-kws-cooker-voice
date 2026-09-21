# Embedded LLM Guardrails

用于在嵌入式、单片机、RTOS、驱动和板级工程中使用 LLM 辅助开发时建立安全边界的 skill。

## 核心原则

- 未显式允许修改的文件默认禁止修改。
- 每次只做当前任务要求的最小可验证修改。
- 先定义验证方式，再实现最小代码，验证通过后再扩展。
- 保护带 `VERIFIED_ON_HARDWARE` 等标记的已验证代码。
- 添加新功能时，业务逻辑放到独立 `.c` / `.h` 文件，`main.c` 只放函数调用。

## Skill 包结构

```text
embedded-llm-guardrails/
  SKILL.md                          skill 主文件：执行顺序和硬规则
  assets/
    LLM_RULES.md                    项目规则模板（含 {{...}} 占位符）
    LLM_BOUNDARY.md                 修改边界配置模板
    STM32_RESOURCE_TABLE.md         STM32F1 资源表模板
    llm-change-request.md           单次修改请求模板
    settings.json                   Claude Code 权限配置模板
  references/
    project-patterns.md             嵌入式项目类型识别指南
```

## 目标项目文件

显式调用本 skill 后，目标固件项目只需要维护项目级边界文件：

```text
.ai/
  LLM_RULES.md
  LLM_BOUNDARY.md
  STM32_RESOURCE_TABLE.md           STM32F1 项目按需生成
```

`LLM_RULES.md` 存放项目规则和确认单定义；`LLM_BOUNDARY.md` 存放可改/禁改边界；`STM32_RESOURCE_TABLE.md` 记录 STM32F1 引脚、AFIO remap、DMA、IRQ 和 NVIC 优先级等资源分配。

## 使用方式

在目标固件项目中显式调用 skill，让它扫描工程、生成或检查 `.ai/` 文件，并在每次修改代码前输出本轮确认单：

```text
本轮目标：
Allowed Files：
最小验证方式：
硬件风险：
待确认问题：
```

用户确认前，skill 只允许只读分析，不修改源码、头文件、构建脚本、配置文件或硬件行为。
