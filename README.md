# Helengine Nintendo DS Host

This repository contains the native Nintendo DS host scaffold and Nintendo DS platform builder for Helengine.

## Current milestone

- Docker-only build using devkitPro Nintendo DS tooling
- Managed DS builder scaffold under `builder/`
- Native `.nds` output for direct loading in melonDS or DeSmuME
- Builder-owned startup manifest packaged through NitroFS
- First packaged boot check with manifest-driven screen colors

## Native build

```bash
docker build -t helengine-ds .
docker run --rm -v "$PWD":/workspace -w /workspace helengine-ds make
```

The build emits `build/helengine_ds.nds`.

## Builder tests

```bash
dotnet test builder.tests/helengine.ds.builder.tests.csproj -p:HelengineRoot=/mnt/c/dev/helworks/helengine
```

## Generated core seam

The native build reserves `HELENGINE_CORE_CPP_ROOT` for later `cs2.cpp` integration, but the first milestone does not compile generated core output yet.

## NitroFS startup manifest check

The DS builder writes `runtime/ds_startup_manifest.bin` into a staged NitroFS root and the runtime loads it from `nitro:/runtime/ds_startup_manifest.bin`.

To test the native package manually without the editor, create a staged NitroFS root with a manifest and pass it into the Docker build:

```bash
mkdir -p /tmp/helengine-ds-manual/runtime
printf '\x48\x44\x53\x50\x01\x00\x04\x00\x1F\x80\x00\xFC' > /tmp/helengine-ds-manual/runtime/ds_startup_manifest.bin
docker run --rm -v "$PWD":/workspace -v /tmp/helengine-ds-manual:/workspace-staging -w /workspace helengine-ds make HELENGINE_DS_NITROFS_ROOT=/workspace-staging
```

Boot `build/helengine_ds.nds` in melonDS or DeSmuME. The expected result is a red top screen and a blue bottom screen. If the manifest is missing or invalid, the runtime remains on the green/cyan bootstrap frame.
