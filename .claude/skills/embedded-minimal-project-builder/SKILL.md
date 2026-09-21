---
name: embedded-minimal-project-builder
description: >-
  Create and organize a minimal embedded firmware project repository from user-provided scaffolds, chip datasheets, reference manuals, schematics, vendor examples, SDK snippets, protocol documents, and build/flash notes. Use when Codex needs to generate a complete project folder with README, AGENTS and CLAUDE guidance, changelog, documentation folders, source tree, tools, tests, build/flash/debug instructions, hardware facts, file location indexes, and Git repository initialization.
  Especially use for AI-driven embedded projects where future AI agents will be responsible for reading evidence and writing firmware code.
---

# Embedded Minimal Project Builder

## Purpose

Use this skill to turn scattered embedded project materials into a clean, minimal, AI-driven firmware project repository. Produce a project that a human engineer can open and delegate to an AI coding agent. The generated repository must make it obvious where the AI should read evidence, where it should write code, how it should validate non-destructively, and which hardware actions require human confirmation.

This skill orchestrates project creation and repository hygiene. For low-level MCU extraction, build script details, or source reduction, use available companion skills rather than duplicating their full logic:

- Use `$chip-driver-manual` when extracting register, timing, protocol, pin, electrical, or initialization facts from chip manuals.
- Use `$embedded-project-scaffold` when generating or reducing startup code, linker scripts, HAL/LL files, drivers, board support, or a minimal buildable firmware target.

## Workflow

Before Step 1, do only the minimum intake needed to name and place the project: identify the requested project name or derive one from the chip/board/function, locate the user-provided materials, and ask only if the project root or repository identity is ambiguous.

## Operational Fast Path For Existing Projects

When the project already contains generated handoff documents and project-local operation scripts, especially `AGENTS.md`, `RUNBOOK.md`, and `tools/build.ps1`, and the user asks for an operation such as `build`, `compile`, `make`, `flash`, `download`, `debug`, `run build`, `编译`, `构建`, `烧录`, `下载`, or `调试`, do not restart the numbered project-creation workflow.

Treat the user's operation request as confirmation for that specific operation and follow this fast path:

1. Locate the project root with the smallest necessary check, preferring the current working directory or the only obvious project directory under it.
2. Open `AGENTS.md` quick entry or `RUNBOOK.md` only when needed to confirm the script names.
3. For build/compile requests, run `powershell -ExecutionPolicy Bypass -File tools/build.ps1`.
4. For flash/download requests, run `powershell -ExecutionPolicy Bypass -File tools/flash.ps1` and report the script result.
5. For debug requests, run `powershell -ExecutionPolicy Bypass -File tools/debug.ps1` and report the script result.
6. Do not re-discover the build system, re-read all project marker files, run broad file searches, check Git status, parse `BUILD_FLASH_DEBUG.md`, or regenerate documentation unless the project-local script fails for a reason that requires diagnosis.
7. Keep chat updates short: at most one progress message before the command and one final result message after it. Avoid narrating broad exploration steps for routine operations.
8. Final output for a fast-path operation should report only the project path, script executed, result, generated log/artifact paths, and any remaining blockers.

Fast-path performance budget:

- For an existing project build, use no more than two shell commands after reading this skill: one command to locate the single project root or read `AGENTS.md`/`RUNBOOK.md`, and one command to execute `tools/build.ps1` plus print the result.
- Do not run `rg --files`, recursive `Get-ChildItem`, project marker scans, `git status`, artifact listings, separate `Test-Path` probes, or `BUILD_FLASH_DEBUG.md` parsing before a project-local script.
- If the project root is ambiguous, inspect only the immediate child directory names and choose the single directory containing `BUILD_FLASH_DEBUG.md`. If more than one exists, ask the user to choose.
- Do not open `README.md`, `BUILD_FLASH_DEBUG.md`, `.uvprojx`, `.ioc`, `.uvoptx`, source files, or examples before a project-local build unless the script fails.
- If a project-local script succeeds, do not perform extra artifact or Git checks unless the user explicitly asked for them.

Use the numbered gated workflow only when creating/updating the project structure, copying source materials, generating `BUILD_FLASH_DEBUG.md`, generating project documentation, initializing Git, or repairing incomplete project handoff documents.

## Step Gate Rule

Run this skill as a gated workflow. Do not move from one numbered step to the next without user confirmation.

