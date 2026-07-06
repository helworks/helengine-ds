# Generic Platform Settings Registry Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the hardcoded per-platform processor-settings container with a generic section registry, adapt texture/model/material behind that registry, and add per-platform font pixel size that affects every platform-resolved font build path.

**Architecture:** `helengine.editor` stops treating platform settings as central `Texture`/`Model`/`Material` fields and instead stores per-platform section payloads keyed by section id. Each built-in section is registered through one typed definition that owns defaults, clone/equality behavior, serialization, and section applicability. `AssetImportSettingsView` hosts sections dynamically through the registry, while `AssetImportManager` resolves typed section payloads for processing. The font import path stops using `AssetContentManager.Load<FontAsset>` as an opaque black box and instead passes resolved `FontAssetProcessorSettings` into the registered font importer so `GdiFontImporter` can use the selected per-platform pixel size instead of hardcoded `32f`.

**Tech Stack:** C#/.NET 9, xUnit, Helengine editor asset import pipeline, binary serializer infrastructure, Windows GDI font importer, editor platform font variant cache

---

## File Structure

### Generic platform-settings registry

- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetPlatformSettingsSection.cs`
  Stores one section id plus one typed payload object inside a platform settings record.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\IAssetPlatformSettingsSectionDefinition.cs`
  Defines one registered section contract for defaults, type checks, clone/equality, applicability, and binary serialization.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetPlatformSettingsSectionRegistry.cs`
  Registers built-in section definitions and exposes typed lookup helpers.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\TextureAssetPlatformSettingsSectionDefinition.cs`
  Adapts `TextureAssetProcessorSettings` behind the registry.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\ModelAssetPlatformSettingsSectionDefinition.cs`
  Adapts `ModelAssetProcessorSettings` behind the registry.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\MaterialAssetPlatformSettingsSectionDefinition.cs`
  Adapts `MaterialAssetProcessorSettings` behind the registry.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\FontAssetProcessorSettings.cs`
  Adds the new per-platform font payload with `PixelSize`.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\FontAssetPlatformSettingsSectionDefinition.cs`
  Adapts `FontAssetProcessorSettings` behind the registry.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetPlatformProcessorSettings.cs`
  Replaces the hardcoded `Texture`/`Model`/`Material` fields with a section dictionary and typed registry-backed accessors.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\AssetImportSettingsBinarySerializer.cs`
  Switches `.hasset` processor serialization to the section-based format.

### Dynamic editor integration

- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\components\ui\AssetImportSettingsView.cs`
  Replaces hardcoded platform field access, clone logic, equality checks, and font/text-specific visibility logic with registry-driven section access.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AssetImportSettingsViewTests.cs`
  Adds font section coverage and updates texture/model setup helpers to use the registry-backed shape.

### Font import pipeline

- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\content\font\IFontImporter.cs`
  Expands the importer contract to accept resolved `FontAssetProcessorSettings`.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\content\font\FontImporterContentProcessor.cs`
  Preserves the generic content-manager adapter path by forwarding default font settings when no platform context exists.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetImportManager.cs`
  Resolves typed registry sections, uses registry defaults, and passes the active font settings into direct font import calls.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\testing\ConfigurableFontImporter.cs`
  Records or consumes supplied font settings for deterministic platform tests.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\testing\TestFontImporter.cs`
  Adopts the expanded font importer contract.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\testing\RecordingFontImporter.cs`
  Captures the last requested `FontAssetProcessorSettings` so tests can assert the active platform payload was forwarded.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\asset\AssetPlatformSettingsSectionRegistryTests.cs`
  Verifies registration, default creation, wrong-type failures, and section equality.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\AssetImportSettingsBinarySerializerTests.cs`
  Verifies section-based `.hasset` serialization roundtrips and rejects unknown section ids.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\asset\AssetImportManagerFontPlatformSettingsTests.cs`
  Verifies `BuildFontAssetForPlatform(...)` forwards the active platform `font` section.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorPlatformFontVariantCacheServiceTests.cs`
  Verifies cached font variants change when per-platform font settings change.

### Windows GDI font importer validation

- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.windows\content\font\GdiFontImporter.cs`
  Replaces hardcoded `32f` with the supplied `FontAssetProcessorSettings.PixelSize`.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.windows.tests\content\font\GdiFontImporterTests.cs`
  Verifies the importer still loads vendor fonts and that different pixel sizes produce different runtime font metrics or atlas dimensions.

