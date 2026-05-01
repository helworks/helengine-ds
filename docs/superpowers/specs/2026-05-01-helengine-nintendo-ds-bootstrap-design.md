# Helengine Nintendo DS Bootstrap Design

## Goal

Create the first Nintendo DS native bootstrap for Helengine as a Docker-only build that produces a runnable `.nds` artifact. The first emulator validation target is a stable top green screen and bottom cyan screen in melonDS or DeSmuME.

## Scope

This milestone covers only the native Nintendo DS host bootstrap and its build environment.

Included:
- Docker-based devkitARM build environment
- Native build script that emits `build/helengine_ds.nds`
- Thin native entrypoint
- Nintendo DS boot host that initializes both displays and holds the verification colors
- Minimal README with build and emulator verification instructions
- Reserved `HELENGINE_CORE_CPP_ROOT` seam for future generated-core integration

Excluded:
- Generated core compilation
- Engine/game logic integration
- Asset pipeline
- Input handling
- Audio
- Runtime abstraction beyond the minimal host bootstrap needed for this milestone

## Repository Context

`helengine-ds` currently has no DS-specific scaffold. The repository starts effectively empty apart from the repository instructions and ignore rules, so this design creates the initial project layout from scratch. The structure should stay intentionally close to the GameCube bootstrap so the platform repositories remain predictable.

## Architecture

The project should remain minimal and platform-focused:

- `Dockerfile`
  Builds a DS development container from a devkitPro devkitARM image and installs the Nintendo DS development package set.
- `Makefile`
  Builds the Nintendo DS target, emits `build/helengine_ds.nds`, and reserves `HELENGINE_CORE_CPP_ROOT` for later generated-core integration.
- `README.md`
  Documents the Docker build flow and emulator verification target.
- `src/main.cpp`
  Owns only construction and execution of the DS host bootstrap.
- `src/platform/ds/NintendoDsBootHost.hpp`
  Declares the thin Nintendo DS bootstrap boundary.
- `src/platform/ds/NintendoDsBootHost.cpp`
  Owns DS display initialization, VRAM/framebuffer setup, and the infinite frame loop that keeps the top screen green and the bottom screen cyan.

The first milestone deliberately keeps all DS-specific runtime behavior inside the host class. This preserves a clean seam for later engine integration without pretending the generated core exists yet.

## Build Environment Design

The Docker image should use a devkitPro devkitARM base image rather than manually installing toolchains from a host package manager. The image must explicitly set:

- `DEVKITPRO=/opt/devkitpro`
- `DEVKITARM=${DEVKITPRO}/devkitARM`
- `PATH` entries for devkitARM binaries and devkitPro tooling

The Dockerfile should not use a `# syntax=docker/dockerfile:...` directive unless the file genuinely requires it. The GameCube work showed that unnecessary frontend resolution can create avoidable Docker auth failures on the host machine.

The image must install the Nintendo DS development package group used by modern devkitPro tooling rather than assuming the base image already contains all DS-specific libraries and rules.

## Build Script Design

The Makefile should follow the same principles used in the GameCube repository after the toolchain debugging work:

- avoid relying on implicit PATH lookup for compiler and conversion tools when a stable absolute path is available
- prefer the current devkitPro platform rules over stale hardcoded specs paths
- keep all intermediate files under `build/`
- reserve `HELENGINE_CORE_CPP_ROOT` exactly as a future seam, not as an active dependency

The Makefile should emit:

- `build/helengine_ds.elf`
- `build/helengine_ds.nds`

If the current devkitARM image provides DS platform rules, the Makefile should consume those rules and derive machine-dependent flags from the supported toolchain layout instead of hardcoding obsolete linker assumptions. If the image layout differs from expectations, the implementation should adapt to the actual layout rather than adding best-effort compatibility hacks.

## Runtime Behavior Design

The DS bootstrap should initialize both screens into a deterministic visual state suitable for emulator verification.

Target behavior:

- top display: solid green
- bottom display: solid cyan
- application remains alive indefinitely
- no crash loop
- no automatic exit

The runtime should do only the DS video setup necessary to reach that state. It should not add text consoles, diagnostics, input loops, or other systems unless the DS platform API requires them as part of correct display initialization.

## Data and Control Flow

The runtime flow should remain straightforward:

1. `main()` constructs `NintendoDsBootHost`
2. `main()` calls `Run()`
3. `Run()` performs DS display initialization
4. `Run()` configures the top and bottom display buffers or backgrounds according to the DS graphics path selected during implementation
5. `Run()` writes the top screen green state
6. `Run()` writes the bottom screen cyan state
7. `Run()` enters an infinite loop that preserves a stable boot frame

No other subsystems are part of the first milestone.

## Error Handling

This milestone should not introduce silent fallbacks or synthetic defaults. The build should fail if:

- the toolchain path assumptions are wrong
- the DS development package is missing
- the linker rules differ from the Makefile assumptions
- required platform libraries are absent

The implementation should correct those root causes in the Dockerfile or Makefile rather than masking them in code.

## Verification Plan

Repository-level verification:

```bash
docker build -t helengine-ds .
docker run --rm -v "$PWD":/workspace -w /workspace helengine-ds make
```

Expected build result:

- `build/helengine_ds.nds` exists

Emulator-level verification:

- open `build/helengine_ds.nds` in melonDS or DeSmuME
- confirm the top screen is solid green
- confirm the bottom screen is solid cyan
- confirm the process remains stable without resetting or exiting immediately

## Testing Strategy

There is no meaningful automated runtime emulator test in this first milestone. Verification is therefore split between:

- build verification in Docker
- manual boot verification in a DS emulator

The implementation should still preserve a small, testable structure by keeping the native entrypoint thin and centralizing DS setup inside a dedicated host class.

## Constraints

- Docker-only build flow
- Native Nintendo DS output only
- No generated core integration yet
- Minimal architecture, intentionally parallel to the GameCube bootstrap
- No speculative abstractions beyond the DS host boundary

## Recommended Implementation Direction

Use a minimal devkitARM plus Nintendo DS Docker scaffold, a Makefile aligned to the current devkitPro DS toolchain layout, and a single `NintendoDsBootHost` that directly owns display initialization and the verification frame loop. This gives the fastest path to a reliable `.nds` artifact while preserving a clean seam for later Helengine integration.
