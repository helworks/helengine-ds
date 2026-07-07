namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS 3D renderer source so the frame loop no longer depends on removed software 2D presentation.
/// </summary>
public class NintendoDsRenderManager3DSourceAuditTests {
    /// <summary>
    /// Verifies the Nintendo DS frame renderer still traverses every camera through the DS 2D renderer and presents the bottom-screen text background path explicitly.
    /// </summary>
    [Fact]
    public void Source_whenCameraListContains2dViewports_dispatchesEveryCameraAndPresentsBottomSoftwareFrame() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("renderManager2D->BeginFrame();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("for (int32_t cameraIndex = 0; cameraIndex < cameras->Count(); cameraIndex++)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("IRenderQueue2D* renderQueue2D = camera->get_RenderQueue2D();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->DrawCamera(camera);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("renderManager2D->PresentFrame();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->PresentBottomScreenFrame();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies hardware-3D target resolution no longer short-circuits through the removed top-screen proof-mode suppression path.
    /// </summary>
    [Fact]
    public void Source_whenResolvingHardware3dTarget_doesNotUseTopScreenProofShortCircuit() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("#include \"platform/ds/NintendoDsScreenTarget.hpp\"", headerSource, StringComparison.Ordinal);
        Assert.Contains("NintendoDsScreenTarget ResolveHardware3DScreenTarget", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("if (renderManager2D->get_TopScreenProofModeActive()) {", sourceCode, StringComparison.Ordinal);
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
        Assert.DoesNotContain("videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("EnsureNativeDebugOverlayInitialized();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("DrawNativeDebugOverlay(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("consoleInit(&NativeDebugConsole", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 3D renderer preserves the caller-selected bottom-screen presentation policy instead of forcing it on every frame.
    /// </summary>
    [Fact]
    public void Source_whenRenderingFrame_doesNotForceBottomScreenPresentationEnabledTrue() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("renderManager2D->SetBottomScreenPresentationEnabled(true);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the pure-2D top-screen presentation path keeps BG0 enabled so DS runtime text can render when no camera owns hardware 3D.
    /// </summary>
    [Fact]
    public void Source_whenTopScreenFallsBackToPure2d_keepsBg0ActiveForRuntimeText() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT | DISPLAY_SPR_EXT_PALETTE);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("videoSetMode(MODE_5_2D | DISPLAY_BG0_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT);", sourceCode, StringComparison.Ordinal);
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

    /// <summary>
    /// Verifies the Nintendo DS fixed-function texture path resolves the generic runtime-material primary texture instead of assuming shader-only texture accessors.
    /// </summary>
    [Fact]
    public void Source_whenConfiguringHardwareTexture_usesGenericPrimaryTextureResolution() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("runtimeMaterial->ResolvePrimaryTexture()", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("runtimeMaterial->ResolveTexture()", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 3D queue traversal uses the prebuilt static display-list path only when the helper decides the untextured mesh is large enough to amortize the kick cost.
    /// </summary>
    [Fact]
    public void Source_whenRenderingOrdered3dQueue_routesDisplayListsThroughSizeAwareHelper() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        int drawRenderQueueStart = sourceCode.IndexOf("int32_t NintendoDsRenderManager3D::DrawRenderQueue(ICamera* camera)", StringComparison.Ordinal);
        int submitOpaqueDrawableStart = sourceCode.IndexOf("void NintendoDsRenderManager3D::SubmitOpaqueDrawable(", StringComparison.Ordinal);
        int submitStaticDisplayListStart = sourceCode.IndexOf("void NintendoDsRenderManager3D::SubmitStaticHardwareDisplayList(", StringComparison.Ordinal);
        int shouldUseDisplayListStart = sourceCode.IndexOf("bool NintendoDsRenderManager3D::ShouldUseStaticHardwareDisplayList(", StringComparison.Ordinal);
        int resolveTrianglePrimitiveCountStart = sourceCode.IndexOf("int32_t NintendoDsRenderManager3D::ResolveTrianglePrimitiveCount(", StringComparison.Ordinal);
        int applyDrawableTransformStart = sourceCode.IndexOf("void NintendoDsRenderManager3D::ApplyDrawableTransformToHardwareMatrix(", StringComparison.Ordinal);
        string drawRenderQueueBody = sourceCode[drawRenderQueueStart..submitOpaqueDrawableStart];
        string submitStaticDisplayListBody = sourceCode[submitStaticDisplayListStart..shouldUseDisplayListStart];
        string shouldUseDisplayListBody = sourceCode[shouldUseDisplayListStart..resolveTrianglePrimitiveCountStart];
        string resolveTrianglePrimitiveCountBody = sourceCode[resolveTrianglePrimitiveCountStart..submitOpaqueDrawableStart];
        string submitOpaqueDrawableBody = sourceCode[submitOpaqueDrawableStart..applyDrawableTransformStart];

        Assert.Contains("for (int32_t drawableIndex = 0; drawableIndex < drawables->Count(); drawableIndex++)", drawRenderQueueBody, StringComparison.Ordinal);
        Assert.Contains("IDrawable3D* drawable = (*drawables)[drawableIndex];", drawRenderQueueBody, StringComparison.Ordinal);
        Assert.Contains("std::vector<NintendoDsRuntimeModelInstanceCountEntry> runtimeModelInstanceCounts;", drawRenderQueueBody, StringComparison.Ordinal);
        Assert.Contains("NintendoDsRuntimeModel* runtimeModel = ResolveRuntimeModel(drawable->get_Model());", drawRenderQueueBody, StringComparison.Ordinal);
        Assert.Contains("Array<RuntimeMaterial*>* runtimeMaterials = drawable->get_Materials();", drawRenderQueueBody, StringComparison.Ordinal);
        Assert.Contains("NintendoDsRuntimeMaterial* runtimeMaterial = ResolveRuntimeMaterial(firstRuntimeMaterial);", drawRenderQueueBody, StringComparison.Ordinal);
        Assert.Contains("IncrementRuntimeModelInstanceCount(runtimeModelInstanceCounts, runtimeModel);", drawRenderQueueBody, StringComparison.Ordinal);
        Assert.Contains("int32_t runtimeModelInstanceCount = ResolveRuntimeModelInstanceCount(runtimeModelInstanceCounts, runtimeModel);", drawRenderQueueBody, StringComparison.Ordinal);
        Assert.Contains("SubmitOpaqueDrawable(drawable, runtimeModel, runtimeMaterial, runtimeModelInstanceCount);", drawRenderQueueBody, StringComparison.Ordinal);
        Assert.Contains("submittedDrawables++;", drawRenderQueueBody, StringComparison.Ordinal);
        Assert.Contains("bool ShouldUseStaticHardwareDisplayList(NintendoDsRuntimeModel* runtimeModel, bool useHardwareTexture, int32_t runtimeModelInstanceCount) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ResolveTrianglePrimitiveCount(NintendoDsRuntimeModel* runtimeModel) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("static NintendoDsRuntimeModel* ResolveRuntimeModel(RuntimeModel* runtimeModel);", headerSource, StringComparison.Ordinal);
        Assert.Contains("static NintendoDsRuntimeMaterial* ResolveRuntimeMaterial(RuntimeMaterial* runtimeMaterial);", headerSource, StringComparison.Ordinal);
        Assert.Contains("static NintendoDsRuntimeTexture2D* ResolveRuntimeTexture(RuntimeTexture* runtimeTexture);", headerSource, StringComparison.Ordinal);
        Assert.Contains("static NintendoDsRenderManager2D* ResolveNintendoDsRenderManager2D(RenderManager2D* renderManager2D);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void IncrementRuntimeModelInstanceCount(std::vector<NintendoDsRuntimeModelInstanceCountEntry>& runtimeModelInstanceCounts, NintendoDsRuntimeModel* runtimeModel) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t ResolveRuntimeModelInstanceCount(const std::vector<NintendoDsRuntimeModelInstanceCountEntry>& runtimeModelInstanceCounts, NintendoDsRuntimeModel* runtimeModel) const;", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("if (false && !useHardwareTexture && runtimeModel->HardwareLitDisplayList != nullptr)", submitOpaqueDrawableBody, StringComparison.Ordinal);
        Assert.Contains("if (ShouldUseStaticHardwareDisplayList(runtimeModel, useHardwareTexture, runtimeModelInstanceCount))", submitOpaqueDrawableBody, StringComparison.Ordinal);
        Assert.Contains("SubmitStaticHardwareDisplayList(runtimeModel, useHardwareTexture);", submitOpaqueDrawableBody, StringComparison.Ordinal);
        Assert.Contains("glBegin(GL_TRIANGLES);", submitOpaqueDrawableBody, StringComparison.Ordinal);
        Assert.Contains("glCallList(reinterpret_cast<u32*>(displayList));", submitStaticDisplayListBody, StringComparison.Ordinal);
        Assert.Contains("StaticDisplayListTriangleThreshold", shouldUseDisplayListBody, StringComparison.Ordinal);
        Assert.Contains("return ResolveTrianglePrimitiveCount(runtimeModel) > StaticDisplayListTriangleThreshold;", shouldUseDisplayListBody, StringComparison.Ordinal);
        Assert.Contains("return runtimeModel->Indices32->Length / 3;", resolveTrianglePrimitiveCountBody, StringComparison.Ordinal);
        Assert.Contains("return runtimeModel->Indices16->Length / 3;", resolveTrianglePrimitiveCountBody, StringComparison.Ordinal);
        Assert.Contains("return runtimeModel->Positions->Length / 3;", resolveTrianglePrimitiveCountBody, StringComparison.Ordinal);
        Assert.DoesNotContain("dynamic_cast<NintendoDsRuntimeModel*>", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("dynamic_cast<NintendoDsRuntimeMaterial*>", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("dynamic_cast<NintendoDsRuntimeTexture2D*>", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("dynamic_cast<NintendoDsRenderManager2D*>", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("std::unordered_map<NintendoDsRuntimeModel*, int32_t>", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("FIFO_END", sourceCode, StringComparison.Ordinal);
    }
}
