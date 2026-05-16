# DS Main Menu 2D Rendering Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make Nintendo DS builds boot into the authored `DemoDiscMainMenu` scene and visually render its first-pass 2D presentation on the top screen.

**Architecture:** The implementation stays DS-specific. The builder rewrites the staged generated-core startup manifest and validates `DemoDiscMainMenu` as the effective DS startup scene, `NintendoDsBootHost` adds a top-screen 2D menu mode alongside the existing 3D mode, and `NintendoDsRenderManager2D` grows from a no-op surface into a narrow presenter for the menu's sprite, text, and rounded-rectangle primitives. The slice remains visual-only and does not add menu input or mixed 2D/3D composition.

**Tech Stack:** C#, xUnit builder/source-audit tests, generated-core staging, native C++ Nintendo DS runtime, devkitARM DS video APIs, `melonDS`.

---

## File Structure

- Create: `builder/NintendoDsStartupSceneIds.cs`
  Stores the DS-owned startup-scene constants for `DemoDiscMainMenu`, including the scene id and cooked relative path.
- Modify: `builder/NintendoDsGeneratedCoreStager.cs`
  Rewrites staged `runtime/runtime_startup_manifest.cpp` so DS builds boot into the menu cooked scene.
- Modify: `builder/NintendoDsPlatformAssetBuilder.cs`
  Treats `DemoDiscMainMenu` as the effective DS startup scene when validating staged NitroFS payloads.
- Modify: `builder.tests/NintendoDsGeneratedCoreStagerTests.cs`
  Adds focused regression coverage for the staged startup-manifest rewrite.
- Modify: `builder.tests/NintendoDsPlatformAssetBuilderTests.cs`
  Adds DS build coverage proving the effective startup scene and cooked payload are the menu scene.
- Modify: `src/platform/ds/NintendoDsBootHost.hpp`
  Declares the menu/3D top-screen preparation seam and any helper methods needed to detect the configured startup scene.
- Modify: `src/platform/ds/NintendoDsBootHost.cpp`
  Chooses top-screen 2D for `DemoDiscMainMenu`, injects the framebuffer into the DS 2D renderer, and preserves the existing 3D path for other scenes.
- Modify: `builder.tests/NintendoDsBootHostSourceAuditTests.cs`
  Audits the boot host source for menu 2D and 3D display-path selection.
- Create: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
  Audits the DS 2D renderer source to prove sprite/text/rounded-rect draw calls are no longer pure no-ops.
