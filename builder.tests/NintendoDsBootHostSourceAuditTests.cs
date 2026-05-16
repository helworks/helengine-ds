namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS boot host source so release startup enters the generated-core runtime loop instead of the diagnostic smoke test.
/// </summary>
public class NintendoDsBootHostSourceAuditTests {
    /// <summary>
    /// Verifies the Nintendo DS boot host run path enters checkpointed startup and does not stop in the status-console smoke test.
    /// </summary>
    [Fact]
    public void Source_whenGeneratedCoreIsEnabled_runUsesCheckpointedStartupInsteadOfSmokeTest() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("RunCheckpointedStartup();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RunStatusConsoleSmokeTest();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host hands screen ownership to the render loop instead of hardcoding startup-scene presentation mode.
    /// </summary>
    [Fact]
    public void Source_whenCheckpointedStartupCompletes_doesNotOwnPermanentStartupScenePresentationMode() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("PrepareMainScreenForConfiguredStartupScene();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("void NintendoDsBootHost::PrepareMainScreenForMenu2D()", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("void NintendoDsBootHost::PrepareMainScreenFor3D()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RunMainLoop();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host emits recurring runtime timing diagnostics from the live FPS overlay state.
    /// </summary>
    [Fact]
    public void Source_whenMainLoopRuns_emitsRecurringRuntimeTimingDiagnostics() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("constexpr int32_t DiagnosticSampleFrameInterval = 120;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("FPSComponent* FindFirstFpsComponent(ObjectManager* objectManager)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void EmitRuntimeTimingDiagnostic(int32_t frameIndex, Core* core)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EmitRuntimeTimingDiagnostic(frameIndex, EngineCore);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host derives real engine update slices from counted VBlanks instead of always reporting a synthetic fixed 60 FPS.
    /// </summary>
    [Fact]
    public void Source_whenMainLoopRuns_usesVBlankCountToDeriveElapsedSeconds() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("volatile uint32_t VBlankCount = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void HandleVBlankInterrupt()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("irqSet(IRQ_VBLANK, HandleVBlankInterrupt);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("irqEnable(IRQ_VBLANK);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint32_t previousVBlankCount = VBlankCount;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint32_t currentVBlankCount = VBlankCount;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("uint32_t elapsedVBlanks = currentVBlankCount > previousVBlankCount ? currentVBlankCount - previousVBlankCount : 1;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("double elapsedSeconds = static_cast<double>(elapsedVBlanks) * NintendoDsFrameDeltaSeconds;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EngineCore->Update(elapsedSeconds);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host builds and injects a runtime scene catalog before initializing generated-core startup.
    /// </summary>
    [Fact]
    public void Source_whenInitializingCore_buildsRuntimeSceneCatalogFromNativeManifest() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("#include \"RuntimeSceneCatalog.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("#include \"RuntimeSceneCatalogEntry.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("#include \"runtime/runtime_scene_catalog_manifest.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("::RuntimeSceneCatalog* BuildRuntimeSceneCatalog()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("const HERuntimeSceneCatalogEntry* sceneEntries = he_runtime_scene_catalog_entries(&sceneCount);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("new ::RuntimeSceneCatalogEntry(sourceEntry.SceneId, sourceEntry.CookedRelativePath)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EngineOptions->set_SceneCatalog(BuildRuntimeSceneCatalog());", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host keeps runtime diagnostics independent from menu-specific bottom-screen console profiling.
    /// </summary>
    [Fact]
    public void Source_whenMainLoopRuns_doesNotRouteMenuProfilingThroughBottomScreenConsole() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("EngineRenderManager2D->SetBottomScreenPresentationEnabled(false);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("NintendoDsRenderManager2DProfileSnapshot snapshot = EngineRenderManager2D->get_ProfileSnapshot();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("iprintf(\"profiling...\\n\");", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("BottomConsoleProfileFrameInterval", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("EmitBottomConsoleProfileDiagnostic(frameIndex);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EmitRuntimeTimingDiagnostic(frameIndex, EngineCore);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host no longer owns permanent menu-versus-3D screen policy once generated core has entered the runtime loop.
    /// </summary>
    [Fact]
    public void Source_whenGeneratedCoreRuns_bootHostNoLongerOwnsPermanentMenuVersus3dScreenPolicy() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("PrepareMainScreenForConfiguredStartupScene();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("PrepareMainScreenForMenu2D();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("PrepareMainScreenFor3D();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("PrepareBottomScreenForMenuProfilingConsole();", sourceCode, StringComparison.Ordinal);
    }
}
