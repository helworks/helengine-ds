# TextComponent Build-Time Sprite Conversion Design

## Goal

Add an editor-authored `TextComponent` option that keeps text authoring intact in the editor, but converts that text into a sprite during scene packaging. The built game should receive only sprite-backed output for flagged text and should not know that the authored component started as text.

This is primarily needed for DS-style UI constraints where live text renders through `BG0`, so text that must visually sit on top of a sprite also needs to become a sprite.

## Requirements

- Authoring remains a normal `TextComponent`.
- The editor shows one build-time conversion flag on `TextComponent`.
- The flag is visible for all platforms.
- When the flag is disabled, packaging keeps the existing text pipeline unchanged.
- When the flag is enabled, packaging emits runtime sprite data instead of runtime text data.
- The baked sprite must preserve the authored `TextComponent.Size` box rather than shrinking to tight glyph bounds.
- The generated texture exists only during packaging/build output.
- The build must fail clearly if baked text sprite generation fails.

## Existing System Context

### Authored Text Surface

`TextComponent` already carries the data needed to bake a stable visual result:

- `Text`
- `WrapText`
- `Font`
- `FontScale`
- `Alignment`
- `Color`
- `Size`
- `RenderOrder2D`
- `LayerMask`
- `Rotation`
- `SourceRect`

`TextComponent` also already has a `RuntimeTexture Texture` property, but it is marked with `ScenePersistenceIgnore` and the normal text rendering path does not currently use it as a replacement for glyph emission.

### Editor Capture Support

The editor already has `EditorExact2DPreviewCaptureService`, which can clone a `TextComponent` into an offscreen render target using the authored text layout inputs. That existing capture path is the right rendering source for the bake operation where practical.

### Packaging Transform Support

`SceneComponentPackagingTransformService` already rewrites authored scene component records into packaged runtime payloads. `TextComponent` and `SpriteComponent` both already have dedicated rewrite paths there. This is the correct place to replace flagged text records with sprite records during packaging.

### Asset Reference Support

`RuntimeTexture` is already a supported asset-backed member type in the scene serialization/runtime reference system. The packaged sprite replacement can therefore reference a generated packaged texture through the same scene asset reference model the engine already uses for textures.

## Proposed Approach

Use a build-time component rewrite in `SceneComponentPackagingTransformService`.

Why this approach:

- it keeps editor authoring unchanged
- it keeps the runtime contract clean
- it uses the existing scene packaging architecture instead of introducing a second transform phase
- it avoids runtime text-to-sprite logic entirely

Alternatives considered and rejected:

1. Runtime conversion during scene load:
   rejected because the shipped game would still need to know about authored text conversion.
2. Platform-added component replacement path:
   rejected because it is heavier than necessary for a per-component build instruction and obscures the transform logic.

## Design

### 1. Authored Component Contract

Add a new boolean property to `TextComponent`:

- `ConvertTextToSprite`

Behavior:

- The property is editor-visible and serialized with the authored scene.
- The property does not change editor rendering or editor interaction behavior.
- The authored component remains a normal `TextComponent` in the editor.
- The property is interpreted only by the packaging pipeline.

### 2. Packaging Contract

When `ConvertTextToSprite` is `false`:

- keep the current `TextComponent` packaging rewrite unchanged

When `ConvertTextToSprite` is `true`:

- do not emit a runtime `TextComponent` payload for that authored component
- bake the authored text into a transient texture during packaging
- emit a runtime `SpriteComponent` payload instead

The built scene must contain only the sprite replacement for that component.

### 3. Baked Visual Contract

The baked output must preserve the authored text layout semantics as closely as possible.

Preserved authored inputs:

- text content
- wrap setting
- font
- font scale
- alignment
- color
- size
- render order
- layer mask
- rotation

Box sizing rule:

- the baked sprite uses the full authored `TextComponent.Size` box
- it does not shrink to tight glyph bounds

Transparency rule:

- the baked texture contains only the rendered text with transparency
- no implicit background fill is added

### 4. Bake Pipeline

For flagged text during packaging:

1. Read the authored text record.
2. Resolve the authored font reference.
3. Render the text into an offscreen texture using the authored `Size` box.
4. Materialize that capture as a transient raw `TextureAsset`.
5. Send that texture through the normal platform texture cook pipeline.
6. Write the cooked/generated texture into the build output.
7. Emit a `SpriteComponent` scene record referencing that packaged texture.

The bake should produce a normal raw texture input and let the existing platform texture cook path choose the final format. No DS-specific baked-text texture path is introduced in the first pass.

### 5. Generated Asset Ownership

The baked text texture is build-owned only.

It must not:

- write a source PNG into the project
- create an editor asset browser entry
- persist a texture reference back into the authored scene

It must:

- exist in build output under a deterministic generated path
- be reproducible from authored text inputs
- participate in normal texture cooking for the target platform

The generated path should be derived from stable bake inputs such as:

- scene id
- entity/component identity
- relevant text bake properties

This keeps rebuild behavior deterministic and avoids collisions.

### 6. Runtime Result

The built game receives:

- a normal packaged `SpriteComponent`
- a packaged texture asset reference

The built game does not receive:

- the authored `ConvertTextToSprite` intent
- a runtime `TextComponent` for that converted component
- any runtime text baking logic

## Error Handling

If packaging cannot bake a flagged text component, the build must fail.

There is no fallback to live runtime text for flagged components.

The failure message should identify enough authored context to find the source component quickly, at minimum:

- scene id or scene path
- component type
- entity/component identity when available

## Testing

### Persistence Tests

Add persistence coverage proving that `TextComponent.ConvertTextToSprite` round-trips through editor scene save/load.

### Packaging Transform Tests

Add focused tests for `SceneComponentPackagingTransformService` proving:

- unflagged text still emits `TextComponent`
- flagged text emits `SpriteComponent`
- the emitted sprite preserves authored `Size`
- key visual fields such as `RenderOrder2D`, `LayerMask`, `Rotation`, and `Color` are preserved
- the emitted sprite record contains a packaged texture reference

### Build Packager Tests

Add build-level packaging coverage proving:

- the generated baked texture is written into build output
- the rewritten scene payload references that generated texture
- no runtime `TextComponent` record is emitted for the converted component

### Editor UI Tests

Add editor property-view coverage only if the new boolean needs explicit inspector filtering or custom handling beyond the standard reflected property path.

## Non-Goals

- No one-shot editor conversion button that replaces authored text with authored sprites.
- No persistent generated source texture inside the project workspace.
- No DS-only implementation path in the first pass.
- No runtime fallback that silently leaves flagged components as text when baking fails.
