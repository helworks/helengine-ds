# DS BG0 Authored Bottom Text Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Restore real authored bottom-screen `TextComponent` rendering on DS through the stable `BG0` text background path using the cooked font atlas.

**Architecture:** Keep the stable sub-screen `BG0` ownership and fallback 3D path unchanged, remove the proof-only `H` tile path, and route real authored strings through the existing glyph upload, tile index resolution, and tilemap writing helpers. Continue rejecting unsupported authored text shapes explicitly instead of adding new fallbacks.

**Tech Stack:** C++, libnds BG text backgrounds, existing DS renderer source audits, native Docker DS ROM build, melonDS.

---

### Task 1: Replace the proof-only bottom text path with authored cooked-font submission

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`

- [ ] **Step 1: Remove the static proof tile dependency from the one-time BG0 init path**

Delete the `PaintHardcodedBottomScreenProofTiles();` call from `EnsureBottomScreenTextBackgroundReady()` and remove the unused proof helper declaration/definition if nothing references it afterward.

- [ ] **Step 2: Restore `TryDrawHardwareText(...)` to the authored path**

Use:
- `text->get_Text()`
- `text->get_Font()`
- `text->get_FontScale()`
- `text->get_WrapText()`
- `text->get_Alignment()`
- `text->get_Size()`
- `text->get_Parent()->get_Position()`

Keep explicit rejects for:
- null text
- non-bottom screen
- null font
- null parent
- unsupported color
- unsupported source rect
- unsupported wrap mode
- unsupported font scale if it does not map cleanly to the BG0 cell grid

- [ ] **Step 3: Route authored content through the existing BG0 helpers**

Call:
- `EnsureBottomScreenTextBackgroundReady()`
- `EnsureBottomScreenFontGlyphTilesReady(font)`
- `TryResolveBottomScreenGlyphTileIndex(...)`
- `WriteBottomScreenTextLine(...)`

Split authored text on newlines, compute base row/column from parent position in 8x8 cells, derive visible width from `text->get_Size()`, and use `ResolveAlignedConsoleColumn(...)` for each rendered line.

- [ ] **Step 4: Preserve explicit unsupported behavior**

If the authored text cannot be represented through the current DS BG0 path, keep `TraceUnsupportedTextDrawable(...)` with a concrete reason and return `false`.

### Task 2: Update the source audits to match the authored path

**Files:**
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Replace proof-only audit expectations with authored-path expectations**

Remove assertions that require:
- `HardcodedHTileIndex`
- `GlyphTileRow = 10`
- `PaintHardcodedBottomScreenProofTiles()`
- proof-only rejection strings

Add assertions that require:
- `text->get_Text()`
- `text->get_Size()`
- `EnsureBottomScreenFontGlyphTilesReady(font);`
- `WriteBottomScreenTextLine(`
- `ResolveAlignedConsoleColumn(`

- [ ] **Step 2: Keep the stable BG0 ownership assertions**

Preserve the assertions that prevent:
- per-frame `videoSetModeSub(...)` churn in `PresentBottomScreenFrame()`
- per-frame `vramSetBankC(...)` churn in `PresentBottomScreenFrame()`
- `consoleInit(...)` in the runtime text path

### Task 3: Verify source shape, rebuild, and validate live behavior

**Files:**
- Verify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Verify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Output: `build/helengine_ds.nds`

- [ ] **Step 1: Run the narrowest practical verification**

Preferred:
`rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter NintendoDsRenderManager2DSourceAuditTests`

If the builder DLL is still locked, verify the edited source markers directly and note the lock in the final report.

- [ ] **Step 2: Rebuild the DS ROM**

Run:
`rtk docker run --rm -v C:\dev\helworks\helengine-ds:/workspace -v C:\Users\beatriz\AppData\Local\Temp\helengine-platform-build\ds\7774d3aa6d3d4104a2dc4f65cac102a4:/workspace-staging -w /workspace helengine-ds make -B HELENGINE_DS_NITROFS_ROOT=/workspace-staging/builder/ds/nitrofs HELENGINE_CORE_CPP_ROOT=/workspace-staging/generated-core`

- [ ] **Step 3: Copy the rebuilt ROM and launch melonDS**

Run:
- `Copy-Item C:\dev\helworks\helengine-ds\build\helengine_ds.nds C:\dev\helprojs\city\ds-build\helengine_ds.nds -Force`
- `powershell -NoProfile -ExecutionPolicy Bypass -File artifacts\launch-melonds-rom.ps1 -RomPath C:\dev\helprojs\city\ds-build\helengine_ds.nds`

- [ ] **Step 4: Live verification with the user**

Confirm:
- top-screen cubes still render
- bottom-screen authored text appears
- bottom-screen text is stable
- proof `H` is gone