## Task 1: Build the Generic Registry and Lock the New `.hasset` Format

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetPlatformSettingsSection.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\IAssetPlatformSettingsSectionDefinition.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetPlatformSettingsSectionRegistry.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\TextureAssetPlatformSettingsSectionDefinition.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\ModelAssetPlatformSettingsSectionDefinition.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\MaterialAssetPlatformSettingsSectionDefinition.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\FontAssetProcessorSettings.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\FontAssetPlatformSettingsSectionDefinition.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetPlatformProcessorSettings.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\AssetImportSettingsBinarySerializer.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\asset\AssetPlatformSettingsSectionRegistryTests.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\AssetImportSettingsBinarySerializerTests.cs`

- [ ] **Step 1: Write the failing registry and serializer tests**

```csharp
using Xunit;

namespace helengine.editor.tests.managers.asset {
    /// <summary>
    /// Verifies generic platform settings section registration and typed retrieval.
    /// </summary>
    public sealed class AssetPlatformSettingsSectionRegistryTests {
        /// <summary>
        /// Ensures the built-in font section creates a default payload with the historical effective pixel size.
        /// </summary>
        [Fact]
        public void GetOrCreateSection_WhenFontSectionIsMissing_CreatesDefaultFontSettings() {
            AssetPlatformProcessorSettings platformSettings = new AssetPlatformProcessorSettings();

            FontAssetProcessorSettings fontSettings = AssetPlatformSettingsSectionRegistry.Shared.GetOrCreateSection<FontAssetProcessorSettings>(platformSettings, "font");

            Assert.Equal(32, fontSettings.PixelSize);
        }

        /// <summary>
        /// Ensures requesting a section through the wrong payload type fails explicitly.
        /// </summary>
        [Fact]
        public void GetOrCreateSection_WhenRequestedWithWrongType_ThrowsInvalidOperationException() {
            AssetPlatformProcessorSettings platformSettings = new AssetPlatformProcessorSettings();
            AssetPlatformSettingsSectionRegistry.Shared.GetOrCreateSection<FontAssetProcessorSettings>(platformSettings, "font");

            InvalidOperationException exception = Assert.Throws<InvalidOperationException>(
                () => AssetPlatformSettingsSectionRegistry.Shared.GetOrCreateSection<TextureAssetProcessorSettings>(platformSettings, "font"));

            Assert.Contains("font", exception.Message, StringComparison.Ordinal);
        }
    }
}

namespace helengine.editor.tests.serialization {
    /// <summary>
    /// Verifies section-based asset import settings serialization.
    /// </summary>
    public sealed class AssetImportSettingsBinarySerializerTests {
        /// <summary>
        /// Ensures all built-in platform sections survive one binary roundtrip.
        /// </summary>
        [Fact]
        public void Serialize_WhenProcessorUsesSectionRegistry_RoundtripsBuiltInSections() {
            AssetImportSettings settings = new AssetImportSettings();
            settings.Importer.ImporterId = "test-font";
            settings.Importer.SourceChecksum = "abc123";
            settings.Importer.AssetId = "asset-id";

            AssetPlatformProcessorSettings windowsSettings = new AssetPlatformProcessorSettings();
            AssetPlatformSettingsSectionRegistry.Shared.SetSection(windowsSettings, "texture", new TextureAssetProcessorSettings {
                MaxResolution = 128,
                ColorFormat = TextureAssetColorFormat.Indexed8,
                AlphaPrecision = TextureAssetAlphaPrecision.A8,
                IndexingMethodId = TextureAssetIndexingMethod.QuantizedIndexed.ToString()
            });
            AssetPlatformSettingsSectionRegistry.Shared.SetSection(windowsSettings, "model", new ModelAssetProcessorSettings {
                FlipWinding = true
            });
            AssetPlatformSettingsSectionRegistry.Shared.SetSection(windowsSettings, "material", new MaterialAssetProcessorSettings {
                SchemaId = "lit",
                FieldValues = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase) {
                    ["BaseColor"] = "#ffffff"
                }
            });
            AssetPlatformSettingsSectionRegistry.Shared.SetSection(windowsSettings, "font", new FontAssetProcessorSettings {
                PixelSize = 14
            });
            settings.Processor.Platforms["windows"] = windowsSettings;

