# Helengine Nintendo DS Host

This repository contains the Nintendo DS native host, the DS platform builder integration, and the DS-specific runtime source audits for Helengine.

## Current status

- The shared editor CLI can build DS packages with platform id `ds`.
- The native DS target still builds through the checked-in devkitPro Docker image and `Makefile`.
- The DS renderer now follows a hardware-only policy.
- The old CPU-composited 2D framebuffer path is removed.
- Unsupported 2D work is skipped instead of falling back to software rendering.
- Debug builds show unsupported DS 2D work through a magenta hardware marker and debug logging.
- Source-audit coverage exists for the hardware-only DS renderer contract and DS boot-host startup flow.

## Editor CLI build

If your workspace keeps `helengine-ds`, `helengine`, and `helprojs` as sibling directories, use the shared wrapper like this:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 `
  -Project ..\helprojs\city\project.heproj `
  -Platform ds `
  -Output ..\helprojs\city\ds-build
```

That wrapper runs the main editor CLI with `--build ds` and writes the generated platform package to the output directory you provide.

## Launching in melonDS

Use the checked-in launcher script:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\artifacts\launch-melonds-rom.ps1
```

Optional overrides:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\artifacts\launch-melonds-rom.ps1 `
  -RomPath .\build\helengine_ds.nds `
  -MelonDsPath ..\emus\melonDS-1.1-windows-x86_64\melonDS.exe
```

The launcher prints the ROM path, ROM timestamp, and the spawned melonDS process id. It exits with:

- `0` on success
- `2` for invalid launcher arguments
- `3` when the ROM file is missing
- `4` when the melonDS executable is missing
- `5` when melonDS fails to launch

## Verification

Run the focused DS audit suite:

```bash
dotnet test builder.tests/helengine.ds.builder.tests.csproj --filter "NintendoDsRenderManager2DSourceAuditTests|NintendoDsRenderManager3DSourceAuditTests|NintendoDsRenderManager3DPerformanceSourceAuditTests|NintendoDsBootHostSourceAuditTests"
```

This verifies the current DS-specific contract around:

- hardware-only 2D renderer behavior
- removal of software 2D presentation coupling from the 3D renderer
- explicit rejection of fake DS render-target support
- removal of boot-time prewarm logic for deleted software text caches

## NitroFS note

The native target still accepts `HELENGINE_DS_NITROFS_ROOT` through the `Makefile` when you need to stage NitroFS content manually for native packaging experiments.
