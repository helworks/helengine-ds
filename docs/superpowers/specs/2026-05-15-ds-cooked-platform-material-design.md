# DS Cooked Platform Material Design

## Problem

Nintendo DS currently cannot participate in the generic `CookedPlatformOwned` material path end to end.

Current blockers:

- `NintendoDsPlatformAssetBuilder.CookMaterial(...)` still throws `NotSupportedException`.
- The DS platform definition does not expose material schemas or opt into the cooked-platform-owned runtime contract.
- The shared editor packager still forces standard-schema material cook defaults through `ForwardStandardShader` with `ShaderCompileTarget.DirectX11`.
- The shared editor packager still writes `cooked/shaders/ForwardStandardShader.dx11.hasset` for DS because the generated-standard-material gate only excludes PS2.

That leaves DS depending on shader-backed packaging behavior even though the DS renderer is fixed-pipeline and the runtime material seam is now generic.

## Goals

- Make Nintendo DS a real `CookedPlatformOwned` material target.
- Implement `NintendoDsPlatformAssetBuilder.CookMaterial(...)` so it emits serialized `PlatformMaterialAsset`.
- Return no referenced shader asset ids from DS material cooking.
- Stop exporting `ForwardStandardShader.dx11.hasset` for DS in the editor packager.
- Keep the DS material schema minimal for this pass.

## Non-Goals

- Final editor-facing fixed-pipeline authoring UX.
- A DS-specific cooked material asset type.
- Full builder-driven generated material resolver specialization.
- A broad redesign of standard material authoring.

## Recommended Approach

Use the new generic `CookedPlatformOwned` seam directly for DS now.

That means:

- DS platform definition opts into `RuntimeMaterialResolutionMode.CookedPlatformOwned`.
- DS builder exposes a minimal fixed-pipeline material schema.
- DS `CookMaterial(...)` lowers that schema into a serialized `PlatformMaterialAsset`.
- Editor packaging treats DS like other builder-owned cooked-platform targets for generated standard materials and does not emit DX11 shader payloads.

This keeps the contract generic and avoids another DS-specific runtime workaround.

## Alternatives Considered

### 1. Emit `PlatformMaterialAsset` directly now

Recommended.

Benefits:
- Uses the generic seam we just introduced.
- Removes DS shader-export dependency immediately.
- Keeps later DS runtime/material evolution on the intended contract.

Trade-off:
- Requires a small DS material schema now.

### 2. Keep writing `MaterialAsset` and only suppress shader export

Rejected.

Benefits:
- Smaller first patch.

Trade-off:
- Leaves DS outside the cooked-platform-owned contract.
- Forces another transition later.

### 3. Reuse PS2 schema semantics for DS temporarily

Rejected.

Benefits:
- Faster short-term implementation.

Trade-off:
- Leaks the wrong platform semantics into DS authoring.
- Makes future fixed-pipeline editing harder to untangle.

## Design

### DS Platform Definition

The DS builder definition should grow the minimum material metadata needed to participate in builder-owned cooking.

Required changes:

- set `RuntimeGenerationContract.MaterialResolutionMode` to `CookedPlatformOwned`
- expose at least one DS material schema for standard textured materials

The first DS schema should stay intentionally small and only represent behavior the DS renderer already understands or can trivially preserve.

Minimum fields:

- `texture-relative-path`
- `double-sided`
- `vertex-color-mode`
- `base-color`
- optional simple `lighting-mode` if DS runtime material construction needs an explicit lit/unlit choice

### DS `CookMaterial(...)`

`NintendoDsPlatformAssetBuilder.CookMaterial(...)` should:

- validate the incoming request
- resolve the minimal supported field set
- build one `PlatformMaterialAsset`
- serialize it with `AssetSerializer.SerializeToBytes(...)`
- return `new PlatformMaterialCookResult(bytes, Array.Empty<string>())`

The cooked DS material must not report any shader dependencies.

### `PlatformMaterialAsset` Growth

The generic `PlatformMaterialAsset` stub may need a small expansion in the shared engine for DS to preserve today’s runtime behavior.

Allowed additions in this pass:

- generic base color fields
- generic texture-relative-path field
- generic double-sided field
- generic coarse lighting or vertex-color flags

Disallowed additions:

- DS-specific field names
- hardware-register-style DS fields
- platform-name-specific serializer branches

If a field is needed for DS, it must still be named and described generically.

### Editor Packager Changes

The editor packager should stop treating DS as shader-backed for generated standard materials.

Required changes:

- the generated standard shader write gate must stop writing `ForwardStandardShader.dx11.hasset` for DS
- standard-schema cook field defaults must not force `ShaderCompileTarget.DirectX11` when the target builder resolves materials through `CookedPlatformOwned`

The simplest acceptable implementation in this pass is contract-driven:

- if the target platform definition uses `CookedPlatformOwned`, do not emit DX11 standard shader payloads
- do not seed cook field defaults from DX11 shader metadata for those targets

That is better than hardcoding another `TargetPlatformId == "ds"` branch.

### Generated Standard Material Path

Generated standard materials referenced by scenes must flow through the DS material builder path, not raw desktop material serialization.

Expected outcome:

- DS generated standard material writes cooked `PlatformMaterialAsset`
- no `cooked/shaders/ForwardStandardShader.dx11.hasset`

### Imported Diffuse Texture Handling

DS cook requests should continue to receive imported textures as cooked runtime-relative paths under `cooked/imported/...`.

That existing packager behavior should remain intact for DS builder-backed materials.

## Files Expected To Change

### In `helengine-ds`

- `builder/NintendoDsPlatformAssetBuilder.cs`
- `builder/NintendoDsPlatformDefinitionFactory.cs`
- new DS material schema helper file if the builder metadata would become noisy inline
- `builder.tests/NintendoDsPlatformAssetBuilderTests.cs`

### In `helengine`

- `engine/helengine.core/assets/raw/platform/PlatformMaterialAsset.cs`
- serializer files if `PlatformMaterialAsset` gains generic fields
- `engine/helengine.editor/managers/project/EditorWindowsBuildScenePackager.cs`
- `engine/helengine.editor.tests/managers/project/EditorWindowsBuildScenePackagerTests.cs`

## Testing Strategy

Follow TDD for both repos.

### DS Builder Tests

Add focused tests for:

- `CookMaterial(...)` returns serialized `PlatformMaterialAsset`
- returned `ReferencedShaderAssetIds` is empty
- imported diffuse texture maps to `cooked/imported/...`
- DS definition exposes material schemas and `CookedPlatformOwned`

### Editor Packager Tests

Mirror the PS2 cooked-material coverage for DS:

- generated standard material cooks through the builder
- result deserializes as `PlatformMaterialAsset`
- no `cooked/shaders/ForwardStandardShader.dx11.hasset`

### Verification

Run focused DS builder tests and focused shared editor packager tests before claiming success.

## Risks

### Over-growing `PlatformMaterialAsset`

If this pass pushes too much DS-specific meaning into the generic asset, the neutral seam becomes dishonest in the wrong way. Keep the payload generic and minimal.

### Half-converted packager defaults

If DS stops writing shader packages but the cook field defaults still assume DX11 shader metadata, the build can remain internally inconsistent. The generated-standard-material gate and the default field seeding need to move together.

## Success Criteria

- DS `CookMaterial(...)` emits serialized `PlatformMaterialAsset`
- DS material cooking returns no shader references
- DS platform definition opts into `CookedPlatformOwned`
- shared editor packager stops exporting `ForwardStandardShader.dx11.hasset` for DS
- focused DS builder tests and shared editor packager tests pass
