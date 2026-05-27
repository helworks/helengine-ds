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
    /// Verifies the Nintendo DS render loop still dispatches cameras through the 2D renderer when their 2D queue is empty so each screen is cleared every frame.
    /// </summary>
    [Fact]
    public void Source_whenCameraHasNo2dDrawables_stillDispatchesCameraThroughNintendoDs2DRendererForScreenClear() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int queueConditionIndex = sourceCode.IndexOf("if (renderQueue2D != nullptr && renderQueue2D->get_Count() > 0) {", StringComparison.Ordinal);
        int conditionalCountWriteIndex = sourceCode.IndexOf("LastBottomScreen2DQueueCount = renderQueue2D->get_Count();", StringComparison.Ordinal);
        int drawCameraIndex = sourceCode.IndexOf("renderManager2D->DrawCamera(camera);", StringComparison.Ordinal);

        Assert.True(queueConditionIndex >= 0, "Expected the DS renderer to keep queue-count bookkeeping behind the 2D queue presence check.");
        Assert.True(conditionalCountWriteIndex > queueConditionIndex, "Expected the conditional queue-count bookkeeping to remain inside the 2D queue presence check.");
        Assert.True(drawCameraIndex > conditionalCountWriteIndex, "Expected the DS renderer to dispatch every camera through DrawCamera after conditional queue-count bookkeeping.");
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
    /// Verifies the Nintendo DS renderer owns the cooked material path overload used by the shared runtime resolver.
    /// </summary>
    [Fact]
    public void Source_whenRuntimeResolverUsesCookedMaterialPath_overridesStringCookedMaterialBridge() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("RuntimeMaterial* BuildMaterialFromCooked(std::string cookedAssetPath) override;", headerSource, StringComparison.Ordinal);
        Assert.Contains("RuntimeMaterial* NintendoDsRenderManager3D::BuildMaterialFromCooked(std::string cookedAssetPath)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastBuildStage = \"BuildMaterialFromCookedOpened\";", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastBuildStage = \"BuildMaterialFromCookedDeserialized\";", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastBuildStage = \"BuildMaterialFromCookedTyped\";", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BuildMaterialFromCooked(materialAsset)", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS renderer owns cooked model creation when runtime scene references resolve packaged generated models.
    /// </summary>
    [Fact]
    public void Source_whenSceneUsesCookedGeneratedModel_overridesBuildModelFromCooked() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("RuntimeModel* BuildModelFromCooked(std::string cookedAssetPath) override;", headerSource, StringComparison.Ordinal);
        Assert.Contains("RuntimeModel* NintendoDsRenderManager3D::BuildModelFromCooked(std::string cookedAssetPath)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastBuildStage = \"BuildModelFromCookedBegin\";", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BuildModelFromRaw(modelAsset)", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies cooked Nintendo DS materials expose the standard diffuse texture binding before resolver-assigned textures are applied.
    /// </summary>
    [Fact]
    public void Source_whenBuildingCookedMaterial_createsDiffuseTextureBindingLayout() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("#include \"MaterialLayout.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("#include \"MaterialLayoutBinding.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("#include \"StandardMaterialTextureBindingDefaults.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("new ::MaterialLayoutBinding(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, ShaderResourceType::Texture2D, 0, 0, 0)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("runtimeMaterial->SetLayout(dsMaterialLayout);", sourceCode, StringComparison.Ordinal);
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
        Assert.Contains("videoSetMode(MODE_0_3D | DISPLAY_BG0_ACTIVE);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("vramSetBankA(VRAM_A_TEXTURE);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("vramSetBankA(VRAM_A_MAIN_BG);", sourceCode, StringComparison.Ordinal);
        Assert.True(
            sourceCode.IndexOf("glInit();", StringComparison.Ordinal) < sourceCode.IndexOf("vramSetBankA(VRAM_A_TEXTURE);", StringComparison.Ordinal),
            "libnds examples initialize GL before mapping texture VRAM, so the renderer must preserve that texture allocator order.");
        Assert.DoesNotContain("videoSetMode(MODE_0_3D | DISPLAY_BG3_ACTIVE);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE | DISPLAY_BG0_ACTIVE);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS camera projection uses the same authored camera contract as the other 3D backends.
    /// </summary>
    [Fact]
    public void Source_whenConfiguringCamera_usesSharedPerspectiveFovAndAuthoredClipPlanes() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("constexpr float DefaultPerspectiveFieldOfViewDegrees = 45.0f;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("float nearPlaneDistance = camera->get_NearPlaneDistance();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("float farPlaneDistance = camera->get_FarPlaneDistance();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gluPerspective(DefaultPerspectiveFieldOfViewDegrees, 256.0f / 192.0f, nearPlaneDistance, farPlaneDistance);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("gluPerspective(70.0f", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("gluPerspective(45.0f, 256.0f / 192.0f, 0.1f, 40.0f)", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS mixed-presentation path preserves a native bottom-screen console when 2D bitmap presentation has been explicitly disabled for diagnostics.
    /// </summary>
    [Fact]
    public void Source_whenBottomScreenBitmapPresentationIsDisabled_doesNotForceSubScreenBackToBitmapMode() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool bottomScreenPresentationEnabled = renderManager2D == nullptr || renderManager2D->get_BottomScreenPresentationEnabled();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (bottomScreenPresentationEnabled) {", sourceCode, StringComparison.Ordinal);
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
    /// Verifies the Nintendo DS renderer releases only renderer-owned 3D payloads and leaves top-level runtime objects to SceneManager.
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
        Assert.Contains("delete runtimeModel->Positions;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("delete runtimeModel->Indices16;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("delete runtimeModel->Indices32;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("material->Dispose();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("model->Dispose();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("delete material;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("delete model;", sourceCode, StringComparison.Ordinal);
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
        Assert.Contains("runtimeModel->TexCoords = data->TexCoords;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("runtimeModel->Indices16 = data->Indices16;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("runtimeModel->Indices32 = data->Indices32;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("data->Positions = Array<float3>::Empty();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("data->TexCoords = Array<float2>::Empty();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("data->Indices16 = Array<uint16_t>::Empty();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("data->Indices32 = Array<uint32_t>::Empty();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS textured geometry path preserves model UVs and submits texture coordinates when a material has a runtime texture.
    /// </summary>
    [Fact]
    public void Source_whenRuntimeMaterialHasTexture_submitsTexturedGeometryWithModelTexCoords() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string modelHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRuntimeModel.hpp");
        string rendererHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string modelHeaderSource = File.ReadAllText(modelHeaderPath);
        string rendererHeaderSource = File.ReadAllText(rendererHeaderPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("Array<float2>* TexCoords;", modelHeaderSource, StringComparison.Ordinal);
        Assert.Contains("bool TryConfigureHardwareTexture(NintendoDsRuntimeMaterial* runtimeMaterial, NintendoDsRuntimeTexture2D*& runtimeTexture);", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("runtimeTexture = dynamic_cast<NintendoDsRuntimeTexture2D*>(runtimeMaterial->ResolveTexture());", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glBindTexture(0, runtimeTexture->HardwareTextureId);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t uploadResult = glTexImage2D(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("GL_RGB,", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("GL_RGBA,", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glTexImage2D(\n            0,", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (uploadResult == 0) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Nintendo DS hardware texture upload failed.", sourceCode, StringComparison.Ordinal);
        Assert.Contains("DC_FlushRange(hardwarePixels.data(), hardwarePixels.size() * sizeof(uint16_t));", sourceCode, StringComparison.Ordinal);
        Assert.Contains("TEXGEN_OFF,", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ForceHardwareTextureDiagnosticCoordinates = false", sourceCode, StringComparison.Ordinal);
        Assert.Contains("floattot16(texCoord.X * static_cast<float>(runtimeTexture->get_Width()))", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glTexCoord2t16(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("if (useHardwareTexture) {\n            glDisable(GL_TEXTURE_2D);\n        }", sourceCode, StringComparison.Ordinal);
        Assert.Contains("bool lightingEnabled,", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("const float3& modelFaceNormal,", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("SubmitHardwareTexturedTriangle(positions, texCoords, hardwareTexture, runtimeMaterial->LightingEnabled", sourceCode, StringComparison.Ordinal);
        Assert.Contains("SubmitHardwareTexturedVertex(positions, texCoords, runtimeTexture, lightingEnabled, modelFaceNormal, indexA);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("SubmitHardwareTexturedTriangle(", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("SubmitHardwareTexturedTriangle(", sourceCode, StringComparison.Ordinal);

        int texturedVertexMethodIndex = sourceCode.IndexOf("void NintendoDsRenderManager3D::SubmitHardwareTexturedVertex(", StringComparison.Ordinal);
        int texturedVertexTexCoordIndex = sourceCode.IndexOf("glTexCoord2t16(", texturedVertexMethodIndex, StringComparison.Ordinal);
        int texturedVertexNormalIndex = sourceCode.IndexOf("glNormal(NORMAL_PACK(", texturedVertexMethodIndex, StringComparison.Ordinal);
        int texturedVertexPositionIndex = sourceCode.IndexOf("glVertex3v16(", texturedVertexMethodIndex, StringComparison.Ordinal);

        Assert.True(texturedVertexMethodIndex >= 0);
        Assert.True(texturedVertexTexCoordIndex > texturedVertexMethodIndex);
        Assert.True(texturedVertexNormalIndex > texturedVertexTexCoordIndex);
        Assert.True(texturedVertexPositionIndex > texturedVertexNormalIndex);
    }

    /// <summary>
    /// Verifies the Nintendo DS textured diagnostic path keeps cooked lighting intent and publishes texture upload state on the native overlay.
    /// </summary>
    [Fact]
    public void Source_whenDebuggingTexturedMaterials_reportsTextureUploadStateAndHonorsUnlitMaterials() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string materialHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRuntimeMaterial.hpp");
        string materialSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRuntimeMaterial.cpp");
        string rendererHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string materialHeaderSource = File.ReadAllText(materialHeaderPath);
        string materialSourceCode = File.ReadAllText(materialSourcePath);
        string rendererHeaderSource = File.ReadAllText(rendererHeaderPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool LightingEnabled;", materialHeaderSource, StringComparison.Ordinal);
        Assert.Contains(", LightingEnabled(true)", materialSourceCode, StringComparison.Ordinal);
        Assert.Contains("runtimeMaterial->LightingEnabled = materialAsset->Lit;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (runtimeMaterial->LightingEnabled) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (!lightingEnabled) {\n            glColor3b(255, 255, 255);\n        }", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastHardwareTextureUploadAttempted", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("LastHardwareTextureUploaded", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("LastHardwareTextureFormat", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("ForceHardwareTextureDiagnosticPattern = false", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BuildHardwareTextureDiagnosticPixels(textureWidth, textureHeight)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RecordHardwareTextureDiagnostics(runtimeTexture, true);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RecordHardwareTextureDiagnostics(runtimeTexture, false);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("PrintNativeDebugOverlayLine(13, FormatHardwareTextureDiagnostics());", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("PrintNativeDebugOverlayLine(14, FormatHardwareTextureLightingDiagnostics());", sourceCode, StringComparison.Ordinal);
        Assert.Contains("FormatHardwareTextureDiagnostics()", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("FormatHardwareTextureDiagnostics()", sourceCode, StringComparison.Ordinal);
    }
}
