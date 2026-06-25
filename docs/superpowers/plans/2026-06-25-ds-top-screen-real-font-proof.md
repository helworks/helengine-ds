# DS Top-Screen Real-Font Proof Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Keep the stable handwritten top-screen `HELLO` proof line and add a second renderer-owned `HELLO` line below it using the real cooked `Fonts/DemoDiscBody.ttf` font through the shared top-screen glyph pipeline.

**Architecture:** The DS renderer keeps the current renderer-owned proof orchestration and top-screen white OBJ square, but rewrites `WriteTopScreenProofText()` to compose two BG0 lines: a handwritten control line and a cooked-font line. The cooked-font line resolves `Fonts/DemoDiscBody.ttf` through the generated-core runtime scene asset resolver on `Core`, uploads glyphs with the shared `NintendoDsScreenTarget::Top` helper, and writes the second line with the shared text-map writer.

**Tech Stack:** C++, generated-core runtime types (`Core`, `RuntimeSceneAssetReferenceResolver`, `SceneAssetReferenceFactory`, `FontAsset`), xUnit source audits, Nintendo DS native Docker build, melonDS.

---

## File Map

- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
  - Add the private helper declaration for resolving the proof font used by the renderer-owned top-screen cooked-font proof line.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
  - Add generated-core includes needed for runtime font resolution.
  - Add the stable proof font relative-path constant `Fonts/DemoDiscBody.ttf`.
  - Resolve the proof font through `Core::get_Instance()->get_SceneAssetReferenceResolver()`.
  - Rewrite `WriteTopScreenProofText()` so it clears the map, writes handwritten row 1, uploads cooked glyphs for the top screen, and writes cooked-font row 2.
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
  - Replace the old handwritten-only top-screen proof assertions with assertions that require both the handwritten control line and the cooked-font line.
- Modify: `docs/superpowers/plans/2026-06-25-ds-top-screen-real-font-proof.md`
  - Mark steps complete during execution if using `executing-plans`.

### Task 1: Lock the New Proof Behavior in Source Audits

**Files:**
- Modify: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Write the failing source audit for the two-line proof**

```csharp
/// <summary>
/// Verifies the top-screen proof keeps the handwritten control line and adds a cooked-font line from the real demo-disc body font.
/// </summary>
[Fact]
public void Source_whenDrawingTopScreenCamera_keepsHandwrittenControlLineAndAddsCookedFontProofLine() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
    string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
    string headerSource = File.ReadAllText(headerPath);
    string sourceCode = File.ReadAllText(sourcePath);

    Assert.Contains("void WriteTopScreenProofText();", headerSource, StringComparison.Ordinal);
    Assert.Contains("FontAsset* ResolveRequiredTopScreenProofFont() const;", headerSource, StringComparison.Ordinal);
    Assert.Contains("constexpr const char* TopScreenProofFontRelativePath = \"Fonts/DemoDiscBody.ttf\";", sourceCode, StringComparison.Ordinal);
    Assert.Contains("void NintendoDsRenderManager2D::WriteTopScreenProofText()", sourceCode, StringComparison.Ordinal);
    Assert.Contains("ClearTopScreenTextMap();", sourceCode, StringComparison.Ordinal);
    Assert.Contains("std::array<uint16_t, 5> proofGlyphTileIndices = {", sourceCode, StringComparison.Ordinal);
    Assert.Contains("FontAsset* proofFont = ResolveRequiredTopScreenProofFont();", sourceCode, StringComparison.Ordinal);
    Assert.Contains("EnsureTopScreenFontGlyphTilesReady(proofFont);", sourceCode, StringComparison.Ordinal);
    Assert.Contains("WriteTopScreenTextLine(2, 1, \"HELLO\", 5);", sourceCode, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the targeted source audit to verify it fails**

Run:

```powershell
rtk dotnet test builder.tests\helengine.ds.builder.tests.csproj --filter Source_whenDrawingTopScreenCamera_keepsHandwrittenControlLineAndAddsCookedFontProofLine
```

Expected:

```text
FAIL
Assert.Contains() Failure
Not found: FontAsset* ResolveRequiredTopScreenProofFont() const;
```

If `helengine.editor.app` is still running and the build is file-locked, stop the editor first, then rerun the exact command until the failure is an assertion failure instead of `MSB3021/MSB3027`.

- [ ] **Step 3: Commit the failing test-only change**

```bash
git add builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "test: pin DS top-screen real-font proof behavior"
```

### Task 2: Resolve the Real Font and Rewrite the Top-Screen Proof

**Files:**
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
- Test: `builder.tests/helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Add the new private helper declaration in the renderer header**

