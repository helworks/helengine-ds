# Generic Font Atlas Platform Cook Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add generic per-platform font-atlas texture cook settings, drive font-atlas cooking through the dedicated `font-atlas-texture` capability when available, and make the DS cooked-font proof use a DS-supported cooked atlas format.

**Architecture:** Extend the registry-backed asset import settings model with a dedicated `font-atlas-texture` section that reuses texture processor settings. Use that section in the font asset settings UI and in scene/font packaging so platforms that publish `font-atlas-texture` cook generated font atlases with the correct capability while still emitting normal cooked texture assets such as `.hetex`.

**Tech Stack:** C# / .NET 9, xUnit, Helengine editor asset settings registry, Helengine scene packaging transform, Nintendo DS builder cook pipeline.

---

### Task 1: Add registry-backed font-atlas texture platform settings

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\FontAtlasTextureAssetPlatformSettingsSectionDefinition.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetPlatformProcessorSettings.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetPlatformSettingsSectionRegistry.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\asset\AssetPlatformSettingsSectionRegistryTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\AssetImportSettingsBinarySerializerTests.cs`

- [ ] **Step 1: Write the failing registry and serialization tests**

```csharp
[Fact]
public void Shared_WhenResolvingFontAtlasTextureSection_ReturnsTextureProcessorSettings() {
    AssetPlatformProcessorSettings settings = new AssetPlatformProcessorSettings();

    TextureAssetProcessorSettings fontAtlas = settings.FontAtlasTexture;

    Assert.NotNull(fontAtlas);
    Assert.Equal(TextureAssetColorFormat.Rgba32, fontAtlas.ColorFormat);
}

[Fact]
public void Serialize_WhenFontAtlasTextureSettingsExist_RoundTripsSectionPayload() {
    AssetImportSettings settings = new AssetImportSettings();
    settings.Importer = new AssetImporterSettings { ImporterId = "gdi-font", SourceChecksum = "checksum", AssetId = "font-id" };
    settings.Processor = new AssetProcessorSettings();
    settings.Processor.Platforms["ds"] = new AssetPlatformProcessorSettings();
    settings.Processor.Platforms["ds"].FontAtlasTexture.ColorFormat = TextureAssetColorFormat.Indexed4;
    settings.Processor.Platforms["ds"].FontAtlasTexture.AlphaPrecision = TextureAssetAlphaPrecision.Binary;

    using MemoryStream stream = new MemoryStream();
    AssetImportSettingsBinarySerializer.Serialize(stream, settings);
    stream.Position = 0;

    AssetImportSettings result = AssetImportSettingsBinarySerializer.Deserialize(stream);

    Assert.Equal(TextureAssetColorFormat.Indexed4, result.Processor.Platforms["ds"].FontAtlasTexture.ColorFormat);
    Assert.Equal(TextureAssetAlphaPrecision.Binary, result.Processor.Platforms["ds"].FontAtlasTexture.AlphaPrecision);
}
```

- [ ] **Step 2: Run the focused tests to verify they fail**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "AssetPlatformSettingsSectionRegistryTests|AssetImportSettingsBinarySerializerTests"
```

Expected:
- FAIL because `AssetPlatformProcessorSettings` does not yet expose `FontAtlasTexture`
- FAIL because the new section is not registered or serialized

- [ ] **Step 3: Implement the dedicated section definition and registry-backed property**

```csharp
public sealed class FontAtlasTextureAssetPlatformSettingsSectionDefinition : IAssetPlatformSettingsSectionDefinition {
    public const string SectionIdValue = "font-atlas-texture";

    public string SectionId => SectionIdValue;
    public Type SettingsType => typeof(TextureAssetProcessorSettings);

    public object CreateDefaultSettings() {
        return new TextureAssetProcessorSettings();
    }

    public object CloneSettings(object settings) {
        TextureAssetProcessorSettings source = TextureAssetPlatformSettingsSectionDefinition.RequireSettings(settings);
        return TextureAssetPlatformSettingsSectionDefinition.CloneSettingsCore(source);
    }

    public bool SettingsEqual(object left, object right) {
        return TextureAssetPlatformSettingsSectionDefinition.SettingsEqualCore(
            TextureAssetPlatformSettingsSectionDefinition.RequireSettings(left),
            TextureAssetPlatformSettingsSectionDefinition.RequireSettings(right));
    }

    public void Serialize(EngineBinaryWriter writer, object settings) {
        TextureAssetPlatformSettingsSectionDefinition.SerializeSettingsCore(
            writer,
            TextureAssetPlatformSettingsSectionDefinition.RequireSettings(settings));
    }

    public object Deserialize(EngineBinaryReader reader) {
        return TextureAssetPlatformSettingsSectionDefinition.DeserializeSettingsCore(reader);
    }
}
```

