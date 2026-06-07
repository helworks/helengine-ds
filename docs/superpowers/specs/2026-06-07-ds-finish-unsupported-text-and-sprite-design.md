# DS Finish Unsupported Text And Sprite Design

## Goal

Finish the remaining unsupported Nintendo DS `TextComponent` and sprite cases that still show up in the bottom-screen overlay counters, while keeping the renderer hardware-only.

This pass does **not** add rounded-rectangle support. Rounded rectangles remain unsupported and visible through the existing `UR` counter and magenta-marker behavior.

## Problem Statement

The current DS backend is policy-correct but still incomplete:

- bottom-screen text is partially hardware-backed
- first-pass sprite support exists
- remaining unsupported text and sprite drawables are still visited every frame
- those unsupported drawables still consume CPU time before being skipped
- the overlay now reports that unsupported work explicitly through `UT` and `US`

The immediate objective is to eliminate the known remaining unsupported text and sprite cases in the active runtime scene before optimizing the surviving path further.

## Scope

In scope:

- trace the exact remaining unsupported DS text drawables
- trace the exact remaining unsupported DS sprite drawables
- group those failures by shared reject reason
- implement only DS-honest support extensions for those grouped cases
- keep the `UT`, `US`, and `UR` overlay counters so progress is measurable
- remove temporary tracing after the unsupported text and sprite cases are understood and handled

Out of scope:

- rounded-rectangle support
- software fallback of any kind
- generic "support everything" widening
- broader text-path performance work after unsupported counts reach zero or the expected floor

## Approach

### 1. Exact Failure Tracing

Add temporary debug-only tracing for unsupported text and sprite drawables.

The trace should identify:

- drawable category: text or sprite
- parent or entity name when available
- screen target
- reject reason code
- the small set of authored properties that explain the rejection

The trace must stay rate-limited and category-aware so it remains readable. It should be suitable for melonDS stdout/stderr capture and should not reintroduce noisy on-screen console spam.

### 2. Grouped Rule Expansion

After tracing, support should be added by grouped failure mode, not per-instance hacks.

Examples of acceptable grouped fixes:

- one more bottom-screen text alignment mode if it maps cleanly to the DS text background
- one additional sprite source-rect or placement rule if it maps cleanly to DS OBJ hardware
- one more allowed text-color or sprite-color mode only if it is honestly expressible on DS hardware

Examples of unacceptable fixes:

- special-casing one named UI element
- recreating unsupported authored behavior in software
- adding "best effort" silent fallback branches

### 3. Verification Contract

This pass is complete when:

- the traced remaining unsupported text cases are either supported or intentionally rejected with a documented DS limitation
- the traced remaining unsupported sprite cases are either supported or intentionally rejected with a documented DS limitation
- the active runtime scene shows the expected `UT` and `US` reduction
- temporary tracing is removed
- focused tests and the DS ROM rebuild pass

## Data And Diagnostics Contract

The existing DS overlay remains the main runtime truth source:

- `Tx`: DS text-path milliseconds
- `S`: DS sprite-path milliseconds
- `UT`: unsupported text count
- `US`: unsupported sprite count
- `3D`: 3D geometry milliseconds
- `F`: 3D flush milliseconds
- `P`: present milliseconds
- `UR`: unsupported rounded-rectangle count

Temporary tracing may supplement the overlay during investigation, but the overlay stays in place after the tracing is removed.

## Risks

### Over-broad support expansion

If tracing is skipped and support is widened generically, the backend can drift back toward fake flexibility. This pass must stay anchored to real failing cases and real DS hardware behavior.

### Performance confusion

Reducing `UT` and `US` should lower wasted frame cost, but this pass is not the main optimization pass. If `Tx` or `S` remain high after unsupported counts fall, that becomes a separate performance task.

### Trace noise

If temporary tracing is not aggressively scoped, it will be hard to extract the actual remaining failure groups. The trace output must be compact and deduplicated enough to act on.

## Testing

Required verification:

- focused editor tests for the DS-specific overlay labels when needed
- focused DS source audits for any new profile counters or mapping changes
- rebuilt DS ROM through the shared `build-platform.ps1` flow
- runtime confirmation in melonDS that the `UT` and `US` counters move in the expected direction

## Expected Outcome

After this pass, the current DS runtime scene should have:

- fewer or zero unsupported text drawables
- fewer or zero unsupported sprite drawables
- unchanged rounded-rectangle unsupported behavior
- clearer evidence about whether the next task should be performance optimization instead of capability completion
