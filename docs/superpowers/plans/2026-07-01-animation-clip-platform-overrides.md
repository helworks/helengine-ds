# Animation Clip Platform Overrides Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add base-plus-platform override authoring to `.hanim` assets, resolve flat platform-specific clips at cook time, and make the DS demo-disc logo use the shared animation asset with DS-specific authored motion.

**Architecture:** Extend `AnimationClipAsset` to carry editor-facing platform override data and stable frame IDs, but keep runtime playback flat by resolving overrides into a plain clip during platform cooking. Add an animation-clip asset editor view in the editor properties panel using the same per-platform tab pattern as material editing, then update the city demo-disc DS logo path to attach the same animation player used elsewhere.

**Tech Stack:** C#, `helengine.core`, `helengine.files`, `helengine.editor`, xUnit, source-audit tests, city menu scene factory, `.hanim` binary assets.

---

## File Structure

### Core animation asset model

- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\animation\AnimationClipAsset.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\animation\PositionKeyframeAsset.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\animation\RotationKeyframeAsset.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\animation\AnimationClipPlatformOverrideMode.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\animation\AnimationClipPlatformOverrideAsset.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\animation\PlatformPositionKeyframeTrackAsset.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\animation\PlatformRotationKeyframeTrackAsset.cs`

These files define the editor-authored shape of one `.hanim` asset, including frame IDs and per-platform override mode/data.

### Serialization and cook resolution

- Modify: `C:\dev\helworks\helengine\engine\helengine.files\assets\EditorAssetBinarySerializer.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\EditorAssetBinarySerializer.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\AnimationClipPlatformResolutionService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorPlatformAssetCookService.cs`

These files round-trip the new asset data and flatten it into one runtime-ready clip during cook.

### Editor asset UI

- Create: `C:\dev\helworks\helengine\engine\helengine.editor\components\ui\AnimationClipAssetView.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\components\ui\AnimationClipAssetPlatformPanel.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\components\ui\PropertiesPanel.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\EditorSession.cs`

These files add animation clip editing to the existing properties-panel asset workflow.

### Tests

- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AnimationClipPlatformOverrideSerializationTests.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AnimationClipPlatformResolutionTests.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AnimationClipAssetEditorSourceTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityDemoDiscLogoAnimationSourceTests.cs`

These files lock serialization, cook resolution, editor wiring, and the DS logo source path.

### City integration and authored asset

- Modify: `C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs`
- Modify: `C:\dev\helprojs\city\assets\Animations\DemoDiscLogoIdle.hanim`
- Create: `C:\dev\helworks\helengine-ds\scratch\DemoDiscLogoDsOverrideAuthoring\DemoDiscLogoDsOverrideAuthoring.csproj`
- Create: `C:\dev\helworks\helengine-ds\scratch\DemoDiscLogoDsOverrideAuthoring\Program.cs`

These files switch the DS logo path back to the shared clip and author deterministic DS override content into the logo asset.

---

### Task 1: Extend AnimationClipAsset With Platform Override Data

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AnimationClipPlatformOverrideSerializationTests.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\animation\AnimationClipAsset.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\animation\PositionKeyframeAsset.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\animation\RotationKeyframeAsset.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\animation\AnimationClipPlatformOverrideMode.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\animation\AnimationClipPlatformOverrideAsset.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\animation\PlatformPositionKeyframeTrackAsset.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.core\assets\raw\animation\PlatformRotationKeyframeTrackAsset.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.files\assets\EditorAssetBinarySerializer.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.core\assets\EditorAssetBinarySerializer.cs`

- [ ] **Step 1: Write the failing serialization test**

```csharp
using Xunit;