At the end of every step, stop and report:

- What was completed.
- What was inferred from files.
- What is still unclear.
- What the next step will do.
- Any files that will be created, copied, overwritten, or left untouched.

Ask the user to confirm or supplement unclear content before continuing. If the user does not confirm, do not proceed to the next step. For file copies, overwrites, Git initialization, documentation generation, flash, debug, or any hardware-facing action during project-generation steps, confirmation is mandatory even when the agent is confident.

Exception: when the Operational Fast Path applies, the user's explicit build/compile, flash/download, or debug request is sufficient confirmation to run the corresponding project-local script without asking another question. Do not add destructive commands such as mass erase, option bytes, fuse/security, lock/unlock, or readout-protection unless the user explicitly asks for that specific action.

### 1. Generate The Project Directory

Create or update the requested project directory with this baseline layout unless the user explicitly gives another structure:

The project directory name must not contain Chinese characters. Use ASCII letters, digits, underscores, and hyphens only. If the user provides a Chinese project name, generate a concise English directory name from the chip, board, and function, then ask the user to confirm it before creating the directory.

```text
name/
|-- README.md
|-- AGENTS.md
|-- CLAUDE.md
|-- BUILD_FLASH_DEBUG.md
|-- CHANGELOG.md
|-- docs/
|   |-- datasheets/
|   |-- schematics/
|   |-- examples/
|   `-- protocols/
|-- core/
|-- tools/
`-- tests/
```

Generate both `AGENTS.md` for Codex and `CLAUDE.md` for Claude Code. Keep the first firmware target narrow and verifiable, such as blink, UART hello, WHOAMI/register read, JEDEC ID, or one requested peripheral transaction.

Step 1 may create empty placeholder files only. Do not write formal `README.md`, `AGENTS.md`, `CLAUDE.md`, `BUILD_FLASH_DEBUG.md`, or `CHANGELOG.md` content in this step.

Gate before Step 2: show the created directory tree, list any pre-existing files that were preserved, and ask the user to confirm the project structure before copying source materials.

### 2. Copy Source Materials Into The Project

Read the provided material paths and classify them before copying files:

- Product goals or firmware function notes.
- Datasheets, reference manuals, programming manuals, errata.
- Schematics, PCB, BOM, wiring, pin maps.
- Vendor SDKs, examples, startup files, linker scripts, headers.
- Protocol documents, command sets, frame formats.
- Existing build, flash, debug, or test notes.

Copy input files into the correct folders. Do not move originals. Preserve original filenames when useful for later lookup. Do not silently rewrite vendor files or strip license headers. Step 2 does not need original-location explanations; it only needs to make clear where each copied material can be found inside the project.

When copying source materials, detect whether the inputs contain multiple firmware projects. Treat directories containing project/build markers such as `CMakeLists.txt`, `Makefile`, `*.uvprojx`, `*.uvproj`, `*.ewp`, `.cproject`, `platformio.ini`, `west.yml`, `sdkconfig`, or IDE project files as candidate projects.

If multiple candidate projects are found, stop and ask the user to choose the main project before continuing. Do not pick the main project silently unless the user already named it explicitly. Put the selected main project under `core/` or use it as the source for the minimal `core/` firmware organization. Put all non-selected candidate projects under `docs/examples/` as reference source code, preserving their original directory names.

Use these placement rules:

- `docs/datasheets/`: datasheets, reference manuals, programming manuals, errata, register references.
- `docs/schematics/`: schematics, PCB exports, BOM, pin maps, connection tables, board photos when useful.
- `docs/examples/`: non-selected vendor examples, alternate demos, legacy projects, and reference firmware projects.
- `docs/protocols/`: communication protocols, command sets, register command flows, frame examples.
- `core/`: minimal firmware source, startup, linker scripts, board support, drivers, build files when the project convention keeps them with source.
- `tools/`: build, flash, debug, serial, conversion, packaging, or verification scripts.
- `tests/`: unit tests, host tests, hardware smoke tests, test vectors, captured logs.

Do not write `README.md`, `AGENTS.md`, `CLAUDE.md`, or `CHANGELOG.md` in Step 2. Instead, prepare a file placement table for the Step 2 gate message. Step 4 will turn the confirmed placement table into the file location index inside the project documents.

