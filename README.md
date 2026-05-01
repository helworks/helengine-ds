# Helengine Nintendo DS Host

This repository contains the native Nintendo DS host scaffold for Helengine.

## Current milestone

- Docker-only build using devkitPro Nintendo DS tooling
- Native `.nds` output for direct loading in melonDS or DeSmuME
- First boot check with a green top screen and cyan bottom screen

## Build

```bash
docker build -t helengine-ds .
docker run --rm -v "$PWD":/workspace -w /workspace helengine-ds make
```

The build emits `build/helengine_ds.nds`.

## Generated core seam

The native build reserves `HELENGINE_CORE_CPP_ROOT` for later `cs2.cpp` integration, but the first milestone does not compile generated core output yet.

## Boot check

Load `build/helengine_ds.nds` in melonDS or DeSmuME. The expected result for this milestone is a green top screen and a cyan bottom screen with no immediate crash or reset loop.
