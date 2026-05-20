# DS Return Overlay Ownership Design

## Summary

Nintendo DS companion scenes currently have two return-to-menu problems:

1. The bottom-screen `Back` button is not clickable.
2. Pressing `B` can request the main menu more than once and throw `Runtime scene 'DemoDiscMainMenuDs' is already loaded.`

The correct fix is to make DS companion-scene return behavior owned by one DS-specific overlay controller instead of splitting responsibility across scene-authored return components and the DS scaffold button.

## Problem

The current DS companion-scene flow mixes two separate responsibilities:

- scene-authored `DemoDiscReturnToMenuComponent` instances attached by rendering showcase scene factories
- a DS scaffold `Back` button that also carries a return component

That split creates two different failure modes:

- `B` input can be observed by more than one return component in the same DS scene lifetime
- stylus input reaches the DS backend, but bottom-screen pointer routing misses because the backend reports bottom-screen touch in local screen coordinates instead of full dual-screen window coordinates

Trying to absorb this in `SceneManager` would hide the ownership bug instead of fixing it.

## Goals

- Make DS companion-scene return behavior owned by one runtime path.
- Keep `SceneManager` strict. Duplicate scene loads should still fail.
- Make the DS bottom-screen `Back` button clickable through the shared pointer/interactable path.
- Keep non-DS return behavior unchanged.
- Keep DS platform knowledge out of shared menu providers and scene ids.

## Non-Goals

- Do not add generic scene-load deduplication to `SceneManager`.
- Do not redesign shared menu navigation.
- Do not change authored default scene ids or scene-map semantics.
- Do not add a DS-only alternate input system outside the existing shared mouse/pointer contracts.

## Design

### 1. DS overlay owns return behavior

DS companion scenes will have exactly one active return owner:

- the DS bottom overlay controller

The DS overlay controller will handle:

- `B` return
- any other DS-specific back bind we explicitly keep, if needed
- pointer/touch activation from the bottom-screen `Back` button

Scene-authored `DemoDiscReturnToMenuComponent` instances will not remain active inside DS companion-scene variants.

### 2. Scene-authored return components are stripped from DS variants

The DS companion-scene scaffold generator will continue removing authored `DemoDiscReturnToMenuComponent` instances from top-screen content before the DS scene is serialized.

That means:

- base generated scenes can still author return components for non-DS usage
- DS variants will not keep those components alive
- the bottom overlay becomes the only DS return path

This preserves current authored scene structure while giving DS a single owner.

### 3. Replace scaffold button return wiring with one DS controller

The bottom `Back` button should not carry a generic scene-return component directly.

Instead:

- the button remains a visual + `InteractableComponent`
- a DS-specific controller component lives on the overlay root or button host
- that controller subscribes to the button interactable and listens for DS back input
- the controller requests the logical `DemoDiscMainMenu` scene id exactly once
- the active `SceneMapComponent` resolves it to `DemoDiscMainMenuDs`

This keeps all DS companion-scene return logic in one place.

### 4. Fix DS bottom-screen pointer coordinates at the backend

The DS input backend currently reads stylus coordinates from the Nintendo DS bottom screen in local `256x192` space. Shared pointer routing expects full window-space coordinates across the stacked dual-screen layout.

The DS backend will therefore publish stylus coordinates in full DS window space:

- `X` remains unchanged
- `Y` is offset to the bottom-screen window origin

For the current DS layout, that means bottom-screen touch must be reported with the bottom-screen vertical offset applied before it reaches shared mouse/pointer consumers.

This keeps the shared pointer system generic and fixes bottom-screen interactables, not just the `Back` button.

### 5. One-shot behavior belongs in the DS controller, not in SceneManager

The DS overlay controller will track whether it already requested a return during its current lifetime.

If a return load was already requested:

- additional `B` presses do nothing
- additional pointer releases on the button do nothing

This is not scene-manager deduplication. It is input ownership and component-lifetime control at the feature seam.

## Data Flow

### `B` return

1. DS input backend reports gamepad state.
2. DS overlay return controller polls the configured back bind.
3. If no return was requested yet, it resolves `DemoDiscMainMenu` through `SceneMapComponent.ResolveSceneId(...)`.
4. The controller requests one `SceneManager.LoadScene(..., SceneLoadMode.Single)`.
5. Further input during the same component lifetime is ignored.

### Touch return

1. DS input backend reports stylus position in full dual-screen window coordinates.
2. Shared pointer routing resolves the bottom-screen `InteractableComponent`.
3. The DS overlay return controller receives the button click.
4. If no return was requested yet, it resolves `DemoDiscMainMenu`.
5. The controller requests one scene load.

## Component Boundaries

### `NintendoDsInputBackend`

Responsible for:

- converting raw DS stylus coordinates into shared window-space mouse/pointer coordinates

Not responsible for:

- return-to-menu logic
- scene-loading policy

### `NintendoDsRenderingSceneScaffoldFactory`

Responsible for:

- stripping authored return components from DS companion-scene top roots
- emitting the bottom overlay structure
- attaching the DS-specific return controller

Not responsible for:

- manually loading scenes
- remapping scene ids itself

### DS overlay return controller

Responsible for:

- owning DS companion-scene return input
- handling `B` and button click
- making one mapped scene-load request

Not responsible for:

- global scene-load deduplication
- generic menu behavior outside DS companion scenes

## Error Handling

- If the DS overlay cannot find its required `InteractableComponent`, it should fail clearly instead of silently becoming non-clickable.
- If `SceneManager` is unavailable, the controller should throw the existing initialization failure.
- If `SceneMapComponent` does not map `DemoDiscMainMenu`, the logical id remains valid and loads the default menu id as designed.

## Testing

## Source-level verification

- Verify DS input backend applies the bottom-screen vertical offset before populating mouse/pointer coordinates.
- Verify DS scaffold source creates one DS return controller instead of attaching a generic return component directly to the button.
- Verify DS scaffold source still strips authored `DemoDiscReturnToMenuComponent` instances from top-scene roots.

## Behavior-level verification

- Verify a DS companion scene can request return with `B` exactly once.
- Verify repeated `B` presses during the same lifetime do not request a second load.
- Verify the bottom `Back` button click requests the mapped DS main menu exactly once.
- Verify pointer routing can hit bottom-screen interactables after the coordinate fix.

## Recommendation

Implement the DS return path as a DS-specific overlay controller plus a DS input-coordinate fix. Keep `SceneManager` strict and remove split ownership from DS companion scenes entirely.
