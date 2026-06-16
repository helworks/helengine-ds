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

        Assert.Contains("UpdateRuntimeHeartbeat(frameIndex, touchIsDown);", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("RuntimeHeartbeatFrameInterval", sourceCode, StringComparison.Ordinal);
        Assert.DoesNotContain("if ((frameIndex % RuntimeHeartbeatFrameInterval) == 0)", sourceCode, StringComparison.Ordinal);
    }

    /// <summary>
    /// Verifies the Nintendo DS boot host forwards raw touch state into the runtime heartbeat path so one visible bottom-screen probe can confirm whether stylus input reaches the native backend.
    /// </summary>
    [Fact]
    public void Source_whenRuntimeMainLoopRuns_forwardsRawTouchStateIntoHeartbeatProbe() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string bootHostHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.hpp");
        string bootHostSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string rendererHeaderPath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.hpp");
        string rendererSourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsRenderManager2D.cpp");
        string bootHostHeader = File.ReadAllText(bootHostHeaderPath);
        string bootHostSource = File.ReadAllText(bootHostSourcePath);
        string rendererHeader = File.ReadAllText(rendererHeaderPath);
        string rendererSource = File.ReadAllText(rendererSourcePath);

        Assert.Contains("void UpdateRuntimeHeartbeat(int32_t frameIndex, bool touchIsDown);", bootHostHeader, StringComparison.Ordinal);
        Assert.Contains("scanKeys();", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("uint32_t heldKeys = keysHeld();", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("bool touchIsDown = (heldKeys & (1u << 14)) != 0;", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("UpdateRuntimeHeartbeat(frameIndex, touchIsDown);", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("void SetTouchProbeActive(bool active);", rendererHeader, StringComparison.Ordinal);
        Assert.Contains("void SetInteractionProbeText(const std::string& text);", rendererHeader, StringComparison.Ordinal);
        Assert.Contains("EngineRenderManager2D->SetTouchProbeActive(touchIsDown);", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("EngineRenderManager2D->SetInteractionProbeText(interactionProbeText);", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("inputSystem->GetMouseLeftButtonState() == ::ButtonState::Pressed", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("pointerInteractionSystem->get_Hovering() != nullptr", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("pointerInteractionSystem->get_Highlighted() != nullptr", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("::SceneManager* sceneManager = EngineCore != nullptr ? EngineCore->get_SceneManager() : nullptr;", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("const std::string& lastTraceStage = sceneManager->get_LastTraceStage();", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("lastTraceStage == \"LoadSceneRequest\"", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("lastTraceStage == \"LoadSceneDeferred\"", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("sceneManager->get_LastTracePendingOperationCount() > 0", bootHostSource, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsRenderManager2D::SetTouchProbeActive(bool active)", rendererSource, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsRenderManager2D::SetInteractionProbeText(const std::string& text)", rendererSource, StringComparison.Ordinal);
        Assert.Contains("InteractionProbeText.empty()", rendererSource, StringComparison.Ordinal);
        Assert.Contains("WriteBottomScreenTextLine(", rendererSource, StringComparison.Ordinal);
    }
}
