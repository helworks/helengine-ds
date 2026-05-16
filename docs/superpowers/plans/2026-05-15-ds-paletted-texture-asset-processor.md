# DS Paletted Texture Asset Processor Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add DS-only editor-authored paletted texture settings and runtime decode so DS menu textures and fonts can use explicit per-asset format, alpha, and resolution policies.

**Architecture:** The shared texture schema grows to carry DS-oriented `ColorFormat`, `AlphaPrecision`, and optional palette payloads, while the editor import pipeline becomes the sole owner of DS texture format and resolution decisions. The DS runtime then decodes the authored cooked payloads directly, and the previous blanket DS `128` resize fallback is removed from the normal path so the menu logo and fonts can carry different explicit DS overrides.

**Tech Stack:** C#, xUnit, shared `helengine` editor/core asset pipeline, generated-core asset serialization, native C++ Nintendo DS runtime, `melonDS`.

---

## File Structure

- Create: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\TextureAssetAlphaPrecision.cs`
  Defines the shared texture alpha-precision enum used by editor settings, cooked texture assets, and generated-core runtime decode.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\TextureAsset.cs`
  Adds palette-payload and alpha-precision fields to cooked texture assets.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\TextureAssetColorFormat.cs`
  Adds `Indexed4` and `Indexed8` to the cooked texture format contract.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\TextureAssetProcessorSettings.cs`
  Adds the DS-authored `AlphaPrecision` setting alongside `MaxResolution` and `ColorFormat`.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\TextureAssetImportSettingsBinarySerializer.cs`
  Persists the new texture processor settings in typed sidecars and bumps the document version.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\TextureAssetProcessor.cs`
  Implements indexed texture cooking, palette generation, alpha packing, and validation.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetImportManager.cs`
  Removes the blanket DS `MaxResolution = 128` default and ensures cache identity responds to all DS texture settings.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\components\ui\AssetImportSettingsView.cs`
  Exposes DS texture processor fields in the editor so artists can author per-asset DS overrides.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AssetImportManagerTests.cs`
  Covers sidecar defaults, cache invalidation, and per-platform DS processor settings.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AssetImportSettingsViewTests.cs`
  Covers texture-setting editing and apply-request forwarding.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\TextureAssetImportSettingsBinarySerializerTests.cs`
  Covers typed sidecar round-trips for `ColorFormat`, `AlphaPrecision`, and `MaxResolution`.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\asset\TextureAssetProcessorTests.cs`
  Covers `Indexed4`, `Indexed8`, `Rgba4444`, alpha packing, and payload-shape validation.
- Modify: `src\platform\ds\NintendoDsRuntimeTexture2D.hpp`
  Carries palette payload and alpha metadata into the DS 2D runtime texture type.
- Modify: `src\platform\ds\NintendoDsRenderManager2D.cpp`
  Validates indexed payload lengths and decodes `Rgba32`, `Rgba4444`, `Indexed4`, and `Indexed8`.
- Modify: `builder\NintendoDsSceneAssetSanitizer.cs`
  Stops being the primary DS texture policy mechanism and becomes a validation-oriented safety net.
- Modify: `builder.tests\NintendoDsRenderManager2DSourceAuditTests.cs`
  Audits the DS decode branches for indexed texture support.
- Modify: `builder.tests\NintendoDsSceneAssetSanitizerTests.cs`
  Covers the new non-resizing safety-net behavior.
- Modify: `C:\tmp\helengine-ds-city-cube-project\city\assets\Images\Menu\helengine-logo.png.hasset`
  Adds the explicit DS override for the menu logo.
- Modify: `C:\tmp\helengine-ds-city-cube-project\city\assets\Fonts\DemoDiscTitle.ttf.hasset`
  Adds the explicit DS override for the menu title font.
- Modify: `C:\tmp\helengine-ds-city-cube-project\city\assets\Fonts\DemoDiscBody.ttf.hasset`
  Adds the explicit DS override for the menu body font.

