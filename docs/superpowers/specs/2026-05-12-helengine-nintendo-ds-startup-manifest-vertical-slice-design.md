# Helengine Nintendo DS Startup Manifest Vertical Slice Design

## Goal

Create the first end-to-end Nintendo DS player and builder slice for Helengine. The slice should prove that the editor-facing DS builder can emit a DS-owned runtime payload, the native DS package can include that payload through NitroFS, and the DS runtime can load that payload at boot and change visible behavior based on its contents.

## Scope

This milestone covers the first packaged runtime contract between the Nintendo DS builder and the Nintendo DS native player.

Included:
- DS platform builder scaffold under `builder/`
- DS builder tests under `builder.tests/`
- DS platform metadata exposed through the builder descriptor and definition
- DS startup manifest writer that emits a DS-owned binary manifest into a staged NitroFS path
- Native DS packaging updates so the staged NitroFS root is included in the `.nds`
- Native DS runtime manifest loading from NitroFS
- Visible screen-color changes driven by the loaded manifest

Excluded:
- Startup scene loading
- Scene catalogs
- General cooked asset loading
- DS material cooking beyond any metadata needed to expose the builder
- ARM7 runtime services
- Audio, input, save data, and text rendering
- Generated core integration beyond preserving the existing seam

## Repository Context

`helengine-ds` already contains a completed bootstrap milestone with a Docker build, a native Nintendo DS host, and a stable verification frame. The current host presents a hardcoded green top screen and cyan bottom screen through `NintendoDsBootHost`.

Sibling repositories define the platform shape that DS should follow:

- `helengine-windows` separates the native player host in `src/` from the editor-facing platform builder in `builder/`
- `helengine-ps2` follows the same split and already stages packaged runtime data through its builder before invoking the native packaging flow

This slice should adopt that same split instead of introducing a DS-specific structure.

## Recommended Approach

Use a DS-owned binary startup manifest staged into NitroFS by the builder and loaded by the DS runtime at boot.

This is the recommended direction because it proves the packaged runtime contract without adding JSON parsing, text rendering, or early asset-cooking complexity. The first runtime proof should be as small and deterministic as possible: the builder writes two screen colors, the package carries them, and the player displays them.

Alternative approaches rejected for this slice:

- JSON manifest in NitroFS: easier to inspect manually, but it adds parser complexity with little value
- Generated C++ startup data: simpler runtime path, but it would not prove packaged file loading
- Startup scene or asset payloads first: valid later slices, but they widen the contract before the package-loading seam is stable

## Architecture

The milestone should introduce the same top-level split used by the Windows and PS2 repositories.

### Builder side

The builder side owns platform metadata and package preparation:

- `builder/`
  - `NintendoDsPlatformAssetBuilder`
  - `NintendoDsPlatformDefinitionFactory`
  - `NintendoDsStartupManifestWriter`
  - DS build workspace and native build orchestration helpers
- `builder.tests/`
  - descriptor and definition tests
  - startup manifest writer tests
  - build orchestration tests where practical

The builder is responsible for creating the staged NitroFS content consumed by the native DS package.

### Native side

The native side remains small and player-focused:

- `src/main.cpp`
  - still owns only host construction and execution
- `src/platform/ds/NintendoDsBootHost.hpp`
- `src/platform/ds/NintendoDsBootHost.cpp`

`NintendoDsBootHost` should expand from a pure bootstrap host into the first packaged-runtime consumer. It should still own bootstrap video setup, but it should also mount NitroFS, load the startup manifest, validate it, and switch the runtime frame colors when the load succeeds.

## Startup Manifest Contract

The startup manifest should be a DS-owned binary file stored at a stable NitroFS path.

Recommended path:

- `nitro:/runtime/ds_startup_manifest.bin`

Recommended layout:

1. 4-byte magic: `HDSP`
2. 16-bit manifest version
3. 16-bit payload size
4. 16-bit top-screen color
5. 16-bit bottom-screen color

The top and bottom colors should be stored in the exact native DS representation the runtime uses for writing pixels, including the visibility bit. The first slice should not add color translation logic when the payload is builder-owned and fixed-format.

### Validation rules

The runtime should reject the manifest when:

- the NitroFS mount fails
- the file is missing
- the file cannot be opened or read fully
- the magic does not match
- the version is unsupported
- the payload size is smaller than the required contract

The runtime should not construct default manifest values when validation fails.

## Runtime Behavior

The DS host should preserve the existing bootstrap behavior as a clear pre-runtime state:

- bootstrap top screen: hardcoded green
- bootstrap bottom screen: hardcoded cyan

After video initialization, the host should attempt to mount NitroFS and load the startup manifest.

If loading succeeds:

- the top screen should change to the manifest-defined top color
- the bottom screen should change to the manifest-defined bottom color
- the host should remain alive indefinitely, as in the bootstrap milestone

If loading fails:

- the host should remain in the bootstrap color state
- the failure should be represented through an explicit DS-side load result path owned by the host
- the runtime should not silently fabricate manifest content