            using MemoryStream stream = new MemoryStream();
            AssetImportSettingsBinarySerializer.Serialize(stream, settings);
            stream.Position = 0;

            AssetImportSettings deserialized = AssetImportSettingsBinarySerializer.Deserialize(stream);
            FontAssetProcessorSettings fontSettings = AssetPlatformSettingsSectionRegistry.Shared.GetOrCreateSection<FontAssetProcessorSettings>(
                deserialized.Processor.Platforms["windows"],
                "font");

            Assert.Equal(14, fontSettings.PixelSize);
        }
    }
}
```

- [ ] **Step 2: Run the focused registry and serializer tests to verify they fail**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~AssetPlatformSettingsSectionRegistryTests|FullyQualifiedName~AssetImportSettingsBinarySerializerTests" -v minimal
```

Expected:

```text
FAIL
CS0246 or CS0103 errors for AssetPlatformSettingsSectionRegistry / FontAssetProcessorSettings / SetSection
```

- [ ] **Step 3: Implement the registry-backed platform settings model**

```csharp
namespace helengine.editor {
    /// <summary>
    /// Stores one registered section payload for a target platform.
    /// </summary>
    public sealed class AssetPlatformSettingsSection {
        /// <summary>
        /// Initializes one registered section payload.
        /// </summary>
        /// <param name="sectionId">Registered section identifier.</param>
        /// <param name="settings">Typed section payload.</param>
        public AssetPlatformSettingsSection(string sectionId, object settings) {
            if (string.IsNullOrWhiteSpace(sectionId)) {
                throw new ArgumentException("Section id must be provided.", nameof(sectionId));
            } else if (settings == null) {
                throw new ArgumentNullException(nameof(settings));
            }

            SectionId = sectionId;
            Settings = settings;
        }

        /// <summary>
        /// Gets the registered section identifier.
        /// </summary>
        public string SectionId { get; }

        /// <summary>
        /// Gets or sets the typed section payload.
        /// </summary>
        public object Settings { get; set; }
    }

    /// <summary>
    /// Defines one registered platform settings section.
    /// </summary>
    public interface IAssetPlatformSettingsSectionDefinition {
        /// <summary>
        /// Gets the registered section id.
        /// </summary>
        string SectionId { get; }

        /// <summary>
        /// Gets the payload type owned by the section.
        /// </summary>
        Type SettingsType { get; }

        /// <summary>
        /// Creates one default payload.
        /// </summary>
        /// <returns>Default payload instance.</returns>
        object CreateDefaultSettings();

        /// <summary>
        /// Creates one deep clone of the supplied payload.
        /// </summary>
        /// <param name="settings">Payload to clone.</param>
        /// <returns>Cloned payload.</returns>
        object CloneSettings(object settings);

        /// <summary>
        /// Compares two payload instances for value equality.
        /// </summary>
        /// <param name="left">First payload.</param>
        /// <param name="right">Second payload.</param>
        /// <returns>True when values match.</returns>
        bool SettingsEqual(object left, object right);

        /// <summary>
        /// Writes one payload to the binary stream.
        /// </summary>
        /// <param name="writer">Binary writer.</param>
        /// <param name="settings">Payload to serialize.</param>
        void Serialize(EngineBinaryWriter writer, object settings);

        /// <summary>
        /// Reads one payload from the binary stream.
        /// </summary>
        /// <param name="reader">Binary reader.</param>
        /// <returns>Deserialized payload.</returns>
        object Deserialize(EngineBinaryReader reader);
    }
}
```

```csharp
namespace helengine.editor {
    /// <summary>
    /// Stores processor settings for one target platform through registered sections.
    /// </summary>
    public class AssetPlatformProcessorSettings {
        /// <summary>
        /// Initializes the registered section map.
        /// </summary>
        public AssetPlatformProcessorSettings() {
            Sections = new Dictionary<string, AssetPlatformSettingsSection>(StringComparer.OrdinalIgnoreCase);
        }

        /// <summary>
        /// Gets or sets the registered section payloads keyed by section id.
        /// </summary>
        public Dictionary<string, AssetPlatformSettingsSection> Sections { get; set; }
    }
}
```

