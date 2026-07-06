# Helengine Nintendo DS Host

This repository contains the Nintendo DS platform host and builder integration for Helengine.

## Build

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\artifacts\build-platform.ps1 `
  -Project ..\helprojs\city\project.heproj `
  -Platform ds `
  -Output ..\helprojs\city\ds-build
```

## Run In Emulator

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\launch_in_emulator.ps1 `
  -ArtifactPath ..\helprojs\city\ds-build\helengine_ds.nds
```

The launcher writes emulator and runtime traces to:

- `C:\tmp\helengine-ds-logs\melonDS-stdout.log`
- `C:\tmp\helengine-ds-logs\melonDS-stderr.log`
- `C:\tmp\helengine-ds-logs\helengine-ds-boot.log`
- `C:\tmp\helengine-ds-logs\helengine-ds-scene-transition-trace.log`

## More Docs

- [Docker Build Notes](docs/Docker.md)