namespace helengine.editor.tests {
    /// <summary>
    /// Verifies animation clips round-trip editor-authored platform override data.
    /// </summary>
    public sealed class AnimationClipPlatformOverrideSerializationTests {
        /// <summary>
        /// Ensures base frames, editor-only frame ids, and per-platform override payloads survive serialization.
        /// </summary>
        [Fact]
        public void AssetSerializer_AnimationClipAsset_RoundTripsPlatformOverrides() {
            AnimationClipAsset asset = new AnimationClipAsset {
                Id = "Animations/TestPlatformOverrides.hanim",
                Duration = 1.25f,
                PositionTracks = [
                    new PositionKeyframeTrackAsset {
                        Keyframes = [
                            new PositionKeyframeAsset(0f, new float3(0f, 0f, 0f), AnimationInterpolationMode.Step) {
                                FrameId = "base-pos-000"
                            },
                            new PositionKeyframeAsset(1f, new float3(16f, 0f, 0f), AnimationInterpolationMode.Linear) {
                                FrameId = "base-pos-001"
                            }
                        ]
                    }
                ],
                PlatformOverrides = [
                    new AnimationClipPlatformOverrideAsset {
                        PlatformId = "ds",
                        Mode = AnimationClipPlatformOverrideMode.OverrideFrames,
                        PositionTracks = [
                            new PlatformPositionKeyframeTrackAsset {
                                Keyframes = [
                                    new PositionKeyframeAsset(1f, new float3(8f, -4f, 0f), AnimationInterpolationMode.Linear) {
                                        FrameId = "base-pos-001"
                                    },
                                    new PositionKeyframeAsset(0.5f, new float3(3f, 2f, 0f), AnimationInterpolationMode.Step) {
                                        FrameId = "ds-insert-000"
                                    }
                                ]
                            }
                        ]
                    }
                ]
            };

            byte[] data = AssetSerializer.SerializeToBytes(asset);
            AnimationClipAsset deserialized = Assert.IsType<AnimationClipAsset>(AssetSerializer.DeserializeFromBytes(data));

            Assert.Equal("base-pos-000", deserialized.PositionTracks[0].Keyframes[0].FrameId);
            AnimationClipPlatformOverrideAsset dsOverride = Assert.Single(deserialized.PlatformOverrides);
            Assert.Equal("ds", dsOverride.PlatformId);
            Assert.Equal(AnimationClipPlatformOverrideMode.OverrideFrames, dsOverride.Mode);
            Assert.Equal("base-pos-001", dsOverride.PositionTracks[0].Keyframes[0].FrameId);
            Assert.Equal("ds-insert-000", dsOverride.PositionTracks[0].Keyframes[1].FrameId);
        }
    }
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter FullyQualifiedName~AnimationClipPlatformOverrideSerializationTests`

Expected: FAIL because `FrameId`, `PlatformOverrides`, `AnimationClipPlatformOverrideAsset`, and the override-mode enum do not exist yet.

- [ ] **Step 3: Add the minimal asset model and serializer support**

```csharp
namespace helengine {
    /// <summary>
    /// Selects how one platform resolves a clip relative to the authored base timeline.
    /// </summary>
    public enum AnimationClipPlatformOverrideMode {
        InheritBase = 0,
        ReplaceWholeClip = 1,
        OverrideFrames = 2
    }

    /// <summary>
    /// Stores one platform-specific override payload for an animation clip.
    /// </summary>
    public class AnimationClipPlatformOverrideAsset {
        /// <summary>
        /// Gets or sets the platform identifier that owns this override payload.
        /// </summary>
        public string PlatformId { get; set; } = string.Empty;

        /// <summary>
        /// Gets or sets how this platform resolves relative to the base clip.
        /// </summary>
        public AnimationClipPlatformOverrideMode Mode { get; set; }

        /// <summary>
        /// Gets or sets the platform-authored position tracks.
        /// </summary>
        public PlatformPositionKeyframeTrackAsset[] PositionTracks { get; set; } = Array.Empty<PlatformPositionKeyframeTrackAsset>();

        /// <summary>
        /// Gets or sets the platform-authored additive position tracks.
        /// </summary>
        public PlatformPositionKeyframeTrackAsset[] PositionOffsetTracks { get; set; } = Array.Empty<PlatformPositionKeyframeTrackAsset>();

