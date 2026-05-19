namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS renderer source so generated-core staging and native compilation keep the same cooked-material contract.
/// </summary>
public class NintendoDsRenderManager3DSourceAuditTests {
    /// <summary>
    /// Verifies the Nintendo DS frame renderer traverses every camera so menu scenes can target both screens through 2D camera viewports.
    /// </summary>
    [Fact]
    public void Source_whenCameraListContains2dViewports_dispatchesEveryCameraThroughNintendoDs2DRenderer() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("renderManager2D->BeginFrame();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("for (int32_t cameraIndex = 0; cameraIndex < cameras->Count(); cameraIndex++)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("IRenderQueue2D* renderQueue2D = camera->get_RenderQueue2D();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->DrawCamera(camera);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->PresentFrame();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("NintendoDsRenderManager2D* renderManager2D", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS renderer always compiles its cooked platform-owned material override when generated core is enabled.
    /// </summary>
    [Fact]
    public void Source_whenDsUsesGeneratedCore_doesNotGateCookedMaterialOverrideBehindUndefinedSymbol() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("#if HELENGINE_RUNTIME_MATERIAL_RESOLUTION_COOKED_PLATFORM_OWNED", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("#if HELENGINE_RUNTIME_MATERIAL_RESOLUTION_COOKED_PLATFORM_OWNED", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RuntimeMaterial* BuildMaterialFromCooked(PlatformMaterialAsset* materialAsset) override;", headerSource, StringComparison.Ordinal);
        Assert.Contains("RuntimeMaterial* NintendoDsRenderManager3D::BuildMaterialFromCooked(PlatformMaterialAsset* materialAsset)", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS renderer source includes the full constant-buffer asset definition before reading generated-core buffer metadata.
    /// </summary>
    [Fact]
    public void Source_whenResolvingStandardMaterialBaseColor_includesMaterialConstantBufferAssetDefinition() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("#include \"MaterialConstantBufferAsset.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("constantBuffer->get_Name()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("constantBuffer->get_Data()", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 3D renderer resolves top-first hardware 3D ownership and still hands 2D presentation back to the DS 2D renderer.
    /// </summary>
    [Fact]
    public void Source_whenBothScreensContain3d_topScreenWinsAnd2dPresentationStillRunsForBothScreens() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("#include \"platform/ds/NintendoDsScreenTarget.hpp\"", headerSource, StringComparison.Ordinal);
        Assert.Contains("NintendoDsScreenTarget ResolveHardware3DScreenTarget", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool topScreenHas3D = false;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("bool bottomScreenHas3D = false;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (topScreenHas3D) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("return NintendoDsScreenTarget::Top;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (bottomScreenHas3D) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("return NintendoDsScreenTarget::Bottom;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->SetHardware3DScreenTarget(hardware3DScreenTarget);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->DrawCamera(camera);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->PresentFrame();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS mixed-presentation path keeps the main engine in hardware 3D mode instead of switching the chosen 3D screen back to pure 2D.
    /// </summary>
    [Fact]
    public void Source_whenConfiguringHardware3dTarget_keepsMainEngineInMode0_3d() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void NintendoDsRenderManager3D::ConfigureHardware3DTarget(NintendoDsScreenTarget targetScreen, NintendoDsRenderManager2D* renderManager2D)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("videoSetMode(MODE_0_3D | DISPLAY_BG3_ACTIVE);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE | DISPLAY_BG0_ACTIVE);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS mixed-presentation path preserves a native bottom-screen console when 2D bitmap presentation has been explicitly disabled for diagnostics.
    /// </summary>
    [Fact]
    public void Source_whenBottomScreenBitmapPresentationIsDisabled_doesNotForceSubScreenBackToBitmapMode() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("if (renderManager2D == nullptr || renderManager2D->get_BottomScreenPresentationEnabled()) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 3D renderer records per-frame screen ownership and draw-submission counts for on-device diagnostics.
    /// </summary>
    [Fact]
    public void Source_whenDrawing3dFrame_recordsHardwareTargetAndSubmissionCountsForDiagnostics() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("NintendoDsScreenTarget get_LastHardware3DScreenTarget() const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t get_LastCamera3DQueueCount() const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t get_LastSubmittedDrawableCount() const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t get_LastTopScreen2DQueueCount() const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t get_LastBottomScreen2DQueueCount() const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("LastHardware3DScreenTarget = hardware3DScreenTarget;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastCamera3DQueueCount = renderQueue3D->get_Count();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastSubmittedDrawableCount = DrawRenderQueue(camera);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastTopScreen2DQueueCount = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastBottomScreen2DQueueCount = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastTopScreen2DQueueCount = renderQueue2D->get_Count();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastBottomScreen2DQueueCount = renderQueue2D->get_Count();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS renderer restores the main LCD to bitmap 2D mode when a scene transitions away from hardware 3D content.
    /// </summary>
    [Fact]
    public void Source_whenFrameContainsNo3d_restoresTopScreenBitmapPresentationBeforePresenting2d() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("if (hardware3DScreenTarget == NintendoDsScreenTarget::None) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("lcdMainOnTop();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->PresentFrame();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS renderer overrides runtime 3D asset release so scene unload can actually free DS-owned model and material state.
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
        Assert.Contains("material->Dispose();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("model->Dispose();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("delete material;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("delete model;", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS runtime model build path transfers owned geometry buffers away from transient model assets before they are later released.
    /// </summary>
    [Fact]
    public void Source_whenBuildingRuntimeModel_transfersOwnedBuffersOffSourceModelAsset() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("runtimeModel->Positions = data->Positions;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("runtimeModel->Indices16 = data->Indices16;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("runtimeModel->Indices32 = data->Indices32;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("data->Positions = Array<float3>::Empty();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("data->Indices16 = Array<uint16_t>::Empty();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("data->Indices32 = Array<uint32_t>::Empty();", sourceCode, StringComparison.Ordinal);
    }
}
