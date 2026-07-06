# DS Logo Interior Seam Overlap

## Goal

Eliminate the thin background lines that appear between adjacent Nintendo DS logo OBJ tiles during affine animation.

## Current Context

- The remaining artifact is not mainly motion wobble.
- The visible problem is an occasional thin background line between adjacent affine OBJ tiles.
- The DS runtime currently splits the logo into strict non-overlapping OBJ tiles and places them from quantized affine transforms.
- Even with hardware-matched quantization, screen-space rounding can still leave a 1-pixel interior separation.

## Proposed Change

Keep the current quantized affine basis and change only final tile placement:

- use the snapped composed-sprite center as the shared placement basis
- compute each tile position as a relative offset from that center
- round each relative offset toward the center instead of to nearest
- keep non-affine sprite placement unchanged

## Why This Approach

- directly targets visible seam leakage instead of motion smoothness
- keeps the overlap symmetric around the composed sprite center
- avoids hard-growing the logo silhouette on the outer edges
- stays localized to the DS sprite placement path

## Validation

1. Update the focused affine source audit to require center-based inward overlap rounding.
2. Run the focused renderer audit and confirm it fails before implementation.
3. Build the DS ROM and launch melonDS.
4. Verify that the interior background seam is reduced or gone.

## Non-Goals

- No `.hanim` changes.
- No city scene-authoring changes.
- No sprite tile graphic bleeding in this pass.