```csharp
writer.WriteInt32(entry.Value.Sections.Count);
foreach (KeyValuePair<string, AssetPlatformSettingsSection> sectionEntry in entry.Value.Sections) {
    writer.WriteString(sectionEntry.Key);
    AssetPlatformSettingsSectionRegistry.Shared.SerializeSection(writer, sectionEntry.Key, sectionEntry.Value.Settings);
}
```

```csharp
int sectionCount = reader.ReadInt32();
for (int sectionIndex = 0; sectionIndex < sectionCount; sectionIndex++) {
    string sectionId = reader.ReadString();
    object sectionSettings = AssetPlatformSettingsSectionRegistry.Shared.DeserializeSection(reader, sectionId);
    platformSettings.Sections.Add(sectionId, new AssetPlatformSettingsSection(sectionId, sectionSettings));
}
```

- [ ] **Step 4: Run the focused tests to verify the registry and serializer pass**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~AssetPlatformSettingsSectionRegistryTests|FullyQualifiedName~AssetImportSettingsBinarySerializerTests" -v minimal
```

Expected:

```text
PASS
```

- [ ] **Step 5: Commit the registry and serializer foundation**

```bash
git add C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetPlatformSettingsSection.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\IAssetPlatformSettingsSectionDefinition.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetPlatformSettingsSectionRegistry.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\TextureAssetPlatformSettingsSectionDefinition.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\ModelAssetPlatformSettingsSectionDefinition.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\MaterialAssetPlatformSettingsSectionDefinition.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\FontAssetProcessorSettings.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\FontAssetPlatformSettingsSectionDefinition.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetPlatformProcessorSettings.cs C:\dev\helworks\helengine\engine\helengine.editor\serialization\AssetImportSettingsBinarySerializer.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\asset\AssetPlatformSettingsSectionRegistryTests.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\AssetImportSettingsBinarySerializerTests.cs
git commit -m "feat: add generic platform settings section registry"
```

## Task 2: Refactor the Asset Import Settings View to Host Registered Sections

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\components\ui\AssetImportSettingsView.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AssetImportSettingsViewTests.cs`

- [ ] **Step 1: Write the failing editor tests for the new font section and platform-scoped edits**

```csharp
/// <summary>
/// Ensures font assets expose the registered font section for the active platform.
/// </summary>
[Fact]
public void Show_WhenFontSettingsExist_UsesTheActivePlatformFontPixelSize() {
    AssetImportSettingsView view = new AssetImportSettingsView(CreateFont(), 1);
    AssetProcessorSettings settings = new AssetProcessorSettings();
    AssetPlatformProcessorSettings dsSettings = new AssetPlatformProcessorSettings();
    AssetPlatformSettingsSectionRegistry.Shared.SetSection(dsSettings, "font", new FontAssetProcessorSettings {
        PixelSize = 10
    });
    AssetPlatformSettingsSectionRegistry.Shared.SetSection(dsSettings, "texture", new TextureAssetProcessorSettings {
        MaxResolution = 64,
        ColorFormat = TextureAssetColorFormat.Indexed8,
        AlphaPrecision = TextureAssetAlphaPrecision.A8
    });
    settings.Platforms["ds"] = dsSettings;

    view.Show(
        ["test-font"],
        "test-font",
        settings,
        ["windows", "ds"],
        "ds",
        AssetEntryKind.Font,
        CreatePlatformDefinitionsById());

    Assert.True(view.IsFontProcessorVisible);
    Assert.Equal(10, view.CurrentFontPixelSizeValue);
}

/// <summary>
/// Ensures changing one platform font pixel size raises an apply request only for the selected platform.
/// </summary>
[Fact]
public void Apply_WhenFontPixelSizeChanges_RaisesPlatformScopedRequest() {
    AssetImportSettingsView view = new AssetImportSettingsView(CreateFont(), 1);
    AssetImportSettingsApplyRequest raisedRequest = null;
    AssetProcessorSettings settings = new AssetProcessorSettings();
    AssetPlatformProcessorSettings windowsSettings = new AssetPlatformProcessorSettings();
    AssetPlatformSettingsSectionRegistry.Shared.SetSection(windowsSettings, "font", new FontAssetProcessorSettings {
        PixelSize = 32
    });
    settings.Platforms["windows"] = windowsSettings;
    view.ApplyRequested += request => raisedRequest = request;

    view.Show(
        ["test-font"],
        "test-font",
        settings,
        ["windows"],
        "windows",
        AssetEntryKind.Font);

    TextBoxComponent fontPixelSizeTextBox = GetPrivateField<TextBoxComponent>(view, "FontPixelSizeTextBox");
    fontPixelSizeTextBox.Text = "12";
    InvokePrivate(view, "HandleApplyClicked");

    Assert.NotNull(raisedRequest);
    FontAssetProcessorSettings savedSettings = AssetPlatformSettingsSectionRegistry.Shared.GetOrCreateSection<FontAssetProcessorSettings>(
        raisedRequest.ProcessorSettings.Platforms["windows"],
        "font");
    Assert.Equal(12, savedSettings.PixelSize);
}
```

