namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS 3D renderer source so the frame loop no longer depends on removed software 2D presentation.
/// </summary>
public class NintendoDsRenderManager3DSourceAuditTests {
    /// <summary>
    /// Verifies the Nintendo DS frame renderer still traverses every camera through the DS 2D renderer without issuing software present calls.
    /// </summary>
    [Fact]
    public void Source_whenCameraListContains2dViewports_dispatchesEveryCameraWithoutSoftwarePresent() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("renderManager2D->BeginFrame();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("for (int32_t cameraIndex = 0; cameraIndex < cameras->Count(); cameraIndex++)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("IRenderQueue2D* renderQueue2D = camera->get_RenderQueue2D();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->DrawCamera(camera);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("renderManager2D->PresentFrame();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies hardware-3D target resolution still records screen ownership while avoiding software-present decisions.
    /// </summary>
    [Fact]
    public void Source_whenResolvingHardware3dTarget_tracksScreenOwnershipWithoutSoftwarePresentDecision() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("#include \"platform/ds/NintendoDsScreenTarget.hpp\"", headerSource, StringComparison.Ordinal);
        Assert.Contains("NintendoDsScreenTarget ResolveHardware3DScreenTarget", headerSource, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->SetHardware3DScreenTarget(hardware3DScreenTarget);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (hardware3DScreenTarget == NintendoDsScreenTarget::None)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("PublishPerformanceOverlayMetrics(core, renderManager2D, true);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("PublishPerformanceOverlayMetrics(core, renderManager2D, false);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("usesMetrics ? profileSnapshot.TextMilliseconds : 0.0", sourceCode, StringComparison.Ordinal);
        Assert.Contains("usesMetrics ? profileSnapshot.SpriteMilliseconds : 0.0", sourceCode, StringComparison.Ordinal);
        Assert.Contains("usesMetrics ? static_cast<double>(profileSnapshot.UnsupportedRoundedRectPrimitiveCount) : 0.0", sourceCode, StringComparison.Ordinal);
        Assert.Contains("usesMetrics ? profileSnapshot.UnsupportedTextPrimitiveCount : 0", sourceCode, StringComparison.Ordinal);
        Assert.Contains("usesMetrics ? profileSnapshot.UnsupportedSpritePrimitiveCount : 0", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("ShouldPresent2DFrame(", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("ShouldPresent2DFrame(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("get_FrameHasVisibleSoftware2DWork()", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("Draw2DCameraList(cameras, renderManager2D);\r\n            LastPresentMilliseconds = 0.0;\r\n            PublishPerformanceOverlayMetrics(core, renderManager2D, false);\r\n            return;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS renderer still releases only renderer-owned 3D payloads and leaves scene-level ownership outside the platform renderer.
    /// </summary>
    [Fact]
    public void Source_whenSceneManagerReleasesOwned3dAssets_overridesMaterialAndModelReleaseInNintendoDsRenderer() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void ReleaseMaterial(RuntimeMaterial* material) override;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void ReleaseModel(RuntimeModel* model) override;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsRenderManager3D::ReleaseMaterial(RuntimeMaterial* material)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsRenderManager3D::ReleaseModel(RuntimeModel* model)", sourceCode, StringComparison.Ordinal);
    }
}