## Task 1: Extend the Shared Texture Settings and Sidecar Contract

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\TextureAssetAlphaPrecision.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\TextureAsset.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\TextureAssetColorFormat.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\TextureAssetProcessorSettings.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\TextureAssetImportSettingsBinarySerializer.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\TextureAssetImportSettingsBinarySerializerTests.cs`

- [ ] **Step 1: Write the failing sidecar round-trip test**

Add a typed serializer test that proves DS texture sidecars round-trip `MaxResolution`, `ColorFormat`, and `AlphaPrecision`.

```csharp
/// <summary>
/// Verifies typed texture sidecars preserve DS texture format, alpha precision, and resolution settings.
/// </summary>
[Fact]
public void SerializeDeserialize_WhenDsTextureSettingsUseIndexedFormat_RoundTripsAllProcessorFields() {
    TextureAssetImportSettings settings = new TextureAssetImportSettings();
    settings.Importer.ImporterId = "pfim";
    settings.Importer.SourceChecksum = "sha256:test";
    settings.Importer.AssetId = "asset/test";
    settings.Processor.Platforms["ds"] = new TextureAssetProcessorSettings {
        MaxResolution = 256,
        ColorFormat = TextureAssetColorFormat.Indexed8,
        AlphaPrecision = TextureAssetAlphaPrecision.A4
    };

    using MemoryStream stream = new MemoryStream();
    TextureAssetImportSettingsBinarySerializer.Serialize(stream, settings);
    stream.Position = 0;

    TextureAssetImportSettings roundTripped = TextureAssetImportSettingsBinarySerializer.Deserialize(stream);

    TextureAssetProcessorSettings dsSettings = Assert.Single(roundTripped.Processor.Platforms).Value;
    Assert.Equal(256, dsSettings.MaxResolution);
    Assert.Equal(TextureAssetColorFormat.Indexed8, dsSettings.ColorFormat);
    Assert.Equal(TextureAssetAlphaPrecision.A4, dsSettings.AlphaPrecision);
}
```

- [ ] **Step 2: Run the focused serializer test to verify it fails**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~TextureAssetImportSettingsBinarySerializerTests.SerializeDeserialize_WhenDsTextureSettingsUseIndexedFormat_RoundTripsAllProcessorFields" -p:BaseOutputPath='C:\dev\helworks\helengine\.codex-test\ds-palette-task1\' -v minimal
```

Expected: FAIL because `TextureAssetColorFormat` does not include indexed values, `TextureAssetProcessorSettings` does not expose `AlphaPrecision`, and the binary serializer does not persist the new field.

- [ ] **Step 3: Implement the shared enums, fields, and serializer version bump**

Add the new alpha enum, extend the color enum, add the cooked texture fields, and persist the new processor setting in typed sidecars.

```csharp
// C:\dev\helworks\helengine\engine\helengine.core\assets\raw\TextureAssetAlphaPrecision.cs
namespace helengine {
    /// <summary>
    /// Identifies the alpha precision used by one cooked texture payload.
    /// </summary>
    public enum TextureAssetAlphaPrecision : byte {
        /// <summary>
        /// Stores no alpha data and treats all texels as opaque.
        /// </summary>
        Opaque = 0,

        /// <summary>
        /// Stores thresholded transparent or opaque alpha.
        /// </summary>
        Binary = 1,

        /// <summary>
        /// Stores 4-bit alpha precision.
        /// </summary>
        A4 = 2,

        /// <summary>
        /// Stores 8-bit alpha precision.
        /// </summary>
        A8 = 3
    }
}

// C:\dev\helworks\helengine\engine\helengine.core\assets\raw\TextureAssetColorFormat.cs
public enum TextureAssetColorFormat : byte {
    Rgba32 = 0,
    Rgba4444 = 1,
    Indexed4 = 2,
    Indexed8 = 3
}

// C:\dev\helworks\helengine\engine\helengine.core\assets\raw\TextureAsset.cs
public class TextureAsset : Asset {
    public byte[] Colors;
    public byte[] PaletteColors;
    public ushort Width;
    public ushort Height;
    public TextureAssetColorFormat ColorFormat;
    public TextureAssetAlphaPrecision AlphaPrecision;
    public bool IsEngineOwned;
}

// C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\TextureAssetProcessorSettings.cs
public class TextureAssetProcessorSettings {
    public int MaxResolution { get; set; }
    public TextureAssetColorFormat ColorFormat { get; set; }
    public TextureAssetAlphaPrecision AlphaPrecision { get; set; }
}

// C:\dev\helworks\helengine\engine\helengine.editor\serialization\TextureAssetImportSettingsBinarySerializer.cs
public const byte CurrentVersion = 3;

writer.WriteString(entry.Key);
writer.WriteInt32(entry.Value.MaxResolution);
writer.WriteByte((byte)entry.Value.ColorFormat);
writer.WriteByte((byte)entry.Value.AlphaPrecision);

platformSettings.ColorFormat = header.Version >= 2
    ? ReadTextureAssetColorFormat(reader)
    : TextureAssetColorFormat.Rgba32;
platformSettings.AlphaPrecision = header.Version >= CurrentVersion
    ? ReadTextureAssetAlphaPrecision(reader)
    : TextureAssetAlphaPrecision.A8;
```

