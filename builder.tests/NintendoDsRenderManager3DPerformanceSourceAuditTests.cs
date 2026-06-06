namespace helengine.ds.builder.tests;

/// <summary>
/// Audits Nintendo DS renderer source so performance-sensitive draw-path contracts stay intact.
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
    /// Verifies pure hardware-3D frames do not copy invisible CPU bitmap backbuffers after geometry submission.
    /// </summary>
    [Fact]
    public void Source_whenFrameHasOnlyHardware3d_skipsCpuBitmapPresent() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool ShouldPresent2DFrame(NintendoDsScreenTarget hardware3DScreenTarget, NintendoDsRenderManager2D* renderManager2D) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool shouldPresent2DFrame = ShouldPresent2DFrame(hardware3DScreenTarget, renderManager2D);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("if (shouldPresent2DFrame) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("} else {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastPresentNetByteDelta = 0;", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies pure hardware-3D cameras do not clear or traverse invisible 2D bitmap buffers before geometry submission.
    /// </summary>
    [Fact]
    public void Source_whenCameraOnlyHas3dQueue_skips2dCameraTraversal() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool cameraHas3DQueue = renderQueue3D != nullptr && renderQueue3D->get_Count() > 0;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("renderManager2D->DrawCamera(camera);\r\n            }\r\n\r\n            AccumulateCameraScreenQueues(camera", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Draw2DCameraList(cameras, renderManager2D);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies hardware-3D target resolution inspects queue ownership without drawing hidden software 2D work.
    /// </summary>
    [Fact]
    public void Source_whenResolvingHardware3dTarget_doesNotDrawSoftware2dCameras() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);
        int resolveStart = sourceCode.IndexOf("NintendoDsScreenTarget NintendoDsRenderManager3D::ResolveHardware3DScreenTarget", StringComparison.Ordinal);
        int resolveEnd = sourceCode.IndexOf("void NintendoDsRenderManager3D::Draw2DCameraList", StringComparison.Ordinal);
        string resolveBody = sourceCode.Substring(resolveStart, resolveEnd - resolveStart);

        Assert.Contains("void Draw2DCameraList(List<ICamera*>* cameras, NintendoDsRenderManager2D* renderManager2D);", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("renderManager2D->DrawCamera(camera);", resolveBody, StringComparison.Ordinal);
        Assert.Contains("Draw2DCameraList(cameras, renderManager2D);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the arithmetic-heavy 3D renderer is built in ARM mode on devkitARM.
    /// </summary>
    [Fact]
    public void Makefile_whenBuildingDs3dRenderer_compilesRenderManager3dInArmMode() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string makefilePath = Path.Combine(repositoryRootPath, "Makefile");
        string makefileSource = File.ReadAllText(makefilePath);

        Assert.Contains("NintendoDsRenderManager3D.o: CXXFLAGS := $(filter-out -mthumb,$(CXXFLAGS)) -marm", makefileSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the generated core translation units are built in ARM mode so BEPU and math-heavy runtime code do not run through Thumb codegen.
    /// </summary>
    [Fact]
    public void Makefile_whenBuildingGeneratedCore_compilesGeneratedRuntimeInArmMode() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string makefilePath = Path.Combine(repositoryRootPath, "Makefile");
        string makefileSource = File.ReadAllText(makefilePath);

        Assert.Contains("$(GENERATED_CORE_TRANSLATION_UNIT:.cpp=.o): CXXFLAGS := $(filter-out -mthumb,$(CXXFLAGS)) -marm", makefileSource, StringComparison.Ordinal);
        Assert.Contains("runtime_startup_manifest.o: CXXFLAGS := $(filter-out -mthumb,$(CXXFLAGS)) -marm", makefileSource, StringComparison.Ordinal);
        Assert.Contains("runtime_scene_catalog_manifest.o: CXXFLAGS := $(filter-out -mthumb,$(CXXFLAGS)) -marm", makefileSource, StringComparison.Ordinal);
        Assert.Contains("runtime_code_module_manifest.o: CXXFLAGS := $(filter-out -mthumb,$(CXXFLAGS)) -marm", makefileSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies frame lighting is resolved once per draw from runtime-managed light collections.
    /// </summary>
    [Fact]
    public void Source_whenResolvingFrameLighting_usesObjectManagerLightCollectionsOutsideDrawableLoop() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("float3 FrameLightDirection;", headerSource, StringComparison.Ordinal);
        Assert.Contains("float3 FrameDirectionalRadiance;", headerSource, StringComparison.Ordinal);
        Assert.Contains("float3 FrameAmbientRadiance;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void ResolveFrameLighting(ObjectManager* objectManager);", headerSource, StringComparison.Ordinal);
        Assert.Contains("ResolveFrameLighting(objectManager);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("objectManager->get_DirectionalLights()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("objectManager->get_AmbientLights()", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("ResolveSceneLighting(lightDirection, directionalRadiance, ambientRadiance);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the DS 3D renderer publishes per-frame CPU timing buckets for the debug overlay without relying on nested timer resets.
    /// </summary>
    [Fact]
    public void Source_whenDrawing3dFrame_publishesStepTimingBucketsForDebugOverlay() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("#include <nds/timers.h>", sourceCode, StringComparison.Ordinal);
        Assert.Contains("#include <nds/arm9/console.h>", sourceCode, StringComparison.Ordinal);
        Assert.Contains("double Last2DTraversalMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("double Last3DSetupMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("double Last3DQueueSnapshotMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("double Last3DGeometryEmitMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("double Last3DFlushMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("double LastPresentMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("PrintConsole NativeDebugConsole;", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool NativeDebugOverlayInitialized;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void PublishPerformanceOverlayMetrics(Core* core, NintendoDsRenderManager2D* renderManager2D, bool usesMetrics);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void DrawNativeDebugOverlay(Core* core, ObjectManager* objectManager, NintendoDsRenderManager2D* renderManager2D, bool usesMetrics);", headerSource, StringComparison.Ordinal);
        Assert.Contains("cpuStartTiming(0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("cpuGetTiming()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Last2DTraversalMilliseconds = ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - traversalStartTimingTicks);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Last3DSetupMilliseconds = ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - setupStartTimingTicks);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Last3DQueueSnapshotMilliseconds = ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - queueSnapshotStartTimingTicks);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Last3DGeometryEmitMilliseconds = ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - geometryEmitStartTimingTicks);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Last3DFlushMilliseconds = ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - flushStartTimingTicks);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastPresentMilliseconds = ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - presentStartTimingTicks);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("core->SetPerformanceOverlayMetrics(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("PerformanceOverlayDebugLineRefreshSeconds", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("LastPerformanceOverlayDebugLineElapsedSeconds", sourceCode, StringComparison.Ordinal);
        Assert.Contains("usesMetrics ? Last3DGeometryEmitMilliseconds : 0.0", sourceCode, StringComparison.Ordinal);
        Assert.Contains("usesMetrics ? Last3DFlushMilliseconds : 0.0", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::string FormatDebugMilliseconds(double milliseconds)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("consoleInit(&NativeDebugConsole, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("iprintf(\"\\x1b[%d;0H%-32.32s\", row, text.c_str());", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("DrawNativeDebugOverlay(core, objectManager, renderManager2D, true);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("DebugComponent::SetAdditionalLine(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("NintendoDsRenderManager2DProfileSnapshot profileSnapshot = renderManager2D->get_ProfileSnapshot();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"D2D T\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\" C\"", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("\"D3A 2D\"", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("\"D3B G\"", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("\"DS3D\"", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("\"Geo\"", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the DS native debug overlay splits aggregate geometry time into actionable per-drawable buckets.
    /// </summary>
    [Fact]
    public void Source_whenSubmitting3dGeometry_publishesTransformMaterialAndDisplayListBuckets() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("double Last3DTransformMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("double Last3DMaterialMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("double Last3DDisplayListMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("double Last3DFallbackGeometryMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("uint32_t transformStartTimingTicks = cpuGetTiming();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Last3DTransformMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - transformStartTimingTicks);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint32_t materialStartTimingTicks = cpuGetTiming();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Last3DMaterialMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - materialStartTimingTicks);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint32_t displayListStartTimingTicks = cpuGetTiming();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Last3DDisplayListMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - displayListStartTimingTicks);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint32_t fallbackGeometryStartTimingTicks = cpuGetTiming();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Last3DFallbackGeometryMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - fallbackGeometryStartTimingTicks);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("\"D3D X\"", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the native DS debug overlay exposes resolved light-vector signs while axis-test lighting parity is being tuned.
    /// </summary>
    [Fact]
    public void Source_whenDrawingNativeDebugOverlay_printsResolvedAndPackedLightDirections() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("std::string FormatDebugSignedUnit(float value)", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("\"D3L W\"", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies hardware-3D frames keep the sub 2D bitmap path available for scene-authored bottom-screen overlays.
    /// </summary>
    [Fact]
    public void Source_whenHardware3dUsesSceneAuthoredBottomOverlay_keepsBottomBitmapPresentationEnabled() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool useNativeDebugOverlay = false;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("renderManager2D->SetBottomScreenPresentationEnabled(!useNativeDebugOverlay);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Draw2DCameraList(cameras, renderManager2D);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies drawable-scope transform inputs are cached outside the triangle loop.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingTriangles_cachesEntityTransformOutsidePerTriangleWork() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void SubmitHardwareLitTriangle(", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("const float3& entityPosition,\r\n            const float3& entityScale,\r\n            const float4& entityOrientation);", headerSource, StringComparison.Ordinal);
        Assert.Contains("Entity* entity = drawable->get_Parent();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("float3 entityPosition = float3::get_Zero();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("float3 entityScale = float3::get_One();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("float4 entityOrientation = float4::get_Identity();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ApplyDrawableTransformToHardwareMatrix(entityPosition, entityScale, entityOrientation);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("float3 faceNormal = float4::RotateVector(modelFaceNormal, entityOrientation);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glVertex3v16(floattov16((*positions)[indexA].X)", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("TransformVertex(drawable, (*positions)[indexA])", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("TransformVertex((*positions)[indexA], entityPosition, entityScale, entityOrientation)", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies lit geometry uses Nintendo DS fixed-function lighting instead of per-triangle CPU color shading.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingLitTriangles_usesNintendoDsFixedFunctionLighting() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void ConfigureFrameHardwareLight();", headerSource, StringComparison.Ordinal);
        Assert.Contains("void ConfigureHardwareMaterial(NintendoDsRuntimeMaterial* runtimeMaterial);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void SubmitHardwareLitTriangle(", headerSource, StringComparison.Ordinal);
        Assert.Contains("glLight(0,", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glMaterialf(GL_AMBIENT,", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glMaterialf(GL_DIFFUSE,", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint16_t packedDirectionalLight = NintendoDsColorPacker::PackRegisterColor(NintendoDsLightingMath::ClampColor(FrameDirectionalRadiance));", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint16_t packedAmbientMaterial = NintendoDsColorPacker::PackRegisterColor(ambientMaterial);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint16_t packedDiffuseMaterial = NintendoDsColorPacker::PackRegisterColor(runtimeMaterial->BaseColor);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("glNormal(NORMAL_PACK(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("float3 shapedLighting = NintendoDsLightingMath::ApplyDisplayContrastCurve(lighting);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("float3 finalColor = NintendoDsLightingMath::MultiplyColor(runtimeMaterial->BaseColor, shapedLighting);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("glColor(NintendoDsColorPacker::PackOpaqueColor(finalColor));", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS hardware light vector preserves the shared engine XYZ direction convention.
    /// </summary>
    [Fact]
    public void Source_whenConfiguringHardwareLight_preservesAuthoredAxisDirections() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("PackHardwareNormalComponent(FrameLightDirection.X)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("PackHardwareNormalComponent(FrameLightDirection.Y)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("PackHardwareNormalComponent(FrameLightDirection.Z)", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("PackHardwareNormalComponent(-FrameLightDirection.X)", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("PackHardwareNormalComponent(-FrameLightDirection.Y)", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("PackHardwareNormalComponent(-FrameLightDirection.Z)", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies constant Nintendo DS polygon and material-lighting state is configured once per frame instead of once per drawable.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingLitDrawables_hoistsConstantHardwareStateOutOfDrawableLoop() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        int frameLightStart = sourceCode.IndexOf("void NintendoDsRenderManager3D::ConfigureFrameHardwareLight()", StringComparison.Ordinal);
        int materialStart = sourceCode.IndexOf("void NintendoDsRenderManager3D::ConfigureHardwareMaterial", StringComparison.Ordinal);
        int materialEnd = sourceCode.IndexOf("int32_t NintendoDsRenderManager3D::PackHardwareNormalComponent", StringComparison.Ordinal);
        int submitStart = sourceCode.IndexOf("void NintendoDsRenderManager3D::SubmitOpaqueDrawable", StringComparison.Ordinal);
        int submitEnd = sourceCode.IndexOf("void NintendoDsRenderManager3D::ApplyDrawableTransformToHardwareMatrix", StringComparison.Ordinal);

        Assert.True(frameLightStart >= 0);
        Assert.True(materialStart > frameLightStart);
        Assert.True(materialEnd > materialStart);
        Assert.True(submitStart >= 0);
        Assert.True(submitEnd > submitStart);

        string frameLightAndSetupBody = sourceCode.Substring(frameLightStart, materialStart - frameLightStart);
        string materialBody = sourceCode.Substring(materialStart, materialEnd - materialStart);
        string submitBody = sourceCode.Substring(submitStart, submitEnd - submitStart);

        Assert.Contains("glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0);", frameLightAndSetupBody, StringComparison.Ordinal);
        Assert.Contains("glMaterialf(GL_SPECULAR, 0);", frameLightAndSetupBody, StringComparison.Ordinal);
        Assert.Contains("glMaterialf(GL_EMISSION, 0);", frameLightAndSetupBody, StringComparison.Ordinal);
        Assert.DoesNotContain("glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0);", submitBody, StringComparison.Ordinal);
        Assert.DoesNotContain("glMaterialf(GL_SPECULAR, 0);", materialBody, StringComparison.Ordinal);
        Assert.DoesNotContain("glMaterialf(GL_EMISSION, 0);", materialBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies frame light direction is submitted from camera/view space before drawable model transforms can affect it.
    /// </summary>
    [Fact]
    public void Source_whenDrawing3dQueue_configuresFrameLightBeforeDrawableTransforms() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void ConfigureFrameHardwareLight();", headerSource, StringComparison.Ordinal);
        Assert.Contains("void ConfigureHardwareMaterial(NintendoDsRuntimeMaterial* runtimeMaterial);", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("void ConfigureHardwareLighting(NintendoDsRuntimeMaterial* runtimeMaterial);", headerSource, StringComparison.Ordinal);
        Assert.Contains("ConfigureFrameHardwareLight();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("ConfigureHardwareMaterial(runtimeMaterial);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("ConfigureHardwareLighting(runtimeMaterial);", sourceCode, StringComparison.Ordinal);

        int cameraConfigureIndex = sourceCode.IndexOf("ConfigureCamera(camera);", StringComparison.Ordinal);
        int lightConfigureIndex = sourceCode.IndexOf("ConfigureFrameHardwareLight();", StringComparison.Ordinal);
        int drawQueueIndex = sourceCode.IndexOf("LastSubmittedDrawableCount = DrawRenderQueue(camera);", StringComparison.Ordinal);
        int transformIndex = sourceCode.IndexOf("ApplyDrawableTransformToHardwareMatrix(entityPosition, entityScale, entityOrientation);", StringComparison.Ordinal);
        Assert.True(cameraConfigureIndex >= 0);
        Assert.True(lightConfigureIndex > cameraConfigureIndex);
        Assert.True(drawQueueIndex > lightConfigureIndex);
        Assert.True(transformIndex > drawQueueIndex);
    }

    /// <summary>
    /// Verifies static lit geometry is submitted through a packed Nintendo DS display list instead of rebuilt every frame.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingStaticLitGeometry_usesPackedDisplayLists() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string modelHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRuntimeModel.hpp");
        string rendererHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string modelHeaderSource = File.ReadAllText(modelHeaderPath);
        string rendererHeaderSource = File.ReadAllText(rendererHeaderPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("uint32_t* HardwareLitDisplayList;", modelHeaderSource, StringComparison.Ordinal);
        Assert.Contains("uint32_t HardwareLitDisplayListWordCount;", modelHeaderSource, StringComparison.Ordinal);
        Assert.Contains("bool UsesHardwareLitQuadDisplayList;", modelHeaderSource, StringComparison.Ordinal);
        Assert.Contains("uint32_t* BuildHardwareLitDisplayList(", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void AppendHardwareLitDisplayListTriangle(", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("runtimeModel->HardwareLitDisplayList = BuildHardwareLitDisplayList(runtimeModel, runtimeModel->HardwareLitDisplayListWordCount);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("delete[] runtimeModel->HardwareLitDisplayList;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("FlushHardwareLitDisplayList(displayList);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("FIFO_COMMAND_PACK(FIFO_BEGIN, FIFO_NOP, FIFO_NOP, FIFO_NOP)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("FIFO_COMMAND_PACK(FIFO_NORMAL, FIFO_VERTEX16, FIFO_VERTEX16, FIFO_VERTEX16)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("FIFO_COMMAND_PACK(FIFO_END, FIFO_NOP, FIFO_NOP, FIFO_NOP)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("displayList[0] = static_cast<uint32_t>(displayListWords.size());", sourceCode, StringComparison.Ordinal);
        Assert.Contains("SubmitStaticHardwareDisplayList(runtimeModel);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("glCallList(runtimeModel->HardwareLitDisplayList);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies static display lists are flushed once when built and later submitted without per-frame cache flushing.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingStaticDisplayLists_avoidsPerFrameCacheFlush() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("void FlushHardwareLitDisplayList(uint32_t* displayList) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void SubmitStaticHardwareDisplayList(NintendoDsRuntimeModel* runtimeModel);", headerSource, StringComparison.Ordinal);
        Assert.Contains("DC_FlushRange(displayList + 1, displayList[0] * sizeof(uint32_t));", sourceCode, StringComparison.Ordinal);
        Assert.Contains("while ((DMA_CR(0) & DMA_BUSY) || (DMA_CR(1) & DMA_BUSY) || (DMA_CR(2) & DMA_BUSY) || (DMA_CR(3) & DMA_BUSY)) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("DMA_SRC(0) = reinterpret_cast<uint32_t>(displayList + 1);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("DMA_DEST(0) = 0x4000400;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("DMA_CR(0) = DMA_FIFO | displayList[0];", sourceCode, StringComparison.Ordinal);
        Assert.Contains("while (DMA_CR(0) & DMA_BUSY) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("SubmitStaticHardwareDisplayList(runtimeModel);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("glCallList(runtimeModel->HardwareLitDisplayList);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies native DS diagnostics expose display-list call volume separately from display-list CPU time.
    /// </summary>
    [Fact]
    public void Source_whenDrawingDisplayLists_reportsDisplayListVolumeDiagnostics() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string rendererHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string modelHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRuntimeModel.hpp");
        string modelSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRuntimeModel.cpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string rendererHeaderSource = File.ReadAllText(rendererHeaderPath);
        string modelHeaderSource = File.ReadAllText(modelHeaderPath);
        string modelSourceCode = File.ReadAllText(modelSourcePath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool UsesHardwareLitQuadDisplayList;", modelHeaderSource, StringComparison.Ordinal);
        Assert.Contains(", UsesHardwareLitQuadDisplayList(false)", modelSourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t Last3DDisplayListCallCount;", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("uint32_t Last3DDisplayListSubmittedWordCount;", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("int32_t Last3DQuadDisplayListCallCount;", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("runtimeModel->UsesHardwareLitQuadDisplayList = false;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("runtimeModel->UsesHardwareLitQuadDisplayList = true;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Last3DDisplayListCallCount++;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Last3DDisplayListSubmittedWordCount += runtimeModel->HardwareLitDisplayListWordCount;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Last3DQuadDisplayListCallCount++;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("\"D3E W\"", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies indexed triangle pairs that represent planar quads are packed as DS quads to reduce FIFO traffic.
    /// </summary>
    [Fact]
    public void Source_whenStaticLitGeometryFormsQuads_packsQuadDisplayListCommands() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("uint32_t* BuildHardwareLitQuadDisplayList(", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool TryAppendHardwareLitDisplayListQuad(", headerSource, StringComparison.Ordinal);
        Assert.Contains("void AppendHardwareLitDisplayListQuad(", headerSource, StringComparison.Ordinal);
        Assert.Contains("uint32_t* quadDisplayList = BuildHardwareLitQuadDisplayList(runtimeModel, displayListWordCount);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("displayListWords.push_back(GL_QUADS);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("FIFO_COMMAND_PACK(FIFO_NORMAL, FIFO_VERTEX16, FIFO_VERTEX16, FIFO_VERTEX16)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("FIFO_COMMAND_PACK(FIFO_VERTEX16, FIFO_NOP, FIFO_NOP, FIFO_NOP)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("AppendHardwareLitDisplayListQuad(displayListWords, positions, indexA, indexD, indexC, indexB, useVertex10);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("MinimumHardwareLitDisplayListWordCount", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies small static meshes use the one-word DS VTX10 command instead of two-word VTX16 vertices.
    /// </summary>
    [Fact]
    public void Source_whenStaticLitGeometryFitsVtx10_packsDisplayListsWithOneWordVertices() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("bool CanUseHardwareLitVertex10DisplayList(Array<float3>* positions) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("int32_t PackHardwareVertex10Component(float value) const;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void AppendHardwareLitDisplayListVertex10(std::vector<uint32_t>& displayListWords, const float3& position);", headerSource, StringComparison.Ordinal);
        Assert.Contains("bool useVertex10 = CanUseHardwareLitVertex10DisplayList(positions);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("FIFO_COMMAND_PACK(FIFO_NORMAL, FIFO_VERTEX10, FIFO_VERTEX10, FIFO_VERTEX10)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("FIFO_COMMAND_PACK(FIFO_VERTEX10, FIFO_NOP, FIFO_NOP, FIFO_NOP)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("AppendHardwareLitDisplayListVertex10(displayListWords, vertexA);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("PackHardwareVertex10Component(position.X)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::round(clampedValue * 64.0f)", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies static display-list submission exposes DMA wait phases separately from aggregate list time.
    /// </summary>
    [Fact]
    public void Source_whenSubmittingStaticDisplayLists_reportsDmaWaitBuckets() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");

        string headerSource = File.ReadAllText(headerPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("double Last3DDisplayListPreWaitMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("double Last3DDisplayListKickMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("double Last3DDisplayListPostWaitMilliseconds;", headerSource, StringComparison.Ordinal);
        Assert.Contains("void SubmitStaticHardwareDisplayList(NintendoDsRuntimeModel* runtimeModel);", headerSource, StringComparison.Ordinal);
        Assert.Contains("uint32_t preWaitStartTimingTicks = cpuGetTiming();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Last3DDisplayListPreWaitMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - preWaitStartTimingTicks);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint32_t dmaKickStartTimingTicks = cpuGetTiming();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Last3DDisplayListKickMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - dmaKickStartTimingTicks);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint32_t postWaitStartTimingTicks = cpuGetTiming();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Last3DDisplayListPostWaitMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - postWaitStartTimingTicks);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("\"D3F A\"", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the DS native debug overlay reports hardware VBlank pacing separately from draw-call CPU time.
    /// </summary>
    [Fact]
    public void Source_whenDrawingNativeDebugOverlay_reportsVBlankPacingDiagnostics() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string rendererHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.hpp");
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string pacingHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsFramePacing.hpp");

        string rendererHeaderSource = File.ReadAllText(rendererHeaderPath);
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.True(File.Exists(pacingHeaderPath), "Expected a DS frame-pacing header to expose the hardware VBlank counter.");
        Assert.Contains("#include \"platform/ds/NintendoDsFramePacing.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint32_t LastNativeDebugOverlayVBlankCount;", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("int32_t LastNativeDebugOverlayVBlankDelta;", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("int32_t NativeDebugOverlayMissedVBlankCount;", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("void SampleNativeDebugOverlayFramePacing();", rendererHeaderSource, StringComparison.Ordinal);
        Assert.Contains("SampleNativeDebugOverlayFramePacing();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint32_t currentVBlankCount = GetNintendoDsVBlankCount();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("NativeDebugOverlayMissedVBlankCount += LastNativeDebugOverlayVBlankDelta - 1;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("\"D3C VB\"", sourceCode, StringComparison.Ordinal);
    }
}
