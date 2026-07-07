namespace helengine.ds.builder.tests;

/// <summary>
/// Audits the Nintendo DS boot host source so startup reflects the hardware-only DS renderer policy.
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
    /// Verifies the Nintendo DS boot host publishes the DS platform label on the left and the formatted runtime version on the right through platform metadata.
    /// </summary>
    [Fact]
    public void Source_whenInitializingPlatformInfo_usesDsLabelAndFormattedVersionString() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("EnginePlatformInfo = new PlatformInfo(\"DS\", \"2.0\");", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("EnginePlatformInfo = new PlatformInfo(\"ds\", \"2\");", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host hands screen ownership to the render loop instead of hardcoding startup-scene presentation mode.
    /// </summary>
    [Fact]
    public void Source_whenCheckpointedStartupCompletes_doesNotOwnPermanentStartupScenePresentationMode() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);
        int prepareBottomScreenStart = sourceCode.IndexOf("void NintendoDsBootHost::PrepareBottomScreenForRuntimePresentation()", StringComparison.Ordinal);
        int paintCheckpointStart = sourceCode.IndexOf("void NintendoDsBootHost::PaintCheckpoint(", StringComparison.Ordinal);
        string prepareBottomScreenBody = sourceCode[prepareBottomScreenStart..paintCheckpointStart];

        Assert.DoesNotContain("PrepareMainScreenForConfiguredStartupScene();", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("void NintendoDsBootHost::PrepareMainScreenForMenu2D()", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("void NintendoDsBootHost::PrepareMainScreenFor3D()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("RunMainLoop();", sourceCode, StringComparison.Ordinal);
        Assert.True(prepareBottomScreenStart >= 0, "Expected runtime bottom-screen handoff function.");
        Assert.True(paintCheckpointStart > prepareBottomScreenStart, "Expected PaintCheckpoint after PrepareBottomScreenForRuntimePresentation.");
        Assert.DoesNotContain("InitializeStatusConsole();", prepareBottomScreenBody, StringComparison.Ordinal);
        Assert.DoesNotContain("consoleClear();", prepareBottomScreenBody, StringComparison.Ordinal);
        Assert.DoesNotContain("videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);", prepareBottomScreenBody, StringComparison.Ordinal);
        Assert.DoesNotContain("videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT);", prepareBottomScreenBody, StringComparison.Ordinal);
        Assert.Contains("videoSetModeSub(MODE_0_2D);", prepareBottomScreenBody, StringComparison.Ordinal);
        Assert.Contains("vramSetBankC(VRAM_C_SUB_BG);", prepareBottomScreenBody, StringComparison.Ordinal);
        Assert.Contains("PaintBottomScreenBg0ProofTile();", prepareBottomScreenBody, StringComparison.Ordinal);
        Assert.Contains("bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 31, 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("BG_PALETTE_SUB[1] = RGB15(31, 0, 0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("std::memset(tilePixels, 0x11, 32);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host no longer prewarms software text caches that were removed with the DS hardware-only renderer policy.
    /// </summary>
    [Fact]
    public void Source_whenStartupSceneLoads_doesNotPrewarmRemovedSoftwareTextCaches() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("PrewarmEntityTreeTextCaches", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("PrewarmLoadedSceneTextCaches", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("PrewarmTextDrawable(", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host no longer emits periodic runtime allocation telemetry lines during the main loop.
    /// </summary>
    [Fact]
    public void Source_whenRuntimeMainLoopRuns_doesNotEmitPeriodicAllocationTelemetryTrace() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.DoesNotContain("RuntimeAllocationTelemetryFrameInterval", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("watchLive=", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("watchPeak=", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("alloc=%lu peak=%lu", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host publishes the runtime heartbeat every frame so hardware text retention uses real frame cadence.
    /// </summary>
    [Fact]
    public void Source_whenRuntimeMainLoopRuns_updatesRuntimeHeartbeatEveryFrame() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("UpdateRuntimeHeartbeat(frameIndex);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("RuntimeHeartbeatFrameInterval", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("if ((frameIndex % RuntimeHeartbeatFrameInterval) == 0)", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host uses a coarse DS-specific physics cadence to trade simulation fidelity for runtime frame budget.
    /// </summary>
    [Fact]
    public void Source_whenInitializingCore_usesCoarseDsSpecificPhysicsCadence() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("EngineOptions->set_PhysicsFixedStepSeconds(1.0 / 12.0);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EngineOptions->set_PhysicsMaxStepsPerUpdate(1);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("EngineOptions->set_PhysicsFixedStepSeconds(1.0 / 20.0);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host explicitly enables platform-owned FPS overlay presentation so DS diagnostics render through the hardware-owned text path.
    /// </summary>
    [Fact]
    public void Source_whenInitializingCore_enablesPlatformOwnedPerformanceOverlayPresentation() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("EngineCore->SetPlatformOwnedPerformanceOverlayPresentation(true);", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host passes the initialized core into the generated BEPU runtime factory so platform-specific runtime choices stay available during native builds.
    /// </summary>
    [Fact]
    public void Source_whenInitializingGeneratedBepuRuntime_passesCoreIntoRuntimeWorldFactory() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("BepuRuntimeComponentRegistration::CreateRuntimeWorld(EngineCore);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("BepuRuntimeComponentRegistration::CreateRuntimeWorld();", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies startup-scene failure diagnostics use fixed buffers instead of stringstream-heavy formatting helpers so the DS boot host avoids pulling extra iostream code into the ROM.
    /// </summary>
    [Fact]
    public void Source_whenFormattingStartupSceneFailureDiagnostics_usesFixedBuffersInsteadOfOstringstream() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);
        int loadStartupSceneStart = sourceCode.IndexOf("void NintendoDsBootHost::LoadStartupScene()", StringComparison.Ordinal);
        int resolveStartupSceneIdStart = sourceCode.IndexOf("std::string NintendoDsBootHost::ResolveStartupSceneId(", StringComparison.Ordinal);
        string loadStartupSceneBody = sourceCode[loadStartupSceneStart..resolveStartupSceneIdStart];

        Assert.True(loadStartupSceneStart >= 0, "Expected startup-scene loader implementation.");
        Assert.True(resolveStartupSceneIdStart > loadStartupSceneStart, "Expected startup-scene id resolver after the loader implementation.");
        Assert.DoesNotContain("std::ostringstream", loadStartupSceneBody, StringComparison.Ordinal);
        Assert.Contains("std::array<char, 160>", loadStartupSceneBody, StringComparison.Ordinal);
        Assert.Contains("std::snprintf(", loadStartupSceneBody, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies native DS host tracing and status-console bootstrap logic are guarded behind the runtime diagnostics compile-time flag so release-oriented builds can trim that footprint.
    /// </summary>
    [Fact]
    public void Source_whenRuntimeDiagnosticsAreOptional_guardsNativeTracingAndStatusConsoleSetupBehindCompileTimeFlag() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string bootHostSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string runtimeDiagnosticsSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRuntimeDiagnosticsProvider.cpp");
        string inputBackendSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsInputBackend.cpp");
        string renderManager2DSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string renderManager3DSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager3D.cpp");
        string bootHostSource = File.ReadAllText(bootHostSourcePath);
        string runtimeDiagnosticsSource = File.ReadAllText(runtimeDiagnosticsSourcePath);
        string inputBackendSource = File.ReadAllText(inputBackendSourcePath);
        string renderManager2DSource = File.ReadAllText(renderManager2DSourcePath);
        string renderManager3DSource = File.ReadAllText(renderManager3DSourcePath);

        Assert.Contains("#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS", runtimeDiagnosticsSource, StringComparison.Ordinal);
        Assert.Contains("#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS", inputBackendSource, StringComparison.Ordinal);
        Assert.Contains("#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS", renderManager2DSource, StringComparison.Ordinal);
        Assert.Contains("#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS", renderManager3DSource, StringComparison.Ordinal);
        Assert.Contains("consoleDebugInit(DebugDevice_NOCASH);", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("InitializeStatusConsole();", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("ShowFatalErrorAndHalt(", bootHostSource, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host no longer emits the temporary touch and interaction probe state that was used during DS input debugging.
    /// </summary>
    [Fact]
    public void Source_whenRuntimeMainLoopRuns_doesNotEmitTemporaryTouchInteractionProbeState() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string bootHostHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.hpp");
        string bootHostSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string rendererHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string rendererSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string bootHostHeader = File.ReadAllText(bootHostHeaderPath);
        string bootHostSource = File.ReadAllText(bootHostSourcePath);
        string rendererHeader = File.ReadAllText(rendererHeaderPath);
        string rendererSource = File.ReadAllText(rendererSourcePath);
        int heartbeatStart = bootHostSource.IndexOf("void NintendoDsBootHost::UpdateRuntimeHeartbeat(int32_t frameIndex)", StringComparison.Ordinal);
        int heartbeatEnd = bootHostSource.IndexOf("void NintendoDsBootHost::RecordRuntimeFailureDiagnostics(", StringComparison.Ordinal);
        string heartbeatBody = bootHostSource[heartbeatStart..heartbeatEnd];

        Assert.Contains("void UpdateRuntimeHeartbeat(int32_t frameIndex);", bootHostHeader, StringComparison.Ordinal);
        Assert.Contains("UpdateRuntimeHeartbeat(frameIndex);", bootHostSource, StringComparison.Ordinal);
        Assert.True(heartbeatStart >= 0, "Expected runtime heartbeat implementation.");
        Assert.True(heartbeatEnd > heartbeatStart, "Expected runtime failure diagnostics after the heartbeat implementation.");
        Assert.DoesNotContain("scanKeys();", bootHostSource, StringComparison.Ordinal);
        Assert.DoesNotContain("uint32_t heldKeys = keysHeld();", bootHostSource, StringComparison.Ordinal);
        Assert.DoesNotContain("bool touchIsDown = (heldKeys & (1u << 14)) != 0;", bootHostSource, StringComparison.Ordinal);
        Assert.DoesNotContain("void SetTouchProbeActive(bool active);", rendererHeader, StringComparison.Ordinal);
        Assert.DoesNotContain("void SetInteractionProbeText(const std::string& text);", rendererHeader, StringComparison.Ordinal);
        Assert.DoesNotContain("LastInteractionEdgeProbe", bootHostHeader, StringComparison.Ordinal);
        Assert.DoesNotContain("PointerInteractionSystem", heartbeatBody, StringComparison.Ordinal);
        Assert.DoesNotContain("EngineRenderManager2D->SetTouchProbeActive", heartbeatBody, StringComparison.Ordinal);
        Assert.DoesNotContain("EngineRenderManager2D->SetInteractionProbeText", heartbeatBody, StringComparison.Ordinal);
        Assert.DoesNotContain("WasMouseLeftButtonPressed()", heartbeatBody, StringComparison.Ordinal);
        Assert.DoesNotContain("WasMouseLeftButtonReleased()", heartbeatBody, StringComparison.Ordinal);
        Assert.DoesNotContain("GetMouseLeftButtonState()", heartbeatBody, StringComparison.Ordinal);
        Assert.DoesNotContain("get_LastTraceStage()", heartbeatBody, StringComparison.Ordinal);
        Assert.DoesNotContain("get_LastTracePendingOperationCount()", heartbeatBody, StringComparison.Ordinal);
        Assert.DoesNotContain("void NintendoDsRenderManager2D::SetTouchProbeActive(bool active)", rendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("void NintendoDsRenderManager2D::SetInteractionProbeText(const std::string& text)", rendererSource, StringComparison.Ordinal);
        Assert.DoesNotContain("InteractionProbeText.empty()", rendererSource, StringComparison.Ordinal);
    }
}
