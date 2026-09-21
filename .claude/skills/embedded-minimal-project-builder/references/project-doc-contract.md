# Project Documentation Contract

Use this reference when generating the required documents for a minimal AI-driven embedded firmware project.

All generated project documents must be written in Chinese. Keep code, commands, tool names, file paths, and variable names in English or original spelling.

The default project model is:

- `README.md` is for humans.
- `AGENTS.md` is the Codex coding manual.
- `CLAUDE.md` is the Claude Code coding manual.

Generate both `AGENTS.md` and `CLAUDE.md` by default. Keep their engineering facts consistent; adapt only agent-specific wording where useful.

## README.md

Include:

- Project name and one-sentence AI-driven goal.
- Human/AI responsibility split: humans define goals and approve hardware actions; AI writes code and documents evidence.
- Current firmware behavior and status.
- How a human should delegate coding tasks to AI: function goal, allowed edit scope, build permission, hardware permission, and acceptance criteria.
- Quick start with build command, artifact path, and prerequisites.
- Directory overview matching the generated repository.
- Key entry points: `AGENTS.md`, `CLAUDE.md`, `BUILD_FLASH_DEBUG.md`, main project path, build project file, CubeMX config, schematics, datasheets, examples.
- Safety gates for build, flash, debug, erase, option bytes, fuses, locks, and hardware-affecting actions.

Avoid:

- Marketing-style descriptions.
- Claims that build, flash, or debug works without local verification.
- Datasheet values without copied project file paths when available.
- Long AI operation manuals. Put AI workflow details in `AGENTS.md` and `CLAUDE.md`.

## AGENTS.md and CLAUDE.md

Include:

- AI role and working principles: read evidence first, write scoped code, verify non-destructively, and ask before hardware-affecting actions.
- Evidence and material location index. This must be useful even if the future AI reads only one of these files.
- Mandatory hardware facts table.
- AI code-writing workflow from task intake to final report.
- Code ownership and edit boundaries: application code, generated code, vendor code, startup/linker files, board support, docs, tools, tests.
- Validation strategy: file checks, static checks, build, simulator or host checks, hardware smoke tests, serial logs, flash, debug.
- Forbidden/destructive actions.
- Current unknowns that affect correctness.
- Expected AI delivery/report format after code changes.
- Main project path and reference example project paths when multiple projects were provided.

## Mandatory Hardware Facts Table

Include:

- MCU/chip model.
- Package.
- Flash/RAM.
- Crystal or clock source.
- Power voltage.
- Debug interface.
- Boot mode.
- Key pins.
- Peripheral connections.
- Copied project file paths used to confirm facts.

Place this table in both `AGENTS.md` and `CLAUDE.md` for AI-driven projects. `README.md` may link to it or summarize the key chip and status. Each row should include value, confidence or status, and copied project file path. Mark each uncertain item as `TBD` or `待确认` with the project file or user input needed to resolve it.

## BUILD_FLASH_DEBUG.md

Include:

- Host OS and shell assumed by generated commands.
- Build system and exact command.
- Compiler, SDK, IDE, and version or detection status.
- Expected output artifacts.
- Flash tool and command only when target/probe/address/artifact are confirmed.
- Debug server command and GDB/client command only when safe and confirmed.
- Serial monitor command when port/baud are known.
- Commands actually run by the agent, with result.
- Commands intentionally not run, with reason.
- Missing tools or parameters.

Never include destructive commands by default: mass erase, chip erase, option bytes, fuses, read protection, secure boot, OTP, lock/unlock, or write-protection changes.

## CHANGELOG.md

Create a simple keep-a-changelog style file:

```markdown
# 变更记录

## 0.1.0 - YYYY-MM-DD

- 创建初始最小工程结构。
- 复制并归档工程资料。
- 添加首版编译、烧录、调试说明。
```

Use the actual current date from the environment.

## Baseline .gitignore

When creating `.gitignore`, include common embedded outputs while keeping project files trackable:

```gitignore
# Build outputs
build/
out/
dist/
*.o
*.obj
*.a
*.elf
*.axf
*.hex
*.bin
*.map
*.lst
*.d

# Logs and temporary files
*.log
*.tmp
*.bak
*.swp
*.swo

# IDE and local settings
.vscode/
.idea/
*.uvguix.*
*.uvoptx
*.dep
Debug/
Release/

# Python/tool caches
__pycache__/
.pytest_cache/
.mypy_cache/
```

Adjust for project conventions: keep required vendor IDE project files such as `.uvprojx`, `.ewp`, `.cproject`, `.project`, `CMakeLists.txt`, `Makefile`, `platformio.ini`, `west.yml`, and linker scripts under version control.