This keeps the current green/cyan frame as an intentional bootstrap fallback rather than an accidental hidden default.

## Builder And Packaging Flow

The DS builder should implement the smallest possible `BuildAsync` flow that still proves packaged runtime data.

Required responsibilities:

1. Resolve and validate the DS build request inputs
2. Create a DS build workspace with separate staging paths
3. Create a staged NitroFS root
4. Emit `ds_startup_manifest.bin` into the staged NitroFS root
5. Invoke the native DS packaging flow with the staged NitroFS root path
6. Verify that the final `.nds` artifact is produced

The first slice should not cook generic assets, build scene catalogs, or stage startup scenes. The only staged runtime payload should be the DS startup manifest.

## Native Build Design

The native DS build should accept a staged NitroFS root supplied by the builder rather than assuming the repository owns a fixed payload directory.

The `Makefile` should:

- preserve the existing generated-core seam
- accept a staged NitroFS root input
- package that NitroFS root into the final `.nds`
- fail if the staged NitroFS root is required by the build flow but missing

The native packaging flow should adapt to the actual devkitPro toolchain layout already used by the repository rather than layering compatibility fallbacks.

## Data And Control Flow

The runtime flow for this slice should remain straightforward:

1. `main()` constructs `NintendoDsBootHost`
2. `main()` calls `Run()`
3. `Run()` initializes the DS video bootstrap state
4. `Run()` paints the bootstrap verification colors
5. `Run()` mounts NitroFS
6. `Run()` loads and validates `nitro:/runtime/ds_startup_manifest.bin`
7. If validation succeeds, `Run()` paints the manifest-driven colors
8. `Run()` enters the existing infinite frame loop

The builder flow should remain equally narrow:

1. `NintendoDsPlatformAssetBuilder.BuildAsync()` creates the DS workspace
2. The builder writes the startup manifest into the staged NitroFS path
3. The builder invokes the native DS build
4. The builder verifies the `.nds` output
5. The builder returns a successful build report only when all prior steps succeed

## Error Handling

This slice should preserve strict failure behavior.

Builder failures should stop the build when:

- the request is invalid
- the DS workspace cannot be created
- the staged NitroFS root cannot be created
- the startup manifest cannot be written
- the native package build fails
- the final `.nds` artifact is missing

Runtime load failures should not mutate into fake success. The host may remain alive on the bootstrap colors, but the manifest load should still be treated as a real failure in the host-owned state path.

This is consistent with the repository rule against masking root-cause failures with best-effort runtime patches.

## Verification Plan

Builder-side verification:

- DS platform builder descriptor matches the expected builder id, target platform id, supported backend ids, and manifest compatibility range
- DS platform definition exposes the initial platform metadata required by the editor
- startup manifest writer emits the exact expected bytes for known top and bottom colors
- build orchestration passes the staged NitroFS root into the native build flow

Native-side verification:

- package build produces the `.nds` artifact
- a package built with colors different from green/cyan produces different runtime screen colors than the bootstrap host
- failure to load the startup manifest leaves the bootstrap frame visible

Manual emulator verification for this slice:

1. Build a package with known manifest colors
2. Boot the package in melonDS or DeSmuME
3. Confirm the runtime frame changes away from the bootstrap green/cyan state
4. Confirm the application remains stable without resetting or exiting immediately

## Testing Strategy

The first slice should concentrate automated coverage on the builder and keep native verification targeted.

Automated tests:

- builder metadata tests
- startup manifest serialization tests
- DS builder workspace and native-build handoff tests where they can be isolated

Manual verification:

- emulator boot check for manifest-driven colors

This is the same practical balance already used by the bootstrap milestone: keep the native runtime small and deterministic, then prove the visual contract in an emulator.

## File Changes

Expected repository changes for this slice:

- add `builder/` scaffold for the Nintendo DS builder
- add `builder.tests/` scaffold for builder test coverage
- add DS startup manifest writer and supporting contract types
- add DS workspace and native-build orchestration helpers
- update `Makefile` to accept and package a staged NitroFS root
- extend `NintendoDsBootHost` with NitroFS mount, manifest loading, validation, and manifest-driven frame colors
- update `README.md` if the DS build flow needs to document builder-owned NitroFS packaging expectations

The slice should not add speculative runtime abstractions beyond the host and builder boundaries needed to prove the contract.

## Constraints

- Keep the implementation small and platform-owned
- Follow the Windows and PS2 repository split between `src/` and `builder/`
- Preserve the existing bootstrap host as the fallback visual state
- Do not add JSON parsing or generic asset cooking to this slice
- Do not create silent default manifest values
- Keep the DS runtime on the ARM9 side only for this milestone

## Success Criteria

This vertical slice is successful when all of the following are true:

- the DS repository exposes a builder assembly under `builder/`
- the builder can emit a DS startup manifest into a staged NitroFS root
- the native DS build packages that NitroFS payload into the `.nds`
- the DS runtime loads the packaged manifest at boot
- the runtime colors change based on the manifest instead of remaining hardcoded
- the bootstrap green/cyan frame still exists as the explicit fallback path when manifest loading fails