        /// <summary>
        /// Gets or sets the platform-authored scale tracks.
        /// </summary>
        public PlatformPositionKeyframeTrackAsset[] ScaleTracks { get; set; } = Array.Empty<PlatformPositionKeyframeTrackAsset>();

        /// <summary>
        /// Gets or sets the platform-authored rotation tracks.
        /// </summary>
        public PlatformRotationKeyframeTrackAsset[] RotationTracks { get; set; } = Array.Empty<PlatformRotationKeyframeTrackAsset>();
    }
}
```

```csharp
namespace helengine {
    /// <summary>
    /// Represents one keyframe-based animation clip containing multiple typed transform tracks.
    /// </summary>
    public class AnimationClipAsset : Asset {
        /// <summary>
        /// Gets or sets the authored clip duration in seconds.
        /// </summary>
        public float Duration { get; set; }

        /// <summary>
        /// Gets or sets the absolute position tracks stored by this clip.
        /// </summary>
        public PositionKeyframeTrackAsset[] PositionTracks { get; set; } = Array.Empty<PositionKeyframeTrackAsset>();

        /// <summary>
        /// Gets or sets the additive position-offset tracks stored by this clip.
        /// </summary>
        public PositionOffsetKeyframeTrackAsset[] PositionOffsetTracks { get; set; } = Array.Empty<PositionOffsetKeyframeTrackAsset>();

        /// <summary>
        /// Gets or sets the absolute scale tracks stored by this clip.
        /// </summary>
        public ScaleKeyframeTrackAsset[] ScaleTracks { get; set; } = Array.Empty<ScaleKeyframeTrackAsset>();

        /// <summary>
        /// Gets or sets the absolute rotation tracks stored by this clip.
        /// </summary>
        public RotationKeyframeTrackAsset[] RotationTracks { get; set; } = Array.Empty<RotationKeyframeTrackAsset>();

        /// <summary>
        /// Gets or sets platform-authored override payloads for this clip.
        /// </summary>
        public AnimationClipPlatformOverrideAsset[] PlatformOverrides { get; set; } = Array.Empty<AnimationClipPlatformOverrideAsset>();
    }
}
```

```csharp
public class PositionKeyframeAsset {
    /// <summary>
    /// Gets or sets the stable editor-only frame identifier.
    /// </summary>
    public string FrameId { get; set; } = string.Empty;