For AI-driven projects, Step 2 must also prepare an AI coding map for the gate message. Step 4 will write and refine this map in both `AGENTS.md` and `CLAUDE.md`. This map must answer:

- Where the AI should edit code.
- Where the AI should read MCU/CubeMX/build configuration.
- Where the AI should read schematics, pin maps, and board facts.
- Where the AI should read datasheets before writing drivers.
- Which copied examples are reference-only and must not overwrite the main project.
- Which folders are currently empty but reserved for protocols, tools, and tests.

Gate before Step 3: show the planned or completed file placement table, including classification, copied project path, lookup purpose, and how a future AI coding agent should use or avoid the material. Ask the user to correct misclassified files or provide missing materials before generating build, flash, and debug methods.

### 3. Generate Build, Flash, And Debug Method

Inspect the copied main project under `core/` and any build, flash, debug, or tool notes copied into `tools/` or `docs/examples/`. Read actual files before summarizing them.

Use this detection protocol before writing `BUILD_FLASH_DEBUG.md`:

1. Identify the project from real files. Search for build-system markers: `CMakeLists.txt`, `CMakePresets.json`, `Makefile`, `*.mk`, `*.uvprojx`, `*.uvproj`, `*.uvmpw`, `*.ewp`, `platformio.ini`, `west.yml`, `sdkconfig`, `idf_component.yml`, and `*.cproject`.
2. Open the relevant marker files. Extract the active target MCU, target name, build configuration, board, environment, preset, compiler defines, linker script, startup file, artifact path, and existing debug or flash settings from the active configuration, not from random unused files.
3. Search for likely artifacts: `*.elf`, `*.axf`, `*.hex`, `*.bin`, `*.uf2`, and `*.dfu`. Skip `*.map` as a primary artifact.
4. Check existing scripts under `tools/` and any copied `scripts/` directories. Re-list the directory immediately before reading or overwriting any script because files may have changed between steps. Preserve user-owned scripts.
5. Ask the user which tool-detection mode to use before probing:
   - Auto-search: run local probes.
   - Manual paths: do not probe; list the required tool paths and wait for the user to provide them.
6. In Auto-search mode, adapt probes to the host OS. Examples:

```powershell
Get-Command cmake, ninja, make, mingw32-make, arm-none-eabi-gcc, arm-none-eabi-gdb, openocd, JLink.exe, JLinkGDBServerCL.exe, STM32_Programmer_CLI.exe, ST-LINK_gdbserver.exe, pyocd, pio -ErrorAction SilentlyContinue
```

```bash
for t in cmake ninja make arm-none-eabi-gcc arm-none-eabi-gdb openocd JLinkExe JLinkGDBServerCLExe STM32_Programmer_CLI pyocd pio; do command -v "$t" >/dev/null && echo "found: $t -> $(command -v "$t")"; done
```

Resolve every required tool through this ordered protocol, stopping at the first real executable path:

1. PATH lookup: `Get-Command`, `where.exe`, or `command -v`.
2. Known install roots: Keil `C:\Keil_v5\UV4\UV4.exe`, `D:\Keil_v5\UV4\UV4.exe`, `C:\Keil\UV4\UV4.exe`, `D:\Keil\UV4\UV4.exe`; STM32CubeProgrammer `C:\ST\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe`; SEGGER J-Link `C:\Program Files\SEGGER\JLink`; common `arm-none-eabi-gcc` and OpenOCD `bin` folders.
3. Windows registry for Keil/IAR install paths, then append the known builder path such as `UV4\UV4.exe`.
4. Environment variables: `UV4`, `Keil`, `ARMCC5BIN`, `ARM_COMPILER_6_PATH`, `IARBUILD`, `STM32CP`, `JLINK`, `OPENOCD`, `GCC_ARM`.
5. The copied project's own toolchain trail: build logs, `.uvoptx`, `.ewp`, IDE metadata, or prior command logs that reveal compiler, IDE, or programmer paths.

The ordered protocol above is a generation-time detection protocol only. Do not embed known install roots or machine-specific absolute tool paths directly into generated operation scripts. Absolute executable paths detected on the current machine may be recorded in a local toolchain configuration file and summarized in documentation, but the scripts themselves must resolve tools through configuration, explicit parameters, environment variables, PATH, registry, or documented fallback discovery.