- [ ] **Step 2: Run the focused editor view tests to verify they fail**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~Show_WhenFontSettingsExist_UsesTheActivePlatformFontPixelSize|FullyQualifiedName~Apply_WhenFontPixelSizeChanges_RaisesPlatformScopedRequest" -v minimal
```

Expected:

```text
FAIL
CS1061 for IsFontProcessorVisible / CurrentFontPixelSizeValue or missing font controls
```

- [ ] **Step 3: Refactor `AssetImportSettingsView` to use the registry-backed section model**

```csharp
bool IsFontProcessorVisible => CurrentEntryKind == AssetEntryKind.Font;

int CurrentFontPixelSizeValue {
    get {
        FontAssetProcessorSettings fontSettings = AssetPlatformSettingsSectionRegistry.Shared.GetOrCreateSection<FontAssetProcessorSettings>(
            GetPendingPlatformSettings(CurrentPlatformId),
            "font");
        return fontSettings.PixelSize;
    }
}

AssetPlatformProcessorSettings ClonePlatformProcessorSettings(AssetPlatformProcessorSettings platformSettings) {
    AssetPlatformProcessorSettings clone = new AssetPlatformProcessorSettings();
    if (platformSettings == null) {
        return clone;
    }

    foreach (KeyValuePair<string, AssetPlatformSettingsSection> pair in platformSettings.Sections) {
        clone.Sections[pair.Key] = AssetPlatformSettingsSectionRegistry.Shared.CloneSection(pair.Key, pair.Value);
    }

    return clone;
}

bool ProcessorSettingsMatch(AssetProcessorSettings left, AssetProcessorSettings right) {
    for (int i = 0; i < SupportedPlatformIds.Count; i++) {
        string platformId = SupportedPlatformIds[i];
        AssetPlatformProcessorSettings leftPlatform = ResolvePlatformSettings(left, platformId);
        AssetPlatformProcessorSettings rightPlatform = ResolvePlatformSettings(right, platformId);
        if (!AssetPlatformSettingsSectionRegistry.Shared.SectionsEqual(leftPlatform, rightPlatform)) {
            return false;
        }
    }

    return true;
}
```

```csharp
FontAssetProcessorSettings fontSettings = AssetPlatformSettingsSectionRegistry.Shared.GetOrCreateSection<FontAssetProcessorSettings>(
    GetPendingPlatformSettings(CurrentPlatformId),
    "font");
FontPixelSizeTextBox.Text = fontSettings.PixelSize.ToString(System.Globalization.CultureInfo.InvariantCulture);
```

- [ ] **Step 4: Run the focused editor view tests to verify the dynamic font section passes**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~Show_WhenFontSettingsExist_UsesTheActivePlatformFontPixelSize|FullyQualifiedName~Apply_WhenFontPixelSizeChanges_RaisesPlatformScopedRequest|FullyQualifiedName~Show_WhenFontTextureCapabilityMetadataExists_ConstrainsFontTextureFormatOptions" -v minimal
```

Expected:

```text
PASS
```

- [ ] **Step 5: Commit the dynamic editor refactor**

```bash
git add C:\dev\helworks\helengine\engine\helengine.editor\components\ui\AssetImportSettingsView.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\AssetImportSettingsViewTests.cs
git commit -m "feat: drive asset import settings view from section registry"
```

## Task 3: Thread the `font` Section Through the Import Pipeline and Cache

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\content\font\IFontImporter.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\content\font\FontImporterContentProcessor.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetImportManager.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\testing\ConfigurableFontImporter.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\testing\TestFontImporter.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\testing\RecordingFontImporter.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\asset\AssetImportManagerFontPlatformSettingsTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorPlatformFontVariantCacheServiceTests.cs`

- [ ] **Step 1: Write the failing font pipeline tests**

```csharp
using helengine.editor.tests.testing;
using Xunit;

