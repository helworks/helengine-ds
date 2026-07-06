# Scene Asset Reference Hardening Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make unsupported scene asset references impossible to author through normal engine APIs, fail malformed references at early editor boundaries, and update our project code to the sanctioned factory paths.

**Architecture:** `helengine.core` owns the constrained `SceneAssetReference` shape plus the file-backed and engine-generated factories. `helengine.editor` owns editor-generated reference creation and early validation at save-time boundaries. Existing engine/editor/project code is migrated off raw `new SceneAssetReference { ... }` construction and onto named factory helpers, while resolver and packaging checks remain strict invariant backstops.

**Tech Stack:** C#/.NET 9, xUnit, Helengine core/editor scene serialization, editor build pipeline, Nintendo DS project build verification

---

## File Structure

### Core reference model

- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\scene\SceneAssetReference.cs`
  Makes scene asset references immutable to ordinary callers and exposes validated construction paths only.
- Create: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\scene\SceneAssetReferenceFactory.cs`
  Owns file-backed scene asset reference creation.
- Create: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\scene\EngineSceneAssetReferenceFactory.cs`
  Owns engine-supported generated scene asset reference creation.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\EditorAssetBinarySerializer.cs`
  Rehydrates serialized scene asset references through the new validated construction path.
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\AutomaticScriptComponentRuntimeDeserializer.cs`
  Rehydrates runtime payload scene references through the new validated construction path.

### Editor reference ownership and validation

- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\SceneAssetReferenceFactory.cs`
  Renames or reshapes the existing entry-based builder so it delegates to sanctioned core/editor factories instead of constructing raw references.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\EditorSceneAssetReferenceFactory.cs`
  Owns editor-supported generated references such as the editor UI font.
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\SceneAssetReferenceValidationService.cs`
  Centralizes early save-time validation with asset-type-aware error messages.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\FontAssetScenePersistenceSupport.cs`
  Replaces raw reference construction and routes stored font references through validation.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\SceneAssetReferenceInferenceService.cs`
  Replaces raw file-backed and engine-generated inference helpers with sanctioned factories and validation.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\SceneComponentBinaryFieldEncoding.cs`
  Rehydrates serialized references through the constrained construction path.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\ComponentPlatformOverridePayloadService.cs`
  Rehydrates override payload references through the constrained construction path.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\SceneSaveService.cs`
  Triggers early validation while collecting scene dependencies and component payloads.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\AutomaticScriptComponentPersistenceDescriptor.cs`
  Validates typed asset references before they are written into tagged payloads.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\EditorSceneAssetReferenceResolver.cs`
  Switches tests and helper code to sanctioned factory construction while keeping strict backstop checks.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs`
  Replaces internal reference creation helpers with sanctioned factories while preserving invariant checks.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\ScriptComponentPlayerDeserializerGenerator.cs`
  Emits sanctioned reference construction instead of raw object initialization in generated code.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\EditorSession.cs`
  Replaces entry-based raw reference construction with the sanctioned factory path.

### Tests

- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneAssetReferenceFactoryTests.cs`
  Verifies immutability and sanctioned factory output shapes.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\EditorSceneAssetReferenceResolverTests.cs`
  Updates reference construction to sanctioned factories and keeps unsupported generated font assertions.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneSaveServiceTests.cs`
  Adds early save-time failure coverage for unsupported references.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\SceneComponentPackagingTransformServiceTests.cs`
  Updates helper references to sanctioned factories and keeps packaging as a backstop-only failure case.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\ComponentPropertiesViewGeneratedAssetTests.cs`
  Updates generated asset expectations to sanctioned factory output.
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\EditorSessionGeneratedAssetTests.cs`
  Updates session-generated reference assertions to sanctioned factory output.

### Project-side follow-through

- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs`
  Replaces raw DS debug-font reference creation with sanctioned file-backed font references.
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\DemoSceneInstructionOverlayFactory.cs`
  Replaces raw DS debug-font reference creation with sanctioned file-backed font references.
- Modify: `C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs`
  Replaces raw DS debug-font reference creation with sanctioned file-backed font references.

### Task 1: Lock Down `SceneAssetReference` In Core

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\scene\SceneAssetReference.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\scene\SceneAssetReferenceFactory.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\scene\EngineSceneAssetReferenceFactory.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\EditorAssetBinarySerializer.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\AutomaticScriptComponentRuntimeDeserializer.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneAssetReferenceFactoryTests.cs`

- [ ] **Step 1: Write the failing core/reference-factory tests**

```csharp
using Xunit;

