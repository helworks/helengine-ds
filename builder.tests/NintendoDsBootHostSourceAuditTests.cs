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
    /// Verifies the Nintendo DS boot host no longer emits recurring runtime timing diagnostics from the live FPS overlay state.
    /// </summary>
    [Fact]
    public void Source_whenMainLoopRuns_doesNotEmitRecurringRuntimeTimingDiagnostics() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("constexpr int32_t DiagnosticSampleFrameInterval = 120;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("FPSComponent* FindFirstFpsComponent(ObjectManager* objectManager)", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("void EmitRuntimeTimingDiagnostic(int32_t frameIndex, Core* core)", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("EmitRuntimeTimingDiagnostic(frameIndex, EngineCore);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("[helengine-ds] timing frame=", sourceCode, StringComparison.Ordinal);
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
    /// Verifies the Nintendo DS boot host uses the shared DS platform id so return-to-menu scene resolution targets the DS menu scene.
    /// </summary>
    [Fact]
    public void Source_whenInitializingPlatformInfo_usesSharedDsPlatformId() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("EnginePlatformInfo = new PlatformInfo(\"ds\", \"1\");", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("EnginePlatformInfo = new PlatformInfo(\"nintendo-ds\", \"1\");", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host keeps runtime diagnostics independent from menu-specific bottom-screen console profiling.
    /// </summary>
    [Fact]
    public void Source_whenMainLoopRuns_doesNotRouteMenuProfilingThroughBottomScreenConsole() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("NintendoDsRenderManager2DProfileSnapshot snapshot = EngineRenderManager2D->get_ProfileSnapshot();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("iprintf(\"profiling...\\n\");", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("BottomConsoleProfileFrameInterval", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("EmitBottomConsoleProfileDiagnostic(frameIndex);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("EmitRuntimeTimingDiagnostic(frameIndex, EngineCore);", sourceCode, StringComparison.Ordinal);
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

    /// <summary>
    /// Verifies the Nintendo DS boot host emits runtime scene-manager diagnostics from the main loop so scene-transition freezes can be diagnosed on-device.
    /// </summary>
    [Fact]
    public void Source_whenMainLoopRuns_emitsRuntimeSceneManagerDiagnostics() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("constexpr int32_t SceneManagerDiagnosticFrameInterval = 15;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("InitializeStatusConsole();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EngineRenderManager2D->SetBottomScreenPresentationEnabled(false);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsBootHost::EmitSceneManagerDiagnostic(int32_t frameIndex, int32_t accumulatedUpdateNetByteDelta, int32_t accumulatedDrawNetByteDelta)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("iprintf(\"version: 3.4\\n\");", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Own tex=%ld font=%ld mat=%ld mdl=%ld\\n", sourceCode, StringComparison.Ordinal);
        Assert.Contains("iprintf(\"\\x1b[1;1H\");", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("iprintf(\"frame %ld\\n\",", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"Cap d2=%ld/%ld int=%ld q2=%ld\\n\",", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"Ent e=%ld/%ld cmp=%ld ch=%ld\\n\",", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t totalComponentCapacity = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("int32_t totalChildCapacity = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("objectManager->get_Entities()->get_Count()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("objectManager->get_EntityCapacity()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("entity->get_Components()->get_Capacity()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("entity->get_Children()->get_Capacity()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EmitSceneManagerDiagnostic(frameIndex, accumulatedUpdateNetByteDelta, accumulatedDrawNetByteDelta);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RecordRuntimeFailureDiagnostics(\"Update\", frameIndex);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RecordRuntimeFailureDiagnostics(\"Draw\", frameIndex);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EngineRenderManager3D->get_LastHardware3DScreenTarget()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EngineRenderManager3D->get_LastCamera3DQueueCount()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EngineRenderManager3D->get_LastSubmittedDrawableCount()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EngineRenderManager3D->get_LastTopScreen2DQueueCount()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EngineRenderManager3D->get_LastBottomScreen2DQueueCount()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"3D screen=%s q=%ld draw=%ld\\n\",", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"2D top=%ld bottom=%ld\\n\",", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"Draw 2d=%ld 3d=%ld prs=%ld\\n\",", sourceCode, StringComparison.Ordinal);
        Assert.Contains("inputSystem->WasGamepadButtonPressed(0, InputGamepadButton::East)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("inputSystem->GetGamepadState(0)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("gamepadState.IsButtonDown(InputGamepadButton::East)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"B p=%d d=%d\\n\",", sourceCode, StringComparison.Ordinal);
        Assert.Contains("NintendoDsAllocationDiagnostics::GetCurrentAllocatedSize()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("NintendoDsAllocationDiagnostics::GetPeakAllocatedSize()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"Mem used=%u peak=%u\\n\",", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EngineRenderManager2D->get_TextBitmapCacheEntryCount()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EngineRenderManager2D->get_OpaqueRoundedRectCacheEntryCount()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"2D cache txt=%ld rect=%ld\\n\",", sourceCode, StringComparison.Ordinal);
        Assert.Contains("NintendoDsAllocationDiagnostics::GetTotalAllocatedSize()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("NintendoDsAllocationDiagnostics::GetTotalFreedSize()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"Alloc dPlus=%u dMinus=%u net=%d\\n\",", sourceCode, StringComparison.Ordinal);
        Assert.Contains("\"Alloc upd=%d draw=%d diag=%d\\n\",", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastEmittedAllocatedByteTotal = currentAllocatedByteTotal;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastEmittedFreedByteTotal = currentFreedByteTotal;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("LastEmittedDiagnosticNetByteDelta", sourceCode, StringComparison.Ordinal);
        Assert.Contains("accumulatedUpdateNetByteDelta += updateNetByteDelta;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("accumulatedDrawNetByteDelta += drawNetByteDelta;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("accumulatedUpdateNetByteDelta = 0;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("accumulatedDrawNetByteDelta = 0;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("iprintf(\"SceneMgr stage=%s\\n\",", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("std::string stage = sceneManager->get_LastTraceStage();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("std::string sceneId = sceneManager->get_LastTraceSceneId();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("std::string sceneLoadStage = sceneLoadService->get_LastTraceStage();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("bool cubeSceneLoaded = sceneManager->IsSceneLoaded(\"cube_test\");", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("bool menuSceneLoaded = sceneManager->IsSceneLoaded(\"DemoDiscMainMenuDs\");", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS startup scene goes through scene-manager loading so startup-owned assets participate in normal release tracking.
    /// </summary>
    [Fact]
    public void Source_whenLoadingStartupScene_routesThroughSceneManagerInsteadOfRawSceneLoadService() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("std::string startupSceneId = ResolveStartupSceneId(startupSceneRelativePath);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EngineCore->get_SceneManager()->LoadScene(startupSceneId, SceneLoadMode::Single);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("EngineCore->get_SceneLoadService()->Load(startupScene);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::string NintendoDsBootHost::ResolveStartupSceneId(const std::string& cookedRelativePath) const", sourceCode, StringComparison.Ordinal);
    }
}
