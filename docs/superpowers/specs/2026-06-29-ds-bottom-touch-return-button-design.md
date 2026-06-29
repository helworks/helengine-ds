# DS Bottom Touch Return Button Design

## Summary

Generated Nintendo DS showcase scenes need a visible bottom-screen back button that the user can tap to return to the demo-disc main menu. The clean path is to restore that behavior inside the shared DS scaffold instead of inventing a second input path.

## Goal

Add one scaffold-owned bottom-screen back button that:

- renders visibly in generated DS showcase scenes
- uses the existing `InteractableComponent` pointer path
- uses `NintendoDsReturnOverlayComponent` for scene navigation
- keeps the existing standard return action binding

## Approach

Update `C:\dev\helprojs\city\assets\codebase\rendering.tools\NintendoDsRenderingSceneScaffoldFactory.cs` so `CreateDefaultBottomOverlay(...)` creates:

- the existing `BOTTOM TEXT` label
- one visible back-button host entity
- one `SpriteComponent` using `Images/Menu/ds-back-button.png`
- one sibling `InteractableComponent` sized to the button
- one sibling `NintendoDsReturnOverlayComponent`
- one child `TextComponent` for the `BACK` label

The button remains scaffold-owned, so generated DS scenes keep one consistent return affordance without requiring per-scene authored setup.

## Non-Goals

This change does not:

- redesign the general return-to-menu component model
- add new DS-only engine input plumbing
- introduce a new generic button component
- change non-DS menu behavior

## Verification

Verification should cover:

- source-audit tests proving the scaffold emits the visible button contract
- a DS build launch confirming the button renders and touch returns to the menu