namespace helengine.editor.tests.serialization.scene {
    /// <summary>
    /// Verifies constrained scene asset reference construction.
    /// </summary>
    public sealed class SceneAssetReferenceFactoryTests {
        /// <summary>
        /// Ensures the scene asset reference no longer exposes a public parameterless constructor or writable properties.
        /// </summary>
        [Fact]
        public void SceneAssetReference_IsNotFreelyMutable() {
            Assert.Null(typeof(SceneAssetReference).GetConstructor(Type.EmptyTypes));
            Assert.False(typeof(SceneAssetReference).GetProperty(nameof(SceneAssetReference.SourceKind)).CanWrite);
            Assert.False(typeof(SceneAssetReference).GetProperty(nameof(SceneAssetReference.RelativePath)).CanWrite);
            Assert.False(typeof(SceneAssetReference).GetProperty(nameof(SceneAssetReference.ProviderId)).CanWrite);
            Assert.False(typeof(SceneAssetReference).GetProperty(nameof(SceneAssetReference.AssetId)).CanWrite);
        }

        /// <summary>
        /// Ensures file-backed references come from the sanctioned file-system factory shape.
        /// </summary>
        [Fact]
        public void CreateFileSystemFont_ReturnsFileBackedReference() {
            SceneAssetReference reference = global::helengine.SceneAssetReferenceFactory.CreateFileSystemFont("Fonts/DemoDiscBody.ttf");

            Assert.Equal(SceneAssetReferenceSourceKind.FileSystem, reference.SourceKind);
            Assert.Equal("Fonts/DemoDiscBody.ttf", reference.RelativePath);
            Assert.Equal(string.Empty, reference.ProviderId);
            Assert.Equal(string.Empty, reference.AssetId);
        }

        /// <summary>
        /// Ensures generated engine references come from the sanctioned engine-generated factory shape.
        /// </summary>
        [Fact]
        public void CreateCubeModel_ReturnsEngineGeneratedReference() {
            SceneAssetReference reference = global::helengine.EngineSceneAssetReferenceFactory.CreateCubeModel();

            Assert.Equal(SceneAssetReferenceSourceKind.Generated, reference.SourceKind);
            Assert.Equal(EngineGeneratedAssetProvider.ProviderIdValue, reference.ProviderId);
            Assert.Equal(EngineGeneratedModelCache.CubeAssetId, reference.AssetId);
            Assert.Equal(EngineGeneratedAssetProvider.CubeRelativePath, reference.RelativePath);
        }
    }
}
```

- [ ] **Step 2: Run the focused reference-factory tests to verify they fail**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneAssetReferenceFactoryTests" -v minimal`

Expected: FAIL because `SceneAssetReference` still has a public parameterless constructor and writable properties, and the new factory types do not exist yet.

- [ ] **Step 3: Implement the constrained core reference API**