```csharp
public TextureAssetProcessorSettings FontAtlasTexture {
    get => GetOrCreateSectionSettings<TextureAssetProcessorSettings>(FontAtlasTextureAssetPlatformSettingsSectionDefinition.SectionIdValue);
    set => SetSectionSettings(FontAtlasTextureAssetPlatformSettingsSectionDefinition.SectionIdValue, value);
}
```

- [ ] **Step 4: Run the focused tests to verify they pass**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "AssetPlatformSettingsSectionRegistryTests|AssetImportSettingsBinarySerializerTests"
```

Expected:
- PASS for the new registry and serialization coverage

- [ ] **Step 5: Commit**

```powershell
git -C C:\dev\helworks\helengine add `
  engine\helengine.editor\managers\asset\FontAtlasTextureAssetPlatformSettingsSectionDefinition.cs `
  engine\helengine.editor\managers\asset\AssetPlatformProcessorSettings.cs `
  engine\helengine.editor\managers\asset\AssetPlatformSettingsSectionRegistry.cs `
  engine\helengine.editor.tests\managers\asset\AssetPlatformSettingsSectionRegistryTests.cs `
  engine\helengine.editor.tests\serialization\AssetImportSettingsBinarySerializerTests.cs
git -C C:\dev\helworks\helengine commit -m "feat: add platform font atlas texture settings section"
```

### Task 2: Expose font-atlas texture settings in the font asset UI

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\components\ui\AssetImportSettingsView.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AssetImportSettingsViewTests.cs`

- [ ] **Step 1: Write the failing UI tests**

```csharp
[Fact]
public void Show_WhenFontAssetAndPlatformSupportsFontAtlasTexture_ShowsFontAtlasTextureControls() {
    AssetImportSettingsView view = CreateView();
    AssetImportSettings settings = CreateFontImportSettings("ds");
    settings.Processor.Platforms["ds"].FontAtlasTexture.ColorFormat = TextureAssetColorFormat.Indexed4;
    ConfigureFontEntry(view, "ds");
    ConfigureFontAtlasCapability("ds");

    view.Show(settings);

    Assert.True(view.IsFontAtlasTextureProcessorVisible);
    Assert.Equal(TextureAssetColorFormat.Indexed4, view.CurrentFontAtlasColorFormatValue);
}

[Fact]
public void Apply_WhenFontAtlasColorFormatChanges_RaisesPlatformScopedRequest() {
    AssetImportSettingsView view = CreateView();
    AssetImportSettings settings = CreateFontImportSettings("ds");
    ConfigureFontEntry(view, "ds");
    ConfigureFontAtlasCapability("ds");

    view.Show(settings);
    view.SetFontAtlasColorFormatForTest(TextureAssetColorFormat.Indexed4);

    AssetImportSettingsApplyRequest request = Assert.Single(CapturedRequests);
    Assert.Equal(TextureAssetColorFormat.Indexed4, request.Settings.Processor.Platforms["ds"].FontAtlasTexture.ColorFormat);
}
```

- [ ] **Step 2: Run the focused UI tests to verify they fail**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter AssetImportSettingsViewTests
```

Expected:
- FAIL because the view has no font-atlas texture controls yet

- [ ] **Step 3: Implement the additional font-atlas texture block**

```csharp
public bool IsFontAtlasTextureProcessorVisible =>
    CurrentEntryKind == AssetEntryKind.Font && CurrentPlatformSupportsFontAtlasTextureCapability();

TextureAssetProcessorSettings CurrentFontAtlasTextureSettings =>
    GetPendingPlatformSettings(CurrentPlatformId).FontAtlasTexture;

void SyncFontAtlasTextureControlsFromPendingSettings() {
    TextureAssetProcessorSettings settings = CurrentFontAtlasTextureSettings;
    FontAtlasColorFormatDropdown.SelectedValue = settings.ColorFormat.ToString();
    FontAtlasAlphaPrecisionDropdown.SelectedValue = settings.AlphaPrecision.ToString();
}
```

Implement the same capability-driven option filtering pattern already used for texture settings, but read it from the platform’s `font-atlas-texture` capability instead of generic `texture`.

- [ ] **Step 4: Run the focused UI tests to verify they pass**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter AssetImportSettingsViewTests
```

Expected:
- PASS for the new visibility and apply-request coverage

- [ ] **Step 5: Commit**

```powershell
git -C C:\dev\helworks\helengine add `
  engine\helengine.editor\components\ui\AssetImportSettingsView.cs `
  engine\helengine.editor.tests\AssetImportSettingsViewTests.cs