- Create: `src/platform/ds/NintendoDsRuntimeTexture2D.hpp`
  Defines the DS-owned runtime texture shape that stores converted 16-bit pixels for menu sprites and font atlases.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
  Declares framebuffer configuration, DS texture conversion, and raster helper seams for menu primitives.
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`
  Implements `BuildTextureFromRaw(...)` and first-pass top-screen sprite, text, and rounded-rect rasterization.

## Task 1: DS Startup Scene Override

**Files:**
- Create: `builder/NintendoDsStartupSceneIds.cs`
- Modify: `builder/NintendoDsGeneratedCoreStager.cs`
- Modify: `builder/NintendoDsPlatformAssetBuilder.cs`
- Modify: `builder.tests/NintendoDsGeneratedCoreStagerTests.cs`
- Modify: `builder.tests/NintendoDsPlatformAssetBuilderTests.cs`

- [ ] **Step 1: Write the failing generated-core and builder tests**

Add one staged-manifest rewrite test and one build-orchestration test that treat `DemoDiscMainMenu` as the DS-owned startup scene even when the authored startup scene points elsewhere.

```csharp
/// <summary>
/// Verifies DS staging rewrites the generated runtime startup manifest to the demo-disc main menu payload.
/// </summary>
[Fact]
public void Stage_whenRuntimeStartupManifestTargetsAnotherScene_rewritesToDemoDiscMainMenu() {
    string sourceRootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-stage-src-" + Guid.NewGuid().ToString("N"));
    string destinationRootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-stage-dst-" + Guid.NewGuid().ToString("N"));

    Directory.CreateDirectory(Path.Combine(sourceRootPath, "runtime"));
    File.WriteAllText(Path.Combine(sourceRootPath, "helcpp_config.hpp"), "#pragma once");
    File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
    File.WriteAllText(Path.Combine(sourceRootPath, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
    File.WriteAllText(Path.Combine(sourceRootPath, "RuntimeComponentRegistry.cpp"), "#include \"RuntimeComponentRegistry.hpp\"\\n#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\\n::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
    File.WriteAllText(
        Path.Combine(sourceRootPath, "runtime", "runtime_startup_manifest.cpp"),
        "const char* he_get_runtime_startup_scene_relative_path() { return \\\"cooked/scenes/rendering/colored_cube_grid.hasset\\\"; }");

    new NintendoDsGeneratedCoreStager().Stage(sourceRootPath, destinationRootPath);

    string stagedManifestSource = File.ReadAllText(Path.Combine(destinationRootPath, "runtime", "runtime_startup_manifest.cpp"));
    Assert.Contains("cooked/scenes/DemoDiscMainMenu.hasset", stagedManifestSource, StringComparison.Ordinal);
}

/// <summary>
/// Verifies DS builds validate and stage the demo-disc main menu as the effective startup scene.
/// </summary>
[Fact]
public async Task BuildAsync_whenManifestStartupSceneDiffersFromDsMenuOverride_usesDemoDiscMainMenuAsEffectiveStartupScene() {
    string repositoryRoot = "/mnt/c/dev/helworks/helengine-ds";
    string workingRoot = Path.Combine(Path.GetTempPath(), "helengine-ds-build-" + Guid.NewGuid().ToString("N"));
    string outputRoot = Path.Combine(workingRoot, "out");
    string generatedCoreRoot = Path.Combine(workingRoot, "generated-core");
    string packageRoot = Path.Combine(workingRoot, "tmp", NintendoDsBuildPathConventions.PackageSourceDirectoryName);

    Directory.CreateDirectory(Path.Combine(generatedCoreRoot, "runtime"));
    Directory.CreateDirectory(Path.Combine(packageRoot, "cooked", "scenes", "rendering"));
    Directory.CreateDirectory(Path.Combine(packageRoot, "cooked", "scenes"));
    File.WriteAllText(Path.Combine(generatedCoreRoot, "helcpp_config.hpp"), "#pragma once");
    File.WriteAllText(Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.hpp"), "#pragma once");
    File.WriteAllText(Path.Combine(generatedCoreRoot, "GeneratedRuntimeComponentDeserializerRegistration.cpp"), "void RegisterGeneratedRuntimeComponentDeserializers(::RuntimeComponentRegistry* registry) { (void)registry; }");
    File.WriteAllText(Path.Combine(generatedCoreRoot, "RuntimeComponentRegistry.cpp"), "#include \"RuntimeComponentRegistry.hpp\"\\n#include \"GeneratedRuntimeComponentDeserializerRegistration.hpp\"\\n::RuntimeComponentRegistry* RuntimeComponentRegistry::CreateDefault() { ::RuntimeComponentRegistry* registry = new ::RuntimeComponentRegistry(); RegisterGeneratedRuntimeComponentDeserializers(registry); return registry; }");
    File.WriteAllText(Path.Combine(generatedCoreRoot, "runtime", "runtime_startup_manifest.cpp"), "const char* he_get_runtime_startup_scene_relative_path() { return \\\"cooked/scenes/rendering/colored_cube_grid.hasset\\\"; }");
    File.WriteAllBytes(Path.Combine(packageRoot, "cooked", "scenes", "rendering", "colored_cube_grid.hasset"), BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));
    File.WriteAllBytes(Path.Combine(packageRoot, "cooked", "scenes", "DemoDiscMainMenu.hasset"), BuildSceneAssetBytes(includeUnsupportedReturnToMenuComponent: false));

    PlatformBuildScene[] scenes = [
        new PlatformBuildScene(
            "colored_cube_grid",
            "Colored Cube Grid",
            "scene",
            [new PlatformBuildPayloadReference("cooked/scenes/rendering/colored_cube_grid.hasset", "cooked/scenes/rendering/colored_cube_grid.hasset")],
            [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/rendering/colored_cube_grid.hasset")]),
        new PlatformBuildScene(
            "DemoDiscMainMenu",
            "Demo Disc Main Menu",
            "scene",
            [new PlatformBuildPayloadReference("cooked/scenes/DemoDiscMainMenu.hasset", "cooked/scenes/DemoDiscMainMenu.hasset")],
            [new KeyValuePair<string, string>(PlatformBuildSceneMetadataKeys.CookedRelativePath, "cooked/scenes/DemoDiscMainMenu.hasset")])
    ];

    PlatformBuildManifest manifest = new(
        3,
        "project",
        "1.0.0",
        "1.0.0",
        "ds",
        "1",
        "colored_cube_grid",
        scenes,
        Array.Empty<PlatformBuildAsset>(),
        Array.Empty<PlatformBuildArtifact>(),
        Array.Empty<PlatformBuildCodeModule>(),
        Array.Empty<PlatformArtifactPlacement>(),
        new PlatformContainerWritePlan("ds-nitrofs-package", Array.Empty<PlatformContainerArtifact>()));

    RecordingDiagnosticReporter diagnosticReporter = new();
    RecordingProgressReporter progressReporter = new();
    FakeNintendoDsNativeBuildExecutor nativeBuildExecutor = new();
    NintendoDsPlatformAssetBuilder builder = new(nativeBuildExecutor, repositoryRoot);

    PlatformBuildRequest request = new(
        manifest,
        [new PlatformBuildTargetVariant("ds-default", "ds", "ds", "ds-default")],
        [new PlatformCookProfile(
            "ds-default",
            "DS Default",
            new PlatformCookProfileCapabilities(
                "ds",
                "raw",
                "raw",
                "ds-scene-v1",
                PlatformSerializationEndianness.LittleEndian))],
        outputRoot,
        Path.Combine(workingRoot, "tmp"),
        selectedBuildProfileId: "ds-default",
        selectedGraphicsProfileId: "ds-main-2d",
        selectedCodegenProfileId: "default",
        selectedBuildOptionValues: new Dictionary<string, string> {
            ["startup-top-screen-color"] = "#FF0000",
            ["startup-bottom-screen-color"] = "#0000FF"
        },
        selectedGraphicsOptionValues: new Dictionary<string, string>(),
        selectedCodegenOptionValues: new Dictionary<string, string>(),
        generatedCoreCppRootPath: generatedCoreRoot,
        selectedMediaProfileId: "ds-cartridge",
        selectedStorageProfileId: "nitrofs-package");

    await builder.BuildAsync(request, progressReporter, diagnosticReporter, CancellationToken.None);

    string stagedStartupScenePath = Path.Combine(nativeBuildExecutor.Workspace!.NitroFsRootPath, "cooked", "scenes", "DemoDiscMainMenu.hasset");
    string stagedRuntimeManifestPath = Path.Combine(nativeBuildExecutor.Workspace.StagedGeneratedCoreRootPath, "runtime", "runtime_startup_manifest.cpp");
    Assert.True(File.Exists(stagedStartupScenePath));
    Assert.Contains("cooked/scenes/DemoDiscMainMenu.hasset", File.ReadAllText(stagedRuntimeManifestPath), StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the focused builder tests to verify they fail**

Run:

```powershell
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsGeneratedCoreStagerTests.Stage_whenRuntimeStartupManifestTargetsAnotherScene_rewritesToDemoDiscMainMenu|FullyQualifiedName~NintendoDsPlatformAssetBuilderTests.BuildAsync_whenManifestStartupSceneDiffersFromDsMenuOverride_usesDemoDiscMainMenuAsEffectiveStartupScene" -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\menu-task1\' -v minimal
```

Expected: FAIL because DS staging still preserves the authored startup manifest path and builder validation still follows `manifest.StartupSceneId`.

- [ ] **Step 3: Implement the DS-owned startup-scene override**

Add one shared DS startup-scene constant file, rewrite the staged runtime startup manifest to the menu cooked path, and validate the effective DS startup scene against the menu scene metadata.

```csharp
// builder/NintendoDsStartupSceneIds.cs
namespace helengine.ds.builder;

/// <summary>
/// Stores the Nintendo DS-owned startup-scene identifiers used by the menu boot path.
/// </summary>
public static class NintendoDsStartupSceneIds {
    /// <summary>
    /// Scene id of the authored demo-disc main menu.
    /// </summary>
    public const string DemoDiscMainMenuSceneId = "DemoDiscMainMenu";

    /// <summary>
    /// Cooked runtime-relative payload path for the authored demo-disc main menu.
    /// </summary>
    public const string DemoDiscMainMenuCookedRelativePath = "cooked/scenes/DemoDiscMainMenu.hasset";
}

// builder/NintendoDsGeneratedCoreStager.cs
static readonly Regex RuntimeStartupScenePathPattern = new(
    "return \\\"[^\\\"]+\\\";",
    RegexOptions.CultureInvariant);

static void RewriteRuntimeStartupScenePath(string destinationRootPath) {
    RewriteFile(
        Path.Combine(destinationRootPath, "runtime", "runtime_startup_manifest.cpp"),
        source => RuntimeStartupScenePathPattern.Replace(
            source,
            $"return \\\"{NintendoDsStartupSceneIds.DemoDiscMainMenuCookedRelativePath}\\\";",
            1));
}

// builder/NintendoDsPlatformAssetBuilder.cs
static PlatformBuildScene FindNintendoDsStartupScene(PlatformBuildManifest manifest) {
    for (int index = 0; index < manifest.Scenes.Length; index++) {
        PlatformBuildScene scene = manifest.Scenes[index];
        if (string.Equals(scene.SceneId, NintendoDsStartupSceneIds.DemoDiscMainMenuSceneId, StringComparison.Ordinal)) {
            return scene;
        }
    }

    throw new InvalidOperationException(
        $"Nintendo DS requires startup scene '{NintendoDsStartupSceneIds.DemoDiscMainMenuSceneId}' to be present in the build manifest.");
}
```

- [ ] **Step 4: Run the focused builder tests to verify they pass**

Run:

```powershell
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsGeneratedCoreStagerTests.Stage_whenRuntimeStartupManifestTargetsAnotherScene_rewritesToDemoDiscMainMenu|FullyQualifiedName~NintendoDsPlatformAssetBuilderTests.BuildAsync_whenManifestStartupSceneDiffersFromDsMenuOverride_usesDemoDiscMainMenuAsEffectiveStartupScene" -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\menu-task1\' -v minimal
```

Expected: PASS with the staged generated-core startup manifest and DS build validation both targeting `cooked/scenes/DemoDiscMainMenu.hasset`.

- [ ] **Step 5: Commit the startup override slice**

```bash
git add builder/NintendoDsStartupSceneIds.cs builder/NintendoDsGeneratedCoreStager.cs builder/NintendoDsPlatformAssetBuilder.cs builder.tests/NintendoDsGeneratedCoreStagerTests.cs builder.tests/NintendoDsPlatformAssetBuilderTests.cs
git commit -m "feat: boot DS builds into demo disc main menu"
```

## Task 2: Add the DS Top-Screen Menu/3D Mode Split

**Files:**
- Modify: `src/platform/ds/NintendoDsBootHost.hpp`
- Modify: `src/platform/ds/NintendoDsBootHost.cpp`
- Modify: `builder.tests/NintendoDsBootHostSourceAuditTests.cs`

- [ ] **Step 1: Write the failing boot-host source audit**

Extend the existing boot-host audit so it proves the checkpointed startup path selects a menu-specific 2D preparation seam instead of always forcing `PrepareMainScreenFor3D()`.

```csharp
/// <summary>
/// Verifies the Nintendo DS boot host distinguishes menu startup presentation from the default 3D startup path.
/// </summary>
[Fact]
public void Source_whenCheckpointedStartupCompletes_usesConfiguredTopScreenPresentationPath() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
    string sourceCode = File.ReadAllText(sourcePath);

    Assert.Contains("PrepareMainScreenForConfiguredStartupScene();", sourceCode, StringComparison.Ordinal);
    Assert.Contains("void NintendoDsBootHost::PrepareMainScreenForMenu2D()", sourceCode, StringComparison.Ordinal);
    Assert.Contains("void NintendoDsBootHost::PrepareMainScreenFor3D()", sourceCode, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the focused boot-host audit to verify it fails**

Run:

```powershell
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsBootHostSourceAuditTests" -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\menu-task2\' -v minimal
```

Expected: FAIL because `RunCheckpointedStartup()` still calls `PrepareMainScreenFor3D()` directly and no menu-2D preparation seam exists.

- [ ] **Step 3: Implement DS menu-screen presentation selection in the boot host**

Add explicit menu detection, inject the top-screen framebuffer into the 2D renderer during core initialization, and split menu 2D setup from the existing 3D setup.

```cpp
// src/platform/ds/NintendoDsBootHost.hpp
bool IsMenuStartupSceneConfigured() const;
void PrepareMainScreenForConfiguredStartupScene();
void PrepareMainScreenForMenu2D();

// src/platform/ds/NintendoDsBootHost.cpp
bool NintendoDsBootHost::IsMenuStartupSceneConfigured() const {
    const char* startupSceneRelativePath = he_get_runtime_startup_scene_relative_path();
    return startupSceneRelativePath != nullptr
        && std::string(startupSceneRelativePath) == "cooked/scenes/DemoDiscMainMenu.hasset";
}

void NintendoDsBootHost::PrepareMainScreenForConfiguredStartupScene() {
    if (IsMenuStartupSceneConfigured()) {
        PrepareMainScreenForMenu2D();
        return;
    }

    PrepareMainScreenFor3D();
}

void NintendoDsBootHost::PrepareMainScreenForMenu2D() {
    videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
    vramSetBankA(VRAM_A_MAIN_BG);
}

// inside InitializeCore()
EngineRenderManager2D = new NintendoDsRenderManager2D();
EngineRenderManager2D->ConfigureTopScreenFrameBuffer(MainFrameBuffer, FrameBufferWidth, ScreenHeight);

// inside RunCheckpointedStartup()
PrepareMainScreenForConfiguredStartupScene();
```

- [ ] **Step 4: Run the focused boot-host audit to verify it passes**

Run:

```powershell
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsBootHostSourceAuditTests" -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\menu-task2\' -v minimal
```

Expected: PASS with the new menu 2D and fallback 3D preparation seams present in source.

- [ ] **Step 5: Commit the boot-host slice**

```bash
git add src/platform/ds/NintendoDsBootHost.hpp src/platform/ds/NintendoDsBootHost.cpp builder.tests/NintendoDsBootHostSourceAuditTests.cs
git commit -m "feat: add DS menu top-screen presentation mode"
```

## Task 3: Implement the Minimal DS 2D Menu Renderer

**Files:**
- Create: `src/platform/ds/NintendoDsRuntimeTexture2D.hpp`
- Create: `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.hpp`
- Modify: `src/platform/ds/NintendoDsRenderManager2D.cpp`

- [ ] **Step 1: Write the failing DS 2D renderer source audit**

Add a source audit that proves the DS 2D renderer no longer discards menu draw calls and instead routes them through concrete raster helpers.

```csharp
/// <summary>
/// Verifies the DS 2D renderer routes sprite, text, and rounded-rectangle draws to concrete raster helpers.
/// </summary>
[Fact]
public void Source_whenDrawingMenuPrimitives_noLongerUsesNoOpDiscardBodies() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
    string sourceCode = File.ReadAllText(sourcePath);

    Assert.Contains("RasterRoundedRect(shape);", sourceCode, StringComparison.Ordinal);
    Assert.Contains("RasterSprite(sprite);", sourceCode, StringComparison.Ordinal);
    Assert.Contains("RasterText(text);", sourceCode, StringComparison.Ordinal);
    Assert.DoesNotContain("(void)shape;", sourceCode, StringComparison.Ordinal);
    Assert.DoesNotContain("(void)sprite;", sourceCode, StringComparison.Ordinal);
    Assert.DoesNotContain("(void)text;", sourceCode, StringComparison.Ordinal);
}
```

- [ ] **Step 2: Run the focused renderer audit to verify it fails**

Run:

```powershell
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests" -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\menu-task3\' -v minimal
```

Expected: FAIL because `DrawRoundedRect(...)`, `DrawSprite(...)`, and `DrawText(...)` are still pure discard bodies.

- [ ] **Step 3: Implement the minimal DS top-screen 2D renderer**

Add a DS-owned runtime texture type, convert RGBA textures into DS 15-bit pixels, and rasterize the menu primitives directly into the injected top-screen framebuffer.

```cpp
// src/platform/ds/NintendoDsRuntimeTexture2D.hpp
#pragma once

#include <vector>

#include "RuntimeTexture.hpp"

namespace helengine::ds {
    /// Stores Nintendo DS 16-bit top-screen texture pixels used by the menu renderer.
    class NintendoDsRuntimeTexture2D : public RuntimeTexture {
    public:
        std::vector<u16> Pixels;
    };
}

// src/platform/ds/NintendoDsRenderManager2D.hpp
void ConfigureTopScreenFrameBuffer(u16* frameBuffer, int frameBufferWidth, int frameBufferHeight);
void RasterRoundedRect(IRoundedRectDrawable2D* shape);
void RasterSprite(ISpriteDrawable2D* sprite);
void RasterText(ITextDrawable2D* text);

// src/platform/ds/NintendoDsRenderManager2D.cpp
RuntimeTexture* NintendoDsRenderManager2D::BuildTextureFromRaw(TextureAsset* data) {
    auto* texture = new NintendoDsRuntimeTexture2D();
    texture->set_Id(data->get_Id());
    texture->set_Width(data->Width);
    texture->set_Height(data->Height);
    texture->Pixels = ConvertRgba8888ToDs1555(data->Colors, data->Width, data->Height);
    return texture;
}

void NintendoDsRenderManager2D::DrawRoundedRect(IRoundedRectDrawable2D* shape) {
    if (shape == nullptr) {
        throw new ArgumentNullException("shape");
    }

    RasterRoundedRect(shape);
}

void NintendoDsRenderManager2D::DrawSprite(ISpriteDrawable2D* sprite) {
    if (sprite == nullptr) {
        throw new ArgumentNullException("sprite");
    }

    RasterSprite(sprite);
}

void NintendoDsRenderManager2D::DrawText(ITextDrawable2D* text) {
    if (text == nullptr) {
        throw new ArgumentNullException("text");
    }

    RasterText(text);
}
```

Implementation notes for this slice:

- Use the injected top-screen framebuffer and clip writes to the visible `256x192` area.
- Keep the first rounded-rect implementation simple but real: fill body pixels, respect border color where `BorderThickness > 0`, and round the corners using the authored radius.
- Render text from `text->Font`, using `FontAsset::Characters`, `AtlasWidth`, `AtlasHeight`, and the DS runtime texture built from the font atlas.
- Treat unsupported drawable state as a hard failure instead of silently dropping pixels.

- [ ] **Step 4: Run the focused renderer audit to verify it passes**

Run:

```powershell
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests" -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\menu-task3\' -v minimal
```

Expected: PASS with sprite/text/rounded-rect draws routed to real raster helpers.

- [ ] **Step 5: Commit the renderer slice**

```bash
git add src/platform/ds/NintendoDsRuntimeTexture2D.hpp src/platform/ds/NintendoDsRenderManager2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs
git commit -m "feat: render demo disc menu on DS top screen"
```

## Task 4: Run Full DS Verification

**Files:**
- Verify only: no source changes expected unless verification exposes a concrete regression

- [ ] **Step 1: Run the DS-focused regression suite**

Run:

```powershell
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsGeneratedCoreStagerTests.Stage_whenRuntimeStartupManifestTargetsAnotherScene_rewritesToDemoDiscMainMenu|FullyQualifiedName~NintendoDsPlatformAssetBuilderTests.BuildAsync_whenManifestStartupSceneDiffersFromDsMenuOverride_usesDemoDiscMainMenuAsEffectiveStartupScene|FullyQualifiedName~NintendoDsBootHostSourceAuditTests|FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests" -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\menu-task4\' -v minimal
```

Expected: PASS with the startup override, boot-host mode split, and DS 2D renderer audits all green.

- [ ] **Step 2: Build the DS city project end to end**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -p:BaseOutputPath='C:\dev\helworks\helengine\.codex-build-ds-menu2d\bin\' -- --project C:\tmp\helengine-ds-city-cube-project\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected: PASS with `Build completed for platform 'ds'` and a rebuilt ROM at `C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds`.

- [ ] **Step 3: Launch the rebuilt ROM in melonDS**

Run:

```powershell
rtk melonDS C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds
```

Expected: the authored `DemoDiscMainMenu` visually appears on the top screen. Input can still be non-functional in this slice.

- [ ] **Step 4: If verification stayed green, commit any final fixups**

```bash
git status --short
git commit -m "test: verify DS main menu 2d startup path"
```

Expected: no new commit is needed unless end-to-end verification required a targeted fix.

## Self-Review

- Spec coverage:
  - DS-only startup-scene override: covered by Task 1.
  - Top-screen 2D menu presentation and 3D fallback seam: covered by Task 2.
  - Minimal sprite/text/rounded-rect DS renderer: covered by Task 3.
  - End-to-end DS build and emulator validation: covered by Task 4.
- Placeholder scan:
  - No `TODO`, `TBD`, or “implement later” placeholders remain.
  - Unsupported-state behavior is explicit: fail loudly.
- Type consistency:
  - Startup-scene constants flow through `NintendoDsStartupSceneIds`.
  - Boot-host seam names stay consistent: `PrepareMainScreenForConfiguredStartupScene()`, `PrepareMainScreenForMenu2D()`, `PrepareMainScreenFor3D()`.
  - DS 2D renderer helper names stay consistent: `RasterRoundedRect(...)`, `RasterSprite(...)`, `RasterText(...)`.
