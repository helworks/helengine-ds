# DS Third Proof Line Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add one fixed `Third` proof line directly beneath the runtime `Update` line on the DS bottom screen without changing the existing `HELLO` proof or runtime text submission paths.

**Architecture:** Keep the current split intact: `HELLO` remains the one-time fixed proof line, `Update` continues to come through `TryDrawHardwareText(...)`, and `PresentBottomScreenFrame()` adds one more fixed BG0 text write for `Third`. Drive the change with the DS source-audit test first, then verify by rebuilding the ROM and capturing melonDS.

**Tech Stack:** C++, libnds BG0 text background rendering, xUnit source-audit tests, `dotnet test`, Docker-backed DS ROM build, melonDS.

---

### Task 1: Update the source audit to require the third proof line

**Files:**
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Extend the bottom-screen proof audit with the new fixed-line expectations**

Add assertions to `Source_whenProvingBottomScreenBg0Text_usesOneCookedFontGlyphTile()` so it requires the new fixed proof line strings in `PresentBottomScreenFrame()`:

```csharp
Assert.Contains("static const std::string ThirdProofLine = \"Third\";", sourceCode, StringComparison.Ordinal);
Assert.Contains("constexpr int32_t ThirdProofRow = 14;", sourceCode, StringComparison.Ordinal);
Assert.Contains("constexpr int32_t ThirdProofColumn = 15;", sourceCode, StringComparison.Ordinal);
Assert.Contains("WriteBottomScreenTextLine(ThirdProofRow, ThirdProofColumn, ThirdProofLine, static_cast<int32_t>(ThirdProofLine.size()));", sourceCode, StringComparison.Ordinal);
```

- [ ] **Step 2: Run the focused audit to verify it fails for the missing third line**

Run:

```bash
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests
```

Expected: FAIL with `Assert.Contains()` not finding one or more of the new `ThirdProofLine` expectations in `NintendoDsRenderManager2D.cpp`.

- [ ] **Step 3: Commit the failing-test update**

```bash
rtk git add builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
rtk git commit -m "test: require DS third proof line"
```

### Task 2: Add the fixed `Third` line beneath the runtime `Update` line

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Test: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

- [ ] **Step 1: Add the fixed third-line write in `PresentBottomScreenFrame()`**

Insert one fixed-line block after the existing one-time `HELLO` proof setup and before the stats text write:

```cpp
        if (BottomScreenSubmittedTextCountThisFrame > 0) {
            static const std::string ThirdProofLine = "Third";
            constexpr int32_t ThirdProofRow = 14;
            constexpr int32_t ThirdProofColumn = 15;
            WriteBottomScreenTextLine(ThirdProofRow, ThirdProofColumn, ThirdProofLine, static_cast<int32_t>(ThirdProofLine.size()));
        }
```

This keeps the line tied to the presence of the runtime `Update` line while leaving the runtime submission path unchanged.

- [ ] **Step 2: Run the focused audit to verify it passes**

Run:

```bash
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests
```

Expected: PASS for the third-line assertions and no new failures inside `NintendoDsRenderManager2DSourceAuditTests`.

- [ ] **Step 3: Commit the renderer change**

```bash
rtk git add src/platform/ds/NintendoDsRenderManager2D.cpp builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
rtk git commit -m "feat: add DS third proof line"
```

### Task 3: Rebuild the DS ROM and verify the live result

**Files:**
- Verify: `build/helengine_ds.nds`
- Verify: `artifacts/melonds-window.png`
- Uses: `artifacts/launch-melonds-rom.ps1`
- Uses: `artifacts/capture-melonds-window.ps1`

- [ ] **Step 1: Rebuild the DS package through the editor-driven build path**

Run:

```bash
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -c Release -- --project C:\dev\helprojs\city\project.heproj --build ds --output C:\dev\helprojs\city\ds-build
```

Expected: `Build completed for platform 'ds': C:\dev\helprojs\city\ds-build`

- [ ] **Step 2: Launch the rebuilt ROM in melonDS**

Run:

```bash
rtk powershell.exe -NoProfile -ExecutionPolicy Bypass -File C:\dev\helworks\helengine-ds\artifacts\launch-melonds-rom.ps1 -RomPath C:\dev\helworks\helengine-ds\build\helengine_ds.nds
```

Expected: output including the ROM path, a fresh `ROM LastWriteTime`, and a `melonDS PID`.

- [ ] **Step 3: Capture the emulator window after startup settles**

Run:

```bash
rtk powershell.exe -NoProfile -Command "Start-Sleep -Seconds 6; & 'C:\dev\helworks\helengine-ds\artifacts\capture-melonds-window.ps1'"
```

Expected: `artifacts/melonds-window.png` updated with the new runtime state.

- [ ] **Step 4: Verify the visible bottom-screen line order**

Inspect `artifacts/melonds-window.png` and confirm the bottom screen shows, top to bottom:

```text
HELLO
Update
Third
```