- [ ] **Step 4: Run the focused serializer test to verify it passes**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~TextureAssetImportSettingsBinarySerializerTests.SerializeDeserialize_WhenDsTextureSettingsUseIndexedFormat_RoundTripsAllProcessorFields" -p:BaseOutputPath='C:\dev\helworks\helengine\.codex-test\ds-palette-task1\' -v minimal
```

Expected: PASS with DS typed sidecars preserving `Indexed8`, `A4`, and `MaxResolution = 256`.

- [ ] **Step 5: Commit the shared texture-contract slice**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.core/assets/raw/TextureAssetAlphaPrecision.cs engine/helengine.core/assets/raw/TextureAsset.cs engine/helengine.core/assets/raw/TextureAssetColorFormat.cs engine/helengine.editor/managers/asset/TextureAssetProcessorSettings.cs engine/helengine.editor/serialization/TextureAssetImportSettingsBinarySerializer.cs engine/helengine.editor.tests/serialization/TextureAssetImportSettingsBinarySerializerTests.cs
git -C C:\dev\helworks\helengine commit -m "feat: extend texture settings for DS paletted formats"
```

## Task 2: Implement Paletted Texture Cooking and Cache Identity

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\TextureAssetProcessor.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetImportManager.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\asset\TextureAssetProcessorTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AssetImportManagerTests.cs`

- [ ] **Step 1: Write the failing processor and cache-id tests**

Add one processor test for indexed cooking and one import-manager test proving cache identity changes when `AlphaPrecision` changes.

```csharp
/// <summary>
/// Verifies the texture processor converts RGBA32 source pixels into an indexed DS payload with palette data.
/// </summary>
[Fact]
public void Apply_WhenIndexed4IsRequested_ProducesPaletteAndPackedIndices() {
    TextureAsset source = new TextureAsset {
        Id = "menu/logo",
        Width = 4,
        Height = 1,
        ColorFormat = TextureAssetColorFormat.Rgba32,
        AlphaPrecision = TextureAssetAlphaPrecision.A8,
        Colors = [
            255, 0, 0, 255,
            0, 255, 0, 255,
            0, 0, 255, 255,
            0, 0, 0, 0
        ]
    };

    TextureAsset processed = new TextureAssetProcessor().Apply(source, new TextureAssetProcessorSettings {
        ColorFormat = TextureAssetColorFormat.Indexed4,
        AlphaPrecision = TextureAssetAlphaPrecision.Binary,
        MaxResolution = 0
    });

    Assert.Equal(TextureAssetColorFormat.Indexed4, processed.ColorFormat);
    Assert.Equal(TextureAssetAlphaPrecision.Binary, processed.AlphaPrecision);
    Assert.NotNull(processed.PaletteColors);
    Assert.Equal(4 * 4, processed.PaletteColors.Length);
    Assert.Equal(2, processed.Colors.Length);
}

