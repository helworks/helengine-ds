# DS Release Console Stripping Design

## Goal

Remove the Nintendo DS host console formatting stack from release-oriented DS builds when `HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS=0`.

## Problem

The fresh DS linker artifacts show the remaining formatted-I/O and locale costs are primarily pulled in by DS host console and status paths:

- `libnds9.a(console.o)`
- `libc_a-iprintf.o`
- `libc_a-siscanf.o`
- `libc_a-vfiprintf.o`
- `libc_a-svfiprintf.o`
- `libc_a-svfiscanf.o`
- `libc_a-categories.o`
- `libc_a-locale.o`

The current build already passes `HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS=0`, but `NintendoDsBootHost.cpp` still compiles console-based fatal, startup-failure, and runtime-failure formatting code into the binary. That keeps the DS console dependency alive even when release-oriented builds are not supposed to ship the diagnostic console.

## Design

Treat the existing `HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS` flag as the compile-time ownership seam for DS host console diagnostics:

- keep console-backed boot traces, bottom-screen status text, formatted failure snapshots, and fatal console dumps only when the flag is enabled
- provide lean no-diagnostics fallbacks that preserve safe behavior without `consoleInit`, `iprintf`, or `std::snprintf`
- leave runtime FPS rows alone because they render through the engine-owned DS text path, not the libnds console

For the no-diagnostics build:

- startup-scene failure catch blocks should record only compact fixed strings or rethrow directly
- runtime failure recording should avoid console row formatting entirely
- fatal handling should avoid console initialization and just preserve the checkpoint color plus trace emission/halt behavior

## Constraints

- This is intentionally DS-specific; the evidence is in the DS host linker chain, not the shared codegen runtime.
- Existing diagnostics builds must keep the richer console and host-trace behavior.
- The change should be compile-time, not a runtime branch, so the unused console code disappears from the final release binary.

## Verification

- DS boot-host source audit proves console-backed fatal/runtime paths are guarded by `HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS`.
- Focused `helengine-ds` builder tests pass.
- A fresh DS build completes successfully.
- The native size report drops the console/printf family contributors measurably from the current baseline:
  - package size: `1,933,312`
  - accounted native binary size: `769,374`
