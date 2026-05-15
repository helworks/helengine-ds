# DS Cooked Platform Material Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make Nintendo DS a real `CookedPlatformOwned` material target by cooking `PlatformMaterialAsset` directly and stopping DX11 shader package export for DS.

**Architecture:** The DS builder will expose a minimal fixed-pipeline material schema, opt into the generic cooked-platform-owned runtime contract, and emit serialized `PlatformMaterialAsset` with no shader references. The shared editor packager will become contract-driven for generated standard materials so DS no longer receives forced `ForwardStandardShader.dx11.hasset` output.

**Tech Stack:** C#, xUnit, `helengine.ds.builder`, `helengine.editor`, `helengine.core`, `helengine.files`

---

### Task 1: Add DS Builder Tests For Cooked Platform-Owned Materials

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsPlatformAssetBuilderTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsPlatformAssetBuilderTests.cs`

- [ ] **Step 1: Write the failing DS definition test for cooked-platform-owned material metadata**

Add a test that asserts the DS builder definition exposes cooked-platform-owned runtime material resolution and at least one material schema.

```csharp
[Fact]
public void Definition_exposes_cooked_platform_owned_material_contract_and_material_schemas() {
    NintendoDsPlatformAssetBuilder builder = new();

    Assert.Equal(RuntimeMaterialResolutionMode.CookedPlatformOwned, builder.Definition.RuntimeGenerationContract.MaterialResolutionMode);
    Assert.NotEmpty(builder.Definition.MaterialSchemas);
    Assert.Contains(builder.Definition.MaterialSchemas, schema => schema.SchemaId == "ds-standard-textured");
}
```

- [ ] **Step 2: Write the failing DS cook test for `PlatformMaterialAsset` output**

Add a test that builds a minimal material cook request and asserts the builder returns a serialized `PlatformMaterialAsset` with no shader refs.

```csharp
[Fact]
public void CookMaterial_whenUsingDsStandardSchema_returnsPlatformMaterialAsset_withoutShaderReferences() {
    NintendoDsPlatformAssetBuilder builder = new();

    PlatformMaterialCookResult result = builder.CookMaterial(new PlatformMaterialCookRequest(
        "Materials/rendering/test/Cube00",
        "Materials/rendering/test/Cube00.helmat",
        "ds",
        "ds-default",
        "ds-main-2d",
        "ds-standard-textured",
        new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase) {
            ["texture-relative-path"] = "cooked/imported/test-texture",
            ["double-sided"] = "false",
            ["vertex-color-mode"] = "multiply",
            ["base-color"] = "#FFFFFFFF",
            ["lighting-mode"] = "lit"
        }));

    Assert.Empty(result.ReferencedShaderAssetIds);

    PlatformMaterialAsset cookedAsset = Assert.IsType<PlatformMaterialAsset>(AssetSerializer.DeserializeFromBytes(result.CookedMaterialBytes));
    Assert.Equal("ds-main-2d", cookedAsset.RendererFamilyId);
}
```

- [ ] **Step 3: Run the focused DS builder tests to verify they fail**

Run: `rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsPlatformAssetBuilderTests" --no-restore`

Expected: FAIL because DS still has no material schemas and `CookMaterial(...)` still throws `NotSupportedException`.

- [ ] **Step 4: Commit the red DS builder tests**

```bash
git -C C:\dev\helworks\helengine-ds add builder.tests/NintendoDsPlatformAssetBuilderTests.cs
git -C C:\dev\helworks\helengine-ds commit -m "test: require DS cooked platform material contract"
```

### Task 2: Implement DS Material Schemas And `CookMaterial(...)`

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsPlatformAssetBuilder.cs`
- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsPlatformDefinitionFactory.cs`
- Create: `C:\dev\helworks\helengine-ds\builder\NintendoDsMaterialSchemaIds.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\platform\PlatformMaterialAsset.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\EditorAssetBinarySerializer.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.files\assets\EditorAssetBinarySerializer.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsPlatformAssetBuilderTests.cs`

- [ ] **Step 1: Add DS material schema constants**

Create a focused helper for schema ids and field ids.