/// <summary>
/// Verifies changing DS alpha precision invalidates the cached texture asset id.
/// </summary>
[Fact]
public void TryLoadTextureAsset_WhenTextureAlphaPrecisionChanges_ReimportsWithANewAssetId() {
    string sourcePath = WriteSourceTexture("alpha-precision-cache-id.tga");
    AssetImportManager manager = CreateTgaManager();
    manager.CurrentPlatformId = "ds";

    TextureAssetImportSettings settings = manager.LoadOrCreateTextureImportSettings(sourcePath);
    settings.Processor.Platforms["ds"] = new TextureAssetProcessorSettings {
        MaxResolution = 256,
        ColorFormat = TextureAssetColorFormat.Indexed8,
        AlphaPrecision = TextureAssetAlphaPrecision.A4
    };
    manager.SaveTextureImportSettings(sourcePath, settings);
    Assert.True(manager.TryLoadTextureAsset(sourcePath, out _));
    string firstAssetId = manager.LoadOrCreateTextureImportSettings(sourcePath).Importer.AssetId;

    settings = manager.LoadOrCreateTextureImportSettings(sourcePath);
    settings.Processor.Platforms["ds"].AlphaPrecision = TextureAssetAlphaPrecision.A8;
    manager.SaveTextureImportSettings(sourcePath, settings);
    Assert.True(manager.TryLoadTextureAsset(sourcePath, out _));
    string secondAssetId = manager.LoadOrCreateTextureImportSettings(sourcePath).Importer.AssetId;

    Assert.NotEqual(firstAssetId, secondAssetId);
}
```

- [ ] **Step 2: Run the focused processor and cache-id tests to verify they fail**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~TextureAssetProcessorTests.Apply_WhenIndexed4IsRequested_ProducesPaletteAndPackedIndices|FullyQualifiedName~AssetImportManagerTests.TryLoadTextureAsset_WhenTextureAlphaPrecisionChanges_ReimportsWithANewAssetId" -p:BaseOutputPath='C:\dev\helworks\helengine\.codex-test\ds-palette-task2\' -v minimal
```

Expected: FAIL because the processor only supports `Rgba32` and `Rgba4444`, `TextureAsset` has no palette payload, and cache identity does not yet incorporate `AlphaPrecision`.

- [ ] **Step 3: Implement indexed cooking, alpha packing, and no-default DS resize policy**

Extend the processor to build palette-backed payloads and update default DS settings to return the shared uncapped defaults instead of forcing `128`.

```csharp
// C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\TextureAssetProcessor.cs
TextureAsset ConvertColorFormat(TextureAsset asset, TextureAssetColorFormat targetFormat, TextureAssetAlphaPrecision alphaPrecision) {
    if (targetFormat == TextureAssetColorFormat.Rgba32) {
        asset.AlphaPrecision = alphaPrecision;
        return ApplyAlphaPrecision(asset, alphaPrecision);
    } else if (targetFormat == TextureAssetColorFormat.Rgba4444) {
        return ConvertToRgba4444(asset, alphaPrecision);
    } else if (targetFormat == TextureAssetColorFormat.Indexed4) {
        return ConvertToIndexed(asset, paletteCapacity: 16, targetFormat, alphaPrecision);
    } else if (targetFormat == TextureAssetColorFormat.Indexed8) {
        return ConvertToIndexed(asset, paletteCapacity: 256, targetFormat, alphaPrecision);
    }

    throw new InvalidOperationException($"Unsupported texture color format '{targetFormat}'.");
}

TextureAsset ConvertToIndexed(TextureAsset asset, int paletteCapacity, TextureAssetColorFormat targetFormat, TextureAssetAlphaPrecision alphaPrecision) {
    List<byte> palette = new List<byte>(paletteCapacity * 4);
    Dictionary<uint, int> paletteIndices = new Dictionary<uint, int>();
    byte[] indices = targetFormat == TextureAssetColorFormat.Indexed4
        ? new byte[(asset.Width * asset.Height + 1) / 2]
        : new byte[asset.Width * asset.Height];

    for (int pixelIndex = 0; pixelIndex < asset.Width * asset.Height; pixelIndex++) {
        int sourceIndex = pixelIndex * 4;
        byte alpha = QuantizeAlpha(asset.Colors[sourceIndex + 3], alphaPrecision);
        uint paletteKey = PackPaletteKey(asset.Colors[sourceIndex], asset.Colors[sourceIndex + 1], asset.Colors[sourceIndex + 2], alpha);
        int paletteIndex = GetOrAddPaletteIndex(paletteIndices, palette, paletteCapacity, paletteKey);
        WritePackedIndex(indices, pixelIndex, paletteIndex, targetFormat);
    }

    return new TextureAsset {
        Id = asset.Id,
        RuntimeAssetId = asset.RuntimeAssetId,
        Width = asset.Width,
        Height = asset.Height,
        ColorFormat = targetFormat,
        AlphaPrecision = alphaPrecision,
        Colors = indices,
        PaletteColors = palette.ToArray()
    };
}

// C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetImportManager.cs
TextureAssetProcessorSettings CreateDefaultTextureProcessorSettings(string platformId) {
    return new TextureAssetProcessorSettings {
        MaxResolution = 0,
        ColorFormat = TextureAssetColorFormat.Rgba32,
        AlphaPrecision = TextureAssetAlphaPrecision.A8
    };
}
```

