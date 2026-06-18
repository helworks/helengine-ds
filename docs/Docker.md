# Nintendo DS Docker Build

Use the low-level native Docker flow when you need to build the DS host directly instead of using the shared editor wrapper.

```bash
docker build -t helengine-ds .
docker run --rm -v "$PWD":/workspace -w /workspace helengine-ds make
```

The native build still runs through the checked-in devkitPro Docker image and `Makefile`.

If you need manual NitroFS experiments, the native target still accepts `HELENGINE_DS_NITROFS_ROOT` through the `Makefile`.
