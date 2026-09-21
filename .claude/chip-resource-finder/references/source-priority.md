# Source Priority And Evidence Rules

Use these rules when deciding which links to trust and how to label them.

## Source Levels

| Level | Source | Use |
|---|---|---|
| Official primary | Vendor product page, document center, SDK portal, official GitHub/Gitee, evaluation board page, tool page | Default source for core documents and downloads |
| Official ecosystem | ARM/CMSIS pack index, Keil pack, IAR, SEGGER/J-Link, OpenOCD, PlatformIO official platform | Toolchain and debug support |
| Authorized partner | Evaluation board vendor, module vendor, distributor, reference design partner | Use only when official primary sources are incomplete |
| Community | Personal GitHub repos, blogs, forums, tutorials, mirrors | Supplement only; label clearly |
| Risky | Random PDF mirrors, SEO download sites, unversioned cloud-drive links, scraped datasheet databases | Avoid as primary evidence |

## Default Policy

- Use official primary sources unless the official trail is missing or incomplete.
- Do not open supplementary sources just to improve coverage.
- Prefer product/document pages over bare PDF mirrors.
- Prefer official repository releases/tags over copied archives.
- Keep version, release, lifecycle, or login-gate details in `备注` only when immediately visible.
- Omit unclear metadata instead of searching deeper.

## Region Priority

| Vendor | Priority |
|---|---|
| WCH / 沁恒 | Chinese official site first |
| Espressif / 乐鑫 | Chinese official site first |
| GigaDevice / 兆易 | Chinese official site first |
| STMicroelectronics | Global official site first |
| NXP | Global official site first |
| Texas Instruments | Global official site first |
| Microchip | Global official site first |
| Nordic Semiconductor | Global official site first |

If a vendor is not in the table, infer conservatively from the official product page and state the assumption when it matters.

## Search Query Pattern

Start with at most three groups:

- Product page: `<chip model> official product page`
- Documents: `<chip model> datasheet reference manual errata`
- SDK/tools: `<chip family> SDK examples programming tool`

Add targeted queries only for missing must-have items, such as hardware design guide, bootloader, ISP, app note, SVD, CAD, or evaluation board schematic.

## Red Flags

- Document title names a different chip or family.
- Download page lacks vendor branding or official ownership.
- SDK examples target a board that does not use the requested chip.
- A package suffix implies memory/package/temperature differences that the found document does not cover.
- A community workaround has no matching errata or vendor note.
