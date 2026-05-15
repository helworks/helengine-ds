# Generic Cooked Platform Material Contract Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Remove PS2-specific cooked-material references from the shared `helengine` runtime by introducing a generic `PlatformMaterialAsset` seam for `CookedPlatformOwned` material resolution.

**Architecture:** The shared engine keeps the existing `RuntimeMaterialResolutionMode.CookedPlatformOwned` switch, but the runtime content manager, runtime scene resolver, and render manager contract will route through a new generic `PlatformMaterialAsset` stub instead of `Ps2MaterialAsset`. This pass only establishes the neutral shared seam and serializer support; DS and other fixed-pipeline builders can adopt it later.

**Tech Stack:** C#, xUnit, `helengine.core`, `helengine.files`, `helengine.editor.tests`

---

### Task 1: Update Shared Engine Source Tests To Demand The Generic Seam

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\RuntimeSceneAssetReferenceResolverSourceTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\RuntimeContentManagerConfigurationSourceTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\RuntimeSceneAssetReferenceResolverSourceTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\RuntimeContentManagerConfigurationSourceTests.cs`

- [ ] **Step 1: Write the failing source-test expectations**

Replace the PS2-specific assertions with generic cooked-material assertions.

```csharp
Assert.Contains("#if HELENGINE_RUNTIME_MATERIAL_RESOLUTION_COOKED_PLATFORM_OWNED", source, StringComparison.Ordinal);
Assert.Contains("PlatformMaterialAsset materialAsset = AssetContentManager.Load<PlatformMaterialAsset>(fullPath, RuntimeContentProcessorIds.MaterialAsset);", source, StringComparison.Ordinal);
Assert.Contains("return Core.Instance.RenderManager3D.BuildMaterialFromCooked(materialAsset);", source, StringComparison.Ordinal);
```

```csharp
Assert.Contains("#if HELENGINE_RUNTIME_MATERIAL_RESOLUTION_COOKED_PLATFORM_OWNED", source, StringComparison.Ordinal);
Assert.Contains("new AssetContentProcessor<PlatformMaterialAsset>()", source, StringComparison.Ordinal);
```

- [ ] **Step 2: Run the focused tests to verify they fail for the expected reason**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~RuntimeSceneAssetReferenceResolverSourceTests|FullyQualifiedName~RuntimeContentManagerConfigurationSourceTests" --no-restore`

Expected: FAIL because the runtime source still references `Ps2MaterialAsset`.

- [ ] **Step 3: Commit the red test change**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.editor.tests/RuntimeSceneAssetReferenceResolverSourceTests.cs engine/helengine.editor.tests/RuntimeContentManagerConfigurationSourceTests.cs
git -C C:\dev\helworks\helengine commit -m "test: require generic cooked platform material seam"
```

### Task 2: Add The Generic `PlatformMaterialAsset` Stub And Serializer Support

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\platform\PlatformMaterialAsset.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\EditorAssetBinaryValueKind.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\EditorAssetBinarySerializer.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.files\assets\EditorAssetBinarySerializer.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\RuntimeContentManagerConfigurationSourceTests.cs`

- [ ] **Step 1: Create the minimal shared cooked material asset stub**

Add the new shared asset type with only the neutral fields needed for the generic seam.

```csharp
namespace helengine {
    /// <summary>
    /// Stores one generic platform-owned cooked material payload selected by a fixed-pipeline or otherwise platform-owned builder.
    /// </summary>
    public class PlatformMaterialAsset : Asset {
        /// <summary>
        /// Gets or sets the builder-owned renderer family identifier that should interpret this cooked payload.
        /// </summary>
        public string RendererFamilyId;
    }
}
```

- [ ] **Step 2: Register a new binary value kind for the generic cooked material asset**

Add a new enum member next to the existing cooked asset kinds.

```csharp
/// <summary>
/// The payload stores a <see cref="PlatformMaterialAsset"/>.
/// </summary>
PlatformMaterialAsset = 8,
```

- [ ] **Step 3: Teach the shared serializer to read and write `PlatformMaterialAsset`**

Add the type checks and read/write methods in `helengine.core/assets/EditorAssetBinarySerializer.cs`.

```csharp
} else if (asset is PlatformMaterialAsset) {
    return EditorAssetBinaryValueKind.PlatformMaterialAsset;
}
```

```csharp
} else if (asset is PlatformMaterialAsset platformMaterialAsset) {
    WritePlatformMaterialAsset(writer, platformMaterialAsset);
}
```

```csharp
case EditorAssetBinaryValueKind.PlatformMaterialAsset:
    return ReadPlatformMaterialAsset(reader, version);
```

```csharp
static void WritePlatformMaterialAsset(EngineBinaryWriter writer, PlatformMaterialAsset asset) {
    writer.WriteString(asset.Id);
    writer.WriteString(asset.RendererFamilyId);
}

static PlatformMaterialAsset ReadPlatformMaterialAsset(EngineBinaryReader reader, byte version) {
    PlatformMaterialAsset asset = new PlatformMaterialAsset();
    asset.Id = reader.ReadString();
    asset.RendererFamilyId = reader.ReadString();
    return asset;
}
```

- [ ] **Step 4: Mirror the same serializer support in `helengine.files`**

Add the same type checks and read/write methods in `engine/helengine.files/assets/EditorAssetBinarySerializer.cs`.

```csharp
} else if (asset is PlatformMaterialAsset) {
    return EditorAssetBinaryValueKind.PlatformMaterialAsset;
}
```

```csharp
} else if (asset is PlatformMaterialAsset platformMaterialAsset) {
    WritePlatformMaterialAsset(writer, platformMaterialAsset);
}
```

