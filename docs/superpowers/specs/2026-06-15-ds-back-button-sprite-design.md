# DS Back Button Sprite Design

## Summary

The Nintendo DS companion-scene back button already exists in authored scene code, but its visible body is currently a `RoundedRectComponent`. That makes the button depend on a DS rounded-rectangle path that we do not want to extend for this task. The correct fix is to keep the current authored interaction and text behavior, replace the button body with a real `SpriteComponent`, and ensure the Nintendo DS hardware sprite path renders that sprite on the bottom screen.

This design does not add CPU rectangle rendering. It also does not broaden scope into a general DS button-skin system. The goal is narrower: make the authored DS back button visible and working by expressing its body through the sprite contract the DS backend should already support.

## Current State

The authored DS scaffold in `C:\dev\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs` creates the bottom-screen back button as:

- one host entity named `DemoDiscBottomScreenBackButton`
- one `InteractableComponent`
- one `NintendoDsReturnOverlayComponent`
- one `RoundedRectComponent` for the lilac button body
- one child `TextComponent` that renders `BACK`

The return behavior lives in `C:\dev\helprojs\city\assets\codebase\menu\NintendoDsReturnOverlayComponent.cs` and already uses the sibling `InteractableComponent` plus the standard platform return action. That interaction logic is not the problem.

On the DS runtime side, `src/platform/ds/NintendoDsRenderManager2D.cpp` already contains a hardware sprite path for `ISpriteDrawable2D`, including texture preparation, OBJ allocation, and bottom-screen submission. The current back button does not benefit from that path because its body is authored as a rounded rectangle instead of a sprite.

## Goal

Make the authored Nintendo DS back button body render as a visible lilac control in melonDS while preserving the existing tap behavior and `BACK` text.

Success means:

- the bottom-screen back button is visible
- the button body is sprite-backed, not rectangle-backed
- the `BACK` label still renders through text
- tapping the button still returns to the menu
- no CPU rendering path is added for this feature

## Non-Goals

This task does not:

- add Nintendo DS CPU rendering for rectangles
- redesign the shared engine `ButtonComponent`
- convert all DS UI elements from rounded rectangles to sprites
- introduce a general reusable DS button-skin framework
- change `NintendoDsReturnOverlayComponent` scene-navigation ownership

## Recommended Approach

Replace the authored `RoundedRectComponent` body with an authored `SpriteComponent` and keep the rest of the back-button composition intact.

This is the best fit because it aligns the authored scene with the DS backend capability we actually want to rely on. The scene already has working interaction and text. The missing piece is only the visual body. Moving that body onto a sprite keeps the change narrowly scoped, avoids introducing a second rendering strategy just for DS, and gives the DS runtime a clear contract for future UI chrome that must appear on hardware.

## Architecture

### Authored Scene Composition

Update `NintendoDsRenderingSceneScaffoldFactory.CreateDefaultBottomOverlay(...)` so the back button host keeps:

- `InteractableComponent`
- `NintendoDsReturnOverlayComponent`
- child `TextComponent` for `BACK`

but replaces:

- `RoundedRectComponent`

with:

- `SpriteComponent`

The `SpriteComponent` becomes the sole visual body for the back button. Its size should continue matching the existing authored bounds of `224 x 32`, and the host entity should keep the current bottom-screen placement unless the sprite swap exposes a small alignment issue.

### Asset Strategy

Add one dedicated texture asset for the DS back-button body. The first version should be sized for the existing authored control rather than solving reusable skinning up front.

The texture should visually encode the current lilac body and border treatment that the rounded rectangle was approximating:

- dark lilac interior
- lighter lilac border
- square corners

Because the current authored control already uses `Radius = 0`, a fixed sprite asset is sufficient for this slice.

### DS Runtime Rendering Contract

The DS runtime should render the back-button body only through the existing hardware sprite path in `NintendoDsRenderManager2D`.

