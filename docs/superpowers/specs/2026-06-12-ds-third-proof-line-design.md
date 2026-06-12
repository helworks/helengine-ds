# DS Third Proof Line Design

## Goal

Add one more fixed proof line beneath the currently rendering `Update` line on the Nintendo DS bottom screen so row placement can be validated in isolation.

## Ground Truth

- The DS build currently boots and renders:
  - the top-screen 3D cube scene
  - a bottom-screen fixed `HELLO` proof line
  - a bottom-screen runtime `Update` line
- The next step should stay small and avoid mixing layout validation with broader runtime text changes.

## Scope

- Add a third visible line beneath `Update`.
- Keep that third line fixed rather than runtime-authored.
- Preserve the current `HELLO` and `Update` behavior.

## Out Of Scope

- Reworking the bottom-screen runtime text pipeline.
- Changing higher-level scene or authored text content.
- Adding new fallback behavior or new rendering helpers.

## Implementation Approach

### 1. Keep The Existing Split

- Leave the existing fixed `HELLO` proof behavior unchanged.
- Leave the existing runtime `Update` submission path unchanged.

### 2. Add One Fixed Proof Line

- Add one more `WriteBottomScreenTextLine(...)` call in the DS bottom-screen presentation path.
- The fixed proof text should be `Third`.
- Its target row should be exactly one text row below the row where `Update` currently appears.
- Its column should align with the existing bottom-screen text layout so the new line reads as a direct continuation.

### 3. Verify Through Source And Runtime

- Update the DS source-audit test so it expects the new fixed proof line in the renderer source.
- Rebuild the DS ROM.
- Launch the ROM in melonDS and confirm the third line appears beneath `Update`.

## Expected Result

- The bottom screen continues to show `HELLO`.
- The runtime path continues to render `Update`.
- A new fixed `Third` line appears directly below `Update`.

## Verification

1. Run the focused DS source-audit test that covers the bottom-screen proof behavior.
2. Rebuild the DS ROM through the existing editor-driven DS build path.
3. Launch the rebuilt ROM in melonDS.
4. Confirm the bottom screen now shows three visible lines in order:
   - `HELLO`
   - `Update`
   - `Third`
