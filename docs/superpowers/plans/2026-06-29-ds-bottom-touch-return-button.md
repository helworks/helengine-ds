# DS Bottom Touch Return Button Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a visible bottom-screen Nintendo DS back button to generated showcase scenes so touch returns to the demo-disc main menu.

**Architecture:** The shared DS scaffold will author a sprite-backed button host beside the existing bottom test text. The button will reuse `InteractableComponent` and `NintendoDsReturnOverlayComponent` so touch routing and scene loading stay on the existing DS return path.

**Tech Stack:** C#, city authored scene generators, xUnit source-audit tests, Nintendo DS build pipeline

---

### Task 1: Lock The Scaffold Contract In Tests

**Files:**
- Modify: `C:\dev\helworks\helengine-ds\builder.tests\CityNintendoDsSceneSourceAuditTests.cs`
- Test: `C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Write the failing test**

Add a source-audit assertion proving `CreateDefaultBottomOverlay(...)` emits:
- `DemoDiscBottomScreenBackButton`
- `new SpriteComponent`
- `new InteractableComponent`
- `new NintendoDsReturnOverlayComponent()`
- `Text = "BACK"`

- [ ] **Step 2: Run test to verify it fails**

Run: `dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter FullyQualifiedName~Sources_whenAddingDsBottomReturnButton_emitVisibleTouchableBackOverlay -v minimal`

Expected: FAIL because the scaffold currently only emits the temporary text row.

### Task 2: Implement The DS Bottom Back Button

**Files:**
- Modify: `C:\dev\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs`

- [ ] **Step 1: Write minimal implementation**

Extend `CreateDefaultBottomOverlay(...)` with one sprite-backed back-button host, sized interactable, return overlay component, texture reference, and `BACK` label child.

- [ ] **Step 2: Run test to verify it passes**

Run: `dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter FullyQualifiedName~Sources_whenAddingDsBottomReturnButton_emitVisibleTouchableBackOverlay -v minimal`

Expected: PASS

### Task 3: Verify The Existing DS Scaffold Contract Still Holds

**Files:**
- Test: `C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj`

- [ ] **Step 1: Run the narrow DS scaffold tests**

Run: `dotnet test C:\dev\helworks\helengine-ds\builder.tests\helengine.ds.builder.tests.csproj --filter FullyQualifiedName~Sources_whenRelocatingFpsToDsBottomScreen_assignRuntimeLayerMaskToViewportAndFpsHosts -v minimal`

Expected: PASS

- [ ] **Step 2: Build and launch the DS output**

Run the existing DS build command and launch in melonDS to confirm the button renders and touch returns to menu.
