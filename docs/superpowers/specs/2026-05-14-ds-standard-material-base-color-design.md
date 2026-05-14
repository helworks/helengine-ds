# DS Standard Material Base Color Design

## Goal

Make the Nintendo DS renderer honor authored standard-material `BaseColorBuffer` data for any scene that uses the standard material contract, while keeping the existing DS face-lighting model and current startup/render stability intact.

## Scope

- Consume cooked `BaseColorBuffer` payloads from runtime `MaterialAsset` objects in the DS renderer.
- Store one normalized base RGB color on the DS runtime material.
- Replace grayscale-only lit triangle output with base-color-modulated lighting output.
- Keep the existing DS lighting inputs:
  - accumulated authored ambient light
  - first authored directional light
  - per-face diffuse from triangle normal
- Keep compatibility with materials that do not carry `BaseColorBuffer` by falling back to white.

## Non-Goals

- No new material schema fields.
- No DS-specific material authoring path.
- No texture sampling support.
- No per-vertex lighting.
- No shadows or specular lighting.
- No scene-specific logic for `colored_cube_grid`.

## Current State

The authored scene side is already correct:

- `colored_cube_grid` writes sixteen distinct `base-color` values through standard material settings.
- The editor cook path already translates `base-color` into one `BaseColorBuffer` constant buffer.

The DS runtime path is still incomplete:

- `NintendoDsRenderManager3D::BuildMaterialFromRaw(...)` initializes every DS runtime material as opaque white.
- `NintendoDsRenderManager3D::SubmitLitTriangle(...)` converts lighting to grayscale and packs that grayscale into the final polygon color.

That means the DS renderer currently discards authored material color twice:

1. at runtime material creation
2. at final triangle color submission

## Recommended Approach

Use the existing standard-material contract directly.

The DS renderer should parse `BaseColorBuffer` from cooked `MaterialAsset::ConstantBuffers` during runtime material creation, store the RGB color on `NintendoDsRuntimeMaterial`, and multiply that color by the existing DS lighting result during triangle submission.

This keeps DS aligned with the rest of the engine instead of inventing a parallel DS-only material color path.

## Data Contract

### Source

The DS renderer should treat one constant buffer named `BaseColorBuffer` as the standard-material base color source.

Expected payload:

- 16 bytes
- four little-endian `float` values
- `R`, `G`, `B`, `A`

The DS renderer only needs `R`, `G`, and `B` for this slice.

### Fallback Behavior

If `BaseColorBuffer` is absent, malformed, or shorter than 16 bytes:

- do not fail runtime material creation
- fall back to white base color

This preserves compatibility with older assets and non-standard materials.

## Runtime Changes

### 1. Extend DS Runtime Material

Add one normalized base RGB color field to `NintendoDsRuntimeMaterial`.

This field should:

- default to white
- represent the authored standard-material base color after decoding from `BaseColorBuffer`

The existing packed white field should no longer be treated as the authoritative material color for lit triangle output.

### 2. Parse `BaseColorBuffer` In `BuildMaterialFromRaw(...)`

In `NintendoDsRenderManager3D::BuildMaterialFromRaw(...)`:

- inspect `materialAsset->ConstantBuffers`
- find the entry named `BaseColorBuffer`
- if present and valid, decode the first three floats into normalized RGB
- clamp each channel to `[0, 1]`
- store the decoded color on `NintendoDsRuntimeMaterial`

If no valid buffer is found, leave the runtime material base color at white.

### 3. Replace Grayscale Final Color Output

In `NintendoDsRenderManager3D::SubmitLitTriangle(...)`:

- keep the current world-space face normal computation
- keep the current directional diffuse term
- keep authored ambient and directional radiance resolution
- compute lighting as:

`lighting = ambientRadiance + directionalRadiance * diffuse`

- clamp the lighting RGB to `[0, 1]`
- apply the existing display-contrast curve channel-wise
- compute final color as:

`finalColor = baseColor * shapedLighting`

- clamp `finalColor` to `[0, 1]`
- pack `finalColor` into DS polygon color

This produces lit colored faces while preserving the current DS renderer structure.

## Error Handling

- Missing `materialAsset`, `shaderAsset`, `drawable`, or `positions` should keep the current defensive behavior.
- Missing or malformed `BaseColorBuffer` should not throw.
- Degenerate triangle normals should continue to use the current safe normal fallback.

## Testing Strategy

### Focused DS Runtime Tests

Add focused tests for:

- decoding a valid `BaseColorBuffer` into DS runtime material RGB
- white fallback when `BaseColorBuffer` is absent
- white fallback when `BaseColorBuffer` data is malformed or too short

### Visual Verification

Verify in `melonDS` using:

1. `cube_test`
   - should remain white-lit and rotating
2. `colored_cube_grid`
   - should show distinct lit cube colors
   - should keep the authored camera clear color
   - should keep stable non-flickering rendering

## Risks

### Non-Standard Materials

Some runtime materials may not use the standard-material contract. White fallback avoids regressions for those cases.

### DS Color Precision

The DS polygon path is low precision. Final colors may look banded or slightly desaturated compared to Windows. That is acceptable for this slice as long as cube colors remain distinct and obviously authored.

### Lighting Shape

Because this remains per-face lighting, color transitions will still be faceted. That is acceptable and consistent with the current geometry-first DS renderer scope.

## Acceptance Criteria

- Any standard material carrying `BaseColorBuffer` renders with lit authored base color on DS.
- `colored_cube_grid` shows distinct colored cubes in `melonDS`.
- `cube_test` still renders and rotates correctly.
- No regression to DS startup, flicker, or camera clear-color behavior.