namespace helengine.editor.tests.managers.asset {
    /// <summary>
    /// Verifies platform font settings are forwarded into font imports.
    /// </summary>
    public sealed class AssetImportManagerFontPlatformSettingsTests : IDisposable {
        readonly string ProjectRootPath;
        readonly string AssetsRootPath;

        public AssetImportManagerFontPlatformSettingsTests() {
            ProjectRootPath = Path.Combine(Path.GetTempPath(), "helengine-asset-import-manager-font-platform-tests", Guid.NewGuid().ToString("N"));
            AssetsRootPath = Path.Combine(ProjectRootPath, "assets");
            Directory.CreateDirectory(AssetsRootPath);
        }

        public void Dispose() {
            if (Directory.Exists(ProjectRootPath)) {
                Directory.Delete(ProjectRootPath, true);
            }
        }

        [Fact]
        public void BuildFontAssetForPlatform_WhenPlatformFontSectionExists_ForwardsFontPixelSizeToImporter() {
            string sourcePath = Path.Combine(AssetsRootPath, "Fonts", "DemoBody.ttf");
            Directory.CreateDirectory(Path.GetDirectoryName(sourcePath) ?? throw new InvalidOperationException("Font directory path could not be resolved."));
            File.WriteAllBytes(sourcePath, [0x01, 0x02, 0x03]);

            ContentManager contentManager = new ContentManager(AssetsRootPath);
            RecordingFontImporter importer = new RecordingFontImporter();
            AssetImportManager manager = new AssetImportManager(ProjectRootPath, contentManager);
            manager.RegisterFontImporter(new FontImporterRegistration("test-font", importer, [".ttf"]));

            AssetImportSettings settings = manager.LoadOrCreateImportSettings(sourcePath);
            AssetPlatformProcessorSettings dsSettings = new AssetPlatformProcessorSettings();
            AssetPlatformSettingsSectionRegistry.Shared.SetSection(dsSettings, "font", new FontAssetProcessorSettings {
                PixelSize = 11
            });
            AssetPlatformSettingsSectionRegistry.Shared.SetSection(dsSettings, "texture", new TextureAssetProcessorSettings {
                MaxResolution = 0,
                ColorFormat = TextureAssetColorFormat.Rgba32,
                AlphaPrecision = TextureAssetAlphaPrecision.A8
            });
            settings.Processor.Platforms["ds"] = dsSettings;
            manager.SaveImportSettings(sourcePath, settings);

            manager.BuildFontAssetForPlatform(sourcePath, "ds");

            Assert.NotNull(importer.LastSettings);
            Assert.Equal(11, importer.LastSettings.PixelSize);
        }
    }
}
```

```csharp
/// <summary>
/// Ensures changing per-platform font settings invalidates the cached platform font variant.
/// </summary>
[Fact]
public void ResolveVariant_WhenPlatformFontSettingsChange_RegeneratesCachedVariant() {
    string sourcePath = WriteSourceFont("Fonts/DemoCaption.ttf");
    AssetImportManager manager = CreateFontManager(new ConfigurableFontImporter(256, 128, new byte[256 * 128 * 4]));
    ConfigureFontSettings(manager, sourcePath, "gamecube", 32);
    EditorPlatformFontVariantCacheService service = new EditorPlatformFontVariantCacheService(manager);

    EditorPlatformFontVariantCacheResult firstResult = service.ResolveVariant(sourcePath, "gamecube");
    ConfigureFontSettings(manager, sourcePath, "gamecube", 12);

    EditorPlatformFontVariantCacheResult secondResult = service.ResolveVariant(sourcePath, "gamecube");

    Assert.False(firstResult.IsCacheHit);
    Assert.False(secondResult.IsCacheHit);
    Assert.NotEqual(firstResult.CachedFontAssetPath, secondResult.CachedFontAssetPath);
}
```

- [ ] **Step 2: Run the focused pipeline tests to verify they fail**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~AssetImportManagerFontPlatformSettingsTests|FullyQualifiedName~ResolveVariant_WhenPlatformFontSettingsChange_RegeneratesCachedVariant" -v minimal
```

Expected:

```text
FAIL
CS1501 / CS7036 because IFontImporter does not accept FontAssetProcessorSettings yet
```