git -C C:\dev\helworks\helengine commit -m "feat: expose platform font atlas texture settings in editor"
```

### Task 3: Route packaged font atlases through the dedicated cook capability

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\SceneComponentPackagingTransformServiceTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorPlatformCookWorkItemFactoryTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorPlatformFontVariantCacheServiceTests.cs`

- [ ] **Step 1: Write the failing packaging tests**

```csharp
[Fact]
public void TryTransform_WhenPlatformOwnsDedicatedFontAtlasCook_UsesFontAtlasTextureWorkItemAndPlatformExtension() {
    SceneComponentPackagingTransformService service = CreateBuilderOwnedFontAtlasService(workItems, new StubTextComponentSpriteBakeService());
    WriteSourceFont("Fonts/DemoDiscTitle.ttf");

    bool transformed = service.TryTransform(CreateDebugRecord(CreateFileFontReference("Fonts/DemoDiscTitle.ttf")), BuildRootPath, out _);

    Assert.True(transformed);
    PlatformCookWorkItem workItem = Assert.Single(workItems);
    Assert.Equal("font-atlas-texture", workItem.SourceAssetKind);
    Assert.Equal("cooked/fonts/demodisctitle.hetex", workItem.OutputRelativePath);
}

[Fact]
public void ResolveVariant_WhenPlatformPublishesDedicatedFontAtlasCook_PreservesFontAtlasSettingsInCachedVariant() {
    EditorPlatformFontVariantCacheService service = CreateService();
    ConfigureFontImportSettings(
        "Fonts/DemoDiscBody.ttf",
        "ds",
        pixelSize: 10,
        colorFormat: TextureAssetColorFormat.Indexed4,
        alphaPrecision: TextureAssetAlphaPrecision.Binary);

    EditorPlatformFontVariantCacheResult result = service.ResolveVariant("Fonts/DemoDiscBody.ttf", "ds");

    Assert.EndsWith("DemoDiscBody.hefont", result.CachedFontAssetPath, StringComparison.OrdinalIgnoreCase);
    Assert.EndsWith("DemoDiscBody.hetex", result.CachedAtlasTextureAssetPath, StringComparison.OrdinalIgnoreCase);
}
```

- [ ] **Step 2: Run the focused packaging tests to verify they fail**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "SceneComponentPackagingTransformServiceTests|EditorPlatformCookWorkItemFactoryTests|EditorPlatformFontVariantCacheServiceTests"
```

Expected:
- FAIL because the transform still keys off generic `texture` assumptions for font atlases

- [ ] **Step 3: Implement dedicated font-atlas capability resolution with generic fallback**

```csharp
PlatformAssetCookCapabilityDefinition ResolveBuilderOwnedFontAtlasCookCapability() {
    return ResolveBuilderOwnedPlatformCookCapability("font-atlas-texture")
        ?? ResolveBuilderOwnedPlatformCookCapability("texture");
}

bool SupportsBuilderOwnedFontAtlasCookKind() {
    return ResolveBuilderOwnedFontAtlasCookCapability() != null;
}

string BuildCookedFontAtlasTextureRelativePath(string relativePath) {
    PlatformAssetCookCapabilityDefinition capability = ResolveBuilderOwnedFontAtlasCookCapability();
    string outputExtension = capability == null || string.IsNullOrWhiteSpace(capability.OutputFileExtension)
        ? ".hetex"
        : capability.OutputFileExtension;
    return NormalizeRelativePath(Path.Combine("cooked", Path.ChangeExtension(relativePath, outputExtension)));
}
```

Use the new helpers in:

- `RewriteFileSystemFontReference(...)`
- `RememberFontCookWorkItem(...)`
- `RememberGeneratedFontCookWorkItem(...)`

- [ ] **Step 4: Run the focused packaging tests to verify they pass**

Run:

```powershell
dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "SceneComponentPackagingTransformServiceTests|EditorPlatformCookWorkItemFactoryTests|EditorPlatformFontVariantCacheServiceTests"
```

Expected:
- PASS for the dedicated font-atlas cook path

- [ ] **Step 5: Commit**

```powershell
git -C C:\dev\helworks\helengine add `
  engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs `
  engine\helengine.editor.tests\managers\project\SceneComponentPackagingTransformServiceTests.cs `
  engine\helengine.editor.tests\managers\project\EditorPlatformCookWorkItemFactoryTests.cs `
  engine\helengine.editor.tests\managers\project\EditorPlatformFontVariantCacheServiceTests.cs