```csharp
namespace helengine.ds.builder;

/// <summary>
/// Stores the schema and field identifiers used by Nintendo DS fixed-pipeline material cooking.
/// </summary>
public static class NintendoDsMaterialSchemaIds {
    /// <summary>
    /// Standard textured DS schema id.
    /// </summary>
    public const string StandardTexturedSchemaId = "ds-standard-textured";

    /// <summary>
    /// Texture-relative-path field id.
    /// </summary>
    public const string TextureRelativePathFieldId = "texture-relative-path";

    /// <summary>
    /// Double-sided field id.
    /// </summary>
    public const string DoubleSidedFieldId = "double-sided";

    /// <summary>
    /// Vertex-color-mode field id.
    /// </summary>
    public const string VertexColorModeFieldId = "vertex-color-mode";

    /// <summary>
    /// Base-color field id.
    /// </summary>
    public const string BaseColorFieldId = "base-color";

    /// <summary>
    /// Lighting-mode field id.
    /// </summary>
    public const string LightingModeFieldId = "lighting-mode";
}
```

- [ ] **Step 2: Add the DS material schema and cooked-platform-owned runtime contract to the platform definition**

Update `NintendoDsPlatformDefinitionFactory.Create()` to include one material schema and a non-default runtime contract.

```csharp
new PlatformMaterialSchemaDefinition(
    NintendoDsMaterialSchemaIds.StandardTexturedSchemaId,
    "DS Standard Textured",
    "Nintendo DS fixed-pipeline textured material.",
    ["ds-main-2d"],
    [
        new PlatformMaterialFieldDefinition(NintendoDsMaterialSchemaIds.TextureRelativePathFieldId, "Texture", PlatformMaterialFieldKind.Text, string.Empty, false, []),
        new PlatformMaterialFieldDefinition(NintendoDsMaterialSchemaIds.DoubleSidedFieldId, "Double Sided", PlatformMaterialFieldKind.Boolean, "false", true, []),
        new PlatformMaterialFieldDefinition(NintendoDsMaterialSchemaIds.VertexColorModeFieldId, "Vertex Color", PlatformMaterialFieldKind.Choice, "multiply", true, ["multiply", "ignore"]),
        new PlatformMaterialFieldDefinition(NintendoDsMaterialSchemaIds.BaseColorFieldId, "Base Color", PlatformMaterialFieldKind.Color, "#FFFFFFFF", true, []),
        new PlatformMaterialFieldDefinition(NintendoDsMaterialSchemaIds.LightingModeFieldId, "Lighting", PlatformMaterialFieldKind.Choice, "lit", true, ["lit", "unlit"])
    ])
```

```csharp
new RuntimeGenerationContract(
    RuntimeMaterialResolutionMode.CookedPlatformOwned,
    true,
    PackagedPathPolicy.ContentRelativeOnly)
```

- [ ] **Step 3: Grow `PlatformMaterialAsset` with the minimal generic payload DS needs**

Add only generic fields.

```csharp
/// <summary>
/// Gets or sets the cooked runtime-relative texture path consumed by the active platform renderer.
/// </summary>
public string TextureRelativePath;

/// <summary>
/// Gets or sets whether the cooked material should render both winding directions.
/// </summary>
public bool DoubleSided;

/// <summary>
/// Gets or sets whether vertex color should modulate the final material color.
/// </summary>
public bool UseVertexColor;

/// <summary>
/// Gets or sets whether the material should use lighting during runtime shading.
/// </summary>
public bool Lit;

/// <summary>
/// Gets or sets the cooked base-color red channel.
/// </summary>
public byte BaseColorR;

/// <summary>
/// Gets or sets the cooked base-color green channel.
/// </summary>
public byte BaseColorG;

/// <summary>
/// Gets or sets the cooked base-color blue channel.
/// </summary>
public byte BaseColorB;

/// <summary>
/// Gets or sets the cooked base-color alpha channel.
/// </summary>
public byte BaseColorA;
```

- [ ] **Step 4: Update both serializers for the expanded `PlatformMaterialAsset` payload**

Extend the new read/write methods in `helengine.core` and `helengine.files`.

```csharp
writer.WriteString(asset.RendererFamilyId);
writer.WriteString(asset.TextureRelativePath);
writer.WriteByte(asset.DoubleSided ? (byte)1 : (byte)0);
writer.WriteByte(asset.UseVertexColor ? (byte)1 : (byte)0);
writer.WriteByte(asset.Lit ? (byte)1 : (byte)0);
writer.WriteByte(asset.BaseColorR);
writer.WriteByte(asset.BaseColorG);
writer.WriteByte(asset.BaseColorB);
writer.WriteByte(asset.BaseColorA);
```