```csharp
namespace helengine {
    /// <summary>
    /// Stores one validated stable reference from scene persistence metadata to a project or generated asset.
    /// </summary>
    public sealed class SceneAssetReference {
        /// <summary>
        /// Initializes one validated scene asset reference.
        /// </summary>
        /// <param name="sourceKind">Reference source kind.</param>
        /// <param name="relativePath">Stable relative path.</param>
        /// <param name="providerId">Generated provider id when applicable.</param>
        /// <param name="assetId">Generated asset id when applicable.</param>
        internal SceneAssetReference(SceneAssetReferenceSourceKind sourceKind, string relativePath, string providerId, string assetId) {
            SourceKind = sourceKind;
            RelativePath = relativePath ?? string.Empty;
            ProviderId = providerId ?? string.Empty;
            AssetId = assetId ?? string.Empty;
        }

        /// <summary>
        /// Gets the source category used to resolve the asset reference.
        /// </summary>
        public SceneAssetReferenceSourceKind SourceKind { get; }

        /// <summary>
        /// Gets the project-relative or generated relative path.
        /// </summary>
        public string RelativePath { get; }

        /// <summary>
        /// Gets the generated provider identifier.
        /// </summary>
        public string ProviderId { get; }

        /// <summary>
        /// Gets the provider-local asset identifier.
        /// </summary>
        public string AssetId { get; }
    }
}

namespace helengine {
    /// <summary>
    /// Creates sanctioned file-backed scene asset references.
    /// </summary>
    public static class SceneAssetReferenceFactory {
        /// <summary>
        /// Creates one validated file-backed font reference.
        /// </summary>
        /// <param name="relativePath">Project-relative font path.</param>
        /// <returns>Validated file-backed font reference.</returns>
        public static SceneAssetReference CreateFileSystemFont(string relativePath) {
            return CreateFileSystem(relativePath);
        }

        /// <summary>
        /// Creates one validated file-backed texture reference.
        /// </summary>
        /// <param name="relativePath">Project-relative texture path.</param>
        /// <returns>Validated file-backed texture reference.</returns>
        public static SceneAssetReference CreateFileSystemTexture(string relativePath) {
            return CreateFileSystem(relativePath);
        }

        /// <summary>
        /// Creates one validated file-backed model reference.
        /// </summary>
        /// <param name="relativePath">Project-relative model path.</param>
        /// <returns>Validated file-backed model reference.</returns>
        public static SceneAssetReference CreateFileSystemModel(string relativePath) {
            return CreateFileSystem(relativePath);
        }

        /// <summary>
        /// Creates one validated file-backed material reference.
        /// </summary>
        /// <param name="relativePath">Project-relative material path.</param>
        /// <returns>Validated file-backed material reference.</returns>
        public static SceneAssetReference CreateFileSystemMaterial(string relativePath) {
            return CreateFileSystem(relativePath);
        }

        /// <summary>
        /// Creates one validated file-backed animation-clip reference.
        /// </summary>
        /// <param name="relativePath">Project-relative animation-clip path.</param>
        /// <returns>Validated file-backed animation-clip reference.</returns>
        public static SceneAssetReference CreateFileSystemAnimationClip(string relativePath) {
            return CreateFileSystem(relativePath);
        }

        /// <summary>
        /// Creates one validated file-backed scene asset reference.
        /// </summary>
        /// <param name="relativePath">Project-relative asset path.</param>
        /// <returns>Validated file-backed scene asset reference.</returns>
        internal static SceneAssetReference CreateFileSystem(string relativePath) {
            if (string.IsNullOrWhiteSpace(relativePath)) {
                throw new ArgumentException("File-backed asset references must include a relative path.", nameof(relativePath));
            }

            return new SceneAssetReference(SceneAssetReferenceSourceKind.FileSystem, relativePath, string.Empty, string.Empty);
        }

        /// <summary>
        /// Rehydrates one serialized scene asset reference.
        /// </summary>
        /// <param name="sourceKind">Serialized source kind.</param>
        /// <param name="relativePath">Serialized relative path.</param>
        /// <param name="providerId">Serialized provider id.</param>
        /// <param name="assetId">Serialized asset id.</param>
        /// <returns>Validated rehydrated scene asset reference.</returns>
        internal static SceneAssetReference Rehydrate(SceneAssetReferenceSourceKind sourceKind, string relativePath, string providerId, string assetId) {
            if (sourceKind == SceneAssetReferenceSourceKind.FileSystem) {
                return CreateFileSystem(relativePath);
            }

            return EngineSceneAssetReferenceFactory.RehydrateGenerated(relativePath, providerId, assetId);
        }
    }
}

namespace helengine {
    /// <summary>
    /// Creates sanctioned engine-generated scene asset references.
    /// </summary>
    public static class EngineSceneAssetReferenceFactory {
        /// <summary>
        /// Creates the generated engine cube model reference.
        /// </summary>
        /// <returns>Validated generated cube model reference.</returns>
        public static SceneAssetReference CreateCubeModel() {
            return CreateGenerated(EngineGeneratedAssetProvider.CubeRelativePath, EngineGeneratedModelCache.CubeAssetId);
        }

        /// <summary>
        /// Creates the generated engine plane model reference.
        /// </summary>
        /// <returns>Validated generated plane model reference.</returns>
        public static SceneAssetReference CreatePlaneModel() {
            return CreateGenerated(EngineGeneratedAssetProvider.PlaneRelativePath, EngineGeneratedModelCache.PlaneAssetId);
        }

        /// <summary>
        /// Creates the generated engine sphere model reference.
        /// </summary>
        /// <returns>Validated generated sphere model reference.</returns>
        public static SceneAssetReference CreateSphereModel() {
            return CreateGenerated(EngineGeneratedAssetProvider.SphereRelativePath, EngineGeneratedModelCache.SphereAssetId);
        }

        /// <summary>
        /// Creates the generated engine standard material reference.
        /// </summary>
        /// <returns>Validated generated standard material reference.</returns>
        public static SceneAssetReference CreateStandardMaterial() {
            return CreateGenerated(EngineGeneratedAssetProvider.StandardMaterialRelativePath, EngineGeneratedMaterialCache.StandardAssetId);
        }

        /// <summary>
        /// Rehydrates one validated engine-generated scene asset reference.
        /// </summary>
        /// <param name="relativePath">Serialized generated relative path.</param>
        /// <param name="providerId">Serialized generated provider id.</param>
        /// <param name="assetId">Serialized generated asset id.</param>
        /// <returns>Validated engine-generated scene asset reference.</returns>
        internal static SceneAssetReference RehydrateGenerated(string relativePath, string providerId, string assetId) {
            if (!string.Equals(providerId, EngineGeneratedAssetProvider.ProviderIdValue, StringComparison.Ordinal)) {
                throw new InvalidOperationException($"Unsupported generated provider '{providerId}'.");
            }
            if (string.IsNullOrWhiteSpace(assetId) || string.IsNullOrWhiteSpace(relativePath)) {
                throw new InvalidOperationException("Generated scene asset references must include both relative path and asset id.");
            }

            return new SceneAssetReference(SceneAssetReferenceSourceKind.Generated, relativePath, providerId, assetId);
        }

        /// <summary>
        /// Creates one validated engine-generated scene asset reference.
        /// </summary>
        /// <param name="relativePath">Generated relative path.</param>
        /// <param name="assetId">Generated asset id.</param>
        /// <returns>Validated engine-generated scene asset reference.</returns>
        static SceneAssetReference CreateGenerated(string relativePath, string assetId) {
            return new SceneAssetReference(SceneAssetReferenceSourceKind.Generated, relativePath, EngineGeneratedAssetProvider.ProviderIdValue, assetId);
        }
    }
}
```

