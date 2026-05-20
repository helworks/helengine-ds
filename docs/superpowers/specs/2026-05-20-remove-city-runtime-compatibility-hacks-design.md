# Remove City Runtime Compatibility Hacks Design

## Summary

The engine and editor currently contain hardcoded compatibility paths for `city.*` scripted components. That is the wrong boundary. Project components must not be engine-owned.

This cleanup removes all engine/editor runtime compatibility hacks for city components and requires regeneration and rebuild from fresh sources.

## Problem

There are engine/editor seams that explicitly know about project component ids:

- `city.menu.DemoDiscReturnToMenuComponent, gameplay`
- `city.menu.PlatformInfoTextComponent, gameplay`

Those seams currently:

- prevent automatic scripted-component runtime deserializer generation
- register built-in runtime deserializers in engine code for city components
- preserve legacy packaging compatibility behavior that belongs to the project, not the engine

This creates exactly the kind of contract breakage we just hit:

- the city component schema changed
- the engine compatibility deserializer still expected the old payload shape
- fresh builds broke because engine code was pretending to own project serialization

## Goals

- Remove all engine/editor runtime compatibility hacks for `city.*` components.
- Ensure fresh generated runtime code owns serialization for city components completely.
- Keep engine runtime aware only of engine-owned component types.
- Require regeneration/rebuild instead of carrying legacy compatibility baggage.

## Non-Goals

- Do not preserve compatibility with old packaged scenes or ROMs.
- Do not add replacement migration shims.
- Do not keep partial legacy support for one city component while removing another.

## Design

### 1. Remove engine-owned filtering for city components

`EditorGeneratedCoreRegenerationService` currently filters some city component type ids out of automatic runtime deserializer generation.

That filtering will be removed entirely for city component ids.

After cleanup:

- `city.*` component ids are resolved like normal project scripted components
- automatic runtime deserializer generation includes them
- engine-generated runtime code owns their payload schema based on the actual project types

### 2. Remove built-in runtime deserializers for city components

The engine runtime currently registers built-in deserializers for city component ids.

Those built-in deserializers will be removed:

- `RuntimeDemoDiscReturnToMenuComponentDeserializer`
- `RuntimePlatformInfoTextComponentDeserializer`

After cleanup:

- `RuntimeComponentRegistry` no longer registers city-specific built-in runtime deserializers
- fresh generated runtime code handles those types through the normal automatic scripted-component path

### 3. Remove editor packaging hacks that exist only for city legacy compatibility

The editor packaging path currently contains city-specific legacy ids for platform-info compatibility and related packaging transforms.

Those legacy compatibility paths will be removed when they exist only to preserve city-specific historical behavior.

The editor may still transform city scene data when doing normal generic packaging work, but it must not contain engine-owned special cases whose only reason to exist is “city used to serialize this differently.”

### 4. Fresh regeneration becomes the only contract

After this cleanup:

- old packaged outputs may fail
- old generated core may fail
- stale ROMs may fail

That is acceptable.

The supported contract becomes:

1. regenerate project scenes/code
2. regenerate native runtime support
3. rebuild the platform
4. run the fresh output only

## Files

Primary cleanup surface:

- `engine/helengine.editor/managers/project/EditorGeneratedCoreRegenerationService.cs`
- `engine/helengine.core/scene/runtime/RuntimeComponentRegistry.cs`
- `engine/helengine.core/scene/runtime/RuntimeDemoDiscReturnToMenuComponentDeserializer.cs`
- `engine/helengine.core/scene/runtime/RuntimePlatformInfoTextComponentDeserializer.cs`
- `engine/helengine.editor/managers/project/EditorWindowsBuildScenePackager.cs`
- `engine/helengine.editor/managers/project/SceneComponentPackagingTransformService.cs`

Project components that should flow through the normal scripted-component path afterward:

- `city.menu.DemoDiscReturnToMenuComponent`
- `city.menu.PlatformInfoTextComponent`
- `city.menu.NintendoDsReturnOverlayComponent`

## Error Handling

- Stale outputs that still depend on removed compatibility deserializers are allowed to fail.
- Fresh regenerated outputs must succeed without city-specific engine hacks.
- If another `city.*` compatibility seam appears during implementation, remove it in the same cleanup pass rather than layering around it.

## Testing

### Engine/editor verification

- Prove city component ids are no longer filtered out by generated-core regeneration.
- Prove runtime registry no longer registers built-in deserializers for city component ids.
- Prove editor packaging tests still pass with fresh automatic scripted-component payloads.

### Fresh-build verification

- Regenerate the fresh project outputs.
- Rebuild generated runtime/native code.
- Rebuild the DS ROM.
- Verify the fresh ROM no longer throws the serialized-member mismatch for city return/menu components.

## Recommendation

Remove all city runtime compatibility hacks in one pass, regenerate everything, and treat any remaining failures as fresh-contract bugs instead of preserving legacy behavior.
