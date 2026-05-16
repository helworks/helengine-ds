## Overview

Nintendo DS currently forces a startup-time choice between pure 2D menu presentation and top-screen 3D presentation. That model blocks the next runtime requirement: allowing 2D camera queues to render as an overlay on the same physical screen that owns the DS hardware 3D pass.

This slice adds a DS-specific mixed presentation path with one explicit hardware constraint:

- the DS has one real 3D engine
- only one physical screen can own that hardware 3D pass in a frame
- both screens can still present 2D

The design therefore makes hardware 3D screen ownership an automatic per-frame decision instead of a startup-scene mode. When a frame contains 3D on both screens, top wins. When only bottom contains 3D, bottom gets the hardware 3D pass. Any 2D camera queue targeting the chosen 3D screen renders after 3D as an overlay. The non-3D screen remains 2D-only for that frame.

## Goals

- Allow 2D overlay on top of hardware 3D on Nintendo DS.
- Support the overlay behavior on either physical screen.
- Auto-select the DS hardware 3D target screen per frame.
- Preserve existing shared engine scene and viewport semantics.
- Keep the first slice DS-specific and narrowly scoped.

## Non-Goals

- True simultaneous hardware 3D on both DS screens.
- Frame-alternating fake dual-screen 3D.
- Arbitrary 2D and 3D interleaving by depth.
- New shared scene schema or new authored scene component types.
- A generalized cross-platform compositor abstraction.

## Hardware Constraint

Nintendo DS has one hardware 3D engine and two 2D display engines. The implementation must assume:

- only one screen can receive the hardware 3D output in a frame
- both screens can still receive 2D in that same frame
- any appearance of “3D on both screens” must come from some approximation or scheduling trick outside this slice

This feature does not try to defeat that hardware limitation. It formalizes the screen-ownership rule around it.

## Runtime Policy

Each frame, the DS runtime determines whether top and bottom screens have any 3D content.

The hardware 3D target screen is chosen with this priority:

1. top, if top has any 3D content
2. bottom, if top has none and bottom has any 3D content
3. none, if neither screen has 3D content

That rule is intentionally simple and automatic for the first slice. No authored per-camera override is introduced yet.

## Queue Ownership Model

The existing shared engine queue model remains intact:

- 3D drawables belong to camera 3D queues
- 2D drawables belong to camera 2D queues
- viewport and camera targeting already determine which physical DS screen a queue contributes to

The DS runtime consumes those existing facts and derives:

- top has 3D or does not
- bottom has 3D or does not
- top has 2D or does not
- bottom has 2D or does not

No shared engine authoring change is required for this slice.

## Per-Frame Presentation Order

The DS presentation order for the first slice is:

1. inspect active cameras and queues
2. resolve per-screen 3D presence
3. choose hardware 3D target screen using the top-first rule
4. render the chosen hardware 3D screen, if any
5. render 2D for both screens
6. on the chosen 3D screen, 2D presents as an overlay after 3D
7. on the non-3D screen, 2D remains the full presentation path

This means 2D overlay is always “after 3D” for the first slice. There is no depth-tested interleaving between them.

## DS Runtime Architecture Changes

### NintendoDsRenderManager3D

`NintendoDsRenderManager3D` becomes the owner of hardware 3D screen selection because it can already inspect active cameras and both render queues during draw.

Responsibilities added in this slice:

- detect whether top and bottom have 3D queue content
- choose the hardware 3D target screen for the frame
- configure DS display mode for the chosen 3D screen
- draw the selected 3D queue only for the winning screen
- coordinate with `NintendoDsRenderManager2D` so 2D presentation knows which screen is currently overlaying onto 3D

If both screens have 3D content in the same frame, only top receives real 3D and bottom must skip its 3D queue that frame.

### NintendoDsRenderManager2D

`NintendoDsRenderManager2D` stops assuming the top screen is always a pure 2D bitmap target.

Responsibilities added in this slice:

- accept the current per-frame hardware 3D ownership decision
- present 2D-only output to the non-3D screen
- present 2D as an overlay on the 3D-owned screen
- keep camera queue routing by viewport target intact

For the first slice, overlay means:

- 3D has already rendered
- 2D draws after it
- the 2D pass visually sits above 3D

No attempt is made to place 3D over UI or split 2D by depth.

### NintendoDsBootHost

`NintendoDsBootHost` must stop owning long-lived screen-mode policy based on startup-scene type.

In the current structure it permanently preserves menu startup in 2D mode or permanently switches the main screen to 3D mode. That is incompatible with mixed presentation.

After this change, boot host should only:

- initialize DS video and diagnostics state
- initialize the core and startup scene
- run the main loop
- keep debug console ownership where needed

Per-frame screen-mode policy moves into the DS render path, where the actual queue facts are known.

## Screen Ownership Semantics

For a frame where:

- top has 3D and 2D
- bottom has 2D only

the result is:

- top gets hardware 3D and top 2D overlay
- bottom gets 2D-only presentation

For a frame where:

- top has 2D only
- bottom has 3D and 2D

the result is:

- bottom gets hardware 3D and bottom 2D overlay
- top gets 2D-only presentation

For a frame where:

- top has 3D
- bottom has 3D

the result is:

- top gets hardware 3D
- bottom does not render its 3D queue
- both screens may still receive 2D according to their 2D camera queues

## Compatibility

This slice preserves:

- current shared camera and viewport scene authoring
- DS dual-screen menu authoring
- runtime scene-manager scene transitions
- 2D-only scenes
- 3D-only scenes

The change is behavioral only at the DS presentation layer:

- scenes that previously forced top-screen 2D-only mode can now coexist with a 3D screen when queues demand it
- scenes that already use only one 3D screen continue to behave as before

## Error Handling

The DS runtime should fail clearly when mixed presentation assumptions are violated.

Expected invariants:

- `NintendoDsRenderManager2D` must know which screen, if any, currently owns hardware 3D before final presentation
- viewport-to-screen routing must remain valid
- a screen spanning both DS displays in one viewport remains unsupported

Violations should throw with direct DS runtime messages rather than silently falling back to an arbitrary mode.

## Testing Strategy

### Source Audits

Add or update DS source audits to verify:

- DS boot host no longer hardcodes permanent startup-scene render-mode ownership
- `NintendoDsRenderManager3D` contains the top-first screen-ownership rule
- `NintendoDsRenderManager2D` exposes the overlay/presentation path needed for a 3D-owned screen

### Build and Runtime Verification

Verify:

- DS builder tests still pass
- DS export still succeeds
- menu multibuild still boots
- at least one DS scene with 3D plus 2D overlay renders correctly

### Behavioral Checks

The first runtime validation scenarios should include:

1. top-screen 3D with top-screen 2D overlay and bottom-screen 2D-only
2. bottom-screen 3D with bottom-screen 2D overlay and top-screen 2D-only
3. both screens request 3D and top wins deterministically

## Success Criteria

- 2D can render on top of 3D on the same physical DS screen
- either screen can own the hardware 3D pass, depending on active queue content
- top wins automatically when both screens request 3D in the same frame
- the non-3D screen can still present 2D in that frame
- no new shared scene authoring feature is required
- existing DS menu and multibuild startup behavior remains functional
