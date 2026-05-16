# DS Dual-Screen Menu And Camera-Bound Viewports Design

## Goal

Introduce a shared engine viewport feature that can optionally bind presentation to a specific camera and optionally apply reference-canvas scaling, then use that feature to build a separate Nintendo DS menu scene that intentionally uses both screens.

This slice should solve two real problems:

- the current shared desktop-authored menu scene does not scale down to `256x192` cleanly and produces unreadable DS text
- the current engine/runtime contract does not let authored scene data say which camera owns which viewport region, which blocks clean dual-screen composition on DS

## Current Context

The engine already has a `ViewportComponent`, but it does not serve as the single authored contract for viewport placement, scaling, and camera targeting.

The engine also has `ReferenceCanvasFitComponent` and `ReferenceCanvasFitSnapshot`, which currently own the reference-canvas scaling behavior used by the desktop menu scene.

The DS backend currently assumes a simpler presentation model:

- the 3D path effectively picks the first camera
- the first DS menu pass was made to render on the top screen only
- the desktop menu scene is still structurally rooted in a `1280x720` layout model that is inappropriate for DS readability

The user wants two architectural corrections:

1. `ViewportComponent` should become the shared engine feature that can optionally bind to a camera and optionally enable scaling.
2. Nintendo DS should use a separate generated menu scene that is authored correctly for dual-screen use instead of shrinking the desktop menu composition.

## Non-Goals

This slice should not:

- add a large general scene-alias framework
- promise automatic migration of all existing authored scenes away from `ReferenceCanvasFitComponent`
- support one viewport rectangle spanning across both DS screens in the first pass
- redesign the desktop menu scene unless needed to keep the shared generator structure coherent

## Requirements

### Shared Engine Requirements

`ViewportComponent` must become the single authored scene contract for:

- viewport placement
- optional reference-canvas scaling
- optional camera binding

The component should preserve existing implicit behavior where possible, but it must also support an explicit camera-bound mode so authored scenes can target a named or direct camera reference instead of relying on camera ordering.

`ReferenceCanvasFitComponent` should stop being the authored contract for newly generated menu scenes. Generator-owned scenes should emit only `ViewportComponent` for viewport and scaling behavior.

### Nintendo DS Requirements

Nintendo DS must support two independently useful screens in the same frame using authored scene data.

The normalized viewport contract for DS should treat the presentation space as vertically stacked:

- `(0, 0, 1, 1)` maps to the full top screen
- `(0, 1, 1, 1)` maps to the full bottom screen

Partial rectangles within either screen should use the same math. A viewport that spans across both screens in one rectangle should be rejected or treated as unsupported in this slice.

### Menu Scene Requirements

Nintendo DS must stop using the desktop menu scene as its startup menu presentation.

Instead, DS should use a separate generated scene with:

- a top-screen camera and viewport for logo/title only
- a bottom-screen camera and viewport for the interactive menu list

This scene should be authored specifically for DS readability. It should not inherit the desktop `1280x720` menu layout assumptions.

### Scene Routing Requirements

Startup scene routing and return-to-menu routing must resolve the correct platform-specific menu scene.

This should be implemented as a focused shared concept for resolving the current platform's menu/home scene target, not as a hardcoded DS exception buried in gameplay or menu logic.

## Proposed Architecture

### 1. Unified `ViewportComponent`

`ViewportComponent` becomes the single high-level scene component for display region ownership.

It should own:

- the viewport rectangle
- the binding mode for how that rectangle is resolved
- the optional reference width and height used for scaling
- the optional camera binding information
- the scaling behavior toggle or mode

This unifies concerns that are currently split between viewport placement and reference-canvas fitting.

The conceptual model should be:

- the viewport decides where content appears
- the bound camera decides what content is rendered there
- optional scaling decides how authored layout coordinates are adapted to that viewport

This is the core engine feature in this slice. DS is simply the first platform that strongly needs the full model.

### 2. Explicit Camera-Bound Viewports

The engine should preserve existing implicit viewport behavior for scenes that do not opt into camera binding.

In addition, `ViewportComponent` should support an explicit camera-bound mode. In that mode, the authored viewport points at a specific camera entity or camera component. The renderer and relevant runtime systems should use that binding instead of assuming first-camera ordering or ancestor inference.

This feature is shared-engine scope, not DS-only scope. It should be authored in shared scene data and available to all backends.

For DS, this enables one scene to define:

- one camera for the top screen
- one camera for the bottom screen
- one viewport root bound to each camera

### 3. Viewport-Owned Scaling

The current `ReferenceCanvasFitComponent` behavior should be absorbed into `ViewportComponent`.

`ViewportComponent` should optionally expose reference width and reference height along with a scaling mode that applies the same kind of reference-canvas fit behavior currently used by menu layout.

The goal is not to preserve the old type forever. The goal is to move authored layout scaling under the same scene contract that owns viewport placement and camera targeting.

This keeps menu generators simpler and gives the engine one consistent way to reason about screen-space layout adaptation.

### 4. DS Dual-Screen Mapping

The DS backend should interpret viewport rectangles against a vertically stacked normalized display space:

- `x` in `0..1`
- `y` in `0..2`
- `0..1` in `y` maps to the top screen
- `1..2` in `y` maps to the bottom screen

This keeps the scene contract shared. No DS-specific "screen" enum is needed in authored scene data for the first pass.

The DS runtime/backend is responsible for:

- resolving whether a viewport belongs to the top or bottom screen
- mapping the normalized rectangle into the correct physical screen rectangle
- rejecting unsupported cross-screen viewport spans

### 5. Separate DS Menu Scene Generation

The menu generator path should produce a separate DS-specific menu scene instead of reusing the desktop menu scene.

The DS menu scene should contain:

