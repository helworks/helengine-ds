# DS Main Menu 2D Rendering Design

## Goal

Make Nintendo DS builds boot into the existing authored `DemoDiscMainMenu` scene and visually render that menu on the top screen through a first-pass DS 2D renderer.

## Constraints

- The change is DS-only.
- Shared project startup-scene defaults must remain unchanged.
- The authored menu scene remains `assets/scenes/DemoDiscMainMenu.helen`.
- The first pass is visual-only. Menu input and navigation are out of scope.
- All menu rendering should stay on the top screen for now.
- Mixed top-screen 2D plus 3D composition is out of scope for this slice.

## Current Problem

The DS runtime can now boot packaged scenes and render 3D showcase content, but it does not yet render the authored menu scene.

Two gaps block that:

- DS startup selection still needs to point at the authored menu scene instead of the current showcase startup flow.
- `NintendoDsRenderManager2D` is still a no-op surface that accepts draw requests without presenting sprites, text, or rounded rectangles.

Because of that, the existing authored `DemoDiscMainMenu` scene cannot currently appear on DS even though the scene asset and its supporting authored content already exist.

## Existing Menu Scene Shape

The authored `DemoDiscMainMenu` scene already exists and should be treated as the source of truth for this slice.

Its visual payload currently depends on:

- `SpriteComponent` for the logo image
- `RoundedRectComponent` for panel surfaces, top bands, and item backgrounds
- `TextComponent` for headings, labels, and descriptions

The scene also includes menu-runtime support components such as `MenuComponent`, `MenuPanelComponent`, `MenuItemComponent`, `MenuSelectedDescriptionComponent`, `ScrollComponent`, `ClipRectComponent`, `ViewportComponent`, `ReferenceCanvasFitComponent`, and `FPSComponent`.

Those broader runtime components do not require full DS behavioral support in this slice. They only need to remain compatible enough for the authored scene to construct and expose its current visual state.

## Recommended Approach

Implement a DS-specific startup-scene override and a narrow top-screen 2D presentation path that supports only the primitives the authored menu scene currently uses.

### 1. DS Startup Scene Override

The shared project startup scene remains unchanged for non-DS platforms.

The DS builder or manifest path becomes responsible for overriding the DS startup scene to `DemoDiscMainMenu`. That keeps the authored project flow intact while making DS boot into the menu scene.

The runtime boot path should continue loading the packaged startup scene through the existing `NintendoDsPackagedAssetLoader` startup-scene seam. The DS runtime should not need a special ad hoc loader for the menu.

### 2. Narrow DS 2D Renderer

`NintendoDsRenderManager2D` should stop being a placeholder and should render the minimum visual primitives needed by the authored menu scene:

- textured sprite draw for the menu logo
- text draw for static menu labels and descriptions
- filled rounded-rectangle presentation for menu surfaces and row backgrounds

This renderer should stay intentionally narrow. It is not a goal in this slice to support arbitrary DS UI scenes or every 2D feature already available on other platforms.

### 3. Top-Screen 2D Presentation Mode

When the active scene is `DemoDiscMainMenu`, the DS host should preserve the top screen in a 2D presentation mode instead of immediately switching to the existing 3D preparation path.

The bottom screen remains out of scope and should not be expanded as part of this feature.

### 4. Future 2D-to-3D Handoff Seam

This slice should preserve a clean handoff point for later scene transitions.

If a future runtime-driven scene load leaves the menu and enters a 3D showcase scene, the DS runtime should have a clear seam for switching the top screen from the menu-oriented 2D mode into the existing 3D mode.

That handoff does not need to be fully exercised by input in this first pass, but the architecture should not hardcode the 2D setup as a process-wide permanent state.

## Architecture Changes

### DS Builder and Startup Manifest

The DS packaging layer should own startup-scene remapping to `DemoDiscMainMenu`.

This keeps DS-specific policy in `helengine-ds` instead of mutating shared city project defaults or changing cross-platform build behavior.

### NintendoDsBootHost

`NintendoDsBootHost` should recognize the DS menu-scene startup path and prepare the top screen for menu rendering in 2D mode.

The host should continue using the existing checkpointed startup flow and packaged startup-scene loading path. The menu scene should be just another packaged startup scene from the host's perspective, with only the display preparation differing from the normal 3D showcase path.

### NintendoDsRenderManager2D

`NintendoDsRenderManager2D` should become the DS runtime-owned surface for the menu's visual primitives.

It should:

- build runtime textures that are usable by the DS 2D path
- render menu sprites on the top screen
- render static text on the top screen
- render rounded menu panels and item backgrounds on the top screen

It should not attempt to implement full menu interactivity, scrolling behavior, or generalized editor-style 2D composition in this slice.

## Failure Policy

This slice should fail loudly when the authored menu scene depends on unsupported DS 2D assumptions.

Examples:

- unsupported texture requirements for the menu logo
- unsupported drawable state required by the authored scene
- startup-scene remapping that resolves to a missing packaged menu asset

The DS path should not silently swallow those problems and pretend the feature is working.

## Testing Strategy

Add or extend focused verification at three levels.

### Builder Verification

Prove that DS startup-scene selection resolves to `DemoDiscMainMenu` without changing non-DS startup behavior.

### Runtime Source Verification

Prove that `NintendoDsBootHost` exposes:

- a menu-oriented top-screen 2D preparation path
- the existing top-screen 3D preparation path for non-menu scenes or later scene transitions

Also prove that `NintendoDsRenderManager2D` is no longer a pure no-op for the menu primitives it now claims to support.

### End-to-End Verification

Build the DS city project, launch the ROM in `melonDS`, and confirm that the authored `DemoDiscMainMenu` visually appears on the top screen.

The first pass does not require working menu input. Visual presentation is the success criterion.

## Non-Goals

- Menu input or navigation
- Scene loading from menu actions
- Bottom-screen menu presentation
- General-purpose DS UI rendering for arbitrary scenes
- Mixed top-screen 2D plus 3D composition
- Broad support for every runtime behavior attached to menu helper components

## Success Criteria

- DS builds boot into the authored `DemoDiscMainMenu` scene.
- The top screen visually presents the menu using DS-native 2D rendering.
- The logo sprite, panel surfaces, item backgrounds, and text labels are visibly rendered.
- Shared startup-scene behavior for non-DS platforms remains unchanged.
- The DS boot path and existing 3D runtime path do not regress.