    public float Time { get; set; }
    public float3 Value { get; set; }
    public AnimationInterpolationMode InterpolationMode { get; set; }
}
```

```csharp
static void WriteAnimationClipAsset(EngineBinaryWriter writer, AnimationClipAsset asset) {
    EnsureRuntimeAssetIdentity(asset);
    WriteAssetIdentity(writer, asset);
    writer.WriteSingle(asset.Duration);
    writer.WriteArray(asset.PositionTracks, WritePositionKeyframeTrackAsset);
    writer.WriteArray(asset.PositionOffsetTracks, WritePositionOffsetKeyframeTrackAsset);
    writer.WriteArray(asset.ScaleTracks, WriteScaleKeyframeTrackAsset);
    writer.WriteArray(asset.RotationTracks, WriteRotationKeyframeTrackAsset);
    writer.WriteArray(asset.PlatformOverrides, WriteAnimationClipPlatformOverrideAsset);
}
```

- [ ] **Step 4: Run serialization tests**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~AnimationClipSerializationTests|FullyQualifiedName~AnimationClipPlatformOverrideSerializationTests"`

Expected: PASS for the existing clip serializer test and the new platform override round-trip test.

- [ ] **Step 5: Commit**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.core/assets/raw/animation engine/helengine.core/assets/EditorAssetBinarySerializer.cs engine/helengine.files/assets/EditorAssetBinarySerializer.cs engine/helengine.editor.tests/AnimationClipPlatformOverrideSerializationTests.cs
git -C C:\dev\helworks\helengine commit -m "Add animation clip platform override data model"
```

### Task 2: Resolve Platform-Specific Animation Clips During Cook

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AnimationClipPlatformResolutionTests.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\AnimationClipPlatformResolutionService.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\managers\project\EditorPlatformAssetCookService.cs`

- [ ] **Step 1: Write the failing resolution tests**

```csharp
using Xunit;

namespace helengine.editor.tests {
    /// <summary>
    /// Verifies cook-time resolution of animation clip platform override modes.
    /// </summary>
    public sealed class AnimationClipPlatformResolutionTests {
        readonly AnimationClipPlatformResolutionService Service = new AnimationClipPlatformResolutionService();

        [Fact]
        public void ResolveForPlatform_WhenModeIsInheritBase_ReturnsBaseTimeline() {
            AnimationClipAsset clip = AnimationClipPlatformOverrideTestFactory.CreateBaseClipWithDsInherit();

            AnimationClipAsset resolved = Service.ResolveForPlatform(clip, "ds");

            Assert.Empty(resolved.PlatformOverrides);
            Assert.Equal("base-pos-001", resolved.PositionTracks[0].Keyframes[1].FrameId);
            Assert.Equal(2, resolved.PositionTracks[0].Keyframes.Length);
        }

        [Fact]
        public void ResolveForPlatform_WhenModeIsOverrideFrames_MergesAndSortsByTimestamp() {
            AnimationClipAsset clip = AnimationClipPlatformOverrideTestFactory.CreateClipWithDsOverrides();

            AnimationClipAsset resolved = Service.ResolveForPlatform(clip, "ds");

            PositionKeyframeAsset[] keyframes = resolved.PositionTracks[0].Keyframes;
            Assert.Collection(
                keyframes,
                keyframe => Assert.Equal(0f, keyframe.Time),
                keyframe => Assert.Equal(0.5f, keyframe.Time),
                keyframe => Assert.Equal(1f, keyframe.Time));
            Assert.All(keyframes, keyframe => Assert.True(string.IsNullOrEmpty(keyframe.FrameId)));
            Assert.Empty(resolved.PlatformOverrides);
        }
    }
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter FullyQualifiedName~AnimationClipPlatformResolutionTests`

Expected: FAIL because `AnimationClipPlatformResolutionService` and the test factory do not exist yet.

- [ ] **Step 3: Implement cook-time clip flattening**

```csharp
namespace helengine.editor {
    /// <summary>
    /// Resolves one editor-authored animation clip into the flat platform-specific clip consumed by runtime playback.
    /// </summary>
    public sealed class AnimationClipPlatformResolutionService {
        /// <summary>
        /// Resolves one clip for the supplied platform identifier.
        /// </summary>
        public AnimationClipAsset ResolveForPlatform(AnimationClipAsset clip, string platformId) {
            if (clip == null) {
                throw new ArgumentNullException(nameof(clip));
            } else if (string.IsNullOrWhiteSpace(platformId)) {
                throw new ArgumentException("Platform id must be provided.", nameof(platformId));
            }

            AnimationClipPlatformOverrideAsset platformOverride = clip.PlatformOverrides.FirstOrDefault(
                entry => string.Equals(entry.PlatformId, platformId, StringComparison.OrdinalIgnoreCase));
            if (platformOverride == null || platformOverride.Mode == AnimationClipPlatformOverrideMode.InheritBase) {
                return CloneResolvedClip(clip, clip.PositionTracks, clip.PositionOffsetTracks, clip.ScaleTracks, clip.RotationTracks);
            }
            if (platformOverride.Mode == AnimationClipPlatformOverrideMode.ReplaceWholeClip) {
                return CloneResolvedClip(
                    clip,
                    ConvertResolvedPositionTracks(platformOverride.PositionTracks),
                    ConvertResolvedPositionOffsetTracks(platformOverride.PositionOffsetTracks),
                    ConvertResolvedScaleTracks(platformOverride.ScaleTracks),
                    ConvertResolvedRotationTracks(platformOverride.RotationTracks));
            }

            return ResolveOverrideFrames(clip, platformOverride);
        }
    }
```

```csharp
if (asset is AnimationClipAsset animationClipAsset) {
    AnimationClipAsset resolvedClip = AnimationClipPlatformResolutionService.ResolveForPlatform(animationClipAsset, request.PlatformId);
    File.WriteAllBytes(fullOutputPath, AssetSerializer.SerializeToBytes(resolvedClip));
    continue;
}
```

- [ ] **Step 4: Run resolution tests**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~AnimationClipPlatformResolutionTests|FullyQualifiedName~AnimationClipPlatformOverrideSerializationTests"`

Expected: PASS, including timestamp ordering and stripped editor-only override metadata in the resolved clip.

- [ ] **Step 5: Commit**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.editor/managers/project/AnimationClipPlatformResolutionService.cs engine/helengine.editor/managers/project/EditorPlatformAssetCookService.cs engine/helengine.editor.tests/AnimationClipPlatformResolutionTests.cs
git -C C:\dev\helworks\helengine commit -m "Resolve animation clip platform overrides during cook"
```

### Task 3: Add Animation Clip Editing To The Properties Panel

**Files:**
- Create: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AnimationClipAssetEditorSourceTests.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\components\ui\AnimationClipAssetView.cs`
- Create: `C:\dev\helworks\helengine\engine\helengine.editor\components\ui\AnimationClipAssetPlatformPanel.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\components\ui\PropertiesPanel.cs`
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor\EditorSession.cs`

- [ ] **Step 1: Write the failing source-audit tests for editor routing**

```csharp
using Xunit;

namespace helengine.editor.tests {
    /// <summary>
    /// Audits editor source so animation clips get a dedicated per-platform asset editor.
    /// </summary>
    public sealed class AnimationClipAssetEditorSourceTests {
        [Fact]
        public void EditorSession_routes_hanim_assets_to_animation_clip_settings_view() {
            string source = File.ReadAllText(@"C:\dev\helworks\helengine\engine\helengine.editor\EditorSession.cs");

            Assert.Contains("bool IsAnimationClipAssetEntry(AssetBrowserEntry entry)", source, StringComparison.Ordinal);
            Assert.Contains("propertiesPanels[index].ShowAnimationClipSettings(", source, StringComparison.Ordinal);
            Assert.Contains("entry.Extension", source, StringComparison.Ordinal);
            Assert.Contains(".hanim", source, StringComparison.Ordinal);
        }