- [ ] **Step 4: Run the focused processor and cache-id tests to verify they pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~TextureAssetProcessorTests.Apply_WhenIndexed4IsRequested_ProducesPaletteAndPackedIndices|FullyQualifiedName~AssetImportManagerTests.TryLoadTextureAsset_WhenTextureAlphaPrecisionChanges_ReimportsWithANewAssetId" -p:BaseOutputPath='C:\dev\helworks\helengine\.codex-test\ds-palette-task2\' -v minimal
```

Expected: PASS with indexed payloads carrying palette bytes and cache ids changing when DS alpha precision changes.

- [ ] **Step 5: Commit the paletted processor slice**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.editor/managers/asset/TextureAssetProcessor.cs engine/helengine.editor/managers/asset/AssetImportManager.cs engine/helengine.editor.tests/managers/asset/TextureAssetProcessorTests.cs engine/helengine.editor.tests/AssetImportManagerTests.cs
git -C C:\dev\helworks\helengine commit -m "feat: cook DS textures into paletted payloads"
```

## Task 3: Expose the DS Texture Settings in the Editor UI

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\components\ui\AssetImportSettingsView.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AssetImportSettingsViewTests.cs`

- [ ] **Step 1: Write the failing editor-view test**

Add a view test that proves texture assets can expose `ColorFormat`, `AlphaPrecision`, and `MaxResolution` for the active DS platform.

```csharp
/// <summary>
/// Verifies texture assets expose DS color format, alpha precision, and max-resolution controls for the active platform.
/// </summary>
[Fact]
public void Show_WhenTextureProcessorSettingsExist_UsesTheActivePlatformTextureValues() {
    AssetImportSettingsView view = new AssetImportSettingsView(CreateFont(), 1);
    AssetProcessorSettings settings = new AssetProcessorSettings();
    settings.Platforms["ds"] = new AssetPlatformProcessorSettings {
        Texture = new TextureAssetProcessorSettings {
            MaxResolution = 256,
            ColorFormat = TextureAssetColorFormat.Indexed8,
            AlphaPrecision = TextureAssetAlphaPrecision.A4
        }
    };

    view.Show(
        ["pfim"],
        "pfim",
        settings,
        ["windows", "ds"],
        "ds",
        AssetEntryKind.Texture);

    Assert.True(view.IsTextureProcessorVisible);
    Assert.Equal(256, view.CurrentTextureMaxResolutionValue);
    Assert.Equal(TextureAssetColorFormat.Indexed8, view.CurrentTextureColorFormatValue);
    Assert.Equal(TextureAssetAlphaPrecision.A4, view.CurrentTextureAlphaPrecisionValue);
}
```

- [ ] **Step 2: Run the focused view test to verify it fails**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~AssetImportSettingsViewTests.Show_WhenTextureProcessorSettingsExist_UsesTheActivePlatformTextureValues" -p:BaseOutputPath='C:\dev\helworks\helengine\.codex-test\ds-palette-task3\' -v minimal
```

Expected: FAIL because the view only exposes model processor controls and has no texture-setting UI state.

- [ ] **Step 3: Implement DS texture-setting controls and apply forwarding**

Add texture-specific controls to the view and bind them to `AssetProcessorSettings.Platforms[platformId].Texture`.