- a top presentation root with a `ViewportComponent` covering `(0, 0, 1, 1)` and bound to a top camera
- top-screen content limited to logo and title
- a bottom presentation root with a `ViewportComponent` covering `(0, 1, 1, 1)` and bound to a bottom camera
- bottom-screen content limited to the interactive menu list and related readable UI

This generator should author the scene correctly up front rather than relying on migration or build-time rewriting of the desktop menu scene.

This also means the generator should stop emitting `ReferenceCanvasFitComponent` for these new scenes and should instead express any needed scaling through `ViewportComponent`.

### 6. Menu Scene Routing

The runtime should resolve "main menu" or "return to menu" through a small shared platform-aware menu scene routing seam.

The scope should stay narrow:

- startup scene selection can resolve the correct generated menu scene for the target platform
- return-to-menu can resolve the same platform-aware menu scene

This should not become a general-purpose scene alias registry in this slice. A focused "resolve current platform menu scene" contract is enough.

## Approach Options

### Option 1: Unify `ViewportComponent`, add DS menu generator, add platform-aware menu routing

This is the recommended approach.

Benefits:

- solves the DS readability issue structurally instead of cosmetically
- lands the shared camera-bound viewport feature in a coherent way
- removes the split between viewport placement and reference scaling in generator-authored scenes
- gives DS dual-screen support through normal authored scene data

Trade-offs:

- touches both shared engine scene contracts and DS backend behavior
- requires generator and routing work in addition to rendering changes

### Option 2: Keep `ReferenceCanvasFitComponent`, add separate camera binding elsewhere, add a DS-only menu generator

Benefits:

- slightly smaller first code change in the engine

Trade-offs:

- keeps viewport, scaling, and camera-targeting concerns fragmented
- creates a weaker long-term scene model
- makes future split-screen and multi-camera authored layout harder to reason about

This is not recommended.

### Option 3: Reuse the desktop menu scene and keep tuning scaling and texture settings

Benefits:

- smallest generator change

Trade-offs:

- does not solve the structural mismatch between desktop and DS layouts
- keeps text readability dependent on shrinking a desktop composition
- does not address the shared camera-bound viewport feature cleanly

This is not recommended.

## Recommended Design

Implement Option 1.

Treat this as one coherent feature family:

- a shared engine `ViewportComponent` upgrade
- a DS-specific dual-screen menu scene generator
- a focused shared menu scene routing seam

This keeps the system understandable:

- authored scene data says where content appears
- authored scene data can say which camera owns that presentation
- authored scene data can opt into scaling behavior
- DS gets a purpose-built menu scene rather than inheriting the wrong layout

## Data And Runtime Flow

### Shared Scene Authoring Flow

1. A generator emits scene roots with `ViewportComponent`.
2. Each viewport root declares:
   - viewport rectangle
   - whether scaling is enabled
   - the reference dimensions if scaling is enabled
   - whether a camera is explicitly bound
   - the target camera when explicitly bound
3. The runtime loads the scene and resolves the viewport contract.

### Rendering Flow

1. Runtime/render systems gather active viewport roots.
2. Each viewport resolves its target rectangle for the current backend.
3. If the viewport is camera-bound, the bound camera becomes the source camera.
4. The backend renders that camera's 2D and or 3D content into the resolved region.
5. On DS, the backend maps the region to the top or bottom hardware screen.

### Menu Routing Flow

1. Build/startup chooses the platform's menu scene target.
2. DS startup loads the DS menu scene instead of the desktop menu scene.
3. Any runtime return-to-menu path resolves through the same platform-aware menu scene target.

## Error Handling

The first slice should fail clearly for unsupported authored states.

Expected guarded cases:

- a DS viewport rectangle that spans across both screens
- an explicit camera-bound viewport that refers to a missing camera target
- a DS menu routing request that cannot resolve a platform-specific menu scene

These failures should produce direct diagnostics rather than silent fallback behavior.

## Testing Strategy

### Shared Engine Tests

Add or update tests that verify:

- `ViewportComponent` can round-trip viewport, scaling, and camera-binding settings
- systems that previously consumed `ReferenceCanvasFitComponent` now resolve equivalent behavior from `ViewportComponent`
- explicit camera-bound viewport selection wins over implicit camera ordering

### Generator Tests

Add tests that verify the DS menu generator emits:

- a top viewport root with top camera binding
- a bottom viewport root with bottom camera binding
- top content limited to logo/title
- bottom content containing the interactive menu list
- no `ReferenceCanvasFitComponent` in the DS-generated scene

### DS Backend Tests

Add tests that verify:

- full-screen top and bottom viewport rectangles map to the expected DS screen targets
- unsupported cross-screen rectangles are rejected clearly
- both bound cameras participate in one frame

### Integration Verification

Build the DS project and verify in `melonDS`:

- startup enters the DS-specific menu scene
- top screen shows logo and title only
- bottom screen shows the interactive menu list
- return-to-menu resolves back to the DS menu scene

## Compatibility And Migration

This slice should prefer generator correctness over broad migration machinery.

Menu scenes should be regenerated correctly under the new design rather than migrated at load time.

Existing non-menu scenes that do not use the new camera-bound viewport behavior should keep working through the existing implicit behavior path.

Wider migration away from `ReferenceCanvasFitComponent` for arbitrary authored scenes can happen later if needed, but it is out of scope for this slice.

## Success Criteria

This design is successful when:

- `ViewportComponent` is the shared authored contract for viewport area, optional scaling, and optional camera binding
- DS can render two independent camera presentations in one frame using authored viewport data
- DS menu startup uses a separate DS-authored/generated scene
- the top screen shows logo/title only
- the bottom screen shows a readable interactive menu list
- return-to-menu resolves to the correct platform-specific menu scene