- [ ] **Step 4: Rewire binary readers to the constrained rehydration path**

```csharp
static SceneAssetReference ReadSceneAssetReference(EngineBinaryReader reader) {
    return SceneAssetReferenceFactory.Rehydrate(
        (SceneAssetReferenceSourceKind)reader.ReadInt32(),
        reader.ReadString(),
        reader.ReadString(),
        reader.ReadString());
}

static SceneAssetReference ReadOptionalReference(EngineBinaryReader reader) {
    if (reader.ReadByte() == 0) {
        return null;
    }

    return SceneAssetReferenceFactory.Rehydrate(
        (SceneAssetReferenceSourceKind)reader.ReadInt32(),
        reader.ReadString(),
        reader.ReadString(),
        reader.ReadString());
}
```

- [ ] **Step 5: Run the focused reference-factory tests to verify they pass**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneAssetReferenceFactoryTests" -v minimal`

Expected: PASS

- [ ] **Step 6: Commit the core hardening slice**

```bash
rtk git -C C:\dev\helworks\helengine add engine/helengine.core/assets/raw/scene/SceneAssetReference.cs engine/helengine.core/assets/raw/scene/SceneAssetReferenceFactory.cs engine/helengine.core/assets/raw/scene/EngineSceneAssetReferenceFactory.cs engine/helengine.core/assets/EditorAssetBinarySerializer.cs engine/helengine.core/scene/runtime/AutomaticScriptComponentRuntimeDeserializer.cs engine/helengine.editor.tests/serialization/scene/SceneAssetReferenceFactoryTests.cs
rtk git -C C:\dev\helworks\helengine commit -m "refactor: harden scene asset reference construction"
```

### Task 2: Move Editor Reference Creation To Sanctioned Factories

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\SceneAssetReferenceFactory.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\EditorSceneAssetReferenceFactory.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\FontAssetScenePersistenceSupport.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\SceneAssetReferenceInferenceService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\SceneComponentBinaryFieldEncoding.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\ComponentPlatformOverridePayloadService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\EditorSession.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\SceneComponentPackagingTransformService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\ScriptComponentPlayerDeserializerGenerator.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\EditorSceneAssetReferenceResolverTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\SceneComponentPackagingTransformServiceTests.cs`

- [ ] **Step 1: Write the failing editor factory tests**

```csharp
using Xunit;

namespace helengine.editor.tests.serialization.scene {
    /// <summary>
    /// Verifies editor-owned generated scene asset reference creation.
    /// </summary>
    public sealed class EditorSceneAssetReferenceFactoryTests {
        /// <summary>
        /// Ensures the editor UI font reference comes from one sanctioned helper.
        /// </summary>
        [Fact]
        public void CreateEditorUiFont_ReturnsEditorGeneratedReference() {
            SceneAssetReference reference = EditorSceneAssetReferenceFactory.CreateEditorUiFont();

            Assert.Equal(SceneAssetReferenceSourceKind.Generated, reference.SourceKind);
            Assert.Equal("editor", reference.ProviderId);
            Assert.Equal("ui-font", reference.AssetId);
            Assert.Equal("generated/editor/fonts/ui.hefont", reference.RelativePath);
        }

        /// <summary>
        /// Ensures browser-selected generated engine assets route through sanctioned generated factories instead of raw object initialization.
        /// </summary>
        [Fact]
        public void CreateFromEntry_WhenEntryIsGeneratedEngineCube_UsesSanctionedGeneratedFactory() {
            AssetBrowserEntry entry = AssetBrowserEntry.CreateGeneratedAsset(
                "Cube",
                EngineGeneratedAssetProvider.CubeRelativePath,
                AssetEntryKind.Model,
                EngineGeneratedAssetProvider.ProviderIdValue,
                EngineGeneratedModelCache.CubeAssetId);
            SceneAssetReferenceFactory factory = new SceneAssetReferenceFactory();

            SceneAssetReference reference = factory.CreateFromEntry(entry);

            Assert.Equal(EngineGeneratedAssetProvider.ProviderIdValue, reference.ProviderId);
            Assert.Equal(EngineGeneratedModelCache.CubeAssetId, reference.AssetId);
            Assert.Equal(EngineGeneratedAssetProvider.CubeRelativePath, reference.RelativePath);
        }
    }
}
```

- [ ] **Step 2: Run the editor factory tests to verify they fail**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorSceneAssetReferenceFactoryTests" -v minimal`