```csharp
// C:\dev\helworks\helengine\engine\helengine.editor\components\ui\AssetImportSettingsView.cs
const string TextureMaxResolutionLabel = "Max Resolution";
const string TextureColorFormatLabel = "Color Format";
const string TextureAlphaPrecisionLabel = "Alpha";

readonly TextBoxComponent TextureMaxResolutionTextBox;
readonly ComboBoxComponent TextureColorFormatComboBox;
readonly ComboBoxComponent TextureAlphaPrecisionComboBox;

void SyncTextureProcessorControlsFromPendingSettings() {
    TextureAssetProcessorSettings textureSettings = EnsurePendingTextureProcessorSettings();
    TextureMaxResolutionTextBox.Text = textureSettings.MaxResolution.ToString(CultureInfo.InvariantCulture);
    TextureColorFormatComboBox.SetSelectionByValue(textureSettings.ColorFormat.ToString());
    TextureAlphaPrecisionComboBox.SetSelectionByValue(textureSettings.AlphaPrecision.ToString());
}

void HandleTextureColorFormatChanged(int selectedIndex, string selectedValue) {
    TextureAssetProcessorSettings textureSettings = EnsurePendingTextureProcessorSettings();
    textureSettings.ColorFormat = Enum.Parse<TextureAssetColorFormat>(selectedValue, ignoreCase: false);
}

void HandleTextureAlphaPrecisionChanged(int selectedIndex, string selectedValue) {
    TextureAssetProcessorSettings textureSettings = EnsurePendingTextureProcessorSettings();
    textureSettings.AlphaPrecision = Enum.Parse<TextureAssetAlphaPrecision>(selectedValue, ignoreCase: false);
}
```

- [ ] **Step 4: Run the focused view test to verify it passes**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~AssetImportSettingsViewTests.Show_WhenTextureProcessorSettingsExist_UsesTheActivePlatformTextureValues" -p:BaseOutputPath='C:\dev\helworks\helengine\.codex-test\ds-palette-task3\' -v minimal
```

Expected: PASS with the DS tab showing texture processor values instead of only model controls.

- [ ] **Step 5: Commit the editor UI slice**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.editor/components/ui/AssetImportSettingsView.cs engine/helengine.editor.tests/AssetImportSettingsViewTests.cs
git -C C:\dev\helworks\helengine commit -m "feat: expose DS texture settings in import UI"
```

## Task 4: Decode Indexed Textures on DS and Demote the Sanitizer to a Safety Net

**Files:**
- Modify: `src\platform\ds\NintendoDsRuntimeTexture2D.hpp`
- Modify: `src\platform\ds\NintendoDsRenderManager2D.cpp`
- Modify: `builder\NintendoDsSceneAssetSanitizer.cs`
- Modify: `builder.tests\NintendoDsRenderManager2DSourceAuditTests.cs`
- Modify: `builder.tests\NintendoDsSceneAssetSanitizerTests.cs`

- [ ] **Step 1: Write the failing DS source-audit and sanitizer tests**

Add one source audit for indexed decode branches and one sanitizer test that proves oversized textures are no longer forcibly resized as the primary policy path.

```csharp
/// <summary>
/// Verifies the Nintendo DS 2D renderer decodes indexed cooked textures through dedicated palette branches.
/// </summary>
[Fact]
public void Source_whenBuildingIndexedTextures_decodesPalettePayloadsForIndexed4AndIndexed8() {
    string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
    string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
    string sourceCode = File.ReadAllText(sourcePath);

    Assert.Contains("TextureAssetColorFormat::Indexed4", sourceCode, StringComparison.Ordinal);
    Assert.Contains("TextureAssetColorFormat::Indexed8", sourceCode, StringComparison.Ordinal);
    Assert.Contains("texture->PaletteColors", sourceCode, StringComparison.Ordinal);
    Assert.Contains("ReadIndexedColor(texture, sampleX, sampleY)", sourceCode, StringComparison.Ordinal);
}

/// <summary>
/// Verifies the Nintendo DS scene sanitizer no longer downsizes cooked imported textures as the normal DS texture-policy path.
/// </summary>
[Fact]
public void SanitizeStagedSceneAssets_whenImportedTextureExceedsOldClamp_preservesAuthoredDimensions() {
    string rootPath = Path.Combine(Path.GetTempPath(), "helengine-ds-scene-sanitizer-" + Guid.NewGuid().ToString("N"));
    string nitroFsRootPath = Path.Combine(rootPath, "nitrofs");
    string importedRootPath = Path.Combine(nitroFsRootPath, "cooked", "imported");
    string texturePath = Path.Combine(importedRootPath, "authored-font-atlas.hasset");

    Directory.CreateDirectory(importedRootPath);
    File.WriteAllBytes(texturePath, helengine.files.AssetSerializer.SerializeToBytes(CreateTextureAsset(256, 256)));

    new NintendoDsSceneAssetSanitizer().SanitizeStagedSceneAssets(nitroFsRootPath);

    TextureAsset sanitizedTexture = Assert.IsType<TextureAsset>(helengine.files.AssetSerializer.DeserializeFromBytes(File.ReadAllBytes(texturePath)));
    Assert.Equal((ushort)256, sanitizedTexture.Width);
    Assert.Equal((ushort)256, sanitizedTexture.Height);
}
```

