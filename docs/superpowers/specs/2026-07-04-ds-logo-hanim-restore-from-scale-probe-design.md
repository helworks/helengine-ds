# DS Logo `.hanim` Restore From Scale Probe

## Goal

Remove the current static Nintendo DS logo scale probe and restore the DS menu logo to the shared `DemoDiscLogoIdle.hanim` animation path.

## Current Context

- The DS logo is authored in `DemoDiscMainMenuSceneFactory.CreateNintendoDsTopScreenLogoEntity(...)`.
- It currently uses a fixed authored probe scale of `1, 1.25, 1`.
- The earlier `AnimationPlayerComponent` path was removed only for scale debugging.
- The DS runtime affine fixes should remain in place.

## Proposed Change

For the DS-specific logo entity:

- restore `LocalScale` to `1, 1, 1`
- reattach one `AnimationPlayerComponent`
- load `Animations/DemoDiscLogoIdle.hanim`
- set `PlayAutomatically = true`
- set `ShouldLoop = true`
- serialize the animation clip reference again

## Why This Approach

- returns the DS menu to the intended authored behavior
- keeps the debug probe fully removed instead of leaving stale DS-only branches
- preserves the runtime affine fixes already made during debugging

## Validation

1. Update the DS logo source audit back to `.hanim` expectations.
2. Regenerate the demo-disc main menu scene.
3. Build the DS ROM.
4. Launch melonDS and verify the logo animates again.

## Non-Goals

- No new debug toggles.
- No `.hanim` asset changes.
- No non-DS menu behavior changes.
