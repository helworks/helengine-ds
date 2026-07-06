# DS Text Overflow Glyph Culling Design

## Goal

Change the Nintendo DS text renderer so glyphs that would extend outside their text bounds are not rendered at all.

This replaces the current clipped/partial rendering behavior with glyph-level culling:

- Glyph fully inside bounds: render it normally.
- Glyph crossing any left, right, top, or bottom bound: skip it entirely.
- Other glyphs in the same text element continue rendering.

## Scope

This change is intentionally local to the Nintendo DS renderer.

In scope:

- `src/platform/ds/NintendoDsRenderManager2D.cpp`
- `src/platform/ds/NintendoDsRenderManager2D.hpp`
- `builder.tests/NintendoDsRenderManager2DSourceAuditTests.cs`

Out of scope:

- Engine-wide text layout behavior
- Editor layout or authoring behavior
- Font atlas generation
- Non-DS backends

## Current Problem

The DS text renderer currently allows glyphs to be partially drawn when they extend beyond the text bounds. For DS UI this produces undesirable clipping artifacts.

Desired behavior is stricter:

- Do not clip the overflowing glyph.
- Do not partially draw the overflowing glyph.
- Simply skip that glyph.

## Chosen Approach

Implement glyph-level bounds rejection at final DS glyph emission time.

Why this approach:

- It matches the requested behavior exactly.
- It keeps engine-side text layout unchanged.
- It is local to the DS backend.
- It avoids introducing DS-specific layout rules into shared engine text code.

Rejected alternatives:

1. Layout-time truncation
This would stop producing later glyphs once overflow begins, but it changes layout semantics instead of just changing DS rendering.

2. Whole-text or whole-line rejection
This is too aggressive because only overflowing glyphs should disappear.

3. Partial clipping
This is the current undesirable behavior.

## Behavior Contract

For every glyph considered by the DS renderer:

1. Compute the glyph quad or screen-space destination rectangle exactly as today.
2. Compare the full glyph rectangle against the effective text bounds.
3. If any edge falls outside the bounds, skip rendering that glyph.
4. Continue processing subsequent glyphs normally.

This rule applies per glyph, not per line and not per text element.

## Implementation Notes

The DS renderer appears to have distinct text submission paths, including the top-screen background text path and the hardware text path. The implementation should apply the same overflow rule anywhere a glyph can currently be partially emitted.

The intended implementation shape is:

- Add one explicit glyph-bounds predicate in `NintendoDsRenderManager2D`.
- Call it immediately before final glyph emission.
- Return early for overflowing glyphs without mutating the rest of the text flow.

Important constraints:

- Do not rewrite shared engine text layout.
- Do not add asset-side hacks or padding rules.
- Do not silently resize or truncate text content.

## Testing Strategy

Use a source-audit test first, then minimal DS validation.

Planned automated coverage:

- Assert the DS text renderer contains a glyph-level bounds rejection guard before final glyph emission.
- Assert the renderer skips overflowing glyphs instead of clipping them.

Planned manual validation:

- Place text near a bound where the final glyph would overflow.
- Confirm the overflowing glyph disappears fully instead of rendering partially.
- Confirm interior glyphs still render normally.

## Risks

1. Inconsistent behavior between DS text paths
If top-screen and bottom-screen text use different emission helpers, both must apply the same rule.

2. Off-by-one bounds checks
The exact inclusive/exclusive edge rule must be consistent so stable glyphs do not flicker when they sit on the edge.

3. Hidden coupling with cache/state reuse
Skipping glyphs must not invalidate unrelated text cache entries or line bookkeeping.

## Acceptance Criteria

- Overflowing glyphs are not partially visible on DS.
- Only the overflowing glyphs are skipped.
- Non-overflowing glyphs in the same text continue rendering.
- Shared engine text layout remains unchanged.
