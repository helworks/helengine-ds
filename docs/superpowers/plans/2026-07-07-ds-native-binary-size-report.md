# DS Native Binary Size Report Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Emit a DS native binary size report that explains final linked ROM composition by section and object file using the real linker map and binary artifacts.

**Architecture:** Extend the DS build workspace with explicit native artifact/report paths, add one generic-native linker map report service in the builder assembly, and invoke it after a successful native build so the exported DS package is accompanied by a text size report. Keep the report binary-only and backed by linked artifacts, not raw object guesses.

**Tech Stack:** C#, xUnit, DS builder assembly, GNU linker map parsing, Dockerized devkitARM native build

---

## File Structure

### Workspace and executor wiring

- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsBuildWorkspace.cs`
- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsNativeBuildExecutor.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsBuildWorkspaceTests.cs`

These files expose the `.map`, `.elf`, and exported report paths and wire report generation into the normal DS build flow.

### Native size reporting service

- Create: `C:\dev\helworks\helengine-ds\builder\NativeBinarySectionSize.cs`
- Create: `C:\dev\helworks\helengine-ds\builder\NativeBinaryObjectContribution.cs`
- Create: `C:\dev\helworks\helengine-ds\builder\NativeBinarySizeReport.cs`
- Create: `C:\dev\helworks\helengine-ds\builder\NativeBinarySizeReportWriter.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\NativeBinarySizeReportWriterTests.cs`

These files parse linker-map rows, aggregate linked contribution, and write one deterministic text report.

## Task 1: Lock workspace artifact paths

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\NintendoDsBuildWorkspaceTests.cs`
- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsBuildWorkspace.cs`

- [ ] **Step 1: Write the failing workspace-path test**

Add assertions for repository `.map` and `.elf` paths plus the exported size-report path.

- [ ] **Step 2: Run the workspace test to verify it fails**

Run: `dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter NintendoDsBuildWorkspaceTests`

Expected: FAIL because the workspace does not expose the new artifact paths yet.

- [ ] **Step 3: Implement the workspace path additions**

Add the new path properties and wire them into `Create(...)`.

- [ ] **Step 4: Run the workspace test to verify it passes**

Run the same command and confirm green.

## Task 2: Add failing report aggregation tests

**Files:**
- Create: `C:\dev\helworks\helengine-ds\builder.tests\NativeBinarySizeReportWriterTests.cs`
- Create: `C:\dev\helworks\helengine-ds\builder\NativeBinarySectionSize.cs`
- Create: `C:\dev\helworks\helengine-ds\builder\NativeBinaryObjectContribution.cs`
- Create: `C:\dev\helworks\helengine-ds\builder\NativeBinarySizeReport.cs`
- Create: `C:\dev\helworks\helengine-ds\builder\NativeBinarySizeReportWriter.cs`

- [ ] **Step 1: Write one failing aggregation test**

Use a small synthetic linker-map fixture that includes `.text`, `.rodata`, `.data`, `.bss`, and `.ARM.exidx` rows across multiple objects. Assert section totals, object totals, and report ordering.

- [ ] **Step 2: Run the report test to verify it fails**

Run: `dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter NativeBinarySizeReportWriterTests`

Expected: FAIL because the report writer types do not exist yet.

- [ ] **Step 3: Implement the minimal report model and parser**

Parse the GNU linker map rows, normalize section roots, aggregate totals, and format deterministic text output.

- [ ] **Step 4: Run the report test to verify it passes**

Run the same filtered test command and confirm green.

## Task 3: Wire the report into DS native builds

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\builder\NintendoDsNativeBuildExecutor.cs`

- [ ] **Step 1: Add the build-time report generation call**

Run the report writer after the native package and linked artifacts are validated and after the `.nds` package has been exported.

- [ ] **Step 2: Keep failure behavior strict**

If the `.map`, `.elf`, or exported `.nds` file is missing, let the build fail rather than silently skipping the report.

- [ ] **Step 3: Run focused builder tests**

Run: `dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj`

Expected: PASS.

## Task 4: Verify against the real DS build

**Files:**
- Verify output: `C:\dev\helprojs\city\ds-build\helengine_ds.nds`
- Verify output: `C:\dev\helprojs\city\ds-build\helengine_ds-native-binary-size-report.txt`

- [ ] **Step 1: Run one real DS build**

Use the existing platform build flow that produces the current `city` DS output.

- [ ] **Step 2: Inspect the generated report**

Confirm the text report exists and includes the package size, ELF size, section totals, and top object contributors.

- [ ] **Step 3: Summarize the largest linked contributors**

Report the top offenders to drive the next generic engine-wide size pass.
