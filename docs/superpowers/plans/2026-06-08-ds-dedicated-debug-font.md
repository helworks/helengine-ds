# DS Dedicated Debug Font Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Switch the Nintendo DS bottom overlay from the editor default font to a DS-appropriate project font asset that cooks as `Indexed4` and renders through the existing hardware background-text path.

**Architecture:** Keep the DS text renderer narrow and hardware-only. Fix the asset boundary instead of making the renderer more permissive: resolve one project-owned debug font for the DS scaffold, persist it into generated DS scenes, and verify the DS build packages and displays that font instead of the oversized editor default font.

**Tech Stack:** C#, City authoring scene generators, DS builder/source audits, shared `build-platform.ps1` wrapper, Nintendo DS runtime renderer

---

## File Map

- Modify: `builder.tests/CityNintendoDsSceneSourceAuditTests.cs`
  - Lock the DS scaffold contract so bottom overlay text no longer depends on `ResolveRequiredEditorFont()`.
- Modify: `..\..\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs`
  - Remove editor-font resolution from the default DS overlay and accept a resolved project font.
- Modify: `..\..\helprojs\city\assets\codebase\rendering.tools\GeneratedAuthoringSceneWriteService.cs`
  - Resolve the DS debug font from the project root and pass it into the scaffold.
- Modify: `..\..\helprojs\city\assets\fonts\DemoDiscBody.ttf.hasset`
  - Persist DS `font-atlas-texture` settings for the chosen DS debug font asset when needed.
- Use: `..\..\helprojs\city\assets\fonts\DemoDiscBody.hefont`
  - Treat this existing project font as the DS debug font asset for the first pass.

### Task 1: Lock The DS Scaffold Font Contract

**Files:**
- Modify: `builder.tests/CityNintendoDsSceneSourceAuditTests.cs`
- Test: `builder.tests/CityNintendoDsSceneSourceAuditTests.cs`

- [ ] **Step 1: Write the failing source audit for the dedicated DS font path**

Add assertions that the DS scaffold uses `DemoDiscBody.hefont` and no longer calls `ResolveRequiredEditorFont()` for the bottom overlay:

```csharp
[Fact]
public void Sources_whenAuthoringDsBottomOverlay_useProjectDebugFontInsteadOfEditorDefaultFont() {
    string scaffoldSource = File.ReadAllText(
        Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "NintendoDsRenderingSceneScaffoldFactory.cs"));
    string writerSource = File.ReadAllText(
        Path.Combine(CityProjectRootPath, "assets", "codebase", "rendering.tools", "GeneratedAuthoringSceneWriteService.cs"));

    Assert.Contains("const string NintendoDsDebugFontRelativePath = \"assets/fonts/DemoDiscBody.hefont\";", scaffoldSource, StringComparison.Ordinal);
    Assert.Contains("Font = bottomOverlayFont,", scaffoldSource, StringComparison.Ordinal);
    Assert.DoesNotContain("Font = ResolveRequiredEditorFont()", scaffoldSource, StringComparison.Ordinal);
    Assert.Contains("FontAsset bottomOverlayFont", scaffoldSource, StringComparison.Ordinal);
    Assert.Contains("ResolveRequiredNintendoDsDebugFont(fullProjectRootPath)", writerSource, StringComparison.Ordinal);
    Assert.Contains("CreateSceneRoots(", writerSource, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the audit to verify it fails**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "CityNintendoDsSceneSourceAuditTests"
```

Expected:

```text
FAIL CityNintendoDsSceneSourceAuditTests.Sources_whenAuthoringDsBottomOverlay_useProjectDebugFontInsteadOfEditorDefaultFont
```

- [ ] **Step 3: Commit the failing audit**

```bash
git add builder.tests/CityNintendoDsSceneSourceAuditTests.cs
git commit -m "test: lock DS debug font scaffold contract"
```

### Task 2: Resolve The DS Debug Font From The Project

**Files:**
- Modify: `..\..\helprojs\city\assets\codebase\rendering.tools\GeneratedAuthoringSceneWriteService.cs`
- Modify: `..\..\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs`
- Test: `builder.tests/CityNintendoDsSceneSourceAuditTests.cs`