```csharp
asset.RendererFamilyId = reader.ReadString();
asset.TextureRelativePath = reader.ReadString();
asset.DoubleSided = reader.ReadByte() != 0;
asset.UseVertexColor = reader.ReadByte() != 0;
asset.Lit = reader.ReadByte() != 0;
asset.BaseColorR = reader.ReadByte();
asset.BaseColorG = reader.ReadByte();
asset.BaseColorB = reader.ReadByte();
asset.BaseColorA = reader.ReadByte();
```

- [ ] **Step 5: Implement `NintendoDsPlatformAssetBuilder.CookMaterial(...)` minimally**

Replace the exception with a narrow builder-owned cook path.

```csharp
public PlatformMaterialCookResult CookMaterial(PlatformMaterialCookRequest request) {
    if (request == null) {
        throw new ArgumentNullException(nameof(request));
    } else if (!string.Equals(request.SchemaId, NintendoDsMaterialSchemaIds.StandardTexturedSchemaId, StringComparison.OrdinalIgnoreCase)) {
        throw new InvalidOperationException($"Nintendo DS does not support material schema '{request.SchemaId}'.");
    }

    PlatformMaterialAsset cookedAsset = new PlatformMaterialAsset {
        Id = request.MaterialAssetId,
        RendererFamilyId = request.SelectedGraphicsProfileId,
        TextureRelativePath = ResolveRequiredString(request.FieldValues, NintendoDsMaterialSchemaIds.TextureRelativePathFieldId),
        DoubleSided = ResolveBoolean(request.FieldValues, NintendoDsMaterialSchemaIds.DoubleSidedFieldId),
        UseVertexColor = ResolveVertexColorMode(request.FieldValues),
        Lit = ResolveLightingMode(request.FieldValues),
        BaseColorR = ResolveBaseColor(request.FieldValues).R,
        BaseColorG = ResolveBaseColor(request.FieldValues).G,
        BaseColorB = ResolveBaseColor(request.FieldValues).B,
        BaseColorA = ResolveBaseColor(request.FieldValues).A
    };

    return new PlatformMaterialCookResult(
        AssetSerializer.SerializeToBytes(cookedAsset),
        Array.Empty<string>());
}
```

- [ ] **Step 6: Add small private helpers for required string, booleans, vertex color mode, lighting mode, and color parsing**

Use explicit validation and throw on invalid values instead of defaulting silently.

```csharp
static bool ResolveBoolean(IReadOnlyDictionary<string, string> fieldValues, string fieldId) {
    string value = ResolveRequiredString(fieldValues, fieldId);
    if (string.Equals(value, "true", StringComparison.OrdinalIgnoreCase)) {
        return true;
    } else if (string.Equals(value, "false", StringComparison.OrdinalIgnoreCase)) {
        return false;
    }

    throw new InvalidOperationException($"Nintendo DS material field '{fieldId}' must be 'true' or 'false'.");
}
```

- [ ] **Step 7: Run the focused DS builder tests to verify they pass**

Run: `rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsPlatformAssetBuilderTests" --no-restore`

Expected: PASS.

- [ ] **Step 8: Commit the DS builder cooking implementation**

```bash
git -C C:\dev\helworks\helengine-ds add builder/NintendoDsPlatformAssetBuilder.cs builder/NintendoDsPlatformDefinitionFactory.cs builder/NintendoDsMaterialSchemaIds.cs builder.tests/NintendoDsPlatformAssetBuilderTests.cs
git -C C:\dev\helworks\helengine-ds commit -m "feat: cook DS platform materials"
git -C C:\dev\helworks\helengine add engine/helengine.core/assets/raw/platform/PlatformMaterialAsset.cs engine/helengine.core/assets/EditorAssetBinarySerializer.cs engine/helengine.files/assets/EditorAssetBinarySerializer.cs
git -C C:\dev\helworks\helengine commit -m "feat: extend generic platform material payload"
```