Expected: FAIL because `EditorSceneAssetReferenceFactory` does not exist and `SceneAssetReferenceFactory.CreateFromEntry` still constructs raw references directly.

- [ ] **Step 3: Implement editor-owned generated reference creation and delegate existing entry-based creation**

```csharp
namespace helengine.editor {
    /// <summary>
    /// Creates sanctioned editor-generated scene asset references.
    /// </summary>
    public static class EditorSceneAssetReferenceFactory {
        /// <summary>
        /// Creates the editor UI font scene asset reference.
        /// </summary>
        /// <returns>Validated editor UI font scene asset reference.</returns>
        public static SceneAssetReference CreateEditorUiFont() {
            return CreateGenerated("generated/editor/fonts/ui.hefont", "ui-font");
        }

        /// <summary>
        /// Rehydrates one serialized editor-generated scene asset reference.
        /// </summary>
        /// <param name="relativePath">Serialized relative path.</param>
        /// <param name="providerId">Serialized provider id.</param>
        /// <param name="assetId">Serialized asset id.</param>
        /// <returns>Validated editor-generated scene asset reference.</returns>
        internal static SceneAssetReference RehydrateGenerated(string relativePath, string providerId, string assetId) {
            if (!string.Equals(providerId, "editor", StringComparison.Ordinal)) {
                throw new InvalidOperationException($"Unsupported generated font provider '{providerId}'.");
            }
            if (string.Equals(assetId, "ui-font", StringComparison.Ordinal)) {
                return CreateEditorUiFont();
            }

            throw new InvalidOperationException($"Unsupported generated font asset id '{assetId}'.");
        }

        /// <summary>
        /// Creates one validated editor-generated scene asset reference.
        /// </summary>
        /// <param name="relativePath">Generated relative path.</param>
        /// <param name="assetId">Generated asset id.</param>
        /// <returns>Validated editor-generated scene asset reference.</returns>
        static SceneAssetReference CreateGenerated(string relativePath, string assetId) {
            return new SceneAssetReference(SceneAssetReferenceSourceKind.Generated, relativePath, "editor", assetId);
        }
    }
}

namespace helengine.editor {
    /// <summary>
    /// Converts selected asset-browser entries into sanctioned stable scene asset references.
    /// </summary>
    public class SceneAssetReferenceFactory {
        /// <summary>
        /// Creates one stable scene asset reference from an asset-browser entry.
        /// </summary>
        /// <param name="entry">Selected browser entry to convert.</param>
        /// <returns>Sanctioned scene asset reference describing the selected asset.</returns>
        public SceneAssetReference CreateFromEntry(AssetBrowserEntry entry) {
            if (entry == null) {
                throw new ArgumentNullException(nameof(entry));
            }

            if (!entry.IsGenerated) {
                if (entry.EntryKind == AssetEntryKind.Font) {
                    return global::helengine.SceneAssetReferenceFactory.CreateFileSystemFont(entry.RelativePath);
                }
                if (entry.EntryKind == AssetEntryKind.Texture) {
                    return global::helengine.SceneAssetReferenceFactory.CreateFileSystemTexture(entry.RelativePath);
                }
                if (entry.EntryKind == AssetEntryKind.Model) {
                    return global::helengine.SceneAssetReferenceFactory.CreateFileSystemModel(entry.RelativePath);
                }
                if (entry.EntryKind == AssetEntryKind.Material) {
                    return global::helengine.SceneAssetReferenceFactory.CreateFileSystemMaterial(entry.RelativePath);
                }
                if (entry.EntryKind == AssetEntryKind.AnimationClip) {
                    return global::helengine.SceneAssetReferenceFactory.CreateFileSystemAnimationClip(entry.RelativePath);
                }

                throw new InvalidOperationException($"Unsupported file-backed asset entry kind '{entry.EntryKind}'.");
            }
            if (string.Equals(entry.ProviderId, EngineGeneratedAssetProvider.ProviderIdValue, StringComparison.Ordinal)) {
                if (string.Equals(entry.AssetId, EngineGeneratedModelCache.CubeAssetId, StringComparison.Ordinal)) {
                    return global::helengine.EngineSceneAssetReferenceFactory.CreateCubeModel();
                }
                if (string.Equals(entry.AssetId, EngineGeneratedModelCache.PlaneAssetId, StringComparison.Ordinal)) {
                    return global::helengine.EngineSceneAssetReferenceFactory.CreatePlaneModel();
                }
                if (string.Equals(entry.AssetId, EngineGeneratedModelCache.SphereAssetId, StringComparison.Ordinal)) {
                    return global::helengine.EngineSceneAssetReferenceFactory.CreateSphereModel();
                }
                if (string.Equals(entry.AssetId, EngineGeneratedMaterialCache.StandardAssetId, StringComparison.Ordinal)) {
                    return global::helengine.EngineSceneAssetReferenceFactory.CreateStandardMaterial();
                }
            }
            if (string.Equals(entry.ProviderId, "editor", StringComparison.Ordinal) &&
                string.Equals(entry.AssetId, "ui-font", StringComparison.Ordinal)) {
                return EditorSceneAssetReferenceFactory.CreateEditorUiFont();
            }

            throw new InvalidOperationException($"Unsupported generated asset reference '{entry.ProviderId}:{entry.AssetId}'.");
        }
    }
}
```