- [ ] **Step 1: Change the scaffold signature to accept the resolved DS font**

Update `NintendoDsRenderingSceneScaffoldFactory` so `CreateSceneRoots(...)` and `CreateDefaultBottomOverlay(...)` take the project font instead of resolving the editor default internally:

```csharp
public Entity[] CreateSceneRoots(
    Entity[] topScreenRoots,
    bool useDefaultBottomOverlay,
    Entity[] bottomScreenRoots,
    FontAsset bottomOverlayFont) {
    if (bottomOverlayFont == null) {
        throw new ArgumentNullException(nameof(bottomOverlayFont));
    }

    // existing setup
    if (useDefaultBottomOverlay) {
        CreateDefaultBottomOverlay(bottomScreenViewportRoot, bottomOverlayFont);
    }
}

void CreateDefaultBottomOverlay(Entity bottomScreenViewportRoot, FontAsset bottomOverlayFont) {
    if (bottomOverlayFont == null) {
        throw new ArgumentNullException(nameof(bottomOverlayFont));
    }

    DebugComponent debugComponent = new DebugComponent();
    debugComponent.Font = bottomOverlayFont;
    // existing setup

    textEntity.AddComponent(new TextComponent {
        Text = "BACK",
        Font = bottomOverlayFont,
        FontScale = NintendoDsBottomOverlayFontScale,
        // existing setup
    });
}
```

- [ ] **Step 2: Add one project-root-aware font resolver in the writer**

Use the City project root already available in `GeneratedAuthoringSceneWriteService` to load the DS debug font once and pass it into the scaffold:

```csharp
FontAsset bottomOverlayFont = ResolveRequiredNintendoDsDebugFont(fullProjectRootPath);
Entity[] nintendoDsSceneRoots = NintendoDsRenderingSceneScaffoldFactoryValue.CreateSceneRoots(
    topScreenRoots,
    useDefaultBottomOverlay,
    bottomScreenRootEntities,
    bottomOverlayFont);

FontAsset ResolveRequiredNintendoDsDebugFont(string fullProjectRootPath) {
    if (string.IsNullOrWhiteSpace(fullProjectRootPath)) {
        throw new ArgumentException("Project root path must be provided.", nameof(fullProjectRootPath));
    }

    string fontPath = Path.Combine(fullProjectRootPath, "assets", "fonts", "DemoDiscBody.hefont");
    if (!File.Exists(fontPath)) {
        throw new InvalidOperationException("Nintendo DS debug font asset is missing: " + fontPath);
    }

    using FileStream stream = File.OpenRead(fontPath);
    return helengine.files.FontAssetBinarySerializer.Deserialize(stream);
}
```

- [ ] **Step 3: Remove the editor-font helper from the DS scaffold**

Delete the old helper and constant references:

```csharp
// remove
FontAsset ResolveRequiredEditorFont() {
    if (Core.Instance is not EditorCore editorCore || editorCore.DefaultFontAssetForEditor == null) {
        throw new InvalidOperationException("A default editor font must be loaded before Nintendo DS rendering showcase scenes can be generated.");
    }

    return editorCore.DefaultFontAssetForEditor;
}
```

- [ ] **Step 4: Run the audit to verify it passes**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "CityNintendoDsSceneSourceAuditTests"
```

Expected:

```text
PASS CityNintendoDsSceneSourceAuditTests.Sources_whenAuthoringDsBottomOverlay_useProjectDebugFontInsteadOfEditorDefaultFont
PASS CityNintendoDsSceneSourceAuditTests.Assets_whenDsBootFlowIsSupported_includeDsMenuSceneAndDsPlayableSceneMappings
```

- [ ] **Step 5: Commit**

```bash
git add builder.tests/CityNintendoDsSceneSourceAuditTests.cs ..\..\helprojs\city\assets\codebase\rendering.tools\GeneratedAuthoringSceneWriteService.cs ..\..\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs
git commit -m "feat: use project debug font for DS bottom overlay"
```

### Task 3: Persist DS Font Format On The Chosen Project Font

**Files:**
- Modify: `..\..\helprojs\city\assets\fonts\DemoDiscBody.ttf.hasset`
- Test: `builder.tests/NintendoDsPlatformAssetBuilderTests.cs`

- [ ] **Step 1: Add the failing builder assertion for the DS font atlas output**

Extend the existing DS font cook coverage so the chosen project debug font atlas is expected to build as `Indexed4` with binary alpha:

```csharp
[Fact]
public void Builder_whenCookingChosenDsDebugFont_preserveIndexed4BinaryAtlasOutput() {
    TextureAsset stagedAtlasTextureAsset = /* arrange existing staged font cook flow for DemoDiscBody */;

    Assert.Equal(TextureAssetColorFormat.Indexed4, stagedAtlasTextureAsset.ColorFormat);
    Assert.Equal(TextureAssetAlphaPrecision.Binary, stagedAtlasTextureAsset.AlphaPrecision);
}
```

- [ ] **Step 2: Run the focused builder test to verify the current project asset state**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsPlatformAssetBuilderTests"
```