Keep detailed probe output in the agent's working notes unless the user asks for a full audit trail. In `BUILD_FLASH_DEBUG.md`, record only the tools that affect the chosen build/flash/debug path plus missing required tools. Use a compact table with: tool, required yes/no, available yes/no, path or version, and source. Omit unrelated available tools such as CMake, OpenOCD, or STM32CubeProgrammer when the selected project system will not use them; mention them only in one short sentence if they explain why an alternative path was skipped.

Pick commands that match the detected project system and available tools:

- CMake/Make projects should use project-local build directories and visible commands.
- Keil/IAR projects should build through the IDE command-line tool such as `UV4.exe` or `IarBuild.exe`.
- Keil `UV4.exe` command-line builds must be generated with a stable working directory and log path. On Windows, prefer `Start-Process -Wait -PassThru -WindowStyle Hidden` with `-WorkingDirectory` set to the folder containing the active `*.uvprojx`, pass the project file and build log as fully resolved absolute paths, and verify the new log timestamp after the process exits. Do not generate commands from the repository root that pass a long relative `-o .\subdir\build.log` path, because older Keil versions may fail with `create file -o ... failed` or silently leave an old log untouched. Do not trust the process exit code alone; verify success from a newly written log file containing the expected target line and error/warning count.
- Keil `UV4.exe` flash/debug scripts must not copy the build command's absolute-project-path style blindly. For `-f` flash/download and `-d` debug, launch `UV4.exe` with `-WorkingDirectory` set to the folder containing the active `*.uvprojx`, pass only the relative project filename such as `"Demo07.uvprojx"`, quote the target name, and keep the `-o` log path local to that working directory. This avoids uVision command-line errors such as `project path ... contains an incorrect path` that can happen when old Keil command-line parsers receive absolute project paths or poorly quoted argument arrays for flash/debug.
- IDE-managed projects should flash through the IDE command-line interface when available; do not introduce OpenOCD, J-Link, pyOCD, or STM32CubeProgrammer unless the user explicitly requests an alternative or the IDE path is unavailable.
- Do not generate flash/debug commands with placeholder hardware values.

Generate project-local operation scripts as the primary daily interface:

- Generate a tracked `tools/project.config.json` before generating scripts. It must contain only portable project facts and relative paths: schema version, active profile, build system type, project file, working directory, target name, log names, artifact paths, and operation capabilities. It must support multiple profiles such as `keil`, `cmake`, `make`, `iar`, or `platformio` when the copied materials contain more than one possible build system.
- Generate a tracked `tools/toolchain.local.example.json` that documents the expected local toolchain keys and value shape.
- When Auto-search mode is used, write detected machine-specific executable paths to `tools/toolchain.local.json`. This file is local to the current machine and must be ignored by Git. If Manual paths mode is used and the user provides paths, write those paths to `tools/toolchain.local.json` as local configuration.
- Generated scripts must load `tools/project.config.json` and may load `tools/toolchain.local.json` when present. Use JSON for these config files because it is easy for PowerShell, Python, Node, CMake helpers, and future AI agents to read. Avoid `.psd1` as the default skill output unless the user explicitly requests a PowerShell-only project.
- Generated scripts must allow explicit command-line overrides for required tool paths, such as `-Uv4Path`, `-CMakePath`, `-NinjaPath`, `-MakePath`, `-IarBuildPath`, or equivalent names for the selected build system.
- Tool resolution inside scripts must follow this order: explicit script parameter, `tools/toolchain.local.json`, relevant environment variables, PATH lookup, platform-specific registry or package metadata, then fail with an actionable message. Do not fall back to hardcoded common install roots inside the script body.
- `tools/build.ps1` (or platform equivalent) must run the selected non-destructive build command, create a fresh log, verify the documented success marker, print a compact structured result, and exit non-zero on failure.
- `tools/flash.ps1` must directly call the selected project flash/download command, preferably the IDE-managed command such as Keil `UV4.exe -f` or IAR's equivalent. For Keil, use a single quoted argument string from the `*.uvprojx` folder, with a relative project filename and local log filename. If no flash command can be derived at all, create a script that fails with `FLASH_NOT_CONFIGURED` and points to the missing project configuration; do not create a hardware-confirmation refusal gate by default.
- `tools/debug.ps1` must directly call or start the selected project debug command, preferably the IDE-managed command such as Keil `UV4.exe -d` or IAR's equivalent. For Keil, use a single quoted argument string from the `*.uvprojx` folder, with a relative project filename. If no debug command can be derived at all, create a script that fails with `DEBUG_NOT_CONFIGURED` and points to the missing project configuration; do not create a hardware-confirmation refusal gate by default.
- `RUNBOOK.md` must list only the daily commands for build, flash, and debug. Keep it short enough that an AI can read it instead of parsing `BUILD_FLASH_DEBUG.md`.
- `BUILD_FLASH_DEBUG.md` should document the evidence, selected profile, toolchain detection summary, detailed commands, expected artifacts, and safety rationale, but it must point daily operations to `RUNBOOK.md` and `tools/` scripts. If a local tool path was detected, identify it as a generation-host fact sourced from `tools/toolchain.local.json`, not as a portable project requirement.

