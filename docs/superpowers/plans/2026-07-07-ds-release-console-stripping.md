# DS Release Console Stripping Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Make `HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS=0` compile out the DS host console and formatted diagnostics stack from `NintendoDsBootHost.cpp`.

**Architecture:** Keep the ownership seam at the existing DS compile-time diagnostics flag. Lock the intended source structure with a builder audit test, then move the console-backed boot, runtime-failure, and fatal formatting paths behind the diagnostics branch while preserving lean no-diagnostics fallbacks.

**Tech Stack:** C#, xUnit, C++, libnds, Nintendo DS native build

---

### Task 1: Lock The Source Contract

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsBootHostSourceAuditTests.cs`
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsBootHost.cpp`

- [ ] **Step 1: Write the failing test**

Add a source audit asserting the no-diagnostics DS boot host path does not keep console-backed fatal/runtime formatting unguarded.

- [ ] **Step 2: Run test to verify it fails**

Run: `rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\builder.tests.csproj --filter NintendoDsBootHostSourceAuditTests`

Expected: FAIL because `NintendoDsBootHost.cpp` still contains unguarded `iprintf` and `std::snprintf` usage in fatal or runtime-failure paths.

- [ ] **Step 3: Write minimal implementation**

Move the console-backed boot-host helpers and failure formatting behind `#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS`, and add lean no-diagnostics fallbacks.

- [ ] **Step 4: Run test to verify it passes**

Run: `rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\builder.tests.csproj --filter NintendoDsBootHostSourceAuditTests`

Expected: PASS.

- [ ] **Step 5: Commit**

Commit the guarded DS host console stripping once the focused audit is green.

### Task 2: Verify Builder Coverage

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\src\platform\ds\NintendoDsBootHost.cpp`
- Modify: any additional DS builder tests only if the implementation needs extra coverage

- [ ] **Step 1: Run focused DS builder tests**

Run: `rtk dotnet test C:\dev\helworks\helengine-ds\builder.tests\builder.tests.csproj`

Expected: PASS for the DS builder suite after the source audit update.

- [ ] **Step 2: Commit**

Commit any follow-up test adjustments only if the focused suite required them.

### Task 3: Measure DS Binary Impact

**Files:**
- Inspect: `C:\dev\helworks\helengine-ds\build\helengine_ds.map`
- Inspect: `C:\dev\helprojs\city\ds-build\helengine_ds-native-binary-size-report.txt`

- [ ] **Step 1: Rebuild the city DS target**

Run the canonical DS build:

`rtk powershell -NoProfile -ExecutionPolicy Bypass -File C:\dev\helworks\helengine\artifacts\build-platform.ps1 -Project C:\dev\helprojs\city\project.heproj -Platform ds -Output C:\dev\helprojs\city\ds-build`

- [ ] **Step 2: Compare the report**

Check the package size, accounted native binary size, and the console/printf chain against the current baseline:

- `Package Size: 1933312`
- `Accounted Native Binary Size: 769374`
- `libc_a-vfiprintf.o: 33370`
- `libc_a-svfprintf.o: 9988`
- `libc_a-svfiprintf.o: 7369`
- `libc_a-svfiscanf.o: 7040`
- `libnds9.a(console.o): 4534`

- [ ] **Step 3: Commit**

Commit the verified DS host size reduction after the native report confirms the effect.
