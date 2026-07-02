# Animation Clip Platform Overrides Design

## Goal

Allow one `.hanim` asset to own:

- one base animation clip
- optional per-platform overrides

This should let the Demo Disc logo use one shared animation asset path across all platforms while Nintendo DS can author its own platform-specific motion.

## Problem

Today the Demo Disc menu uses two different paths:

- non-DS platforms create the overlay logo sprite and attach an `AnimationPlayerComponent`
- DS creates a static top-screen logo sprite with no animation player

That split is brittle. The authored source of truth should be one logo animation asset. Platform differences should live inside that asset, not in scene-factory forks or build-time translation hacks.

## Requirements

### Functional

- One `.hanim` asset can define base frames and per-platform overrides.
- A platform override supports three modes:
  - `InheritBase`
  - `ReplaceWholeClip`
  - `OverrideFrames`
- `OverrideFrames` can:
  - override an existing base frame
  - insert platform-only frames
- Platform-only inserted frames own their own local timestamp.
- At cook time, each platform resolves to one flat runtime clip timeline.
- Runtime animation playback remains unaware of editor-only override data.
- DS menu logo should reference the same animation clip path as other platforms and animate from the DS-resolved clip.

### Authoring

- Frame targeting in the editor uses stable editor-only frame IDs.
- Frame IDs are not part of runtime payloads.
- Platform overrides are edited through the existing per-platform authoring model.
- Users can choose whether a platform inherits, replaces, or partially overrides the base clip.

## Proposed Model

### Animation Asset Structure

Each `.hanim` asset contains:

- `BaseFrames`
- `PlatformOverrides`

Each platform override contains:

- `Mode`
- `Frames`

`Mode` values:

- `InheritBase`
  - no platform-specific frame data is required
- `ReplaceWholeClip`
  - `Frames` is the complete clip for that platform
- `OverrideFrames`
  - `Frames` contains platform-authored frame entries that either:
    - target an existing base-frame ID
    - exist as platform-only inserted frames with their own timestamp

### Editor-Only Frame Identity

Each base frame gets a stable editor-only frame ID.

Purpose:

- allow override entries to target base frames without relying on frame index
- avoid brittle matching when the base timeline is reordered or edited

Rules:

- frame IDs exist only in editor/asset authoring data
- cook strips frame IDs from runtime output
- `ReplaceWholeClip` does not need to preserve base-frame targeting semantics

## Editor Behavior

### Override Surface

The animation editor exposes the same platform-tab concept used elsewhere in the engine.

For each platform, the editor shows:

- override mode selector
- platform-specific frame view for the selected mode

### Mode Semantics

#### InheritBase

- the platform uses the base clip unchanged
- base timeline remains the only editable timeline for inherited data

#### ReplaceWholeClip

- the platform owns a full independent clip timeline
- base inheritance is ignored for resolved output

#### OverrideFrames

- the platform sees base frames as inherited defaults
- the platform can:
  - override an inherited base frame
  - add platform-only frames with local timestamps
  - remove a platform override without deleting the base frame

### Visual Treatment

The editor should clearly distinguish:

- inherited base frames
- overridden platform-owned frames
- inserted platform-only frames

This keeps one asset understandable even when a platform partially diverges.

## Cook and Runtime Resolution

### Cook Output

Cook resolves one final clip per target platform.

Resolution rules:

- `InheritBase`
  - output base frames as-is
- `ReplaceWholeClip`
  - output only the platform frame list
- `OverrideFrames`
  - combine:
    - base frames
    - platform overrides targeting base-frame IDs
    - platform-only inserted frames
  - sort the final result by timestamp

Base frames keep their original timestamps unless the platform override explicitly changes them.

### Runtime Contract

Runtime animation systems continue consuming a flat clip format.

Runtime does not know about:

- override modes
- frame IDs
- base-vs-platform merge logic

All override resolution happens before runtime.

## DS Logo Application

### Desired End State

The Demo Disc logo animation path becomes shared again:

- non-DS menu path keeps using the logo animation asset
- DS menu path also attaches the same animation asset

The difference is authored inside the `.hanim` asset:

- base clip drives existing platforms
- DS platform override drives Nintendo DS motion

### Scene Factory Change

`CreateNintendoDsTopScreenLogoEntity(...)` should stop being a static-logo special case and should attach the same animation player/clip reference pattern used by the generic overlay-logo path.

The DS-specific motion should come from platform-resolved animation data, not from DS-only scene logic.

## Data Resolution Rules

### OverrideFrames Merge Rules

For `OverrideFrames`:

- if a platform frame targets a base-frame ID, it replaces that base frame's resolved values for that platform
- if a platform frame is inserted-only, it participates in the final platform timeline using its own timestamp
- the final cooked platform clip is sorted by timestamp after merge

When timestamps are equal:

- resolved base-frame entries keep their original base relative order
- inserted platform-only frames keep their authored relative order
- inserted platform-only frames at the same timestamp are emitted after resolved base-frame entries

If multiple platform entries target the same base-frame ID, editor validation should reject that state.

## Validation and Testing

### Serialization

- base-only clip round-trip
- clip with `ReplaceWholeClip` round-trip
- clip with `OverrideFrames` round-trip
- stable editor-only frame IDs survive authoring save/load

### Resolution

- `InheritBase` resolves to base timeline
- `ReplaceWholeClip` resolves only to platform frames
- `OverrideFrames` resolves base plus overrides plus inserted frames
- timestamp sorting works deterministically
- targeted base-frame overrides replace the expected frame

### Menu Integration

- Demo Disc menu scene continues referencing one shared logo animation asset path
- DS scene path attaches the animation player instead of leaving the logo static

### Runtime Verification

- DS build resolves and plays the DS logo animation
- non-DS platforms continue using the existing logo animation behavior

## Out of Scope

- runtime clip blending between base and platform data
- generic animation retargeting systems
- cross-asset platform animation composition
- platform-aware runtime animation logic

## Recommendation

Implement this as one logical `.hanim` asset with built-in per-platform override modes.

This keeps:

- one asset path in scenes
- one authoring surface in the editor
- one resolved flat clip contract at runtime

It also matches the engine's existing base-plus-platform-override mental model and avoids another DS-only fork in menu generation.