```cpp
        /// <summary>
        /// Resolves the real demo-disc body font used by the renderer-owned top-screen cooked-font proof line.
        /// </summary>
        /// <returns>Loaded runtime font asset used by the top-screen cooked-font proof.</returns>
        FontAsset* ResolveRequiredTopScreenProofFont() const;
```

- [ ] **Step 2: Add the runtime font-resolution includes and stable proof font path constant**

```cpp
#include "Core.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "SceneAssetReference.hpp"
#include "SceneAssetReferenceFactory.hpp"
```

```cpp
        /// Stable cooked font path used by the renderer-owned top-screen real-font proof line.
        constexpr const char* TopScreenProofFontRelativePath = "Fonts/DemoDiscBody.ttf";
```

- [ ] **Step 3: Implement the proof-font resolver in `NintendoDsRenderManager2D.cpp`**

```cpp
    /// Resolves the real demo-disc body font used by the renderer-owned top-screen cooked-font proof line.
    /// <returns>Loaded runtime font asset used by the top-screen cooked-font proof.</returns>
    FontAsset* NintendoDsRenderManager2D::ResolveRequiredTopScreenProofFont() const {
        Core* core = Core::get_Instance();
        if (core == nullptr) {
            throw new InvalidOperationException("Nintendo DS top-screen proof font resolution requires a live Core instance.");
        }

        RuntimeSceneAssetReferenceResolver* resolver = core->get_SceneAssetReferenceResolver();
        if (resolver == nullptr) {
            throw new InvalidOperationException("Nintendo DS top-screen proof font resolution requires a runtime scene asset reference resolver.");
        }

        SceneAssetReference* reference = SceneAssetReferenceFactory::CreateFileSystemFont(TopScreenProofFontRelativePath);
        if (reference == nullptr) {
            throw new InvalidOperationException("Nintendo DS top-screen proof font reference creation failed.");
        }

        auto cleanup = he_cpp_make_scope_exit([&]() {
            delete reference;
        });

        FontAsset* font = resolver->ResolveFont(reference);
        if (font == nullptr) {
            throw new InvalidOperationException("Nintendo DS top-screen proof font resolution returned null.");
        }

        return font;
    }
```

- [ ] **Step 4: Rewrite `WriteTopScreenProofText()` to keep the handwritten control line and add the cooked-font line**

```cpp
    /// Writes the renderer-owned top-screen BG0 proof lines: one handwritten control line and one cooked-font line.
    void NintendoDsRenderManager2D::WriteTopScreenProofText() {
        if (TopScreenTextMapEntries == nullptr) {
            return;
        }

        constexpr int32_t ConsoleColumns = FrameBufferWidth / 8;
        constexpr int32_t HandwrittenProofRow = 1;
        constexpr int32_t CookedProofRow = 2;
        constexpr int32_t ProofColumn = 1;
        constexpr int32_t ProofVisibleColumns = 5;
        std::array<uint16_t, 5> proofGlyphTileIndices = {
            TopScreenProofHTileIndex,
            TopScreenProofETileIndex,
            TopScreenProofLTileIndex,
            TopScreenProofLTileIndex,
            TopScreenProofOTileIndex
        };

        ClearTopScreenTextMap();

        int32_t rowOffset = HandwrittenProofRow * ConsoleColumns;
        for (int32_t index = 0; index < static_cast<int32_t>(proofGlyphTileIndices.size()); index++) {
            int32_t mapIndex = rowOffset + ProofColumn + index;
            uint16_t tileIndex = proofGlyphTileIndices[static_cast<std::size_t>(index)];
            TopScreenTextShadowEntries[static_cast<std::size_t>(mapIndex)] = tileIndex;
            TopScreenTextMapEntries[mapIndex] = tileIndex;
        }

        FontAsset* proofFont = ResolveRequiredTopScreenProofFont();
        EnsureTopScreenFontGlyphTilesReady(proofFont);
        WriteTopScreenTextLine(CookedProofRow, ProofColumn, "HELLO", ProofVisibleColumns);
    }
```

