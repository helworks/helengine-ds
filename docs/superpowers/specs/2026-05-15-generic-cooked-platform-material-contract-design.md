# Generic Cooked Platform Material Contract Design

## Problem

The shared `helengine` runtime already exposes a generic `RuntimeMaterialResolutionMode.CookedPlatformOwned` contract, but the implementation behind that contract is still PS2-specific in multiple shared-engine files. The runtime content manager registers `Ps2MaterialAsset`, the runtime scene resolver loads `Ps2MaterialAsset`, and the shared render manager contract exposes `BuildMaterialFromCooked(Ps2MaterialAsset)`.

That shape is wrong for the next fixed-pipeline platforms. Nintendo DS should not inherit PS2 names inside the shared engine, and later fixed-pipeline platforms should not need to add more platform-specific branches to generic runtime code.

## Goals

- Remove platform-name-specific cooked material references from shared engine runtime material resolution.
- Preserve the existing two shared runtime material modes:
  - `RawShaderBacked`
  - `CookedPlatformOwned`
- Introduce one neutral shared stub asset type for cooked platform-owned material resolution.
- Keep the first step narrow: this change does not need to finish DS material cooking or editor fixed-pipeline authoring.

## Non-Goals

- Implement the final DS fixed-pipeline material cook path.
- Implement editor UI for fixed-pipeline material profile editing.
- Push fully builder-driven material asset type generation into shared engine in this pass.
- Redesign authored material semantics.

## Recommended Approach

Introduce a generic shared asset type named `PlatformMaterialAsset` and route the existing `CookedPlatformOwned` runtime branch through that type everywhere in the shared engine.

This keeps the current generic mode contract intact while removing PS2-specific naming from the shared runtime. The new asset is intentionally a stub seam. Builders can later decide how to populate it, replace it, or drive more specific generated resolution paths without forcing more platform names into shared runtime code.

## Alternatives Considered

### 1. Generic stub asset now

Recommended.

Benefits:
- Removes `Ps2` naming from shared engine immediately.
- Keeps the runtime contract simple and generic.
- Leaves room for later builder-driven/generated specialization.

Trade-off:
- Adds one intermediate stub type that may later be generalized further.

### 2. Rename the PS2 asset to a generic name without changing its behavior

Rejected.

Benefits:
- Low implementation cost.

Trade-off:
- Risks keeping PS2 assumptions hidden behind a generic name.
- Makes later DS adoption ambiguous because the contract would still effectively be PS2-shaped.

### 3. Move all cooked material type details into generated code immediately

Rejected for this pass.

Benefits:
- Closest to the long-term builder-owned architecture.

Trade-off:
- Too broad for the current refactor.
- Couples the cleanup to a larger generator/editor redesign that is not required to remove shared-engine platform naming now.

## Design

### Shared Runtime Contract

The shared engine continues to expose `RuntimeMaterialResolutionMode.CookedPlatformOwned` as the generic switch for fixed-pipeline or otherwise platform-owned cooked material resolution.

Under that mode:

- `RuntimeContentManagerConfiguration` registers `PlatformMaterialAsset` for `RuntimeContentProcessorIds.MaterialAsset`.
- `RuntimeSceneAssetReferenceResolver` loads `PlatformMaterialAsset` instead of `Ps2MaterialAsset`.
- `RenderManager3D` exposes `BuildMaterialFromCooked(PlatformMaterialAsset materialAsset)`.

No platform-specific names remain in these shared engine seams.

### `PlatformMaterialAsset`

The first version of `PlatformMaterialAsset` is a neutral stub. It should be small and intentionally generic.

Initial shape:

- inherit from `Asset`
- include one stable payload identity field such as `RendererFamilyId`
- optionally include an opaque serialized payload field if needed by existing tests or callers

This asset is not the final fixed-pipeline schema system. It is the generic placeholder consumed by the shared runtime contract until builders take over more of the resolution path.

### Builder Ownership

Builders remain the authoritative source for platform-specific material semantics.

Later work can extend this seam by allowing builders to:

- define cooked fixed-pipeline material schemas for the editor
- emit cooked `PlatformMaterialAsset` payloads
- generate platform-specific runtime translation code
- optionally replace the stub asset contract with generated builder-owned material payload handling

This design does not block that future direction.

### DS Impact

Nintendo DS should eventually opt into `CookedPlatformOwned` and stop participating in shader-backed material export for fixed-pipeline materials.

That follow-up depends on:

- DS material schemas in the builder definition
- DS `CookMaterial(...)` implementation
- editor packaging changes that stop force-emitting DX11 standard shader payloads for DS

Those changes are intentionally deferred from this refactor.

## Files Expected To Change In `helengine`

- `engine/helengine.core/assets/raw/...` for the new generic cooked material asset type
- `engine/helengine.core/content/RuntimeContentManagerConfiguration.cs`
- `engine/helengine.core/scene/runtime/RuntimeSceneAssetReferenceResolver.cs`
- `engine/helengine.core/managers/rendering/RenderManager3D.cs`
- serializer files that must recognize the new shared asset type
- source tests that currently assert `Ps2MaterialAsset`

## Testing Strategy

Follow TDD for the shared-engine refactor.

### Red

Update the shared-engine source tests first so they assert the generic cooked-platform-owned seam rather than PS2-specific types.

Expected failing coverage:

- runtime scene resolver source test should look for `PlatformMaterialAsset`
- runtime content manager source test should look for `AssetContentProcessor<PlatformMaterialAsset>`

### Green

Implement the minimal generic shared asset contract and update the runtime/content-manager/render-manager seams to satisfy the tests.

### Verify

Run the focused shared-engine tests that cover:

- runtime scene resolver source
- runtime content manager source
- any serializer or content-manager tests affected by the new asset type

## Risks

### Hidden PS2 assumptions

If any remaining shared-engine code assumes PS2-specific fields on the cooked material asset, a rename-only approach would leave those assumptions hidden. The stub must stay intentionally minimal so those assumptions surface immediately.

### Partial adoption

DS and other fixed-pipeline builders will still be incomplete after this refactor. That is acceptable as long as the shared engine no longer hardcodes platform names in the cooked-material branch.

## Success Criteria

- Shared engine runtime cooked-material resolution contains no `Ps2` or `NintendoDs` type references.
- `CookedPlatformOwned` remains the generic runtime contract.
- Shared-engine tests validate the neutral cooked material seam.
- The design leaves builder-driven fixed-pipeline editing and cooking as future work without blocking it.