- [ ] **Step 4: Replace raw editor-side reference construction helpers**

```csharp
internal static SceneAssetReference BuildEditorFontReference() {
        return EditorSceneAssetReferenceFactory.CreateEditorUiFont();
}

static SceneAssetReference CreateGeneratedReference(string relativePath, string assetId) {
    if (string.Equals(assetId, EngineGeneratedModelCache.CubeAssetId, StringComparison.Ordinal)) {
        return global::helengine.EngineSceneAssetReferenceFactory.CreateCubeModel();
    }
    if (string.Equals(assetId, EngineGeneratedModelCache.PlaneAssetId, StringComparison.Ordinal)) {
        return global::helengine.EngineSceneAssetReferenceFactory.CreatePlaneModel();
    }
    if (string.Equals(assetId, EngineGeneratedModelCache.SphereAssetId, StringComparison.Ordinal)) {
        return global::helengine.EngineSceneAssetReferenceFactory.CreateSphereModel();
    }
    if (string.Equals(assetId, EngineGeneratedMaterialCache.StandardAssetId, StringComparison.Ordinal)) {
        return global::helengine.EngineSceneAssetReferenceFactory.CreateStandardMaterial();
    }

    throw new InvalidOperationException($"Unsupported generated asset reference '{EngineGeneratedAssetProvider.ProviderIdValue}:{assetId}'.");
}

reference = global::helengine.SceneAssetReferenceFactory.CreateFileSystemMaterial(relativePath);
reference = global::helengine.SceneAssetReferenceFactory.CreateFileSystemModel(relativePath);
reference = global::helengine.SceneAssetReferenceFactory.CreateFileSystemTexture(relativePath);
```

- [ ] **Step 5: Update the serialization helpers and tests to the new construction model**

```csharp
return SceneAssetReferenceFactory.Rehydrate(
    (SceneAssetReferenceSourceKind)reader.ReadInt32(),
    reader.ReadString(),
    reader.ReadString(),
    reader.ReadString());

static SceneAssetReference CreateEditorFontReference() {
    return EditorSceneAssetReferenceFactory.CreateEditorUiFont();
}

static SceneAssetReference CreateFileFontReference(string relativePath) {
    return SceneAssetReferenceFactory.CreateFileSystemFont(relativePath);
}
```

- [ ] **Step 6: Run the focused editor factory, resolver, and packaging tests to verify they pass**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorSceneAssetReferenceFactoryTests|FullyQualifiedName~EditorSceneAssetReferenceResolverTests|FullyQualifiedName~SceneComponentPackagingTransformServiceTests" -v minimal`

Expected: PASS

- [ ] **Step 7: Commit the editor reference-creation slice**

```bash
rtk git -C C:\dev\helworks\helengine add engine/helengine.editor/serialization/scene/SceneAssetReferenceFactory.cs engine/helengine.editor/serialization/scene/EditorSceneAssetReferenceFactory.cs engine/helengine.editor/serialization/scene/FontAssetScenePersistenceSupport.cs engine/helengine.editor/serialization/scene/SceneAssetReferenceInferenceService.cs engine/helengine.editor/serialization/scene/SceneComponentBinaryFieldEncoding.cs engine/helengine.editor/serialization/scene/ComponentPlatformOverridePayloadService.cs engine/helengine.editor/EditorSession.cs engine/helengine.editor/managers/project/SceneComponentPackagingTransformService.cs engine/helengine.editor/managers/project/ScriptComponentPlayerDeserializerGenerator.cs engine/helengine.editor.tests/serialization/scene/EditorSceneAssetReferenceResolverTests.cs engine/helengine.editor.tests/managers/project/SceneComponentPackagingTransformServiceTests.cs engine/helengine.editor.tests/serialization/scene/EditorSceneAssetReferenceFactoryTests.cs
rtk git -C C:\dev\helworks\helengine commit -m "refactor: route editor asset references through sanctioned factories"
```

