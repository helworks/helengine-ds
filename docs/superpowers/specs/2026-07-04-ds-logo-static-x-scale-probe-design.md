# DS Logo Static X-Scale Probe

## Goal

Replace the current Nintendo DS logo animation with one static authored probe that keeps position and rotation fixed while applying one fractional scale only on the X axis.

## Current Context

- The DS logo is authored in `DemoDiscMainMenuSceneFactory.CreateNintendoDsTopScreenLogoEntity(...)`.
- That path currently attaches `AnimationPlayerComponent` using `Animations/DemoDiscLogoIdle.hanim`.
- We want to isolate affine scaling behavior without animation noise.

## Proposed Change

For the DS-specific logo entity only:

- remove the `AnimationPlayerComponent`
- keep the current fixed position
- keep the current fixed orientation
- set `LocalScale` to one fractional X-only probe value
- keep `LocalScale.Y = 1f`
- keep `LocalScale.Z = 1f`

No runtime toggles or extra debug systems are added.

## Why This Approach

- isolates the scaling issue from animation
- keeps the probe in the authored scene path the DS runtime already consumes
- minimizes changes to one city factory method plus scene regeneration

## Validation

1. Regenerate the demo-disc main menu scene.
2. Build the DS ROM.
3. Launch melonDS.
4. Inspect the top-screen logo for static fractional X scaling behavior.

## Non-Goals

- No `.hanim` changes.
- No new runtime debug controls.
- No non-DS menu changes.
