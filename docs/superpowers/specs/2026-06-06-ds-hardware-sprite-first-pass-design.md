# DS Hardware Sprite First Pass Design

## Goal

Add the first real Nintendo DS 2D hardware path without restoring any software fallback behavior.

This slice does two things:

1. Throttle unsupported 2D diagnostics so debug output stays readable.
2. Render a narrow class of authored sprites through DS hardware OBJ submission.

Anything outside that narrow class remains unsupported and is skipped.

## Non-Goals

- No software 2D fallback.
- No CPU framebuffer presentation.
- No text rendering implementation in this slice.
- No sprite slicing across multiple DS OBJ entries.
- No rotation, scaling, tint, or special alpha behavior support.
- No bottom-screen sprite rendering in this slice.

## Current Problem

The DS backend now follows the hardware-only policy, but `NintendoDsRenderManager2D` still reports every sprite and text primitive as unsupported because both `TryDrawHardwareSprite(...)` and `TryDrawHardwareText(...)` return `false`.

That creates two immediate issues:

1. Real authored 2D content disappears completely.
2. Debug builds emit excessive unsupported logs that obscure useful diagnostics.

## Chosen Approach

Implement one honest, limited hardware sprite path first.

Accepted sprites will render only when all of these conditions are true:

- The drawable targets the top screen.
- The drawable resolves to one texture-backed sprite.
- The texture fits within one DS OBJ without slicing.
- The texture format is one we explicitly support for the first pass.
- The drawable does not require rotation, scaling, tinting, or other effects outside the first-pass contract.

Any sprite that violates those constraints is skipped and reported through the throttled unsupported diagnostics.

Text remains unsupported and skipped.

## Sprite Support Contract

### Supported

- Top-screen sprite drawables that can be expressed as one hardware OBJ.
- Plain sprite submission with no transform effects beyond integer-position placement.
- Texture payloads whose cooked data can be converted into DS OBJ graphics without fabricating a software substitute.

### Unsupported

- Bottom-screen sprite submission.
- Oversized sprites that would require slicing into multiple OBJ entries.
- Rotated sprites.
- Scaled sprites.
- Tinted or otherwise color-modified sprites.
- Sprite paths that depend on generic alpha blending behavior outside the accepted DS hardware subset.
- Text drawables.

Unsupported work remains skipped in release builds.

In debug builds, unsupported work is still visible through the existing diagnostic path, but logs are throttled to once per category per frame.

## Runtime Data Changes

`NintendoDsRuntimeTexture2D` will gain a small DS sprite cache for OBJ-compatible graphics.

The cache will store only hardware-owned sprite data needed for accepted plain-sprite submission. It will not preserve any CPU compositor state or fallback bitmap state.

The cache lifetime follows the runtime texture lifetime and is released with `ReleaseTexture(...)`.

## Renderer Behavior

### Unsupported logging

`NintendoDsRenderManager2D` will track whether each unsupported category has already been logged during the current frame.

Frame reset happens in `BeginFrame()`.

Expected result:

- first unsupported sprite in a frame logs once
- remaining unsupported sprites in the same frame do not repeat the same log
- same rule applies independently for text and rounded rectangles

### Hardware sprite submission

`TryDrawHardwareSprite(...)` becomes the first true hardware 2D path.

The method will:

1. validate the drawable against the first-pass sprite contract
2. resolve or build DS OBJ-ready graphics from the runtime texture
3. submit one top-screen OBJ entry through main-screen OAM
4. return `true` only when submission actually happened

If any contract check fails, the method returns `false` and the existing unsupported flow handles the skip.

### Text behavior

`TryDrawHardwareText(...)` remains unsupported in this slice and returns `false`.

## Texture Format Policy

The first pass should only accept formats that are already published by the DS pipeline and can be handled without inventing a fake renderer path.

The source tree already advertises DS texture capability around:

- `Rgba4444`
- `Indexed4`
- `Indexed8`

Implementation may narrow that further if one format is the smallest safe first step, but the code must reject unsupported formats explicitly instead of silently fabricating another path.

## Verification

Add or update focused source audits to lock the intended contract:

- unsupported logs are throttled by category per frame
- `TryDrawHardwareSprite(...)` no longer acts as an unconditional `false`
- text remains explicitly unsupported in this slice
- no software-present or CPU-composite paths are reintroduced

Then run the smallest focused verification needed for the changed DS renderer audits and rebuild the DS ROM for manual emulator verification.

## Risks

The main risk is accepting more sprite cases than the actual DS hardware contract can support cleanly. The implementation should bias toward skipping rather than stretching the rules.

Another risk is mixing the existing debug marker OBJ path with real sprite submission. The first-pass implementation must keep sprite-id allocation and per-frame OAM state predictable so debug markers do not corrupt accepted sprite output.

## Follow-up Work

After this slice is stable:

1. decide whether bottom-screen hardware sprites should exist at all
2. add sprite slicing support if still desired
3. design a true DS hardware text path