- [ ] **Step 3: Expand the font importer contract and bypass the opaque content-manager path for platform font imports**

```csharp
namespace helengine.editor {
    /// <summary>
    /// Provides a contract for importing font assets.
    /// </summary>
    public interface IFontImporter {
        /// <summary>
        /// Imports the representation of a font from a stream using the supplied platform font settings.
        /// </summary>
        /// <param name="stream">Stream containing font data.</param>
        /// <param name="settings">Active platform font settings.</param>
        /// <returns>Imported <see cref="FontAsset"/> for the font.</returns>
        FontAsset ImportFont(Stream stream, FontAssetProcessorSettings settings);
    }
}
```

```csharp
FontAsset BuildImportedFontAsset(string sourcePath, AssetImportSettings settings, string platformId, string fontAssetId) {
    EnsureFontImporterExists(settings.Importer.ImporterId);
    IFontImporter importer = fontImportersById[settings.Importer.ImporterId];
    FontAssetProcessorSettings fontProcessorSettings = GetFontProcessorSettings(settings, platformId);
    using FileStream stream = new FileStream(sourcePath, FileMode.Open, FileAccess.Read, FileShare.Read);
    FontAsset asset = importer.ImportFont(stream, fontProcessorSettings);

    if (asset == null) {
        throw new InvalidOperationException($"Font importer '{settings.Importer.ImporterId}' did not return an asset.");
    }

    TextureAssetProcessorSettings textureProcessorSettings = GetTextureProcessorSettings(settings, platformId);
    if (textureProcessorSettings.UsesGenericColorFormat()) {
        TextureAsset processedSourceTextureAsset = TextureAssetProcessor.Apply(asset.SourceTextureAsset, textureProcessorSettings);
        asset.ApplyProcessedSourceTextureAsset(processedSourceTextureAsset);
    }

    string fontAtlasAssetId = fontAssetId + "#atlas";
    asset.SourceTextureAsset.Id = fontAtlasAssetId;
    asset.SourceTextureAsset.RuntimeAssetId = RuntimeAssetIdGenerator.Generate(fontAtlasAssetId);
    return asset;
}
```

```csharp
FontAsset Read(Stream stream) {
    if (stream == null) {
        throw new ArgumentNullException(nameof(stream));
    }

    return Importer.ImportFont(stream, new FontAssetProcessorSettings {
        PixelSize = 32
    });
}
```

- [ ] **Step 4: Run the focused pipeline tests to verify the section reaches font imports and cache identity**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~AssetImportManagerFontPlatformSettingsTests|FullyQualifiedName~ResolveVariant_WhenPlatformFontSettingsChange_RegeneratesCachedVariant|FullyQualifiedName~ResolveVariant_WhenPlatformTextureSettingsChange_RegeneratesCachedVariant" -v minimal
```

Expected:

```text
PASS
```

- [ ] **Step 5: Commit the font pipeline refactor**

```bash
git add C:\dev\helworks\helengine\engine\helengine.editor\content\font\IFontImporter.cs C:\dev\helworks\helengine\engine\helengine.editor\content\font\FontImporterContentProcessor.cs C:\dev\helworks\helengine\engine\helengine.editor\managers\asset\AssetImportManager.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\testing\ConfigurableFontImporter.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\testing\TestFontImporter.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\testing\RecordingFontImporter.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\asset\AssetImportManagerFontPlatformSettingsTests.cs C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorPlatformFontVariantCacheServiceTests.cs
git commit -m "feat: forward platform font settings through font import pipeline"
```

## Task 4: Make the Windows GDI Importer Honor Per-Platform Pixel Size

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.windows\content\font\GdiFontImporter.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.windows.tests\content\font\GdiFontImporterTests.cs`

- [ ] **Step 1: Write the failing GDI importer regression test**