```csharp
case EditorAssetBinaryValueKind.PlatformMaterialAsset:
    return ReadPlatformMaterialAsset(reader, version);
```

- [ ] **Step 5: Run a focused build or tests to verify the new type compiles**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~RuntimeContentManagerConfigurationSourceTests" --no-restore`

Expected: still FAIL, but now only because the runtime content-manager source still references `Ps2MaterialAsset`.

- [ ] **Step 6: Commit the generic asset stub and serializer support**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.core/assets/raw/platform/PlatformMaterialAsset.cs engine/helengine.core/assets/EditorAssetBinaryValueKind.cs engine/helengine.core/assets/EditorAssetBinarySerializer.cs engine/helengine.files/assets/EditorAssetBinarySerializer.cs
git -C C:\dev\helworks\helengine commit -m "feat: add generic platform cooked material asset stub"
```

### Task 3: Switch Shared Runtime Material Resolution To `PlatformMaterialAsset`

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\content\RuntimeContentManagerConfiguration.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\scene\runtime\RuntimeSceneAssetReferenceResolver.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\managers\rendering\RenderManager3D.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\RuntimeSceneAssetReferenceResolverSourceTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\RuntimeContentManagerConfigurationSourceTests.cs`

- [ ] **Step 1: Update the cooked-platform-owned content processor registration**

Replace the PS2-specific processor registration with the generic asset type.

```csharp
RegisterProcessorIfMissing(
    contentManager,
    RuntimeContentProcessorIds.MaterialAsset,
#if HELENGINE_RUNTIME_MATERIAL_RESOLUTION_COOKED_PLATFORM_OWNED
    new AssetContentProcessor<PlatformMaterialAsset>(),
#else
    new AssetContentProcessor<MaterialAsset>(),
#endif
    new[] { MaterialAssetExtension });
```

- [ ] **Step 2: Update the runtime scene resolver to load the generic cooked material asset**

Replace the PS2-specific cooked branch in `ResolveMaterial`.

```csharp
#if HELENGINE_RUNTIME_MATERIAL_RESOLUTION_COOKED_PLATFORM_OWNED
    PlatformMaterialAsset materialAsset = AssetContentManager.Load<PlatformMaterialAsset>(fullPath, RuntimeContentProcessorIds.MaterialAsset);
    return Core.Instance.RenderManager3D.BuildMaterialFromCooked(materialAsset);
#else
```

- [ ] **Step 3: Update the shared render manager contract to accept the generic stub**

Replace the PS2-specific method signature and exception message.

```csharp
#if HELENGINE_RUNTIME_MATERIAL_RESOLUTION_COOKED_PLATFORM_OWNED
    /// <summary>
    /// Builds a runtime material from one builder-owned cooked material payload.
    /// </summary>
    /// <param name="materialAsset">Builder-owned cooked material payload.</param>
    /// <returns>Runtime material instance.</returns>
    public virtual RuntimeMaterial BuildMaterialFromCooked(PlatformMaterialAsset materialAsset) {
        throw new NotSupportedException("This renderer does not support platform-owned cooked material creation.");
    }
#endif
```

- [ ] **Step 4: Run the focused source tests to verify they pass**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~RuntimeSceneAssetReferenceResolverSourceTests|FullyQualifiedName~RuntimeContentManagerConfigurationSourceTests" --no-restore`

Expected: PASS.

- [ ] **Step 5: Commit the shared runtime seam refactor**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.core/content/RuntimeContentManagerConfiguration.cs engine/helengine.core/scene/runtime/RuntimeSceneAssetReferenceResolver.cs engine/helengine.core/managers/rendering/RenderManager3D.cs
git -C C:\dev\helworks\helengine commit -m "refactor: use generic cooked platform material seam"
```

### Task 4: Verify Shared Engine Coverage And Check For Remaining Platform-Specific Leaks

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\RuntimeContentManagerConfigurationSourceTests.cs` if assertion text or comments still mention PS2
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\RuntimeSceneAssetReferenceResolverSourceTests.cs` if assertion text or comments still mention PS2
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\RuntimeSceneAssetReferenceResolverSourceTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\RuntimeContentManagerConfigurationSourceTests.cs`

- [ ] **Step 1: Search for leftover `Ps2MaterialAsset` references in the generic cooked-material path**

Run: `rtk rg -n "Ps2MaterialAsset|BuildMaterialFromCooked\\(Ps2MaterialAsset|AssetContentProcessor<Ps2MaterialAsset>" C:\dev\helworks\helengine\engine`

Expected: no matches in the shared generic runtime seam files.

- [ ] **Step 2: Clean up any stale PS2-specific test names or comments in the touched source-test files**

Use wording like:

```csharp
/// <summary>
/// Verifies the runtime content manager source keeps the cooked-platform-owned material processor registration aligned with the generic runtime material loader.
/// </summary>
```

```csharp
[Fact]
public void RuntimeContentManagerConfiguration_source_registers_generic_material_processor_for_cooked_platform_owned_material_contract() {
```

- [ ] **Step 3: Run the focused shared-engine tests again as a final verification**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~RuntimeSceneAssetReferenceResolverSourceTests|FullyQualifiedName~RuntimeContentManagerConfigurationSourceTests" --no-restore`

Expected: PASS.

- [ ] **Step 4: Commit the verification cleanup**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.editor.tests/RuntimeSceneAssetReferenceResolverSourceTests.cs engine/helengine.editor.tests/RuntimeContentManagerConfigurationSourceTests.cs
git -C C:\dev\helworks\helengine commit -m "test: remove platform-specific cooked material wording"
```