### Task 3: Add Editor Packager Tests For DS Generated Standard Materials

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorWindowsBuildScenePackagerTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorWindowsBuildScenePackagerTests.cs`

- [ ] **Step 1: Write the failing DS generated standard material packager test**

Mirror the PS2 test but expect `PlatformMaterialAsset`.

```csharp
[Fact]
public void Package_WhenSceneReferencesGeneratedStandardMaterial_CooksDsPlatformMaterialAsset() {
    string sceneId = "Scenes/GeneratedStandardMaterialScene.helen";
    WriteSceneAsset(sceneId, CreateGeneratedStandardMaterialReference());

    RecordingMaterialBuilder materialBuilder = new RecordingMaterialBuilder(
        CreateDsMaterialBuilderDefinition(),
        request => new PlatformMaterialCookResult(
            AssetSerializer.SerializeToBytes(new PlatformMaterialAsset {
                RendererFamilyId = "ds-main-2d",
                TextureRelativePath = string.Empty,
                DoubleSided = false,
                UseVertexColor = true,
                Lit = true,
                BaseColorR = 255,
                BaseColorG = 255,
                BaseColorB = 255,
                BaseColorA = 255
            }),
            Array.Empty<string>()));

    EditorPlatformBuildScenePackager packager = new EditorPlatformBuildScenePackager(
        ProjectRootPath,
        Array.Empty<IAssetImporterRegistration>(),
        "ds",
        materialBuilder,
        "ds-default",
        "ds-main-2d");

    packager.Package(new[] { sceneId }, BuildRootPath);

    using FileStream stream = File.OpenRead(Path.Combine(BuildRootPath, "cooked", "engine", "materials", "standard.hasset"));
    PlatformMaterialAsset cookedMaterial = Assert.IsType<PlatformMaterialAsset>(AssetSerializer.Deserialize(stream));

    Assert.Equal("ds-main-2d", cookedMaterial.RendererFamilyId);
    Assert.False(File.Exists(Path.Combine(BuildRootPath, "cooked", "shaders", "ForwardStandardShader.dx11.hasset")));
}
```

- [ ] **Step 2: Write the failing DS imported texture propagation packager test**

```csharp
[Fact]
public void Package_WhenDsBuilderCooksMaterialWithImportedDiffuseTexture_PopulatesTextureRelativePath() {
    string sceneId = "Scenes/TexturedMaterialScene.helen";
    string materialRelativePath = "Materials/rendering/textured_cube_grid/Cube00.hasset";
    string textureAssetId = "ff8a0f1fafe1f1c4989f73f39db8b800512e09e26439b011cb7afb0fed44dd5a";

    WriteCachedTextureAsset(textureAssetId);
    WriteCityStyleStandardMaterialAsset(materialRelativePath, textureAssetId);
    WriteSceneAsset(sceneId, materialRelativePath);

    RecordingMaterialBuilder materialBuilder = new RecordingMaterialBuilder(
        CreateDsMaterialBuilderDefinition(),
        request => new PlatformMaterialCookResult(
            AssetSerializer.SerializeToBytes(new PlatformMaterialAsset {
                RendererFamilyId = "ds-main-2d",
                TextureRelativePath = request.FieldValues["texture-relative-path"]
            }),
            Array.Empty<string>()));

    EditorPlatformBuildScenePackager packager = new EditorPlatformBuildScenePackager(
        ProjectRootPath,
        Array.Empty<IAssetImporterRegistration>(),
        "ds",
        materialBuilder,
        "ds-default",
        "ds-main-2d");

    packager.Package(new[] { sceneId }, BuildRootPath);

    Assert.Equal("cooked/imported/" + textureAssetId, materialBuilder.LastMaterialCookRequest.FieldValues["texture-relative-path"]);
}
```

- [ ] **Step 3: Run the focused editor packager tests to verify they fail**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorWindowsBuildScenePackagerTests" --no-restore`

Expected: FAIL because the packager still writes DX11 shader payloads for DS and still seeds standard-schema defaults from `ShaderCompileTarget.DirectX11`.

- [ ] **Step 4: Commit the red editor packager tests**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.editor.tests/managers/project/EditorWindowsBuildScenePackagerTests.cs
git -C C:\dev\helworks\helengine commit -m "test: require DS cooked material packaging"
```

### Task 4: Make The Editor Packager Contract-Driven For DS Cooked Materials

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorWindowsBuildScenePackager.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorWindowsBuildScenePackagerTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorWindowsBuildScenePackagerTests.cs`

- [ ] **Step 1: Add a helper that checks whether the current target uses cooked-platform-owned material resolution**

Use the active builder definition contract instead of another `TargetPlatformId == "ds"` check.

