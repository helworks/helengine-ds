# DS Plain Rectangle Support Design

## Summary

The Nintendo DS backend currently skips every `RoundedRectComponent` and renders a magenta marker instead. That is correct for unsupported rounded corners, but it leaves the bottom-screen back button visibly broken even though its authored shape can degrade cleanly to a plain rectangle.

This change adds one narrow hardware-only rectangle path for the DS backend:

- every `RoundedRectComponent` is treated as an axis-aligned rectangle
- `Radius` and corner flags are ignored
- fill and border render through DS hardware sprite submission only
- anything still outside the supported hardware contract remains skipped

The goal is to remove the two remaining back-button markers without reintroducing any software 2D rendering.

## Problem

The current DS 2D renderer already supports a first-pass hardware sprite path and a first-pass bottom-screen text path, but `DrawRoundedRect(...)` still unconditionally increments unsupported counters and emits the magenta marker. The authored bottom-screen back button uses one `RoundedRectComponent` for its panel, so that panel is guaranteed to fail on DS even though the authored `Radius` is `0`.

The user-approved policy is:

- DS uses only real hardware render paths
- unsupported work is skipped, not emulated in software
- release builds should not invent fallback behavior

Under that policy, the missing feature is not “rounded rectangles.” The missing feature is “plain rectangle composition through DS hardware.”

## Goals

- Render authored DS overlay panels that can be represented as plain axis-aligned rectangles.
- Ignore rounded-corner semantics entirely on DS for this first pass.
- Reuse the existing hardware sprite tiling path instead of adding any software raster step.
- Preserve honest failure for rectangle cases that still do not fit the DS contract.

## Non-Goals

- Real rounded-corner rendering.
- Anti-aliased edges.
- General shape rasterization.
- Software rectangle fallback.
- Support for arbitrary transforms, partial transparency, or clipping beyond the current DS sprite rules.

## Approach Options

### Option 1: Keep rectangles unsupported and suppress their markers

This is the smallest change, but it leaves the back button visually incomplete and hides a still-missing capability.

### Option 2: Flatten rounded rects into plain hardware rectangles

This treats every `RoundedRectComponent` as a rectangular panel on DS and ignores `Radius` plus corner-mask semantics. It fixes the current bottom-screen panel without pretending the DS can do true rounded corners.

This is the recommended option.

### Option 3: Implement actual rounded-corner support

This is more work than the current problem justifies and does not fit the “finish the remaining unsupported primitives first” priority.

## Design

### Rectangle contract

`NintendoDsRenderManager2D::DrawRoundedRect(...)` should stop being an unconditional unsupported path.

Instead, it should attempt one DS hardware rectangle submission with these rules:

- the shape is treated as a plain rectangle regardless of `Radius`
- the shape is treated as a plain rectangle regardless of corner flags
- the shape remains axis-aligned
- the fill is optional when alpha is zero
- the border is optional when `BorderThickness <= 0`

If the authored shape violates the supported DS contract, the renderer keeps the current skip-plus-marker behavior.

### Supported first-pass rectangle cases

The first pass should support:

- axis-aligned rectangle bounds
- opaque fill color
- opaque border color
- border thickness expressed as integer-like screen pixels after rounding
- on-screen or partially on-screen placement
- top-screen and bottom-screen routing through the existing per-screen OBJ paths

### Unsupported first-pass rectangle cases

The first pass should still reject:

- rotated rectangles
- translucent fill or border colors
- impossible hardware sprite budget cases
- any case that cannot be decomposed into the existing DS sprite tile spans

Those cases must remain skipped with the existing unsupported marker and counters.

### Rendering strategy

The rectangle path should be implemented in terms of the existing DS hardware sprite path, not a separate renderer.

Recommended structure:

1. Resolve the rectangle anchor and size from the drawable parent and component fields.
2. Normalize border thickness into one integer pixel thickness.
3. Emit one fill rectangle when fill alpha is non-zero.
4. Emit up to four border rectangles when border thickness is positive:
   - top strip
   - bottom strip
   - left strip
   - right strip
5. Route every emitted strip through the same DS tiled-sprite submission rules already used for regular sprite drawables.

This keeps one hardware rectangle implementation instead of inventing another rendering backend inside `DrawRoundedRect(...)`.

### Color handling

The first pass should not invent blending behavior.

Supported colors:

- fully opaque fill color
- fully opaque border color

Unsupported colors:

- any fill color with alpha other than `255`
- any border color with alpha other than `255`

Fully transparent fill may be treated as “no fill,” and fully transparent border may be treated as “no border,” because that does not invent visual output.

### Metric and debug behavior

When a rectangle renders successfully:

- it should count as a supported 2D primitive
- it should no longer increment `UnsupportedRoundedRectPrimitiveCount`
- it should not emit the magenta marker

When a rectangle fails:

- it should keep the current unsupported accounting
- it should keep the magenta marker behavior
- it should expose one concrete reject reason in the DS trace contract, consistent with the sprite/text paths

Recommended reject reasons:

- `null`
- `parent`
- `color`
- `borderColor`
- `size`
- `borderThickness`
- `budget`

### Testing

The source-audit coverage should change from “rounded rects are always unsupported” to “DS exposes a plain rectangle hardware path while still forbidding software fallback.”

Focused audit expectations should cover:

- `DrawRoundedRect(...)` attempts a hardware rectangle path before unsupported fallback
- DS rectangle support explicitly ignores `Radius`
- the path reuses DS hardware sprite/tile submission helpers instead of software framebuffers
- unsupported rectangle accounting still exists for reject cases

Runtime verification for this pass is:

- rebuild the DS ROM with the shared `build-platform.ps1` script
- launch in melonDS
- confirm the two magenta markers to the left of `BACK` disappear
- confirm the button renders as a rectangular panel, not rounded corners

## Risks

- Rectangle borders can consume several DS OBJ entries, which increases bottom-screen sprite pressure.
- Large authored rectangles could exceed the current tile budget and remain unsupported.
- Flattening every rounded rectangle to a plain rectangle is visually lossy by design, but it is intentional and honest about DS capability.

## Success Criteria

This change is successful when:

- the DS back-button panel renders with no magenta markers at that location
- the DS backend still uses hardware-only rendering
- no CPU 2D compositor or software rectangle fallback is reintroduced
- unsupported rectangle cases still fail visibly and honestly
