---
name: chip-resource-finder
description: Find, verify, and organize embedded chip development resources from a chip model or chip family. Use when the user asks for datasheet, reference manual, SDK, HAL/BSP, examples, errata, IDE/toolchain support, schematic/layout guides, pack files, programming/debug documents, or a new-chip bring-up resource package. Requires current web verification and prioritizes official vendor sources.
---

# Chip Resource Finder

Use this skill to turn a chip model into a concise embedded development resource report. The main deliverable is an HTML file that helps the user quickly find hardware design, software development, flashing/programming, and mass-production resources.

## Required Behavior

- Browse the web for current links whenever the user asks for chip documents, SDKs, examples, tools, or download pages.
- Prefer official primary sources: chip vendor product page, document center, official SDK portal, official GitHub/Gitee organization, official evaluation board page, and official tool page.
- If the vendor is based in mainland China, prioritize the Chinese official website, Chinese document center, and domestic download pages first. If the vendor is overseas, prioritize the global official site first.
- Use partner, distributor, board vendor, community, blog, mirror, or cloud-drive sources only when official sources are missing or incomplete, and label them clearly.
- Do not invent links, document versions, SDK versions, tool support, repository ownership, or applicability.
- Distinguish exact ordering codes from family-level documents. Many MCU resources are family-level.
- Keep the workflow lightweight. Do not do repeated probing, bulk HEAD checks, or deep download validation unless the user explicitly asks.

## Workflow

1. Parse the chip identity.
   - Extract vendor, family, exact model, package/suffix, memory variant, wireless/module variant, and whether the input may be a board name.
   - Normalize likely aliases, but keep the user's original model in the answer.

2. Search official sources first.
   - Start with three query groups: product page, document center, SDK/tool page.
   - Search the exact model first, then the family.
   - Add category-specific queries only if the first pass misses an important resource.

3. Classify found resources.
   - Read `references/resource-checklist.md` when deciding what belongs in each table.
   - Use three output tables: hardware development, software development, and flashing/programming or mass-production test.
   - A resource may appear in more than one table only when it genuinely serves multiple roles.

4. Check source quality.
   - Read `references/source-priority.md` when judging source credibility.
   - Prefer stable product/document pages over random direct PDF mirrors.
   - Prefer official repository releases/tags over copied ZIP files.
   - Flag outdated, login-gated, archived, region-specific, or family-mismatched resources in `备注`.
   - Stop at the official primary trail unless it is incomplete.

5. Produce a compact resource package.
   - Start with chip identity and assumptions.
   - Generate an HTML report file with the three Chinese tables shown below.
   - Save the file in the current workspace unless the user specifies another path. Use a stable lowercase filename such as `chip-resource-<normalized-chip-model>.html`.
   - In the chat response, provide only a short summary and a clickable link to the generated HTML file.
   - Include a minimum download set and suggested reading order.
   - List missing or risky items without over-explaining.

## HTML Output

Generate a standalone HTML file. It should be readable when opened directly in a browser, without a dev server.

Required HTML sections:

- 芯片识别
- 硬件开发资料
- 软件开发资料
- 烧录编程 / 量产测试资料
- 最小下载集
- 推荐阅读顺序
- 缺失或风险项
- 检索证据

Use simple semantic HTML:

- `<h1>` for the chip resource report title.
- `<section>` for each section.
- `<table>` for the three resource tables.
- `<a href="...">` for links.
- A small `<style>` block for readable typography, table borders, spacing, and print-friendly layout.

Do not add external CSS, JavaScript, tracking, or decorative assets.

## HTML Visual Style

Use a clean technical dashboard style inspired by shadcn/ui, GitHub Primer, and Tailwind UI documentation tables:

- White or very light gray page background.
- Centered main container with max width around `1120px`.
- Compact header: left side chip name/report title, right side generation date and source strategy.
- Chip identity should be shown in 3-4 compact summary cards.
- Use consistent table structure for all three resource sections.
- Add one short explanatory sentence above each table.
- Use subtle borders, light gray table headers, comfortable cell padding, and optional row hover styling.
- Use small priority badges: `必需`, `推荐`, `可选`.
- Suggested badge tones: required = muted red, recommended = muted blue, optional = gray.
- Use a light yellow callout for risk/missing items.
- Use a light gray information block for search evidence.
- The page should feel like a professional engineering resource package, not a marketing landing page.
- Keep it print-friendly: no dark theme, no large decorative background, no external font or image.

Table schema:

| 优先级 | 类别 | 资料 | 来源 | 链接 | 备注 |
|---|---|---|---|---|---|

Use the same schema for hardware, software, and flashing/production tables.

## Category Guidance

- MCU/SoC: do not stop at the datasheet if reference manual, SDK, errata, examples, and programming tools exist.
- External peripheral chips: prioritize datasheet, application notes, register map, command set, timing, reference driver, and evaluation board files.
- Wireless chips/modules: include RF design guide, antenna/layout guide, certification notes, firmware stack, host interface protocol, and production test/calibration docs when official sources provide them.
- Power/analog/interface chips: include design guides, evaluation board files, layout notes, thermal data, reference schematics, and software only when the part needs firmware interaction.

## Escalation

If the user wants to implement a driver from collected documents, hand off to `chip-driver-manual`. If the user has already parsed a datasheet bundle and asks factual questions from it, use `parsed-chip-datasheet`.