```csharp
bool UsesCookedPlatformOwnedMaterialResolution() {
    if (MaterialBuilder == null) {
        return false;
    }

    PlatformDefinition definition = MaterialBuilder.Definition;
    return definition != null &&
        definition.RuntimeGenerationContract != null &&
        definition.RuntimeGenerationContract.MaterialResolutionMode == RuntimeMaterialResolutionMode.CookedPlatformOwned;
}
```

- [ ] **Step 2: Stop forcing DX11 standard shader defaults for cooked-platform-owned builders**

Change `BuildMaterialCookFieldValues(...)`.

```csharp
bool usesCookedPlatformOwnedMaterialResolution = UsesCookedPlatformOwnedMaterialResolution();
if (IsStandardShaderSchema(materialSettings.SchemaId) && !useCustomShader) {
    fieldValues[VariantFieldId] = StandardShaderVariantName;
    if (!usesCookedPlatformOwnedMaterialResolution) {
        ShaderAsset shaderAsset = EditorBuiltInShaderAssetLibrary.LoadShaderAsset(ShaderCompileTarget.DirectX11, StandardShaderFileName);
        fieldValues[ShaderAssetIdFieldId] = shaderAsset.Id;
        fieldValues[VertexProgramFieldId] = StandardVertexProgramName;
        fieldValues[PixelProgramFieldId] = StandardPixelProgramName;
    }
} else {
    fieldValues[VariantFieldId] = MeshVariantName;
}
```

- [ ] **Step 3: Stop writing generated standard shader assets for cooked-platform-owned builders**

Change `ShouldWriteGeneratedStandardShaderAsset()`.

```csharp
bool ShouldWriteGeneratedStandardShaderAsset() {
    return !UsesCookedPlatformOwnedMaterialResolution();
}
```

- [ ] **Step 4: Update any error messages that still imply PS2-only generated cooked materials**

For example:

```csharp
throw new InvalidOperationException("Generated standard materials for cooked-platform-owned builders require a material builder.");
```

- [ ] **Step 5: Run the focused editor packager tests to verify they pass**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorWindowsBuildScenePackagerTests" --no-restore`

Expected: PASS for the DS-focused cases and no regressions in the existing PS2-focused cooked-material cases.

- [ ] **Step 6: Commit the packager contract-driven change**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.editor/managers/project/EditorWindowsBuildScenePackager.cs engine/helengine.editor.tests/managers/project/EditorWindowsBuildScenePackagerTests.cs
git -C C:\dev\helworks\helengine commit -m "refactor: make cooked material packaging contract driven"
```

### Task 5: Final Focused Verification

**Files:**
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsPlatformAssetBuilderTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\managers\project\EditorWindowsBuildScenePackagerTests.cs`

- [ ] **Step 1: Run the focused DS builder verification**

Run: `rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter "FullyQualifiedName~NintendoDsPlatformAssetBuilderTests" --no-restore`

Expected: PASS.

- [ ] **Step 2: Run the focused shared editor verification**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~EditorWindowsBuildScenePackagerTests" --no-restore`

Expected: PASS.

- [ ] **Step 3: Search for the DS shader export symptom in the touched packager path**

Run: `rtk rg -n "ForwardStandardShader\\.dx11\\.hasset|ShaderCompileTarget\\.DirectX11" C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorWindowsBuildScenePackager.cs`

Expected: any remaining matches should only be in the raw-shader-backed path, not the cooked-platform-owned path.

- [ ] **Step 4: Commit any final wording or cleanup changes**

```bash
git -C C:\dev\helworks\helengine-ds add builder.tests/NintendoDsPlatformAssetBuilderTests.cs builder/NintendoDsPlatformAssetBuilder.cs builder/NintendoDsPlatformDefinitionFactory.cs builder/NintendoDsMaterialSchemaIds.cs docs/superpowers/specs/2026-05-15-ds-cooked-platform-material-design.md docs/superpowers/plans/2026-05-15-ds-cooked-platform-material.md
git -C C:\dev\helworks\helengine add engine/helengine.core/assets/raw/platform/PlatformMaterialAsset.cs engine/helengine.core/assets/EditorAssetBinarySerializer.cs engine/helengine.files/assets/EditorAssetBinarySerializer.cs engine/helengine.editor/managers/project/EditorWindowsBuildScenePackager.cs engine/helengine.editor.tests/managers/project/EditorWindowsBuildScenePackagerTests.cs
git -C C:\dev\helworks\helengine-ds commit -m "feat: add DS cooked platform material plan artifacts"
```

