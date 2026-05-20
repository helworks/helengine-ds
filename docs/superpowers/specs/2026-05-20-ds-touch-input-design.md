# Nintendo DS Touch Input Design

## Goal

Add Nintendo DS stylus support so touch behaves like a normal mouse across the engine, while also feeding the shared pointer contract used by generic 2D interactables.

## Scope

This design covers Nintendo DS platform input capture only.

- It does include stylus-to-shared-input translation in the DS input backend.
- It does include regression coverage in `helengine-ds` source-audit tests.
- It does not include DS-specific menu logic.
- It does not include shared-engine rewrites of `MenuComponent`, `PointerInteractionSystem`, or `InputSystem`.
- It does not include gesture support, drag thresholds, long-press semantics, or multi-touch.

## Current State

The generated-core input contract already has both mouse and pointer state:

- `MenuComponent` uses mouse-style APIs such as `WasMouseLeftButtonPressed()`, `WasMouseLeftButtonReleased()`, and `GetMouseX()/GetMouseY()`.
- `PointerInteractionSystem` and generic 2D interactables use the shared `InputPointerState`.

The current DS backend captures hardware buttons as a single gamepad and explicitly initializes mouse and pointer state to neutral values every frame. That means D-pad menu navigation works, but stylus interaction is discarded at the platform boundary.

## Design Decision

Nintendo DS touch will be implemented as backend-level mouse parity.

The DS input backend will read stylus hardware state every frame and populate both:

- `frame.Mouse`
- `frame.Pointer`

from the same raw stylus sample.

This keeps the contract simple:

- systems written against mouse input start working on DS touch immediately
- systems written against pointer input also work immediately
- no DS-specific branching is added to menu logic or interactable routing

## Alternatives Considered

### 1. Pointer-only DS touch

Only populate `frame.Pointer` and leave `frame.Mouse` neutral.

Rejected because the current menu system is intentionally mouse-driven. This would force a shared-engine refactor before DS touch becomes useful in the menu.

### 2. Shared-engine menu refactor to pointer-first input

Refactor menu and other consumers to stop using mouse APIs and use pointer APIs everywhere.

Rejected for this slice because it is broader than the platform problem and increases cross-platform regression risk without being required for DS touch support.

### 3. Separate DS-side mouse emulation layer after capture

Keep the backend stylus-only and synthesize mouse state later in the stack.

Rejected because it spreads responsibility across layers and makes the platform input contract harder to reason about.

## Architecture

### Nintendo DS backend ownership

`NintendoDsInputBackend` remains the single owner of raw DS hardware sampling.

It will:

- scan pad buttons as it already does
- read stylus position and pressed state from libnds
- translate stylus state into shared mouse and pointer state
- preserve previous stylus state so edge transitions and deltas are stable across frames

No new runtime service is required.

### Mouse contract mapping

The stylus will map to the shared mouse contract as follows:

- Stylus down maps to `MouseState.LeftButton = Pressed`
- Stylus up maps to `MouseState.LeftButton = Released`
- Stylus X/Y map to mouse X/Y in DS window-space pixels
- Mouse wheel and non-left buttons remain neutral

This is the compatibility layer that makes the current menu system work unchanged.

### Pointer contract mapping

The same stylus sample will also populate `InputPointerState`:

- `Connected = true`
- `X/Y` from the stylus position
- `DeltaX/DeltaY` from the difference against the previous stylus position
- `Primary` button bit follows stylus down/up
- `ScrollDelta = 0`

This is the path used by generic shared 2D interactables.

### Previous-frame state

The DS backend will cache:

- previous stylus pressed state
- previous stylus X/Y position
- whether a valid stylus position has been observed yet

This cache is required so the backend can emit stable:

- press edges
- release edges
- deltas

without depending on later systems to reconstruct platform-specific history.

## Behavioral Rules

### While stylus is down

- Mouse left button is pressed
- Pointer primary button is down
- Mouse and pointer coordinates follow the current stylus sample
- Pointer deltas are computed from the previous stylus sample

### On first stylus-down frame

- Mouse left button transitions to pressed
- Pointer primary button transitions to down
- Delta should not spike from an uninitialized position

The first valid touch sample should seed the cached previous position before calculating deltas.

### While stylus is up

- Mouse left button is released
- Pointer primary button is up
- Last known coordinates are preserved so release handling still resolves against the last touch position
- Pointer delta returns to zero

Preserving the last position on release is important because shared menu and interactable release handling often depends on where the press ended.

## File Impact

### `helengine-ds`

- `src/platform/ds/NintendoDsInputBackend.hpp`
  - add cached stylus state fields
- `src/platform/ds/NintendoDsInputBackend.cpp`
  - sample stylus hardware
  - build shared mouse state from touch
  - build shared pointer state from touch
- `builder.tests/NintendoDsInputBackendSourceAuditTests.cs`
  - extend source audits to verify the stylus path and state mapping

### Shared engine

No shared-engine source changes are planned for this slice.

That is deliberate. The DS backend should conform to the existing shared input contracts rather than forcing shared-engine rewrites.

## Error Handling

If the DS stylus is not currently pressed, the backend will emit released mouse/pointer button state and zero delta while preserving the last valid position.

No fallback defaults should be synthesized beyond that contract, and no runtime exceptions are expected from normal no-touch frames.

## Testing Strategy

This repository already uses source-audit tests for DS platform seams. The touch-input slice will follow that pattern.

Tests will verify:

- DS backend source reads stylus state in addition to button state
- stylus state is translated into `MouseState`
- stylus state is translated into `InputPointerState`
- previous stylus position/press caches exist in the header and are used in the source
- neutral wheel and non-primary-button behavior remains explicit

Because this is a platform backend source-audit slice, the tests assert the intended implementation contract directly from DS source files.

## Success Criteria

The feature is complete when all of the following are true:

1. The DS stylus populates shared mouse left-button and X/Y state.
2. The DS stylus populates shared pointer position, delta, and primary-button state.
3. The main menu can be tapped without DS-specific menu code.
4. Generic pointer-driven 2D interactables can consume DS touch through the existing shared pointer contract.
5. Focused DS input backend audit tests pass.

## Risks

### Coordinate-space mismatch

If DS stylus coordinates are not authored in the same window-space used by shared hit testing, touch targets will appear offset. Verification during implementation should compare DS stylus coordinates against the current dual-screen viewport layout.

### Release-position mismatch

If release resets coordinates to zero instead of preserving the last valid touch point, release-driven actions may miss hovered targets. The design explicitly avoids that.

### Over-scoping into shared input refactors

The main risk to delivery is turning this into a shared input architecture rewrite. This design intentionally avoids that by keeping the change inside the DS backend.
