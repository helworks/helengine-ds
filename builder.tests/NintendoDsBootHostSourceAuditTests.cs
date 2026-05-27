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
    /// Verifies the Nintendo DS generated-core startup path registers the shared 3D physics runtime hook before loading packaged scenes.
    /// </summary>
    [Fact]
    public void Source_whenGeneratedCoreInitializes_registersPhysicsRuntimeBeforeStartupSceneLoad() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("#include \"Physics3DRuntimeComponentRegistration.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("Physics3DRuntimeComponentRegistration::Register(EngineCore);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS main loop does not wait for a second VBlank when the renderer already synchronized during the previous draw.
    /// </summary>
    [Fact]
    public void Source_whenRendererAlreadyCrossedVBlank_mainLoopSkipsExtraFramePacingWait() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);
        int runMainLoopStart = sourceCode.IndexOf("void NintendoDsBootHost::RunMainLoop()", StringComparison.Ordinal);
        int showFatalErrorStart = sourceCode.IndexOf("void NintendoDsBootHost::ShowFatalErrorAndHalt", StringComparison.Ordinal);
        string runMainLoopBody = sourceCode[runMainLoopStart..showFatalErrorStart];

        Assert.Contains("if (VBlankCount == previousVBlankCount) {", runMainLoopBody, StringComparison.Ordinal);
        Assert.Contains("swiWaitForVBlank();", runMainLoopBody, StringComparison.Ordinal);
        Assert.Contains("uint32_t currentVBlankCount = VBlankCount;", runMainLoopBody, StringComparison.Ordinal);
        Assert.DoesNotContain("while (true) {\r\n            swiWaitForVBlank();", runMainLoopBody, StringComparison.Ordinal);
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
    /// Verifies the Nintendo DS boot host builds and injects standard platform input actions from the generated runtime manifest.
    /// </summary>
    [Fact]
    public void Source_whenInitializingCore_buildsStandardPlatformInputConfigurationFromNativeManifest() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("#include \"runtime/runtime_standard_platform_input_manifest.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("::StandardPlatformInputConfiguration* BuildStandardPlatformInputConfiguration()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("const HERuntimeStandardPlatformActionEntry* actionEntries = he_runtime_standard_platform_action_entries(&entryCount);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("new ::StandardPlatformActionBinding(", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EngineOptions->set_StandardPlatformInputConfiguration(BuildStandardPlatformInputConfiguration());", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host uses the shared DS platform id and current visible menu version.
    /// </summary>
    [Fact]
    public void Source_whenInitializingPlatformInfo_usesSharedDsPlatformId() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("EnginePlatformInfo = new PlatformInfo(\"ds\", \"2\");", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("EnginePlatformInfo = new PlatformInfo(\"nintendo-ds\", \"2\");", sourceCode, StringComparison.Ordinal);
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
    /// Verifies the Nintendo DS boot host leaves the bottom screen available to normal scene rendering during runtime instead of installing the live diagnostics console.
    /// </summary>
    [Fact]
    public void Source_whenMainLoopRuns_doesNotInstallLiveBottomScreenDiagnostics() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.hpp");
        string sourceCode = File.ReadAllText(sourcePath);
        string headerSource = File.ReadAllText(headerPath);

        Assert.DoesNotContain("constexpr int32_t SceneManagerDiagnosticFrameInterval = 15;", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("#include \"platform/ds/NintendoDsRuntimeDiagnosticsProvider.hpp\"", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("EngineRuntimeDiagnosticsProvider = new NintendoDsRuntimeDiagnosticsProvider(&StatusConsole);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("EngineOptions->set_RuntimeDiagnosticsProvider(EngineRuntimeDiagnosticsProvider);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("EngineRenderManager2D->SetBottomScreenPresentationEnabled(false);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("void NintendoDsBootHost::EmitSceneManagerDiagnostic(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("void NintendoDsBootHost::UpdateLiveStageConsole(", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("LastObservedRuntimePhase", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("LastBootHostStage", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("LastEmittedAllocatedByteTotal", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("LastEmittedFreedByteTotal", headerSource, StringComparison.Ordinal);
        Assert.DoesNotContain("LastEmittedDiagnosticNetByteDelta", headerSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS main loop records native and managed exception messages before rethrowing update or draw failures to the top-level fatal handler.
    /// </summary>
    [Fact]
    public void Source_whenMainLoopFails_logsExceptionMessagesBeforeRethrow() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string headerPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.hpp");
        string sourceCode = File.ReadAllText(sourcePath);
        string headerSource = File.ReadAllText(headerPath);

        Assert.Contains("void RecordRuntimeFailureDiagnostics(const char* phase, int32_t frameIndex, const char* exceptionKind, const char* message);", headerSource, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsBootHost::RecordRuntimeFailureDiagnostics(const char* phase, int32_t frameIndex, const char* exceptionKind, const char* message)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("InitializeStatusConsole();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsBootHost::PrintStatusLine(int row, const char* text)", sourceCode, StringComparison.Ordinal);
        Assert.Contains("iprintf(\"\\x1b[%d;0H%-32.32s\", row, text != nullptr ? text : \"\");", sourceCode, StringComparison.Ordinal);
        Assert.Contains("PrintStatusLine(7, failureLine.data());", sourceCode, StringComparison.Ordinal);
        Assert.Contains("PrintStatusLine(8, exceptionKind != nullptr ? exceptionKind : \"Exception unknown\");", sourceCode, StringComparison.Ordinal);
        Assert.Contains("const char* failureMessage = message != nullptr ? message : \"No exception message.\";", sourceCode, StringComparison.Ordinal);
        Assert.Contains("PrintStatusLine(9, failureMessage);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("PrintStatusLine(10, failureMessageLength > 31 ? failureMessage + 31 : \"\");", sourceCode, StringComparison.Ordinal);
        Assert.Contains("PrintStatusLine(11, failureMessageLength > 62 ? failureMessage + 62 : \"\");", sourceCode, StringComparison.Ordinal);
        Assert.Contains("messageBuilder << \"Exception \" << message;", sourceCode, StringComparison.Ordinal);
        Assert.Contains("} catch (const std::exception& exception) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("} catch (const Exception* exception) {", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RecordRuntimeFailureDiagnostics(\"Update\", frameIndex, \"std::exception\", exception.what());", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RecordRuntimeFailureDiagnostics(\"Draw\", frameIndex, \"std::exception\", exception.what());", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RecordRuntimeFailureDiagnostics(\"Update\", frameIndex, \"managed Exception*\", exception != nullptr ? exception->what() : \"Unknown managed runtime exception.\");", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RecordRuntimeFailureDiagnostics(\"Draw\", frameIndex, \"managed Exception*\", exception != nullptr ? exception->what() : \"Unknown managed runtime exception.\");", sourceCode, StringComparison.Ordinal);
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

    /// <summary>
    /// Verifies the Nintendo DS boot host logs resolver-side texture load diagnostics alongside text and scene load diagnostics after startup-scene failures.
    /// </summary>
    [Fact]
    public void Source_whenStartupSceneLoadFails_logsTextureResolverDiagnostics() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("<< \"TextureLoad stage=\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("<< sceneLoadService->get_LastTextureLoadStage()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("<< \" texture=\"", sourceCode, StringComparison.Ordinal);
        Assert.Contains("<< sceneLoadService->get_LastTextureRelativePath()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("<< \" TX=\"", sourceCode, StringComparison.Ordinal);
    }
}