### Task 3: Fail Unsupported References During Save, Not Packaging

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\SceneAssetReferenceValidationService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\FontAssetScenePersistenceSupport.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\AutomaticScriptComponentPersistenceDescriptor.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\serialization\scene\SceneSaveService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\serialization\scene\SceneSaveServiceTests.cs`

- [ ] **Step 1: Write the failing save-time validation regression**

```csharp
[Fact]
public void Save_WhenTextComponentUsesUnsupportedGeneratedFontReference_ThrowsBeforePackaging() {
    ComponentPersistenceRegistry registry = new ComponentPersistenceRegistry();
    SceneSaveService saveService = new SceneSaveService(TempProjectRootPath, registry);
    string scenePath = Path.Combine(TempProjectRootPath, "assets", "Scenes", "UnsupportedGeneratedFont.helen");

    EditorEntity root = CreateUserEntity("Root", float3.Zero, float3.One, float4.Identity);
    TextComponent textComponent = new TextComponent {
        Font = CreateFont("Body"),
        Text = "Tagged",
        Size = new int2(96, 24)
    };
    root.AddComponent(textComponent);
    GetSaveComponent(root).SetAssetReference(
        textComponent,
        nameof(TextComponent.Font),
        CreateUnsupportedGeneratedFontReference());

    InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() => saveService.Save(scenePath));
    Assert.Contains(nameof(TextComponent.Font), exception.Message);
    Assert.Contains("Unsupported generated", exception.Message);
}

static SceneAssetReference CreateUnsupportedGeneratedFontReference() {
    using MemoryStream stream = new MemoryStream();
    using (EngineBinaryWriter writer = EngineBinaryWriter.Create(stream, EngineBinaryEndianness.LittleEndian)) {
        writer.WriteByte(1);
        writer.WriteInt32((int)SceneAssetReferenceSourceKind.Generated);
        writer.WriteString("generated/editor/fonts/ds-debug.hefont");
        writer.WriteString("editor");
        writer.WriteString("ds-debug-font");
    }

    stream.Position = 0;
    using EngineBinaryReader reader = EngineBinaryReader.Create(stream, EngineBinaryEndianness.LittleEndian);
    return FontAssetScenePersistenceSupport.ReadOptionalReference(reader);
}
```

- [ ] **Step 2: Run the save-service regression to verify it fails for the right reason**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~Save_WhenTextComponentUsesUnsupportedGeneratedFontReference_ThrowsBeforePackaging" -v minimal`

Expected: FAIL because scene save still accepts the unsupported generated font reference and only later build/packaging code rejects it.

- [ ] **Step 3: Implement the early validation service**

```csharp
namespace helengine.editor {
    /// <summary>
    /// Validates that scene asset references are supported for the owning editor save boundary.
    /// </summary>
    public sealed class SceneAssetReferenceValidationService {
        /// <summary>
        /// Validates one font reference before it is persisted.
        /// </summary>
        /// <param name="ownerName">Owning component/member description.</param>
        /// <param name="reference">Reference to validate.</param>
        public void ValidateFontReference(string ownerName, SceneAssetReference reference) {
            if (string.IsNullOrWhiteSpace(ownerName)) {
                throw new ArgumentException("Owner name must be provided.", nameof(ownerName));
            }
            if (reference == null) {
                throw new InvalidOperationException($"{ownerName} requires a font reference.");
            }
            if (reference.SourceKind == SceneAssetReferenceSourceKind.FileSystem) {
                if (string.IsNullOrWhiteSpace(reference.RelativePath)) {
                    throw new InvalidOperationException($"{ownerName} file-backed font reference must include a relative path.");
                }

                return;
            }
            if (reference.SourceKind == SceneAssetReferenceSourceKind.Generated &&
                string.Equals(reference.ProviderId, "editor", StringComparison.Ordinal) &&
                string.Equals(reference.AssetId, "ui-font", StringComparison.Ordinal)) {
                return;
            }

            throw new InvalidOperationException($"{ownerName} uses unsupported generated font reference '{reference.ProviderId}:{reference.AssetId}'.");
        }
    }
}
```

- [ ] **Step 4: Call validation from font persistence and tagged component save paths**

```csharp
readonly SceneAssetReferenceValidationService SceneAssetReferenceValidationService = new SceneAssetReferenceValidationService();

internal static SceneAssetReference ResolveFontReference(string componentName, FontAsset font, EntityComponentSaveState saveState) {
    SceneAssetReference reference = saveState != null && saveState.TryGetAssetReference(FontReferenceName, out SceneAssetReference storedReference)
        ? storedReference
        : TryResolveEditorCoreFont(font, out SceneAssetReference editorFontReference)
            ? editorFontReference
            : throw new InvalidOperationException($"{componentName} Font is assigned but does not have a stored scene asset reference.");

    new SceneAssetReferenceValidationService().ValidateFontReference($"{componentName}.{FontReferenceName}", reference);
    return reference;
}

if (member.ValueType == typeof(FontAsset)) {
    new SceneAssetReferenceValidationService().ValidateFontReference($"{component.GetType().Name}.{member.Name}", reference);
}
```

