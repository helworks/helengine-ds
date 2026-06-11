# DS BG0 Authored Bottom Text Design

## Goal

Restore real authored bottom-screen `TextComponent` rendering on Nintendo DS using the now-stable `BG0` text background path and the cooked DS font atlas, without reintroducing the display-list flicker or the temporary proof-only behavior.

## Ground Truth

- The packaged DS font assets are now cooked correctly.
- The bottom-screen `BG0` presentation path is stable when:
  - the 3D renderer uses the fallback triangle submission path instead of the DMA display-list path
  - `BG0` is initialized once and explicitly shown with priority `0`
  - the renderer does not repaint proof text or reconfigure sub-screen mode every frame
- The temporary proof `H` is no longer needed once authored text is restored.

## Scope

- Keep the stable `BG0` bottom-screen ownership model.
- Keep the stable 3D fallback triangle path unchanged.
- Remove the static proof-tile behavior.
- Restore real authored string submission through the cooked-font `BG0` path.

## Out Of Scope

- Re-enabling the DMA-backed display-list path.
- Reintroducing `consoleInit` or `iprintf` into runtime text rendering.
- Adding bitmap or software-text fallbacks for the bottom screen.

## Implementation Approach

### 1. Preserve Stable Screen Ownership

- Leave the fixed `BG0` initialization path in place.
- Leave the explicit `bgSetPriority(..., 0)` and `bgShow(...)` calls in the one-time bottom-screen text background initialization path.
- Do not reintroduce per-frame sub-screen mode changes or proof-tile uploads.

### 2. Remove Proof-Only Bottom Text

- Remove the one-time proof `H` tile upload and map entry writes.
- Remove the proof-only rejection path from `TryDrawHardwareText(...)`.

### 3. Restore Cooked-Font Text Submission

- Reuse:
  - `EnsureBottomScreenFontGlyphTilesReady(...)`
  - `TryResolveBottomScreenGlyphTileIndex(...)`
  - `WriteBottomScreenTextLine(...)`
- `TryDrawHardwareText(...)` should:
  - validate that the text is eligible for the DS `BG0` path
  - read the authored string from the drawable
  - resolve the font from the drawable
  - upload glyph tiles once per active font
  - compute the target row, column, and visible width from the authored text box and alignment
  - write the authored content into the `BG0` tilemap

### 4. Keep Unsupported Cases Explicit

- If authored text cannot fit the current `BG0` path, continue to reject it with a concrete reason.
- Do not add new runtime fallbacks in this pass.

## Expected Result

- The bottom screen renders real authored DS text again.
- The renderer uses the cooked DS font atlas instead of built-in console glyphs or proof tiles.
- The top-screen 3D cubes remain visible.
- The bottom-screen text remains stable without the previous display-list flicker.

## Verification

1. Rebuild the DS ROM with the native Docker `make` path.
2. Launch the rebuilt ROM in melonDS.
3. Confirm:
   - top-screen cubes still render
   - bottom-screen authored text appears
   - bottom-screen text uses the cooked DS font path
   - no visible bottom-screen flicker is reintroduced
