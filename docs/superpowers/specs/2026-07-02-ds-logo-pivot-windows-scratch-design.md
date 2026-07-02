# DS Logo Pivot Windows Scratch Design

## Goal

Build a disposable Windows scratch app that reproduces the DS logo tile-rotation math in isolation so the remaining pivot/translation error can be debugged visually without DS hardware ambiguity.

## Current State

- The DS runtime now rotates the top logo and the behavior is close to correct.
- The remaining bug is small but visible: during rotation the stitched logo drifts slightly toward the bottom-right, which means the pivot math is still off.
- Reproducing this directly on DS is slow because every hypothesis requires a platform rebuild and manual visual validation.
- The current issue is no longer about asset cooking, animation presence, or whether affine sprite rotation works at all. It is about the exact transform math used to place multiple rotating tiles around one logical sprite center.

## Design

### Scope

The scratch repro stays intentionally narrow:

- live under `scratch` in `helengine-ds`
- target Windows desktop only
- use a synthetic checker image instead of the real logo
- reproduce the current DS tile-placement math as closely as practical
- expose the key pivot assumptions as runtime toggles

This is a debugging tool, not a product feature and not a reusable engine subsystem.

### Scratch App Behavior

The scratch app will open a simple window and draw one logical image composed from separate tiles.

Initial layout:

- one checker image divided into `2x2` tiles
- one visible global pivot marker
- one visible whole-image bounding box
- one visible center marker for each tile
- one visible outline for each tile

Animation:

- the logical image rotates continuously around one parent pivot
- each tile is also drawn with the same local visual rotation the DS path applies
- the CPU computes each tile position using the same orbit formula the DS runtime currently uses

The user should be able to see immediately whether the tiled image behaves like one rigid body or drifts apart.

### Debug Toggles

The scratch app should expose a small set of toggles or hotkeys so one assumption can be changed at a time:

- angle sign
- anchor-offset rule
- raw versus rounded parent center
- tile grid mode: `2x2` versus `3x3`
- optional pause/step mode for frame-by-frame inspection

The app should also display the active mode in a small on-screen legend so screenshots are self-describing.

### Math Source

The scratch app is meant to validate the transform model, so the math should mirror the current DS path rather than invent a cleaner abstraction first.

The first implementation should mirror:

- parent center calculation
- tile local-center calculation
- orbit rotation sign
- tile anchor offset subtraction
- final rounding before draw submission

Once the app reproduces the DS bug visually, it becomes the fast validation harness for trying alternative formulas.

### Rendering Strategy

The rendering path should stay as small as possible:

- one scratch desktop app
- immediate-mode style drawing is fine
- use a generated checker texture or generated colored quads
- no asset pipeline dependency is required for the first version

The point is math visibility, not content fidelity.

## Alternatives Considered

### Reproduce Only On DS

This keeps validation closest to target hardware, but iteration is too slow for a bug that now appears to be a small geometric mismatch.

### Use The Real Logo Asset

This would match the target content more closely, but it adds asset and visual noise when the current need is to understand transform drift.

### Build Only A Unit-Test Math Probe

This would be fast to code, but the failure is fundamentally visual. A numeric-only harness is weaker because it makes it harder to notice rigid-body drift and pivot bias.

## Recommendation

Build the disposable Windows scratch app first, using a checker image and a `2x2` tiled composition. Make it mimic the current DS formula exactly and expose a few toggles for the known pivot assumptions.

This is the fastest path to:

1. reproduce the bug in a fast iteration loop
2. identify which assumption is wrong
3. port the corrected formula back into the DS runtime with higher confidence

## Verification

The smallest useful verification is:

- launch the scratch app on Windows
- confirm the baseline mode reproduces the same style of pivot drift seen on DS
- toggle one assumption at a time
- identify at least one mode where the tiled checker rotates as one rigid image around the intended pivot
- use that winning formula as the candidate fix for the DS runtime