- [ ] **Step 5: Run the save-time regression and the broader scene-save suite to verify they pass**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~SceneSaveServiceTests" -v minimal`

Expected: PASS

- [ ] **Step 6: Commit the early-validation slice**

```bash
rtk git -C C:\dev\helworks\helengine add engine/helengine.editor/serialization/scene/SceneAssetReferenceValidationService.cs engine/helengine.editor/serialization/scene/FontAssetScenePersistenceSupport.cs engine/helengine.editor/serialization/scene/AutomaticScriptComponentPersistenceDescriptor.cs engine/helengine.editor/serialization/scene/SceneSaveService.cs engine/helengine.editor.tests/serialization/scene/SceneSaveServiceTests.cs
rtk git -C C:\dev\helworks\helengine commit -m "refactor: validate scene asset references during save"
```

### Task 4: Migrate Remaining Call Sites And Rebuild The City DS Project

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\DemoSceneInstructionOverlayFactory.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\ComponentPropertiesViewGeneratedAssetTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\EditorSessionGeneratedAssetTests.cs`

- [ ] **Step 1: Write or tighten the project-facing regression coverage**

```csharp
[Fact]
public void CityShowcaseSources_DoNotReferenceRemovedNintendoDsGeneratedFont() {
    string[] sourcePaths = new[] {
        @"C:\dev\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs",
        @"C:\dev\helprojs\city\assets\codebase\rendering.tools\DemoSceneInstructionOverlayFactory.cs",
        @"C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs"
    };

    foreach (string sourcePath in sourcePaths) {
        string source = File.ReadAllText(sourcePath);
        Assert.DoesNotContain("ds-debug-font", source, StringComparison.Ordinal);
        Assert.DoesNotContain("generated/editor/fonts/ds-debug.hefont", source, StringComparison.Ordinal);
    }
}
```

- [ ] **Step 2: Run the project-facing regression to verify it fails**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CityShowcaseSources_DoNotReferenceRemovedNintendoDsGeneratedFont" -v minimal`

Expected: FAIL because the city source files still hardcode `editor:ds-debug-font`.

- [ ] **Step 3: Replace city-side raw font references with sanctioned file-backed font factories**

```csharp
SceneAssetReference bodyFontReference = SceneAssetReferenceFactory.CreateFileSystemFont(definition.BodyFontPath);
SceneAssetReference titleFontReference = SceneAssetReferenceFactory.CreateFileSystemFont(definition.TitleFontPath);

saveState.SetAssetReference(nameof(DebugComponent.Font), bodyFontReference);
saveState.SetAssetReference(nameof(TextComponent.Font), bodyFontReference);
saveState.SetAssetReference(nameof(TextComponent.Font), titleFontReference);
```

- [ ] **Step 4: Update remaining editor tests that still construct raw references directly**

```csharp
SceneAssetReference modelReference = global::helengine.EngineSceneAssetReferenceFactory.CreateCubeModel();
SceneAssetReference materialReference = global::helengine.SceneAssetReferenceFactory.CreateFileSystemMaterial("Materials/Default.hasset");
SceneAssetReference fontReference = EditorSceneAssetReferenceFactory.CreateEditorUiFont();
```

- [ ] **Step 5: Run the targeted test set and the DS build verification**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~ComponentPropertiesViewGeneratedAssetTests|FullyQualifiedName~EditorSessionGeneratedAssetTests|FullyQualifiedName~SceneAssetReferenceFactoryTests|FullyQualifiedName~EditorSceneAssetReferenceResolverTests|FullyQualifiedName~SceneSaveServiceTests|FullyQualifiedName~SceneComponentPackagingTransformServiceTests" -v minimal`

Expected: PASS

Run: `rtk powershell -NoProfile -ExecutionPolicy Bypass -File C:\dev\helworks\helengine\artifacts\build-platform.ps1 -Project C:\dev\helprojs\city -Platform ds -Output C:\dev\helprojs\city\output\ds`

Expected: `Build completed for platform 'ds': C:\dev\helprojs\city\output\ds`

- [ ] **Step 6: Commit the remaining engine-side cleanup**

```bash
rtk git -C C:\dev\helworks\helengine add engine/helengine.editor.tests/ComponentPropertiesViewGeneratedAssetTests.cs engine/helengine.editor.tests/EditorSessionGeneratedAssetTests.cs
rtk git -C C:\dev\helworks\helengine commit -m "test: migrate generated asset reference call sites"
```
