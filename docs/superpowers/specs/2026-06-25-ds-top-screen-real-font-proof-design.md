# DS Top-Screen Real-Font Proof Design

## Context

The Nintendo DS renderer currently proves top-screen BG0 text stability with a renderer-owned handwritten `HELLO` line and a renderer-owned white OBJ square. That proof established that main-screen BG0 and OBJ can coexist stably when the screen stays in pure 2D mode.

The next proof should validate the real cooked-font path on the top screen without depending on scene content. The project decision is that top and bottom screens must converge on the same renderer code path instead of keeping bespoke top-screen text logic.

## Goal

Show two deterministic proof lines on the top screen:

1. A handwritten `HELLO` control line that remains unchanged.
2. A second `HELLO` line directly below it, rendered through the shared cooked-font glyph upload and tile-map write path using the same real font asset the city menu uses.

The white OBJ square remains visible on the same screen so the proof still validates `BG0 + OBJ` coexistence.

## Non-Goals

- Do not route this proof through scene-authored text drawables yet.
- Do not remove the handwritten control line in this pass.
- Do not introduce migration or legacy compatibility behavior.
- Do not add fallback synthetic glyph generation if the real proof font is unavailable.

## Recommended Approach

Keep the proof renderer-owned and deterministic, but replace the second proof line with the normal shared cooked-font path already used by bottom-screen runtime text.

This gives one stable control line and one real-font line on the same top-screen BG0 map:

- control line proves BG0 map stability
- real-font line proves top-screen cooked-font glyph upload, glyph cache population, glyph resolution, and map writes
- white OBJ square proves BG0 and OBJ coexist on the same screen

## Design

### Top-Screen Proof Content

The top screen will display:

- Row 1: handwritten `HELLO`
- Row 2: cooked-font `HELLO`
- Existing white OBJ square to the right

The handwritten line remains sourced from the current renderer-owned proof tiles so there is still a known-good control if the real-font line fails.

### Font Source

The cooked-font proof must use the same real font asset the city menu uses. The renderer proof should resolve that font explicitly from existing runtime/editor-loaded assets instead of inventing a dedicated DS-only proof font.

If the renderer cannot resolve that font, the proof path should fail explicitly rather than silently substituting another font or synthetic glyphs.

### Renderer Flow

The top-screen proof flow should become:

1. Ensure the top-screen text background exists.
2. Resolve the real proof font asset used by the city menu.
3. Upload glyph tiles for the top screen through the shared `NintendoDsScreenTarget::Top` glyph-upload path.
4. Clear the top-screen text map.
5. Write the handwritten control `HELLO`.
6. Write the cooked-font `HELLO` on the next row through the shared text-line writer.
7. Leave the existing top-screen proof OBJ flow unchanged.

This preserves the current deterministic boot-time proof while moving the second line onto the same shared text infrastructure that the engine should ultimately use everywhere.

### Shared-Code Requirement

This design must reuse the shared screen-targeted helpers already introduced in `NintendoDsRenderManager2D`:

- top-screen font glyph upload must go through the shared screen helper, not a bespoke top-screen glyph routine
- top-screen text map writes must go through the shared screen text-line writer
- the proof should not reserve new DS-only top-screen glyph tile indices for the real-font line

The only remaining top-screen-specific behavior after this change should be the renderer-owned proof orchestration itself and the handwritten control line.

## Error Handling

- Missing proof font is a hard failure for this proof path.
- Invalid or unsupported proof font texture state should preserve existing glyph-upload failure behavior.
- The renderer must not silently fall back to handwritten glyphs for the second line.

## Testing

Source audits should verify:

- the renderer-owned top-screen proof still exists
- the handwritten control `HELLO` remains
- the cooked-font proof line uses the shared top-screen glyph upload and shared top-screen text-line writer
- the top-screen real-font proof no longer depends on reserved handwritten proof tile indices for the second line

Runtime validation should remain minimal:

- rebuild the DS ROM
- launch the ROM in melonDS
- visually confirm handwritten `HELLO` on the first line, real-font `HELLO` on the second line, and the white OBJ square on the same top screen

## Scope

This is a narrow proof-only step. It validates the real top-screen cooked-font path before the renderer starts trusting normal top-screen runtime text submission for scene/menu content.