- [ ] **Step 2: Run the focused DS tests to verify they fail**

Run:

```powershell
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests.Source_whenBuildingIndexedTextures_decodesPalettePayloadsForIndexed4AndIndexed8|FullyQualifiedName~NintendoDsSceneAssetSanitizerTests.SanitizeStagedSceneAssets_whenImportedTextureExceedsOldClamp_preservesAuthoredDimensions" -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\ds-palette-task4\' -v minimal
```

Expected: FAIL because the DS renderer only decodes `Rgba32` and `Rgba4444`, and the sanitizer still force-resizes oversized imported textures to `128`.

- [ ] **Step 3: Implement indexed decode and validation-only sanitizer behavior**

Extend the runtime texture to carry palette bytes, add indexed decode branches, and replace sanitizer resizing with validation-only behavior.

```cpp
// src/platform/ds/NintendoDsRuntimeTexture2D.hpp
class NintendoDsRuntimeTexture2D : public RuntimeTexture {
public:
    NintendoDsRuntimeTexture2D();
    TextureAssetColorFormat ColorFormat;
    TextureAssetAlphaPrecision AlphaPrecision;
    Array<uint8_t>* Colors;
    Array<uint8_t>* PaletteColors;
};

// src/platform/ds/NintendoDsRenderManager2D.cpp
byte4 NintendoDsRenderManager2D::ReadIndexedColor(NintendoDsRuntimeTexture2D* texture, int32_t sampleX, int32_t sampleY) {
    int32_t paletteIndex = texture->ColorFormat == TextureAssetColorFormat::Indexed4
        ? ReadPackedNibbleIndex(texture->Colors, texture->get_Width(), sampleX, sampleY)
        : texture->Colors->Data[(sampleY * texture->get_Width()) + sampleX];
    int32_t paletteOffset = paletteIndex * 4;
    return byte4(
        texture->PaletteColors->Data[paletteOffset],
        texture->PaletteColors->Data[paletteOffset + 1],
        texture->PaletteColors->Data[paletteOffset + 2],
        texture->PaletteColors->Data[paletteOffset + 3]);
}

// builder/NintendoDsSceneAssetSanitizer.cs
static void SanitizeImportedTextureAssetFile(string textureAssetPath) {
    TextureAsset textureAsset;
    using (FileStream stream = File.OpenRead(textureAssetPath)) {
        textureAsset = helengine.files.AssetSerializer.Deserialize(stream) as TextureAsset;
    }

    if (textureAsset == null) {
        return;
    }

    ValidateImportedTextureAsset(textureAsset, textureAssetPath);
}
```

- [ ] **Step 4: Run the focused DS tests to verify they pass**

Run:

```powershell
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests.Source_whenBuildingIndexedTextures_decodesPalettePayloadsForIndexed4AndIndexed8|FullyQualifiedName~NintendoDsSceneAssetSanitizerTests.SanitizeStagedSceneAssets_whenImportedTextureExceedsOldClamp_preservesAuthoredDimensions" -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\ds-palette-task4\' -v minimal
```

Expected: PASS with indexed decode branches present and the sanitizer no longer shrinking authored `256x256` font atlases to `128`.

- [ ] **Step 5: Commit the DS decode slice**