git -C C:\dev\helworks\helengine commit -m "feat: route packaged font atlases through dedicated cook capability"
```

### Task 4: Apply font-atlas settings in the DS builder and verify with a real DS build

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsPlatformAssetBuilder.cs`
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsPlatformAssetBuilderTests.cs`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsRenderManager2D.cpp`

- [ ] **Step 1: Write the failing DS builder test**

```csharp
[Fact]
public async Task BuildAsync_WhenCookingFontAtlasTexture_UsesFontAtlasTextureSettingsAndWritesDsSupportedTexture() {
    PlatformCookWorkItem workItem = new PlatformCookWorkItem(
        "ds:font-atlas-texture:cooked/fonts/DemoDiscBody.hetex",
        sourceFontPath,
        "font-atlas-texture",
        "ds",
        "runtime-texture",
        "cooked/fonts/DemoDiscBody.hetex",
        "runtime-texture:cooked/fonts/DemoDiscBody.hetex",
        "sha256:source",
        "sha256:settings",
        NintendoDsTextureCookSettingsSerializer.Serialize(new TextureAssetProcessorSettings {
            MaxResolution = 0,
            ColorFormat = TextureAssetColorFormat.Indexed4,
            AlphaPrecision = TextureAssetAlphaPrecision.Binary
        }),
        [new PlatformCookWorkItemMetadata("source-asset-id", "ui-font#atlas")]);

    PlatformBuildReport report = await BuildSingleWorkItemAsync(workItem);

    Assert.True(report.Succeeded);
    string stagedAtlasTexturePath = Path.Combine(nativeBuildExecutor.Workspace!.NitroFsRootPath, "cooked", "fonts", "DemoDiscBody.hetex");
    TextureAsset stagedAtlasTextureAsset = Assert.IsType<TextureAsset>(AssetSerializer.DeserializeFromBytes(File.ReadAllBytes(stagedAtlasTexturePath)));
    Assert.Equal(TextureAssetColorFormat.Indexed4, stagedAtlasTextureAsset.ColorFormat);
    Assert.Equal(TextureAssetAlphaPrecision.Binary, stagedAtlasTextureAsset.AlphaPrecision);
}
```

- [ ] **Step 2: Run the focused DS builder tests to verify they fail**

Run:

```powershell
dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter NintendoDsPlatformAssetBuilderTests
```

Expected:
- FAIL if DS font-atlas cook is not honoring the dedicated settings payload end-to-end

- [ ] **Step 3: Implement the DS cook-time fix and keep runtime diagnostics minimal**

```csharp
if (string.Equals(workItem.SourceAssetKind, "font-atlas-texture", StringComparison.OrdinalIgnoreCase)) {
    string fontAtlasSourcePath = ResolveFontAtlasCookSourcePath(workItem, packageSourceRootPath);
    TextureAsset cookedFontAtlasTextureAsset = PlatformCookSourceProcessor.CookFontAtlasTexture(fontAtlasSourcePath, assetId, settings);
    File.WriteAllBytes(destinationPath, global::helengine.files.AssetSerializer.SerializeToBytes(cookedFontAtlasTextureAsset));
    return;
}
```

Keep the top-screen proof diagnostic lines only as needed to prove the cooked atlas no longer trips `glyphTextureFormat`, then remove or trim temporary logging before the final commit if it is no longer needed.

- [ ] **Step 4: Run focused DS builder tests and one real DS build**

Run:

```powershell
dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter NintendoDsPlatformAssetBuilderTests
powershell -NoProfile -ExecutionPolicy Bypass -File C:\dev\helworks\helengine\artifacts\build-platform.ps1 -Project C:\dev\helprojs\city\project.heproj -Platform ds -Output C:\dev\helprojs\city\output\ds
powershell -NoProfile -ExecutionPolicy Bypass -File C:\dev\helworks\helengine-ds\scripts\launch_in_emulator.ps1 -ArtifactPath C:\dev\helprojs\city\output\ds\helengine_ds.nds
```

Expected:
- DS builder tests PASS
- build completes successfully
- the top-screen cooked-font proof no longer disappears because of `Rgba32/A8` atlas input

- [ ] **Step 5: Commit**

```powershell
git -C C:\dev\helworks\helengine-ds add `
  builder\NintendoDsPlatformAssetBuilder.cs `
  builder.tests\NintendoDsPlatformAssetBuilderTests.cs `
  src\platform\ds\NintendoDsRenderManager2D.cpp
git -C C:\dev\helworks\helengine-ds commit -m "fix: cook DS font atlases through platform font texture settings"
```
