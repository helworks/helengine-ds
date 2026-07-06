# Scene Asset Reference Hardening Design

## Goal

Prevent engine and project code from inventing unsupported scene asset references by hand.

The current model exposes `SceneAssetReference` as a mutable bag with public setters. That gives any caller enough freedom to assemble provider ids, asset ids, source kinds, and relative paths that the editor and packager do not actually support. The result is late failures during scene loading or platform packaging, which pushes engine-internal knowledge onto project authors.

The new design makes supported scene asset references engine-owned API instead of user-authored structure assembly. Unsupported combinations must become impossible through normal construction paths and must fail immediately at editor boundaries when malformed data still appears.

## Non-Goals

- No backward compatibility layer.
- No migration path for legacy or malformed references.
- No silent upgrade, fallback, or best-effort correction.
- No attempt to support arbitrary generated asset providers from project code.

We are the only users of the engine right now. Breaking old call sites is acceptable and preferred.

## Problem Statement

Today the engine allows code like this:

```csharp
new SceneAssetReference {
    SourceKind = SceneAssetReferenceSourceKind.Generated,
    ProviderId = "editor",
    AssetId = "ds-debug-font",
    RelativePath = "generated/editor/fonts/ds-debug.hefont"
}
```

This is the wrong ownership boundary.

`SceneAssetReference` currently lets callers express combinations that are not guaranteed to be valid for:

- editor-time resolution
- scene save/load
- platform packaging
- runtime consumption

That means project-side authors need hidden knowledge about which provider ids and asset ids are legal. The DS build failure caused by `editor:ds-debug-font` is only one symptom of the broader API problem.

## Design Principles

- Supported references must be created through explicit utility/factory APIs.
- Core types must not expose enough mutability for callers to invent unsupported references casually.
- Editor-only generated references must be created by editor-owned helpers, not arbitrary project code.
- Resolver and packaging checks remain as invariant enforcement, not as the primary user-facing guardrail.
- Failures must happen as early as possible and mention the owning component/member context when available.

## Proposed Architecture

### 1. Constrain `SceneAssetReference` in `helengine.core`

`SceneAssetReference` remains the serialized reference type, but it stops being freely mutable.

Required changes:

- remove the public empty-construction plus public-setter pattern
- expose read-only properties for `SourceKind`, `RelativePath`, `ProviderId`, and `AssetId`
- allow creation only through validated constructors or static factory methods owned by engine code

The important constraint is not the exact syntax. The important constraint is that normal callers cannot assemble arbitrary shapes with object initializers.

### 2. Split creation ownership by responsibility

#### `helengine.core`

Add core-owned creation utilities for structurally valid references:

- `SceneAssetReferenceFactory`
- `EngineSceneAssetReferenceFactory`

`SceneAssetReferenceFactory` owns file-backed references only.

Example responsibilities:

- create file-backed font references
- create file-backed texture references
- create file-backed model references
- create file-backed material references
- create file-backed animation clip references

These APIs validate:

- non-empty relative path
- project-relative shape expectations where appropriate
- no provider/asset id usage for file-backed references

`EngineSceneAssetReferenceFactory` owns engine-supported generated references only.

Example responsibilities:

- create generated cube model reference
- create generated plane model reference
- create generated sphere model reference
- create generated standard material reference

There is intentionally no generic `CreateGenerated(providerId, assetId, relativePath)` public API.

#### `helengine.editor`

Add `EditorSceneAssetReferenceFactory` for editor-supported generated references only.

Example responsibilities:

- create editor UI font reference

This keeps editor-generated asset knowledge inside the editor assembly instead of leaking it into project code.

### 3. Ban open-ended generated reference creation

Generated references are the dangerous class because they encode engine/editor implementation details. The new design must make open-ended generated reference creation unavailable to ordinary callers.

That means:

- no public generic generated-reference builder in core
- no public setter-based mutation path
- no sanctioned way for project code to name arbitrary provider/asset id combinations

If a new generated asset becomes supported in the future, engine or editor code must add a named factory method for it explicitly.

## Validation Boundaries

Constraining construction is the primary fix, but boundary validation still matters because malformed references can still appear through deserialization, hand-authored generated code, or stale code paths.

### Save-Time Validation

Before any scene/component asset reference is written out, the editor validates that the reference is supported for the owning asset type.

This validation must run in the save/serialization path, not only during packaging.

Expected behavior:

- fail hard immediately
- no mutation or repair
- error names the component/member/reference that was invalid

This moves failures closer to the authoring mistake and keeps packaging from being the first discovery point.

### Editor Load/Resolve Validation

`EditorSceneAssetReferenceResolver` keeps strict provider/id/source validation.

This layer now serves as:

- corruption detection
- stale-data detection
- invariant enforcement for references not created through sanctioned paths

### Packaging Validation

`SceneComponentPackagingTransformService` keeps strict checks as well.

This layer remains important, but it is no longer the intended first failure point. If packaging sees an unsupported reference, that means an earlier boundary failed to reject invalid data.

## API Shape

The API names below are the recommended contract and should be used unless an existing repository naming constraint forces a rename without changing the ownership model:

```csharp
SceneAssetReference bodyFontReference = SceneAssetReferenceFactory.CreateFileSystemFont("Fonts/DemoDiscBody.ttf");
SceneAssetReference cubeReference = EngineSceneAssetReferenceFactory.CreateCubeModel();
SceneAssetReference editorFontReference = EditorSceneAssetReferenceFactory.CreateEditorUiFont();
```

The following style must stop compiling or become internal-only:

```csharp
new SceneAssetReference {
    SourceKind = SceneAssetReferenceSourceKind.Generated,
    ProviderId = "editor",
    AssetId = "anything",
    RelativePath = "anything"
}
```

## Enforcement Scope

This hardening applies to both `helengine.core` and `helengine.editor`.

That means:

- core raw scene-reference types become constrained
- editor reference creation moves to editor-owned utilities
- engine/editor call sites that currently build raw references directly must be updated
- project-side authored code on our side must switch to the new sanctioned factories

The breakage is intentional.

## Expected Call Site Churn

The implementation should expect direct construction updates in at least these areas:

- `FontAssetScenePersistenceSupport`
- `EditorSession` asset-reference builders
- scene persistence helpers that currently assemble references directly
- generated-scene or scene-authoring helpers that materialize font/model/material references
- tests that build ad hoc generated references

Project-side code that still constructs unsupported generated references directly must be rewritten to use file-backed or explicitly supported engine/editor factories.

## Error Model

All newly enforced failures should:

- throw immediately
- avoid fallback behavior
- state the unsupported source/provider/asset id combination
- include owning member or component context when available

Examples of acceptable failure shape:

- unsupported generated font asset id for member `Font`
- generated texture references are not supported for component `SpriteComponent`
- file-backed model reference must include a relative path

The point is to turn hidden engine rules into precise failures at the correct boundary.

## Testing Strategy

### Core Tests

- `SceneAssetReference` can no longer be casually object-initialized by normal call sites
- file-backed factory methods produce the expected immutable reference shape
- engine-generated factory methods produce the expected immutable reference shape

### Editor Tests

- editor-generated factory methods produce the expected immutable reference shape
- save/serialization fails immediately for unsupported generated references
- resolver still fails hard for malformed or unsupported persisted references
- packaging still fails hard if malformed references somehow bypass earlier validation

### Regression Coverage

Add a regression test that models the failure class we just hit:

- a text/font-bearing authored component carries an unsupported generated font reference
- the editor fails at the early validation boundary
- packaging is never required to discover the bug

## Rollout Plan

1. Constrain `SceneAssetReference` in `helengine.core`.
2. Add core and editor factory/utility entry points.
3. Add early validation to scene save/serialization boundaries.
4. Update engine/editor call sites to the sanctioned APIs.
5. Update project-side code on our side to the sanctioned APIs.
6. Keep resolver and packaging validation as invariant backstops.

## Trade-Offs

### Benefits

- removes a large footgun from project and engine code
- shifts engine knowledge back into engine/editor APIs
- turns late packaging failures into early authoring failures
- makes supported generated assets explicit and discoverable

### Costs

- intentionally breaks existing direct-construction call sites
- introduces some API churn across core, editor, tests, and project code
- requires explicit factory additions when new generated assets become supported

These costs are acceptable because the current flexibility is harmful rather than useful.

## Recommendation

Adopt the constrained-reference plus named-factory model across both core and editor, with no legacy compatibility path.

This is the smallest change that fixes the actual ownership problem:

- project code should not need deep engine knowledge to create valid references
- engine/editor should own the legal reference space
- unsupported references should fail immediately, not during DS packaging or some other late-stage operation