        [Fact]
        public void PropertiesPanel_owns_animation_clip_asset_view() {
            string source = File.ReadAllText(@"C:\dev\helworks\helengine\engine\helengine.editor\components\ui\PropertiesPanel.cs");

            Assert.Contains("readonly AnimationClipAssetView AnimationClipView;", source, StringComparison.Ordinal);
            Assert.Contains("public void ShowAnimationClipSettings(", source, StringComparison.Ordinal);
            Assert.Contains("AnimationClipView.Show(", source, StringComparison.Ordinal);
        }
    }
}
```

- [ ] **Step 2: Run tests to verify they fail**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter FullyQualifiedName~AnimationClipAssetEditorSourceTests`

Expected: FAIL because the animation clip asset editor view and routing methods do not exist yet.

- [ ] **Step 3: Add the new properties-panel asset view and routing**

```csharp
public class PropertiesPanel : DockableEntity {
    readonly AssetImportSettingsView importSettingsView;
    readonly MaterialAssetView MaterialView;
    readonly AnimationClipAssetView AnimationClipView;

    public void ShowAnimationClipSettings(
        AssetBrowserEntry entry,
        AnimationClipAsset clipAsset,
        IReadOnlyList<string> supportedPlatforms,
        string activePlatformId) {
        if (entry == null) {
            throw new ArgumentNullException(nameof(entry));
        } else if (clipAsset == null) {
            throw new ArgumentNullException(nameof(clipAsset));
        }

        DeactivateSelectedEntityTransformProjection();
        currentEntry = entry;
        HideRemoveComponentDialog();
        importSettingsView.Hide();
        MaterialView.Hide();
        AnimationClipView.Show(entry, clipAsset, supportedPlatforms, activePlatformId);
        ComponentPlatformTabStrip.Root.Enabled = false;
        SetTransformVisible(false);
        ComponentView.Hide();
        ApplyLines(Array.Empty<string>());
        LayoutLines();
    }
}
```

```csharp
bool IsAnimationClipAssetEntry(AssetBrowserEntry entry) {
    if (entry == null) {
        return false;
    }

    return string.Equals(entry.Extension, ".hanim", StringComparison.OrdinalIgnoreCase);
}
```

```csharp
if (IsAnimationClipAssetEntry(entry)) {
    string fullPath = entry.FullPath;
    using FileStream stream = File.OpenRead(fullPath);
    AnimationClipAsset clipAsset = (AnimationClipAsset)AssetSerializer.Deserialize(stream);
    for (int index = 0; index < propertiesPanels.Count; index++) {
        propertiesPanels[index].ShowAnimationClipSettings(entry, clipAsset, SupportedPlatforms, CurrentProjectPlatform);
    }
    return;
}
```

- [ ] **Step 4: Run source-audit tests**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~AnimationClipAssetEditorSourceTests|FullyQualifiedName~AnimationClipPlatformResolutionTests"`

Expected: PASS, proving the editor owns a dedicated animation-clip view and routes `.hanim` assets through it.

- [ ] **Step 5: Commit**

```bash
git -C C:\dev\helworks\helengine add engine/helengine.editor/components/ui/AnimationClipAssetView.cs engine/helengine.editor/components/ui/AnimationClipAssetPlatformPanel.cs engine/helengine.editor/components/ui/PropertiesPanel.cs engine/helengine.editor/EditorSession.cs engine/helengine.editor.tests/AnimationClipAssetEditorSourceTests.cs
git -C C:\dev\helworks\helengine commit -m "Add per-platform animation clip asset editor"
```

### Task 4: Restore The Shared Logo Animation Path And Author A DS Override

**Files:**
- Modify: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityDemoDiscLogoAnimationSourceTests.cs`
- Modify: `C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs`
- Create: `C:\dev\helworks\helengine-ds\scratch\DemoDiscLogoDsOverrideAuthoring\DemoDiscLogoDsOverrideAuthoring.csproj`
- Create: `C:\dev\helworks\helengine-ds\scratch\DemoDiscLogoDsOverrideAuthoring\Program.cs`
- Modify: `C:\dev\helprojs\city\assets\Animations\DemoDiscLogoIdle.hanim`

- [ ] **Step 1: Tighten the source test so DS must use the shared clip path too**

```csharp
[Fact]
public void City_demo_disc_logo_idle_animation_source_uses_animation_player_component_for_ds_and_non_ds_paths() {
    string sourcePath = @"C:\dev\helprojs\city\assets\codebase\menu.tools\DemoDiscMainMenuSceneFactory.cs";
    string source = File.ReadAllText(sourcePath);

    Assert.Contains("void CreateNintendoDsTopScreenLogoEntity(Entity topScreenRootEntity, MenuOverlayImageDefinition overlayImage)", source, StringComparison.Ordinal);
    Assert.Equal(2, Regex.Matches(source, "Clip = LoadRequiredAnimationClipAsset\\(DemoDiscLogoIdleAnimationRelativePath\\),").Count);
    Assert.Equal(2, Regex.Matches(source, "ApplyAnimationClipReference\\(entity, animationPlayerComponent, DemoDiscLogoIdleAnimationRelativePath\\);").Count);
}
```

- [ ] **Step 2: Run the source test to verify it fails**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter FullyQualifiedName~CityDemoDiscLogoAnimationSourceTests`

Expected: FAIL because the DS logo path is still static and only the non-DS path attaches the animation player.

- [ ] **Step 3: Attach the shared animation player in the DS menu path**

```csharp
void CreateNintendoDsTopScreenLogoEntity(Entity topScreenRootEntity, MenuOverlayImageDefinition overlayImage) {
    if (topScreenRootEntity == null) {
        throw new ArgumentNullException(nameof(topScreenRootEntity));
    } else if (overlayImage == null) {
        throw new ArgumentNullException(nameof(overlayImage));
    }

    int displayWidth = ResolveNintendoDsLogoWidth(overlayImage);
    int displayHeight = ResolveNintendoDsLogoHeight(overlayImage, displayWidth);
    Entity entity = Core.Instance.EntityFactory.CreateChild(topScreenRootEntity, "DemoDiscOverlayImage");
    entity.LocalPosition = new float3((NintendoDsScreenWidth - displayWidth) * 0.5f, 0f, 0f);

    SpriteComponent spriteComponent = new SpriteComponent {
        Size = new int2(displayWidth, displayHeight),
        RenderOrder2D = 20,
        LayerMask = RuntimeLayerMask
    };
    entity.AddComponent(spriteComponent);
    ApplyTextureReference(entity, spriteComponent, overlayImage.TexturePath);

    AnimationPlayerComponent animationPlayerComponent = new AnimationPlayerComponent {
        Clip = LoadRequiredAnimationClipAsset(DemoDiscLogoIdleAnimationRelativePath),
        PlayAutomatically = true,
        ShouldLoop = true
    };
    entity.AddComponent(animationPlayerComponent);
    ApplyAnimationClipReference(entity, animationPlayerComponent, DemoDiscLogoIdleAnimationRelativePath);
}
```

- [ ] **Step 4: Author the DS override into the shared `.hanim` asset deterministically**

```csharp
AnimationClipAsset clip = Assert.IsType<AnimationClipAsset>(AssetSerializer.DeserializeFromBytes(File.ReadAllBytes(AnimationPath)));
AnimationClipPlatformOverrideAsset dsOverride = new AnimationClipPlatformOverrideAsset {
    PlatformId = "ds",
    Mode = AnimationClipPlatformOverrideMode.OverrideFrames,
    PositionTracks = [
        new PlatformPositionKeyframeTrackAsset {
            Keyframes = [
                new PositionKeyframeAsset(0f, new float3(0f, 0f, 0f), AnimationInterpolationMode.Step) { FrameId = "ds-logo-pos-000" },
                new PositionKeyframeAsset(0.25f, new float3(0f, -2f, 0f), AnimationInterpolationMode.Linear) { FrameId = "ds-logo-pos-001" },
                new PositionKeyframeAsset(0.5f, new float3(0f, 0f, 0f), AnimationInterpolationMode.Linear) { FrameId = "ds-logo-pos-002" }
            ]
        }
    ],
    ScaleTracks = [
        new PlatformPositionKeyframeTrackAsset {
            Keyframes = [
                new PositionKeyframeAsset(0f, new float3(1f, 1f, 1f), AnimationInterpolationMode.Step) { FrameId = "ds-logo-scale-000" },
                new PositionKeyframeAsset(0.5f, new float3(1.04f, 1.04f, 1f), AnimationInterpolationMode.Linear) { FrameId = "ds-logo-scale-001" },
                new PositionKeyframeAsset(1f, new float3(1f, 1f, 1f), AnimationInterpolationMode.Linear) { FrameId = "ds-logo-scale-002" }
            ]
        }
    ]
};

clip.PlatformOverrides = clip.PlatformOverrides
    .Where(entry => !string.Equals(entry.PlatformId, "ds", StringComparison.OrdinalIgnoreCase))
    .Append(dsOverride)
    .ToArray();
File.WriteAllBytes(AnimationPath, AssetSerializer.SerializeToBytes(clip));
```

- [ ] **Step 5: Run source tests and commit**

Run: `rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~CityDemoDiscLogoAnimationSourceTests|FullyQualifiedName~AnimationClipAssetEditorSourceTests"`

Expected: PASS, proving DS and non-DS both reference the shared clip path.

```bash
git -C C:\dev\helworks\helengine add engine/helengine.editor.tests/CityDemoDiscLogoAnimationSourceTests.cs
git -C C:\dev\helprojs\city add assets/codebase/menu.tools/DemoDiscMainMenuSceneFactory.cs assets/Animations/DemoDiscLogoIdle.hanim
git -C C:\dev\helworks\helengine-ds add scratch/DemoDiscLogoDsOverrideAuthoring
git -C C:\dev\helworks\helengine commit -m "Require shared demo-disc logo animation path"
git -C C:\dev\helprojs\city commit -m "Animate DS demo-disc logo from shared clip"
git -C C:\dev\helworks\helengine-ds commit -m "Add DS logo override authoring tool"
```

### Task 5: Verify Editor, Cook, And DS Runtime Behavior End To End

**Files:**
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AnimationClipPlatformOverrideSerializationTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AnimationClipPlatformResolutionTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\AnimationClipAssetEditorSourceTests.cs`
- Test: `C:\dev\helworks\helengine\engine\helengine.editor.tests\CityDemoDiscLogoAnimationSourceTests.cs`
- Verify build: `C:\dev\helprojs\city\output\ds\helengine_ds.nds`

- [ ] **Step 1: Run the focused editor test suite**

```bash
rtk dotnet test C:\dev\helworks\helengine\engine\helengine.editor.tests\helengine.editor.tests.csproj --filter "FullyQualifiedName~AnimationClipPlatformOverrideSerializationTests|FullyQualifiedName~AnimationClipPlatformResolutionTests|FullyQualifiedName~AnimationClipAssetEditorSourceTests|FullyQualifiedName~CityDemoDiscLogoAnimationSourceTests"
```

Expected: PASS with all four focused test classes green.

- [ ] **Step 2: Build the city DS target**

```bash
rtk powershell -NoProfile -ExecutionPolicy Bypass -File C:\dev\helworks\helengine\artifacts\build-platform.ps1 -Project C:\dev\helprojs\city -Platform ds -Output C:\dev\helprojs\city\output\ds
```

Expected: build completes successfully and updates `C:\dev\helprojs\city\output\ds\helengine_ds.nds`.

- [ ] **Step 3: Launch the fresh DS ROM**

```bash
rtk powershell -NoProfile -ExecutionPolicy Bypass -File C:\dev\helworks\helengine-ds\scripts\launch_in_emulator.ps1 -ArtifactPath C:\dev\helprojs\city\output\ds\helengine_ds.nds
```

Expected: melonDS launches the newly built ROM, closing any previous instance first.

- [ ] **Step 4: Verify runtime behavior manually**

```text
1. Open the main menu and wait on the first screen.
2. Confirm the top-screen DS logo animates automatically.
3. Confirm the bottom-screen menu text still scrolls correctly.
4. Wrap from the last menu item to the top and confirm text remains stable.
```

Expected: the DS logo animates from the shared clip path, and the menu remains stable.

- [ ] **Step 5: Commit final verification-only follow-ups if needed**

```bash
git -C C:\dev\helworks\helengine status --short
git -C C:\dev\helprojs\city status --short
git -C C:\dev\helworks\helengine-ds status --short
```

Expected: only intentional verification follow-ups remain; if code changed during verification, commit them in the repo they belong to with focused commit messages.

---

## Self-Review

### Spec coverage

- One `.hanim` asset with base and platform overrides: covered by Task 1.
- Three modes (`InheritBase`, `ReplaceWholeClip`, `OverrideFrames`): covered by Tasks 1 and 2.
- Stable editor-only frame IDs: covered by Task 1.
- Cook-time flat resolved platform clip: covered by Task 2.
- Editor per-platform override surface: covered by Task 3.
- DS logo uses shared animation path with DS-authored motion: covered by Task 4.
- End-to-end DS build/runtime verification: covered by Task 5.

### Placeholder scan

- No `TODO`, `TBD`, or “similar to” placeholders remain.
- All tasks include concrete files, commands, and code snippets.

### Type consistency

- `AnimationClipPlatformOverrideAsset`, `AnimationClipPlatformOverrideMode`, `PlatformPositionKeyframeTrackAsset`, and `PlatformRotationKeyframeTrackAsset` are introduced in Task 1 and used consistently later.
- `AnimationClipPlatformResolutionService.ResolveForPlatform(...)` is introduced in Task 2 and referenced consistently.
- `ShowAnimationClipSettings(...)` is introduced in Task 3 and referenced consistently.
