# DS Logo `.hanim` Restore

## Goal

Restore the Nintendo DS main-menu logo animation by using the existing `Animations/DemoDiscLogoIdle.hanim` asset on the DS-specific logo entity, now that the DS affine sprite path applies rotation correctly.

## Current Context

- The DS menu logo is authored in `DemoDiscMainMenuSceneFactory.CreateNintendoDsTopScreenLogoEntity(...)`.
- The shared menu factory already exposes:
  - `DemoDiscLogoIdleAnimationRelativePath`
  - `ApplyAnimationClipReference(...)`
  - `LoadRequiredAnimationClipAsset(...)`
- The DS logo currently keeps a fixed authored 180-degree orientation for debugging and does not attach an animation player.
- The DS runtime affine sprite bug was fixed in `NintendoDsRenderManager2D.cpp`, so animated rotation can now be exercised through the normal clip pipeline.

## Proposed Change

Attach one `AnimationPlayerComponent` to the DS top-screen logo entity and bind it to `Animations/DemoDiscLogoIdle.hanim`.

The component should:

- load the clip for live authored preview through `LoadRequiredAnimationClipAsset(...)`
- serialize the clip reference through `ApplyAnimationClipReference(...)`
- set `PlayAutomatically = true`
- set `ShouldLoop = true`

The DS logo entity keeps the current DS-authored setup otherwise:

- same entity name
- same sprite component
- same texture reference
- same fixed DS base orientation
- same positioning and sizing

## Why This Approach

- Reuses the existing per-platform animation asset path instead of adding another DS-only motion system.
- Keeps editor preview and baked runtime on the same clip contract.
- Minimizes scope to one authored scene factory path and one scene regeneration step.

## Validation

1. Regenerate the demo-disc main menu scene.
2. Build the DS ROM.
3. Launch in melonDS.
4. Verify the top-screen logo animates again and no longer exhibits the previous tile-content rotation mismatch.

## Non-Goals

- No changes to the `.hanim` asset format.
- No changes to non-DS menu logo behavior.
- No new debug toggles or alternate animation paths.