If the authored button sprite reveals gaps in that path, the implementation may tighten:

- runtime texture preparation for the button asset
- bottom-screen OBJ allocation and reuse
- sprite budgeting or tile-span decomposition for the button size
- ordering between the button sprite and the existing bottom-screen text overlay

The implementation should not introduce a second special-case DS path for this button body. The sprite must use the normal `ISpriteDrawable2D` route.

### Text and Interaction

The `BACK` label remains a `TextComponent`. The existing text path is already proving itself in the current DS build, so this task should continue using it.

The existing `NintendoDsReturnOverlayComponent` remains the owner of:

- pointer press/release handling from the sibling interactable
- gamepad return handling
- scene-map-aware return-to-menu loading

No behavior change is required there unless the sprite swap exposes a hit-target mismatch, in which case the authored `InteractableComponent.Size` and visual sprite size must remain aligned.

## Alternatives Considered

### 1. Recommended: Replace The Authored Rounded Rectangle With A Sprite

This keeps the current scene structure and interaction behavior intact while moving the button body onto the DS rendering path we actually want. It solves the real missing visual with the smallest architecture change.

### 2. Add DS Rectangle Support For This Button

This would render the current `RoundedRectComponent` body directly on DS. It was rejected because the requirement for this slice is explicit: no CPU rendering, and the long-term direction for DS button chrome should be sprite-backed.

### 3. Add A General DS Button-Skin System Immediately

This would likely produce a broader reusable framework, but it is more scope than this bug needs. The current codebase already has one concrete DS-only button composition that can be fixed directly.

## Risks

### Sprite Size Pressure

The existing back button is `224 x 32`, which is much larger than a single DS OBJ shape. The runtime sprite path will need to express it as multiple hardware sprite tiles. The implementation must confirm the existing decomposition logic handles that authored size cleanly on the bottom screen.

### Layering Drift

The back-button text currently renders through the bottom-screen text path. The new sprite body must sit behind that text without obscuring it or fighting the debug overlay ordering.

### Asset/Hitbox Drift

If the sprite art dimensions diverge from the `InteractableComponent` size, the visible control and interactive target can feel broken even when navigation still works. The authored size and visual dimensions must stay synchronized.

## Testing Strategy

### Source-Audit Coverage

Add or update focused audit coverage that proves the DS scaffold authors the back-button body as a `SpriteComponent` instead of a `RoundedRectComponent`.

If the DS runtime sprite path gains a new invariant specifically needed for this button, add focused audit coverage for that invariant in `builder.tests`. Do not add broad tests unrelated to the button or the sprite path changes it requires.

### Build Verification

Rebuild the DS target through the normal editor-owned build flow targeting:

- `C:\dev\helprojs\city\project.heproj`
- platform `ds`
- output `C:\dev\helprojs\city\ds-build`

### Visual Verification

Visual verification is user-reported in melonDS. The workflow for this task is:

- launch the rebuilt ROM in melonDS
- ask the user what is visible
- do not use window captures or screenshots as the verification source

Manual success criteria:

- the lilac back button body is visible on the bottom screen
- the `BACK` text remains visible
- tapping the button returns to the DS menu

## Files Expected To Change

In `C:\dev\helprojs\city`:

- `assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs`
- one new or updated texture asset for the back-button body

In `C:\dev\helworks\helengine-ds`:

- `builder.tests\...` focused DS scaffold and possibly sprite-path audit coverage
- `src\platform\ds\NintendoDsRenderManager2D.cpp` only if the existing sprite path needs targeted fixes to support the authored button sprite correctly

## Acceptance Criteria

The work is complete when all of the following are true:

- the DS-authored back button body no longer depends on `RoundedRectComponent`
- the body is authored as a `SpriteComponent`
- the DS build completes successfully
- melonDS shows a visible lilac back button
- the `BACK` label remains readable
- pressing the back button still returns to the menu
- no CPU rectangle rendering path was added for this feature
