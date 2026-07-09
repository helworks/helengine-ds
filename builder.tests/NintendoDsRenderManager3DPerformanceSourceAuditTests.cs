namespace helengine.ds.builder.tests;

/// <summary>
/// Audits Nintendo DS 3D renderer source so the performance-sensitive frame path stays hardware-only.
/// </summary>
public class NintendoDsRenderManager3DPerformanceSourceAuditTests {
    /// <summary>
    /// Verifies drawable transforms are submitted through the Nintendo DS matrix hardware instead of rotating every vertex on the ARM9.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingDrawable_usesHardwareMatrixForEntityTransform() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void ApplyDrawableTransformToHardwareMatrix(", headerSource, StringComparison.Ordinal);
        Assert.Contains("void BuildDrawableTransformMatrix(", headerSource, StringComparison.Ordinal);
        Assert.Contains("glPushMatrix();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ApplyDrawableTransformToHardwareMatrix(entityPosition, entityScale, entityOrientation);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glPopMatrix(1);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("m4x3 transformMatrix;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BuildDrawableTransformMatrix(transformMatrix, entityPosition, entityScale, entityOrientation);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glMultMatrix4x3(&transformMatrix);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glVertex3v16(floattov16((*positions)[indexA].X)", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("glRotatef32i(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("std::acos(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("std::sqrt(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("TransformVertex((*positions)[indexA], entityPosition, entityScale, entityOrientation)", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies hardware-3D frames no longer preserve software 2D present plumbing after the DS software compositor removal.
    /// </summary>
    [Fact]
    public void Source_whenFrameHasOnlyHardware3d_removesSoftware2dPresentPath() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("bool ShouldPresent2DFrame(NintendoDsScreenTarget hardware3DScreenTarget, NintendoDsRenderManager2D* renderManager2D) const;", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("bool shouldPresent2DFrame = ShouldPresent2DFrame(hardware3DScreenTarget, renderManager2D);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("LastPresentNetByteDelta", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("renderManager2D->PresentFrame();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies textured lit triangles emit one shared face normal instead of resubmitting the same normal for every vertex.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingTexturedLitTriangle_emitsOneTriangleNormalInsteadOfOnePerVertex() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int triangleStartIndex = sourceCode.IndexOf("void NintendoDsRenderManager3D::SubmitHardwareTexturedTriangle(", StringComparison.Ordinal);
        int vertexStartIndex = sourceCode.IndexOf("void NintendoDsRenderManager3D::SubmitHardwareTexturedVertex(", StringComparison.Ordinal);
        Assert.True(triangleStartIndex >= 0, "SubmitHardwareTexturedTriangle was not found.");
        Assert.True(vertexStartIndex > triangleStartIndex, "SubmitHardwareTexturedVertex should follow SubmitHardwareTexturedTriangle.");

        string triangleSource = sourceCode.Substring(triangleStartIndex, vertexStartIndex - triangleStartIndex);
        Assert.Contains("if (lightingEnabled)", triangleSource, StringComparison.Ordinal);
        Assert.Contains("glNormal(NORMAL_PACK(", triangleSource, StringComparison.Ordinal);
        Assert.Contains("SubmitHardwareTexturedVertex(positions, texCoords, runtimeTexture, lightingEnabled, indexA);", triangleSource, StringComparison.Ordinal);

        int fileEndIndex = sourceCode.IndexOf("}\n}\n#endif", vertexStartIndex, StringComparison.Ordinal);
        Assert.True(fileEndIndex > vertexStartIndex, "SubmitHardwareTexturedVertex end marker was not found.");
        string vertexSource = sourceCode.Substring(vertexStartIndex, fileEndIndex - vertexStartIndex);
        Assert.DoesNotContain("glNormal(NORMAL_PACK(", vertexSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies textured static meshes can use a cached display-list path keyed by the active hardware texture dimensions.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingStaticTexturedGeometry_usesTextureSizedDisplayListCache() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string runtimeModelHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRuntimeModel.hpp");
        string renderHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string renderSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string runtimeModelHeaderSource = File.ReadAllText(runtimeModelHeaderPath);
        string renderHeaderSource = File.ReadAllText(renderHeaderPath);
        string renderSource = File.ReadAllText(renderSourcePath);

        Assert.Contains("uint32_t* HardwareTexturedDisplayList;", runtimeModelHeaderSource, StringComparison.Ordinal);
        Assert.Contains("uint32_t HardwareTexturedDisplayListWordCount;", runtimeModelHeaderSource, StringComparison.Ordinal);
        Assert.Contains("int32_t HardwareTexturedDisplayListTextureWidth;", runtimeModelHeaderSource, StringComparison.Ordinal);
        Assert.Contains("int32_t HardwareTexturedDisplayListTextureHeight;", runtimeModelHeaderSource, StringComparison.Ordinal);
        Assert.Contains("bool UsesHardwareTexturedQuadDisplayList;", runtimeModelHeaderSource, StringComparison.Ordinal);

        Assert.Contains("void EnsureHardwareTexturedDisplayList(", renderHeaderSource, StringComparison.Ordinal);
        Assert.Contains("uint32_t* BuildHardwareTexturedDisplayList(", renderHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void SubmitStaticHardwareDisplayList(NintendoDsRuntimeModel* runtimeModel, bool useHardwareTexture);", renderHeaderSource, StringComparison.Ordinal);

        Assert.Contains("EnsureHardwareTexturedDisplayList(runtimeModel, hardwareTexture);", renderSource, StringComparison.Ordinal);
        Assert.Contains("SubmitStaticHardwareDisplayList(runtimeModel, useHardwareTexture);", renderSource, StringComparison.Ordinal);
        Assert.Contains("if (useHardwareTexture && runtimeModel->HardwareTexturedDisplayList != nullptr)", renderSource, StringComparison.Ordinal);
        Assert.Contains("runtimeModel->HardwareTexturedDisplayListTextureWidth == runtimeTexture->get_Width()", renderSource, StringComparison.Ordinal);
        Assert.Contains("runtimeModel->HardwareTexturedDisplayListTextureHeight == runtimeTexture->get_Height()", renderSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies textured static geometry can reuse the same quad-reduction path as lit display lists instead of always emitting textured triangles.
    /// </summary>
    [Fact]
    public void Source_whenBuildingTexturedStaticDisplayList_reducesReducibleTrianglePairsToQuads() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string renderHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string renderSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string renderHeaderSource = File.ReadAllText(renderHeaderPath);
        string renderSource = File.ReadAllText(renderSourcePath);

        Assert.Contains("uint32_t* BuildHardwareTexturedQuadDisplayList(", renderHeaderSource, StringComparison.Ordinal);
        Assert.Contains("bool TryAppendHardwareTexturedDisplayListQuad(", renderHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void AppendHardwareTexturedDisplayListQuad(", renderHeaderSource, StringComparison.Ordinal);
        Assert.Contains("uint32_t* texturedQuadDisplayList = BuildHardwareTexturedQuadDisplayList(runtimeModel, runtimeTexture, displayListWordCount);", renderSource, StringComparison.Ordinal);
        Assert.Contains("runtimeModel->UsesHardwareTexturedQuadDisplayList = true;", renderSource, StringComparison.Ordinal);
        Assert.Contains("displayListWords.push_back(GL_QUADS);", renderSource, StringComparison.Ordinal);
        Assert.Contains("AppendHardwareTexturedDisplayListQuad(displayListWords, positions, texCoords, runtimeTexture, indexA, indexD, indexC, indexB, useVertex10);", renderSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the DS profiler exposes separate textured 3D timing buckets for cache ensure, texture configure, and texture bind costs.
    /// </summary>
    [Fact]
    public void Source_whenPublishingDsProfilerText_exposesTexturedDisplayListAndTextureStateBuckets() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string renderHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string renderSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string renderHeaderSource = File.ReadAllText(renderHeaderPath);
        string renderSource = File.ReadAllText(renderSourcePath);

        Assert.Contains("double Last3DTexturedDisplayListEnsureMilliseconds;", renderHeaderSource, StringComparison.Ordinal);
        Assert.Contains("double Last3DTextureConfigureMilliseconds;", renderHeaderSource, StringComparison.Ordinal);
        Assert.Contains("double Last3DTextureBindMilliseconds;", renderHeaderSource, StringComparison.Ordinal);
        Assert.Contains("int32_t Last3DTexturedDisplayListBuildCount;", renderHeaderSource, StringComparison.Ordinal);
        Assert.Contains("int32_t Last3DTexturedDisplayListReuseCount;", renderHeaderSource, StringComparison.Ordinal);
        Assert.Contains("Last3DTexturedDisplayListEnsureMilliseconds = 0.0;", renderSource, StringComparison.Ordinal);
        Assert.Contains("Last3DTextureConfigureMilliseconds = 0.0;", renderSource, StringComparison.Ordinal);
        Assert.Contains("Last3DTextureBindMilliseconds = 0.0;", renderSource, StringComparison.Ordinal);
        Assert.Contains("Last3DTexturedDisplayListBuildCount = 0;", renderSource, StringComparison.Ordinal);
        Assert.Contains("Last3DTexturedDisplayListReuseCount = 0;", renderSource, StringComparison.Ordinal);
        Assert.Contains("+ \" TxCfg \" + FormatDebugMilliseconds(Last3DTextureConfigureMilliseconds)", renderSource, StringComparison.Ordinal);
        Assert.Contains("+ \" TxEns \" + FormatDebugMilliseconds(Last3DTexturedDisplayListEnsureMilliseconds)", renderSource, StringComparison.Ordinal);
        Assert.Contains("+ \" TxB \" + FormatDebugMilliseconds(Last3DTextureBindMilliseconds)", renderSource, StringComparison.Ordinal);
        Assert.Contains("+ \" B\" + std::to_string(Last3DTexturedDisplayListBuildCount)", renderSource, StringComparison.Ordinal);
        Assert.Contains("+ \" R\" + std::to_string(Last3DTexturedDisplayListReuseCount)", renderSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies Nintendo DS render-target creation rejects placeholder support instead of fabricating unsupported off-screen capability.
    /// </summary>
    [Fact]
    public void Source_whenCreatingDsRenderTarget_rejectsPlaceholderCapabilityLie() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("RenderTarget* CreateRenderTarget(int32_t width, int32_t height) override;", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("Builds one placeholder render target", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("RenderTarget* renderTarget = new RenderTarget();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("throw new InvalidOperationException(", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the DS on-device performance trace exposes 2D text, sprite, clear, and camera-overhead timings plus primitive counts so mixed 2D/3D frames can be attributed correctly.
    /// </summary>
    [Fact]
    public void Source_whenPublishingDsOverlayTrace_includes2dTextAndSpriteBreakdown() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("std::string FormatPerformanceOverlayAdditionalText(", headerSource, StringComparison.Ordinal);
        Assert.Contains("NintendoDsRenderManager2DProfileSnapshot profileSnapshot = renderManager2D->get_ProfileSnapshot();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("FormatPerformanceOverlayAdditionalText(profileSnapshot)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Txt ", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Spr ", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Cl ", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Cam ", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Ovh ", sourceCode, StringComparison.Ordinal);
        Assert.Contains(" T", sourceCode, StringComparison.Ordinal);
        Assert.Contains(" S", sourceCode, StringComparison.Ordinal);
        Assert.Contains("profileSnapshot.CameraMilliseconds", sourceCode, StringComparison.Ordinal);
        Assert.Contains("profileSnapshot.TextMilliseconds", sourceCode, StringComparison.Ordinal);
        Assert.Contains("profileSnapshot.SpriteMilliseconds", sourceCode, StringComparison.Ordinal);
        Assert.Contains("profileSnapshot.ClearMilliseconds", sourceCode, StringComparison.Ordinal);
        Assert.Contains("profileSnapshot.TextPrimitiveCount", sourceCode, StringComparison.Ordinal);
        Assert.Contains("profileSnapshot.SpritePrimitiveCount", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies repeated Nintendo DS 3D draws reuse cached material and texture state instead of re-emitting identical register writes for every instance.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingRepeatedDs3dDraws_cachesHardwareMaterialAndTextureState() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool CachedHardwareTextureEnabledValid;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void InvalidateHardwareStateCache();", headerSource, StringComparison.Ordinal);
        Assert.Contains("void ApplyHardwarePolyFormat(uint32_t polyFormat);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void ApplyHardwareTextureEnabledState(bool enabled);", headerSource, StringComparison.Ordinal);
        Assert.Contains("InvalidateHardwareStateCache();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ApplyHardwarePolyFormat(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ApplyHardwareAmbientMaterial(packedAmbientMaterial);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ApplyHardwareDiffuseMaterial(packedDiffuseMaterial);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ApplyHardwareTextureEnabledState(false);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ApplyHardwareTextureBinding(runtimeTexture->HardwareTextureId);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies textured Nintendo DS models can use the static hardware display-list path once a compatible textured display list exists, so textured scenes do not fall back to the immediate triangle path every frame.
    /// </summary>
    [Fact]
    public void Source_whenStaticTexturedDisplayListExists_allowsTexturedModelsToUseStaticDisplayListPath() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int methodStart = sourceCode.IndexOf("bool NintendoDsRenderManager3D::ShouldUseStaticHardwareDisplayList(NintendoDsRuntimeModel* runtimeModel, bool useHardwareTexture, int32_t runtimeModelInstanceCount) const {", StringComparison.Ordinal);
        int methodEnd = sourceCode.IndexOf("int32_t NintendoDsRenderManager3D::ResolveTrianglePrimitiveCount", StringComparison.Ordinal);
        string methodBody = sourceCode[methodStart..methodEnd];

        Assert.DoesNotContain("} else if (useHardwareTexture) {", methodBody, StringComparison.Ordinal);
        Assert.Contains("} else if (useHardwareTexture && runtimeModel->HardwareTexturedDisplayList == nullptr) {", methodBody, StringComparison.Ordinal);
        Assert.Contains("} else if (!useHardwareTexture && runtimeModel->HardwareLitDisplayList == nullptr) {", methodBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS 2D runtime no longer routes any drawable work through the legacy CPU bottom-screen framebuffer path.
    /// </summary>
    [Fact]
    public void Source_whenDrawingDs2d_usesOnlyHardwareBgAndObjPaths() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("ActiveCpuFrameBuffer = nullptr;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("RasterSprite(sprite);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("RasterText(text);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("std::fill_n(frameBuffer, VisibleFrameBufferPixelCount, clearColor);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS text and reject trace helpers no longer perform host-side file I/O in the runtime hot path.
    /// </summary>
    [Fact]
    public void Source_whenTracingDs2d_doesNotWriteHostFilesFromRuntimePath() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("BottomScreenTextTracePath", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("TopScreenRejectTracePath", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("::FileStream", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("::File::Delete", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies release-oriented DS builds compile out 3D trace and overlay string formatting so diagnostics-only integer formatting does not stay linked into the native binary.
    /// </summary>
    [Fact]
    public void Source_whenRuntimeDiagnosticsAreDisabled_guards3dTraceAndOverlayFormattingBehindCompileTimeFlag() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::string FormatDebugMilliseconds(double milliseconds) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("return std::to_string(whole) + \".\" + std::to_string(fractional);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("#else\n        /// Returns an empty diagnostics row fragment when release-oriented DS builds compile out native runtime diagnostics.\n        std::string FormatDebugMilliseconds(double milliseconds) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->SetFrameQueueCounts(LastTopScreen2DQueueCount, LastBottomScreen2DQueueCount);\n#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS\n        AppendTopScreenCameraTraceLine(\"[helengine-ds] camera-count=\" + std::to_string(cameraCount));", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::string NintendoDsRenderManager3D::FormatPerformanceOverlayDetailText() const {\n#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS\n        return std::string(\"Q3D \")", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::string NintendoDsRenderManager3D::FormatPerformanceOverlayAdditionalText(const NintendoDsRenderManager2DProfileSnapshot& profileSnapshot) const {\n#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS\n        double cameraOverheadMilliseconds", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::string NintendoDsRenderManager3D::FormatHardwareTextureDiagnostics() const {\n#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS\n        return std::string(\"D3T Tex\")", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::string NintendoDsRenderManager3D::FormatHardwareTextureLightingDiagnostics() const {\n#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS\n        return std::string(\"D3U Lit\")", sourceCode, StringComparison.Ordinal);
    }
}