```csharp
using helengine.editor;

namespace helengine.editor.windows.tests.content.font;

/// <summary>
/// Verifies the GDI-backed font importer respects the requested platform pixel size.
/// </summary>
public sealed class GdiFontImporterTests {
    /// <summary>
    /// Ensures importing the same source font with two different pixel sizes changes the emitted font metrics.
    /// </summary>
    [Fact]
    public void ImportFont_WhenPixelSizeChanges_UsesRequestedPlatformSize() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourceFontPath = Path.Combine(repositoryRootPath, "vendor", "bepuphysics2", "Demos", "Content", "Carlito-Regular.ttf");
        GdiFontImporter importer = new GdiFontImporter();

        using FileStream smallStream = new FileStream(sourceFontPath, FileMode.Open, FileAccess.Read, FileShare.Read);
        FontAsset smallFont = importer.ImportFont(smallStream, new FontAssetProcessorSettings {
            PixelSize = 12
        });

        using FileStream largeStream = new FileStream(sourceFontPath, FileMode.Open, FileAccess.Read, FileShare.Read);
        FontAsset largeFont = importer.ImportFont(largeStream, new FontAssetProcessorSettings {
            PixelSize = 32
        });

        Assert.NotEqual(smallFont.LineHeight, largeFont.LineHeight);
        Assert.True(smallFont.AtlasHeight < largeFont.AtlasHeight);
    }
}
```

- [ ] **Step 2: Run the focused GDI importer tests to verify they fail**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.windows.tests\helengine.editor.windows.tests.csproj --filter "FullyQualifiedName~GdiFontImporterTests" -v minimal
```

Expected:

```text
FAIL
CS1501 / CS7036 because GdiFontImporter still implements ImportFont(Stream) only
```

- [ ] **Step 3: Replace the hardcoded `32f` in `GdiFontImporter` with the supplied font settings**

```csharp
public FontAsset ImportFont(Stream stream, FontAssetProcessorSettings settings) {
    if (stream == null) {
        throw new ArgumentNullException(nameof(stream));
    } else if (settings == null) {
        throw new ArgumentNullException(nameof(settings));
    } else if (settings.PixelSize < 1) {
        throw new InvalidOperationException("Font pixel size must be greater than zero.");
    }

    using MemoryStream buffer = new MemoryStream();
    stream.CopyTo(buffer);
    byte[] bytes = buffer.ToArray();
    if (bytes.Length == 0) {
        throw new InvalidOperationException("Font source stream must contain data.");
    }

    string temporaryFontFilePath = string.Empty;
    try {
        using PrivateFontCollection fontCollection = LoadFontCollection(bytes, ref temporaryFontFilePath);
        using System.Drawing.Font font = new System.Drawing.Font(
            fontCollection.Families[0],
            settings.PixelSize,
            System.Drawing.FontStyle.Regular,
            System.Drawing.GraphicsUnit.Pixel);
        return GDIFontProcessor.ImportFont(font);
    } finally {
        if (!string.IsNullOrWhiteSpace(temporaryFontFilePath) && File.Exists(temporaryFontFilePath)) {
            File.Delete(temporaryFontFilePath);
        }
    }
}
```

- [ ] **Step 4: Run the GDI importer tests to verify the pixel size path passes**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.windows.tests\helengine.editor.windows.tests.csproj --filter "FullyQualifiedName~GdiFontImporterTests" -v minimal
```

Expected:

```text
PASS
```

- [ ] **Step 5: Commit the GDI font pixel-size support**

```bash
git add C:\dev\helworks\helengine\engine\helengine.editor.windows\content\font\GdiFontImporter.cs C:\dev\helworks\helengine\engine\helengine.editor.windows.tests\content\font\GdiFontImporterTests.cs
git commit -m "feat: honor platform font pixel size in GDI font import"
```

## Task 5: Run the Focused End-to-End Suite

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\docs\superpowers\plans\2026-06-25-generic-platform-settings-registry.md`

- [ ] **Step 1: Run the editor test suite that covers the registry, serializer, view, and font pipeline**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~AssetPlatformSettingsSectionRegistryTests|FullyQualifiedName~AssetImportSettingsBinarySerializerTests|FullyQualifiedName~AssetImportManagerFontPlatformSettingsTests|FullyQualifiedName~AssetImportSettingsViewTests|FullyQualifiedName~EditorPlatformFontVariantCacheServiceTests" -v minimal
```

Expected:

```text
PASS
```

- [ ] **Step 2: Run the Windows-specific importer suite**

Run:

```powershell
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.windows.tests\helengine.editor.windows.tests.csproj --filter "FullyQualifiedName~GdiFontImporterTests" -v minimal
```

Expected:

```text
PASS
```

- [ ] **Step 3: Review the final diff before handing off**

Run:

```powershell
rtk git status --short
rtk git diff --stat
```

Expected:

```text
Only the planned engine/editor/test files are modified, with no unrelated reversions.
```
