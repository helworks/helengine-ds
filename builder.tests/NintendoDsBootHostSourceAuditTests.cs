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
        Assert.Contains("InitializeStatusConsole();", prepareBottomScreenBody, StringComparison.Ordinal);
        Assert.Contains("consoleClear();", prepareBottomScreenBody, StringComparison.Ordinal);
        Assert.DoesNotContain("videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);", prepareBottomScreenBody, StringComparison.Ordinal);
        Assert.DoesNotContain("videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT);", prepareBottomScreenBody, StringComparison.Ordinal);
        Assert.DoesNotContain("bgInitSub(", prepareBottomScreenBody, StringComparison.Ordinal);
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
}
