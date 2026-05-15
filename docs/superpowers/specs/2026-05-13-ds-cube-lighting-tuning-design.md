## DS Cube Lighting Tuning

### Goal

Improve the readability of the `cube_test` directional-lighting pass on Nintendo DS without expanding scope into per-vertex lighting or material tinting.

### Approach

Keep the current per-face world-space directional diffuse calculation and tune only the final brightness response for DS display readability.

Changes in scope:
- keep the existing directional light resolution path
- keep grayscale lighting output
- apply a small contrast curve so rotating faces separate more clearly
- remove the runtime debug overlays from normal boot and runtime

Changes out of scope:
- per-vertex lighting
- colored material tinting
- shadowing
- scene or asset format changes

### Verification

- rebuild the editor-driven DS ROM
- launch the fresh ROM in `melonDS`
- confirm the cube still rotates
- confirm the cube remains grayscale-lit with clearer face separation
- confirm normal runtime no longer draws the debug overlay on the bottom screen