```bash
git add src/platform/ds/NintendoDsRuntimeTexture2D.hpp src/platform/ds/NintendoDsRenderManager2D.cpp builder/NintendoDsSceneAssetSanitizer.cs builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs builder.tests/NintendoDsSceneAssetSanitizerTests.cs
git commit -m "feat: decode indexed textures in DS 2D renderer"
```

## Task 5: Author the Demo Project DS Overrides and Verify the Menu

**Files:**
- Modify: `C:\tmp\helengine-ds-city-cube-project\city\assets\Images\Menu\helengine-logo.png.hasset`
- Modify: `C:\tmp\helengine-ds-city-cube-project\city\assets\Fonts\DemoDiscTitle.ttf.hasset`
- Modify: `C:\tmp\helengine-ds-city-cube-project\city\assets\Fonts\DemoDiscBody.ttf.hasset`

- [ ] **Step 1: Apply the explicit DS texture overrides to the menu assets**

Use the editor import-settings UI or the equivalent import-settings save path after Tasks 1-4 land. Apply these exact DS values:

```text
Asset: C:\tmp\helengine-ds-city-cube-project\city\assets\Images\Menu\helengine-logo.png
Platform: ds
MaxResolution: 128
ColorFormat: Rgba4444
AlphaPrecision: A4

Asset: C:\tmp\helengine-ds-city-cube-project\city\assets\Fonts\DemoDiscTitle.ttf
Platform: ds
MaxResolution: 256
ColorFormat: Indexed8
AlphaPrecision: A8

Asset: C:\tmp\helengine-ds-city-cube-project\city\assets\Fonts\DemoDiscBody.ttf
Platform: ds
MaxResolution: 256
ColorFormat: Indexed8
AlphaPrecision: A8
```

- [ ] **Step 2: Rebuild the DS export and verify the cooked menu assets**

Run:

```powershell
rtk dotnet run --project C:\dev\helworks\helengine\helengine.ui\helengine.editor.app\helengine.editor.app.csproj -p:BaseOutputPath='C:\dev\helworks\helengine\.codex-build-ds-palette\bin\' -- --project C:\tmp\helengine-ds-city-cube-project\city\project.heproj --build ds --output C:\tmp\helengine-ds-city-cube-project\output\ds
```

Expected: PASS and new cooked menu assets appear under `C:\tmp\helengine-ds-city-cube-project\output\ds\cooked\`.

- [ ] **Step 3: Run the full focused automated regression suite**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~TextureAssetImportSettingsBinarySerializerTests|FullyQualifiedName~TextureAssetProcessorTests|FullyQualifiedName~AssetImportManagerTests.TryLoadTextureAsset_WhenTextureAlphaPrecisionChanges_ReimportsWithANewAssetId|FullyQualifiedName~AssetImportSettingsViewTests.Show_WhenTextureProcessorSettingsExist_UsesTheActivePlatformTextureValues" -p:BaseOutputPath='C:\dev\helworks\helengine\.codex-test\ds-palette-task5-editor\' -v minimal
rtk dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsRenderManager2DSourceAuditTests|FullyQualifiedName~NintendoDsSceneAssetSanitizerTests" -p:BaseOutputPath='C:\dev\helworks\helengine-ds\.codex-test\ds-palette-task5-ds\' -v minimal
```

Expected: PASS across editor-side and DS-side focused coverage.

- [ ] **Step 4: Launch `melonDS` and verify the menu visually**

Run:

```powershell
& "C:\dev\helworks\emus\melon\melonDS.exe" "C:\tmp\helengine-ds-city-cube-project\output\ds\helengine_ds.nds"
```

Expected:

- no startup `bad_alloc`
- the menu logo still renders
- title and body fonts are readable again
- the earlier top-to-bottom refresh artifact stays fixed

- [ ] **Step 5: Commit the authored DS overrides**

```bash
git -C C:\tmp\helengine-ds-city-cube-project\city add assets/Images/Menu/helengine-logo.png.hasset assets/Fonts/DemoDiscTitle.ttf.hasset assets/Fonts/DemoDiscBody.ttf.hasset
git -C C:\tmp\helengine-ds-city-cube-project\city commit -m "chore: tune DS menu texture import settings"
```
