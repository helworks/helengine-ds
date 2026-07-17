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
    /// Verifies first-frame draw-stage markers are restricted to explicitly enabled diagnostics builds so normal gameplay never paints probe squares or stalls on VBlanks.
    /// </summary>
    [Fact]
    public void Source_whenRenderingNormalFrame_gatesFirstFrameDrawStageMarkersBehindRuntimeDiagnostics() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("constexpr bool EnableFirstFrameDrawStageMarkers = HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS != 0;", sourceCode, StringComparison.Ordinal);
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
    /// Verifies the Nintendo DS frame loop configures the 2D renderer and pure-2D main-screen mode before traversing 2D cameras, so the first handheld-menu frame after a 3D scene does not render against stale 3D state.
    /// </summary>
    [Fact]
    public void Source_whenTransitioningFrom3dSceneToPure2dMenu_configuresMainScreenBefore2dTraversal() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int resolveTargetIndex = sourceCode.IndexOf("NintendoDsScreenTarget hardware3DScreenTarget = ResolveHardware3DScreenTarget(cameras, renderManager2D);", StringComparison.Ordinal);
        int setTargetIndex = sourceCode.IndexOf("renderManager2D->SetHardware3DScreenTarget(hardware3DScreenTarget);", StringComparison.Ordinal);
        int pure2dModeIndex = sourceCode.IndexOf("videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT | DISPLAY_SPR_EXT_PALETTE);", StringComparison.Ordinal);
        int draw2dIndex = sourceCode.IndexOf("Draw2DCameraList(cameras, renderManager2D);", StringComparison.Ordinal);

        Assert.True(resolveTargetIndex >= 0);
        Assert.True(setTargetIndex > resolveTargetIndex);
        Assert.True(draw2dIndex > setTargetIndex);
        Assert.True(pure2dModeIndex > setTargetIndex);
        Assert.True(draw2dIndex > pure2dModeIndex);
    }

    /// <summary>
    /// Verifies pure-2D presentation is configured only when entering that mode instead of resetting the display registers every frame.
    /// </summary>
    [Fact]
    public void Source_whenRenderingPure2dFrames_reusesTheConfiguredDisplayMode() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool Pure2DPresentationConfigured;", headerSource, StringComparison.Ordinal);
        Assert.Contains(", Pure2DPresentationConfigured(false)", sourceCode, StringComparison.Ordinal);

        int pure2dStart = sourceCode.IndexOf("if (hardware3DScreenTarget == NintendoDsScreenTarget::None) {", StringComparison.Ordinal);
        int pure2dEnd = sourceCode.IndexOf("if (hardware3DScreenTarget != NintendoDsScreenTarget::None) {", pure2dStart, StringComparison.Ordinal);
        string pure2dBody = sourceCode[pure2dStart..pure2dEnd];

        Assert.Contains("if (!Pure2DPresentationConfigured) {", pure2dBody, StringComparison.Ordinal);
        Assert.Contains("Pure2DPresentationConfigured = true;", pure2dBody, StringComparison.Ordinal);
        Assert.Contains("Pure2DPresentationConfigured = false;", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS renderer invalidates main-screen OBJ state whenever the main engine switches presentation mode, so returning from 3D scenes does not reuse stale top-screen sprite hardware state.
    /// </summary>
    [Fact]
    public void Source_whenSwitchingMainScreenPresentation_invalidatesMainScreenSpriteHardwareState() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("renderManager2D->InvalidateMainScreenSpriteHardwareState();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 3D main-screen mode enables OBJ display only when the current frame still has top-screen 2D content to present.
    /// </summary>
    [Fact]
    public void Source_whenConfiguringHardware3dTarget_onlyEnablesMainSpritesForTopScreen2dQueues() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int configureStart = sourceCode.IndexOf("void NintendoDsRenderManager3D::ConfigureHardware3DTarget(NintendoDsScreenTarget targetScreen, NintendoDsRenderManager2D* renderManager2D) {", StringComparison.Ordinal);
        int configureEnd = sourceCode.IndexOf("bool NintendoDsRenderManager3D::TryDecodeFloat4ConstantBuffer", StringComparison.Ordinal);
        string configureBody = sourceCode[configureStart..configureEnd];

        Assert.Contains("uint32_t mainVideoMode = MODE_0_3D | DISPLAY_SPR_1D_LAYOUT | DISPLAY_SPR_EXT_PALETTE;", configureBody, StringComparison.Ordinal);
        Assert.Contains("if (LastTopScreen2DQueueCount > 0) {", configureBody, StringComparison.Ordinal);
        Assert.Contains("mainVideoMode |= DISPLAY_BG0_ACTIVE;", configureBody, StringComparison.Ordinal);
        Assert.Contains("mainVideoMode |= DISPLAY_SPR_ACTIVE;", configureBody, StringComparison.Ordinal);
        Assert.Contains("videoSetMode(mainVideoMode);", configureBody, StringComparison.Ordinal);
        Assert.DoesNotContain("videoSetMode(MODE_0_3D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT | DISPLAY_SPR_EXT_PALETTE);", configureBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies hardware 3D entry restores both texture VRAM banks after pure 2D presentation maps bank A to main-screen background memory.
    /// </summary>
    [Fact]
    public void Source_whenEnteringHardware3d_restoresTextureVramBanksAfterPure2dPresentation() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int configureStart = sourceCode.IndexOf("void NintendoDsRenderManager3D::ConfigureHardware3DTarget(NintendoDsScreenTarget targetScreen, NintendoDsRenderManager2D* renderManager2D) {", StringComparison.Ordinal);
        int configureEnd = sourceCode.IndexOf("bool NintendoDsRenderManager3D::TryDecodeFloat4ConstantBuffer", configureStart, StringComparison.Ordinal);
        string configureBody = sourceCode[configureStart..configureEnd];

        Assert.Contains("vramSetBankA(VRAM_A_TEXTURE);", configureBody, StringComparison.Ordinal);
        Assert.Contains("vramSetBankB(VRAM_B_TEXTURE);", configureBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS frame loop configures hardware-3D main-screen ownership before 2D traversal whenever the current frame contains one 3D camera, so stale menu state cannot remain visible for one extra transition frame.
    /// </summary>
    [Fact]
    public void Source_whenCurrentFrameUsesHardware3d_configuresHardwareBefore2dTraversal() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int ensureHardwareIndex = sourceCode.IndexOf("EnsureHardwareInitialized();", StringComparison.Ordinal);
        int configureHardwareIndex = sourceCode.IndexOf("ConfigureHardware3DTarget(hardware3DScreenTarget, renderManager2D);", StringComparison.Ordinal);
        int draw2dIndex = sourceCode.IndexOf("Draw2DCameraList(cameras, renderManager2D);", StringComparison.Ordinal);

        Assert.True(ensureHardwareIndex >= 0);
        Assert.True(configureHardwareIndex > ensureHardwareIndex);
        Assert.True(draw2dIndex > configureHardwareIndex);
    }

    /// <summary>
    /// Verifies the Nintendo DS hardware-3D configuration does not invalidate old top-screen OBJ state during pure top-screen 3D entry, so hidden menu sprites are not zeroed into one black silhouette before the next scene takes over.
    /// </summary>
    [Fact]
    public void Source_whenEnteringPure3dTopScreen_doesNotInvalidateOldMainScreenObjStateDuringConfigure() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int configureStart = sourceCode.IndexOf("void NintendoDsRenderManager3D::ConfigureHardware3DTarget(NintendoDsScreenTarget targetScreen, NintendoDsRenderManager2D* renderManager2D) {", StringComparison.Ordinal);
        int configureEnd = sourceCode.IndexOf("bool NintendoDsRenderManager3D::TryDecodeFloat4ConstantBuffer", StringComparison.Ordinal);
        string configureBody = sourceCode[configureStart..configureEnd];

        Assert.Contains("videoSetMode(mainVideoMode);", configureBody, StringComparison.Ordinal);
        Assert.DoesNotContain("renderManager2D->InvalidateMainScreenSpriteHardwareState();", configureBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS frame loop invalidates top-screen BG0 text hardware state before returning from a 3D frame to a pure-2D menu, so the next 2D traversal rebuilds its BG tiles after main VRAM served as texture memory.
    /// </summary>
    [Fact]
    public void Source_whenTransitioningFrom3dToPure2dMenu_invalidatesMainScreenTextBackgroundHardwareState() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("renderManager2D->InvalidateMainScreenTextBackgroundHardwareState();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS renderer defers 3D resource destruction until a safe renderer frame.
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
        Assert.Contains("runtimeMaterial->OwnsPrimaryTexture = true;", sourceCode, StringComparison.Ordinal);

        int releaseMaterialStart = sourceCode.IndexOf("void NintendoDsRenderManager3D::ReleaseMaterial(RuntimeMaterial* material)", StringComparison.Ordinal);
        int releaseModelStart = sourceCode.IndexOf("void NintendoDsRenderManager3D::ReleaseModel(RuntimeModel* model)", releaseMaterialStart, StringComparison.Ordinal);
        int immediateMaterialStart = sourceCode.IndexOf("void NintendoDsRenderManager3D::ReleaseMaterialImmediately(RuntimeMaterial* material)", releaseMaterialStart, StringComparison.Ordinal);
        string releaseMaterialBody = sourceCode[releaseMaterialStart..immediateMaterialStart];
        Assert.Contains("PendingReleasedMaterials.push_back(material);", releaseMaterialBody, StringComparison.Ordinal);
        Assert.DoesNotContain("delete material;", releaseMaterialBody, StringComparison.Ordinal);

        int immediateModelStart = sourceCode.IndexOf("void NintendoDsRenderManager3D::ReleaseModelImmediately(RuntimeModel* model)", releaseModelStart, StringComparison.Ordinal);
        string releaseModelBody = sourceCode[releaseModelStart..immediateModelStart];
        Assert.Contains("PendingReleasedModels.push_back(model);", releaseModelBody, StringComparison.Ordinal);
        Assert.DoesNotContain("delete model;", releaseModelBody, StringComparison.Ordinal);

        string immediateMaterialBody = sourceCode[immediateMaterialStart..immediateModelStart];
        Assert.Contains("renderManager2D->ReleaseTexture(primaryTexture);", immediateMaterialBody, StringComparison.Ordinal);
        Assert.Contains("runtimeMaterial->OwnsPrimaryTexture = false;", immediateMaterialBody, StringComparison.Ordinal);
        Assert.Contains("material->Dispose();", immediateMaterialBody, StringComparison.Ordinal);
        Assert.Contains("delete material;", immediateMaterialBody, StringComparison.Ordinal);

        int deferredFlushStart = sourceCode.IndexOf("void NintendoDsRenderManager3D::FlushDeferredReleasesForFrame()", immediateModelStart, StringComparison.Ordinal);
        string immediateModelBody = sourceCode[immediateModelStart..deferredFlushStart];
        Assert.Contains("model->Dispose();", immediateModelBody, StringComparison.Ordinal);
        Assert.Contains("delete model;", immediateModelBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the public renderer flush hook drains deferred Nintendo DS model and material releases after scene transitions.
    /// </summary>
    [Fact]
    public void Source_whenSceneManagerFlushesReleasedAssets_drainsNintendoDsDeferred3dReleases() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void FlushReleasedAssets() override;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsRenderManager3D::FlushReleasedAssets()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("FlushDeferredReleasesForFrame();", sourceCode, StringComparison.Ordinal);
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
        Assert.Contains("stream = contentStreamSource != nullptr ? contentStreamSource->OpenRead(cookedAssetPath) : ::File::OpenRead(cookedAssetPath);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 3D renderer does not keep an unnecessary libnds console header dependency after the native fatal-console strip.
    /// </summary>
    [Fact]
    public void Source_whenConsoleDiagnosticsLiveOnlyInBootHost_doesNotIncludeConsoleHeaderInRenderManager3d() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("#include <nds/arm9/console.h>", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("#include <nds/arm9/console.h>", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 3D texture upload path explicitly repacks runtime texels into DS-native tiled texture order before handing them to libnds.
    /// </summary>
    [Fact]
    public void Source_whenUploadingHardwareTexture_reordersTexelsIntoDsNativeTiledLayout() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("int32_t ResolveHardwareTextureTexelIndex(int32_t textureWidth, int32_t textureHeight, int32_t pixelX, int32_t pixelY) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t destinationIndex = ResolveHardwareTextureTexelIndex(textureWidth, textureHeight, pixelX, pixelY);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("hardwarePixels[static_cast<std::size_t>(pixelIndex)] = PackHardwareTexturePixel(", sourceCode, StringComparison.Ordinal);
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

    /// <summary>
    /// Verifies Nintendo DS direct-color 3D uploads use the native <c>GL_RGBA</c> format instead of the slower alias path.
    /// </summary>
    [Fact]
    public void Source_whenUploadingHardwareTexture_usesDirectGlRgbaFormat() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("GL_RGBA,", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("GL_RGB,", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TEXGEN_TEXCOORD,", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("TEXGEN_OFF,", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies a successfully uploaded DS texture is enabled and bound for the current drawable instead of rendering one white fallback frame.
    /// </summary>
    [Fact]
    public void Source_whenTextureUploadsThisDraw_bindsTexturedSamplingImmediately() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool EnsureHardwareTextureUploaded(NintendoDsRuntimeTexture2D* runtimeTexture);", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool uploadedThisCall = EnsureHardwareTextureUploaded(runtimeTexture);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (uploadedThisCall) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ApplyHardwareTextureEnabledState(true);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ApplyHardwareTextureBinding(runtimeTexture->HardwareTextureId);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("return true;", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies Nintendo DS runtime diagnostics can emit one compact textured-draw trace through stdout so emulator log captures show whether the 3D path resolved, uploaded, and submitted one real hardware texture.
    /// </summary>
    [Fact]
    public void Source_whenRuntimeDiagnosticsAreEnabled_emitsCompactHardwareTextureTraceLines() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("std::string GetHardwareTextureDiagnosticsText() const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("std::string GetHardwareTextureLightingDiagnosticsText() const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("[helengine-ds] d3t-config", sourceCode, StringComparison.Ordinal);
        Assert.Contains("[helengine-ds] d3t-draw", sourceCode, StringComparison.Ordinal);
        Assert.Contains("HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::printf(", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies Nintendo DS path-based cooked material loading resolves and attaches the referenced cooked primary texture so textured 3D materials do not silently degrade to flat white shading.
    /// </summary>
    [Fact]
    public void Source_whenBuildingCookedMaterialFromPath_attachesReferencedPrimaryTexture() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("std::string ResolvePackagedContentAssetPath(const std::string& cookedMaterialAssetPath, const std::string& contentRelativePath);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void AttachCookedPrimaryTexture(NintendoDsRuntimeMaterial* runtimeMaterial, PlatformMaterialAsset* materialAsset, const std::string& cookedMaterialAssetPath, IContentStreamSource* contentStreamSource);", headerSource, StringComparison.Ordinal);
        Assert.Contains("AttachCookedPrimaryTexture(runtimeMaterial, materialAsset, cookedAssetPath, contentStreamSource);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (materialAsset->TextureRelativePath.empty())", sourceCode, StringComparison.Ordinal);
        Assert.Contains("const std::string cookedTextureAssetPath = contentStreamSource != nullptr", sourceCode, StringComparison.Ordinal);
        Assert.Contains("? materialAsset->TextureRelativePath", sourceCode, StringComparison.Ordinal);
        Assert.Contains(": ResolvePackagedContentAssetPath(cookedMaterialAssetPath, materialAsset->TextureRelativePath);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RuntimeTexture* runtimeTexture = renderManager2D->BuildTextureFromCooked(cookedTextureAssetPath, contentStreamSource);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("runtimeMaterial->SetPrimaryTexture(runtimeTexture);", sourceCode, StringComparison.Ordinal);
    }
}