Expected:

```text
PASS for the existing DS default metadata tests
FAIL for Builder_whenCookingChosenDsDebugFont_preserveIndexed4BinaryAtlasOutput if DemoDiscBody.ttf.hasset still lacks the DS override
```

- [ ] **Step 3: Persist the DS platform settings on the chosen font asset**

Use the existing City font source asset sidecar and store the DS atlas override on `DemoDiscBody.ttf.hasset` so the chosen overlay font cooks as:

```json
{
  "font-atlas-texture": {
    "ds": {
      "maxResolution": 256,
      "colorFormat": "Indexed4",
      "alphaPrecision": "Binary"
    }
  }
}
```

If the asset sidecar is binary-authored in this project, use the normal City editor asset inspector to persist that exact DS override and commit the resulting `DemoDiscBody.ttf.hasset` bytes.

- [ ] **Step 4: Re-run the focused builder test**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "NintendoDsPlatformAssetBuilderTests"
```

Expected:

```text
PASS Builder_whenCookingChosenDsDebugFont_preserveIndexed4BinaryAtlasOutput
```

- [ ] **Step 5: Commit**

```bash
git add builder.tests/NintendoDsPlatformAssetBuilderTests.cs ..\..\helprojs\city\assets\fonts\DemoDiscBody.ttf.hasset
git commit -m "feat: mark DS debug font atlas as indexed4"
```

### Task 4: Rebuild The DS Scene Output And Verify Runtime

**Files:**
- Use: `..\..\helprojs\city\project.heproj`
- Use: `..\helengine\artifacts\build-platform.ps1`
- Verify output: `..\..\helprojs\city\ds-build\helengine_ds.nds`

- [ ] **Step 1: Run the focused verification suite**

Run:

```powershell
dotnet test builder.tests/helengine.ds.builder.tests.csproj -c Release --no-restore --filter "CityNintendoDsSceneSourceAuditTests|NintendoDsPlatformAssetBuilderTests"
```

Expected:

```text
PASS with 0 failed
```

- [ ] **Step 2: Build the DS ROM with the shared wrapper**

Run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 `
  -Project ..\..\helprojs\city\project.heproj `
  -Platform ds `
  -Output ..\..\helprojs\city\ds-build `
  -Configuration Release
```

Expected:

```text
Build completed successfully for platform ds
```

- [ ] **Step 3: Launch and verify the bottom screen**

Run:

```powershell
.\artifacts\launch-melonds-rom.ps1 -EmulatorPath ..\emus\melonDS-1.1-windows-x86_64\melonDS.exe -RomPath ..\..\helprojs\city\ds-build\helengine_ds.nds
```

Expected:

```text
Bottom screen shows BACK and DS debug text again
No black bottom screen
No dependency on the editor default font
```

- [ ] **Step 4: Commit**

```bash
git add builder.tests/CityNintendoDsSceneSourceAuditTests.cs builder.tests/NintendoDsPlatformAssetBuilderTests.cs ..\..\helprojs\city\assets\codebase\rendering.tools\GeneratedAuthoringSceneWriteService.cs ..\..\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs ..\..\helprojs\city\assets\fonts\DemoDiscBody.ttf.hasset
git commit -m "feat: switch DS overlay to dedicated project font"
```
