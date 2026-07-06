# DS Logo Hardware-Matched Affine Quantization

## Goal

Reduce the remaining tiny Nintendo DS logo shimmer by making CPU-side affine tile placement use the same quantized transform basis that the DS hardware OBJ matrix already uses.

## Current Context

- The DS logo now uses a shared snapped reference tile so all tiles remain more visually locked together.
- A minor residual wobble remains, roughly half a pixel on some frames.
- The current renderer still computes CPU placement from full-precision scale and rotation values, while the DS hardware matrix uses quantized affine angle and scale values.

## Proposed Change

Keep the current shared tile-lock behavior and change only the affine placement basis:

- derive visual placement rotation from the quantized DS affine angle
- derive visual placement scale from the quantized DS affine scale values
- keep affine matrix submission unchanged
- keep `.hanim` animation content unchanged

## Why This Approach

- aligns CPU placement with the actual DS hardware transform instead of approximating it separately
- preserves smooth authored animation better than aggressive frame snapping
- keeps the change localized to the DS renderer

## Validation

1. Update the focused DS affine source-audit test to require hardware-matched quantized placement.
2. Run the focused renderer audit and confirm it fails before implementation.
3. Build the DS ROM and launch melonDS.
4. Verify whether the remaining shimmer is reduced again.

## Non-Goals

- No `.hanim` changes.
- No city project scene-authoring changes.
- No switch to a CPU-only sprite animation path.
