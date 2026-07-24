# Helengine Nintendo DS Host

This repository contains the Nintendo DS platform host and builder integration for Helengine.

## Build

```powershell
dotnet run --project ..\helengine\tools\build-waiter\helengine.buildwaiter.csproj -- `
  --output ..\helprojs\city\ds-build `
  --require helengine_ds.nds `
  -- powershell -NoProfile -ExecutionPolicy Bypass -File ..\helengine\scripts\build-platform.ps1 `
  -Project ..\helprojs\city\project.heproj `
  -Platform ds `
  -Output ..\helprojs\city\ds-build
```

The Build Waiter returns successfully only after `helengine_ds.nds` is fresh and non-empty.

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