- [ ] **Step 5: Run the targeted source audit to verify the new proof path passes**

Run:

```powershell
rtk dotnet test builder.tests\helengine.ds.builder.tests.csproj --filter Source_whenDrawingTopScreenCamera_keepsHandwrittenControlLineAndAddsCookedFontProofLine
```

Expected:

```text
PASS
```

- [ ] **Step 6: Run the shared-helper audit to verify the refactor stays on the shared path**

Run:

```powershell
rtk dotnet test builder.tests\helengine.ds.builder.tests.csproj --filter Source_whenManagingNintendoDsTextScreens_usesSharedScreenTargetHelpers
```

Expected:

```text
PASS
```

- [ ] **Step 7: Commit the renderer proof rewrite**

```bash
git add src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "feat: render DS top-screen proof text with real cooked font"
```

### Task 3: Rebuild the DS ROM and Validate in melonDS

**Files:**
- Modify: `docs/superpowers/plans/2026-06-25-ds-top-screen-real-font-proof.md`
- Output: `build/helengine_ds.nds`
- Output: `C:\dev\helprojs\city\output\ds\helengine_ds.nds`

- [ ] **Step 1: Rebuild the native DS ROM against the latest staged NitroFS/generated-core workspace**

Run:

```powershell
$stagingRoot = Get-ChildItem -Path (Join-Path $env:LOCALAPPDATA 'Temp\helengine-platform-build\ds') -Directory |
    Sort-Object LastWriteTime -Descending |
    Where-Object {
        Test-Path (Join-Path $_.FullName 'builder\ds\nitrofs') -and
        Test-Path (Join-Path $_.FullName 'builder\ds\generated-core')
    } |
    Select-Object -First 1 -ExpandProperty FullName

rtk docker run --rm -v "C:\dev\helworks\helengine-ds:/workspace" -v "$stagingRoot\builder:/workspace-staging" -w /workspace helengine-ds make clean
rtk docker run --rm -v "C:\dev\helworks\helengine-ds:/workspace" -v "$stagingRoot\builder:/workspace-staging" -w /workspace helengine-ds make HELENGINE_DS_NITROFS_ROOT=/workspace-staging/ds/nitrofs HELENGINE_CORE_CPP_ROOT=/workspace-staging/ds/generated-core
```

Expected:

```text
linking helengine_ds.elf
built ... helengine_ds.nds
```

- [ ] **Step 2: Export the rebuilt ROM to the city DS output folder**

Run:

```powershell
Copy-Item -LiteralPath C:\dev\helworks\helengine-ds\build\helengine_ds.nds -Destination C:\dev\helprojs\city\output\ds\helengine_ds.nds -Force
Get-Item -LiteralPath C:\dev\helprojs\city\output\ds\helengine_ds.nds | Select-Object FullName, LastWriteTime, Length
```

Expected:

```text
The command prints one row for C:\dev\helprojs\city\output\ds\helengine_ds.nds and the Length column is greater than 0.
```

- [ ] **Step 3: Launch the rebuilt ROM in melonDS**

Run:

```powershell
rtk powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\launch_in_emulator.ps1 -ArtifactPath C:\dev\helprojs\city\output\ds\helengine_ds.nds
```

Expected:

```text
ARTIFACT=C:\dev\helprojs\city\output\ds\helengine_ds.nds
EMULATOR=C:\dev\helworks\emus\melonDS-1.1-windows-x86_64\melonDS.exe
PROCESS_ID=
```

- [ ] **Step 4: Validate the on-screen proof manually**

Check in melonDS:

```text
Top screen row 1 shows the stable handwritten HELLO.
Top screen row 2 shows HELLO using the cooked DemoDiscBody font.
The white OBJ square is still visible to the right.
```

- [ ] **Step 5: Commit the verified plan progress notes if you tracked execution in this file**

```bash
git add docs/superpowers/plans/2026-06-25-ds-top-screen-real-font-proof.md
git commit -m "docs: track DS top-screen real-font proof execution"
```
