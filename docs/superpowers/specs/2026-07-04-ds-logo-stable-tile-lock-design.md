# DS Logo Stable Tile Lock

## Goal

Reduce the tiny per-frame Nintendo DS logo shimmer during affine animation by prioritizing stable multi-tile placement over mathematically exact centering.

## Current Context

- The DS logo renders as a composed multi-OBJ sprite through `NintendoDsRenderManager2D`.
- Affine rotation and scale are already working well enough visually.
- The remaining artifact is a minor x/y wobble where some frames appear to shift by roughly half a pixel.
- The user prefers stable tile lock over exact center preservation when those goals conflict.

## Proposed Change

Keep the current DS affine matrix path, but change tile placement so the composed sprite uses one shared snapped origin per frame.

- preserve the current affine matrix setup and rotation math
- preserve the current support for multi-tile affine sprites
- stop resolving each tile position through its own independent top-left rounding path
- resolve one shared snapped composed-sprite origin from the same fixed-point center math already used by the affine path
- place each tile from that shared snapped origin with deterministic integer offsets

## Why This Approach

- targets the exact symptom without changing the authored animation
- preserves the current DS affine feature set instead of replacing it with a custom animation path
- favors visual cohesion across all logo tiles, which is the user’s stated priority
- keeps the change localized to the DS 2D renderer

## Validation

1. Add a focused source-audit test that asserts affine tile placement uses one shared snapped origin path instead of rounding each tile independently.
2. Rebuild the focused DS renderer source-audit tests.
3. Build the DS ROM.
4. Launch melonDS and inspect the logo animation for reduced shimmer.

## Non-Goals

- No changes to the `.hanim` asset.
- No changes to menu authoring in the city project.
- No replacement of the DS affine sprite path with a CPU-only animation path.
