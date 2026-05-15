# DS Runtime 60 FPS Design

## Goal

Improve Nintendo DS runtime performance toward a stable 60 fps for the current colored cube scene without changing scene content and while preserving the current look as closely as possible.

## Constraints

- The colored cube scene content must remain unchanged.
- Changes must stay in the renderer or runtime path.
- The current visual look should be preserved as closely as possible.
- Startup-scene loading and cooked-material resolution must continue working.

## Current Problem

The current Nintendo DS renderer does unnecessary CPU work in the hot draw path:

- scene lighting is rediscovered repeatedly instead of being treated as frame state
- drawable state is re-read too deep in the submission path
- each triangle repeats transform and lighting work that is only partially triangle-specific

This makes the current path a poor fit for the Nintendo DS target, especially when the goal is 60 fps without reducing scene content.

## Recommended Approach

Use a staged renderer and runtime optimization plan that removes repeated work first, then upgrades the geometry submission path only if needed.

### Stage 1: Frame-Scope Lighting Cache

The Nintendo DS renderer should resolve frame lighting once at the start of the draw pass and reuse it for every drawable submitted in that frame.

The authoritative light source should come from runtime-managed light state rather than ad hoc entity scanning. The Nintendo DS renderer should consume manager-owned directional and ambient light collections as the per-frame input.

The first pass should preserve the current lighting model:

- use the first active directional light as the direct-light source
- accumulate ambient light from the runtime-managed ambient set when present
- preserve the current radiance shaping and contrast behavior

### Stage 2: Drawable-Scope Submission Setup

The Nintendo DS renderer should move repeated drawable setup out of the per-triangle path.

Per drawable, the renderer should:

- read entity and material references once
- validate runtime-model and runtime-material compatibility once
- compute transform prerequisites once
- apply render-state setup once before triangle submission

Triangles should then consume prevalidated drawable state instead of repeatedly re-fetching the same information.

### Stage 3: Submission-Path Upgrade

If frame time remains too high after the first two stages, the Nintendo DS renderer should shift from CPU-side world-transforming every submitted vertex toward a Nintendo DS native transform submission path.

This stage must still keep the same scene content and broadly preserve the current rendered result. The intent is to move work into the platform submission model rather than changing authored assets or reducing scene complexity.

### Stage 4: Last-Mile Math Approximations

Only if the previous stages are not sufficient, the renderer may replace the most expensive math operations with cheaper approximations or lookup-driven equivalents.

This stage is explicitly lower priority because it carries more visual risk. It should only be used for final tuning after the larger structural waste has already been removed.

## Architecture Changes

### NintendoDsRenderManager3D

`NintendoDsRenderManager3D` becomes responsible for:

- capturing frame lighting once per draw call
- storing frame-scoped lighting state for the active frame
- preparing drawable-scoped submission state before triangle iteration
- keeping per-triangle work focused on the parts that are truly triangle-specific

`ResolveSceneLighting(...)` should stop scanning raw entity/component graphs and should instead read runtime-managed light state.

### Runtime Light Ownership

The renderer should treat runtime-managed light collections as the authoritative source of visible lighting state. That keeps scene-state ownership in the engine and prevents the DS renderer from rebuilding that knowledge independently.

### Boot and Material Contracts

No part of this design changes:

- the checkpointed DS startup path
- the cooked platform-owned material resolution path
- the generated-core staged resolver contract

Those paths are already fixed and must remain stable while performance work proceeds.

## Testing Strategy

Add or keep focused regression coverage for:

- DS boot path still entering checkpointed startup
- DS generated-core staged material resolution still using cooked platform-owned materials
- DS renderer source continuing to expose the cooked-material override contract

For performance-oriented implementation steps, verify behavior with:

- targeted renderer tests where practical
- full DS build of the city project
- emulator validation that the colored cube scene still renders

## Success Criteria

- The colored cube scene still renders with materially the same composition and color impression.
- Scene content remains unchanged.
- Startup-scene loading and cooked-material resolution do not regress.
- Nintendo DS runtime performance improves toward a stable 60 fps.
- Each optimization stage is verified before moving to the next one.

## Recommended Implementation Order

1. Replace per-drawable light discovery with one frame-light snapshot from runtime-managed light state.
2. Move repeated entity, material, and transform setup to drawable scope.
3. Measure the result with the existing colored cube DS runtime path.
4. If needed, upgrade the submission path to a more Nintendo DS native transform flow.
5. Only if still needed, apply look-preserving math approximations for last-mile tuning.