For Keil projects, generated PowerShell scripts must follow these shape rules:

- Build may use `Start-Process -ArgumentList @("-b", $proj, "-t", $target, "-o", $log)` with absolute project/log paths if the new log timestamp and success marker are verified.
- Flash must use `$argLine = "-f `"$projName`" -t `"$target`" -o `"$logName`""` and `Start-Process -WorkingDirectory $work -ArgumentList $argLine`, where `$projName` is only the filename, not an absolute path.
- Debug must use `$argLine = "-d `"$projName`" -t `"$target`""` and `Start-Process -WorkingDirectory $work -ArgumentList $argLine`, where `$projName` is only the filename.
- Do not pass Keil flash/debug arguments as an unquoted PowerShell array when paths or target names may contain spaces, Chinese characters, or backslashes.
- Do not write common Keil install paths such as `C:\Keil_v5\UV4\UV4.exe` or `D:\Keil\UV4\UV4.exe` into generated scripts. They may only be used by the agent while probing, then stored in `tools/toolchain.local.json` if they are actually detected on the current machine.
- Record this Keil-specific path rule in `BUILD_FLASH_DEBUG.md` so future agents do not "simplify" the scripts back to absolute project paths for `-f` or `-d`.

Create `BUILD_FLASH_DEBUG.md` in Chinese even if the exact local toolchain is not installed. Optimize it for a human engineer who wants to build or bring up the firmware. Default to this concise structure:

1. **Quick Entry**: point to `RUNBOOK.md` and the `tools/` scripts.
2. **Summary**: project system, target MCU, required toolchain, selected build path, and current status in 5-8 bullets.
3. **Build**: script path, exact underlying build command, expected artifacts, and where to read logs.
4. **Flash**: script path, exact underlying flash/download command, expected log, and tool configuration source.
5. **Debug**: script path, exact underlying debug command or launch behavior, expected process/log behavior, and tool configuration source.

Do not include an `Open Questions`, `TBD`, `Unresolved`, or similar section in `BUILD_FLASH_DEBUG.md`. Report unresolved blockers only in the step gate message, or later in `README.md`/`AGENTS.md` during Step 4 when project-wide handoff notes are generated.

Keep the default file under about 100 lines. Do not include raw command transcripts, exhaustive tool matrices, full marker inventories, long evidence tables, or general project hygiene issues such as repository naming. Include source file paths inline after facts only when the fact is safety-critical or likely to be questioned. If detailed traceability is needed, write it to `docs/build-flash-debug-audit.md` only after asking the user.

Do not add mass erase, option-byte, fuse, lock/unlock, readout-protection, security, or destructive debug commands to generated scripts unless the user explicitly asks. A normal IDE-managed flash/download command is allowed in `tools/flash.ps1`.

Gate before Step 4: summarize only the generated `BUILD_FLASH_DEBUG.md`, selected build command, flash/debug status, commands actually executed, commands intentionally skipped, and unresolved blockers. Do not repeat full tool-detection tables in the chat unless they changed the decision. Ask the user to confirm before writing the remaining engineering documents.

### 4. Generate Engineering Documentation

Before writing documentation, read `references/project-doc-contract.md`. Generate the required project documents from actual evidence, and mark unknowns clearly instead of inventing values.

Step 4 is the only step that writes formal `README.md`, `AGENTS.md`, `CLAUDE.md`, and `CHANGELOG.md` content. Use the confirmed Step 2 file placement table and AI coding map, plus the Step 3 build/flash/debug result, as inputs.

Required documents:

- `README.md`: human-facing project brief. Explain what the project is, current firmware status, how a human should delegate coding tasks to AI, quick start, key entry points, and human-confirmation boundaries. Do not make README the AI operating manual.
- `AGENTS.md`: Codex coding manual. Treat this as the primary file future Codex agents read before writing code. Include evidence map, hardware facts, code ownership boundaries, AI coding workflow, validation strategy, forbidden actions, current unknowns, and required AI delivery format.
- `CLAUDE.md`: Claude Code coding manual. It should mirror the core engineering facts from `AGENTS.md` while using Claude Code naming where helpful.
- `CHANGELOG.md`: initial version entry and future format.
- `RUNBOOK.md`: terse daily operation entrypoint with only build, flash, and debug script commands.

Generate a mandatory hardware facts table in both `AGENTS.md` and `CLAUDE.md` for AI-driven projects. `README.md` may link to or summarize it. The table must include MCU/chip model, package, Flash/RAM, crystal or clock source, power voltage, debug interface, boot mode, key pins, peripheral connections, and the copied project file path used to confirm each fact. Mark unknown values as `TBD` or `pending confirmation` with the project file or user input needed to resolve them.

For AI-driven projects, keep this division:

- `README.md`: human/operator view. Keep it concise. Include project purpose, current status, how to give AI coding tasks, quick start, key entry points, and safety gates.
- `AGENTS.md` and `CLAUDE.md`: AI programmer view. Include enough context for future AI agents to locate evidence and write code without guessing.

Both `AGENTS.md` and `CLAUDE.md` must include at least these sections or equivalent content:

- AI quick entry at the top: build/flash/debug requests must call `tools/build.ps1`, `tools/flash.ps1`, or `tools/debug.ps1` directly and must not scan project files first.
- AI role and working principles.
- Evidence and material location index.
- Mandatory hardware facts table.
- AI code-writing workflow.
- Code ownership and edit boundaries.
- Validation strategy.
- Forbidden/destructive actions.
- Current unknowns affecting correctness.
- Expected delivery/report format after code changes.

Write all generated documentation in Chinese. Keep script code, variable names, tool commands, and paths in English or original tool spelling.

Gate before Step 5: summarize the generated documents, important assumptions, TBD items, README versus AGENTS/CLAUDE division, and whether both `AGENTS.md` and `CLAUDE.md` are sufficient for future AI coding agents to locate sources, write code, and validate safely. Ask the user to confirm before Git initialization.

### 5. Initialize Git Repository

Initialize Git in the project root when the directory is not already inside a Git repository. If it is already in a repository, do not nest a new repo unless the user explicitly asks.

Add a practical embedded `.gitignore` when none exists, covering build outputs, IDE metadata, temporary logs, object files, generated binaries, and local tool caches while preserving source, docs, scripts, linker scripts, and project files. Always ignore machine-local detected toolchain configuration such as `tools/toolchain.local.json`, while keeping `tools/project.config.json` and `tools/toolchain.local.example.json` trackable.

After files are generated, run `git status --short` from the project root and report created or modified files. Then automatically create an initial commit for the generated project after the user confirms entering Step 5.

Commit rules:

- If the project root is a new repository, run `git add .` and commit the generated project.
- If the project root is inside an existing repository, stage only files created or updated by this skill; do not stage unrelated user changes.
- Use commit message `Initial embedded project scaffold` unless the user provides another message.
- If Git user name or email is missing, stop and report the required `git config` values instead of inventing them.
- After commit, run `git status --short` again and report the result.

Final gate: summarize the generated documents, important assumptions, TBD items, README versus AGENTS/CLAUDE division, BUILD_FLASH_DEBUG coverage, commit hash when created, Git status, generated files, remaining unknowns, and next recommended validation step. Ask whether the user wants more source cleanup, build verification, or hardware bring-up work.

## Verification

Before finishing:

- Verify the required directory tree exists.
- Verify all required documents exist and contain project-specific content, not placeholders.
- Run available non-destructive build or syntax checks when tools exist.
- Do not claim firmware builds if no compiler/build tool was available.
- Report open hardware questions that block flashing, debugging, or electrical bring-up.

## Output Contract

Finish with a concise Chinese report unless the user used another language. Include:

- Project path.
- Generated or updated files.
- Copied materials and where they were placed inside the project.
- Build, flash, and debug status.
- Git status and whether a repository was initialized.
- Remaining unknowns and next validation step.
