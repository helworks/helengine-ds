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
    /// Verifies the Nintendo DS boot host distinguishes menu startup presentation from the default 3D startup path.
    /// </summary>
    [Fact]
    public void Source_whenCheckpointedStartupCompletes_usesConfiguredTopScreenPresentationPath() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("PrepareMainScreenForConfiguredStartupScene();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsBootHost::PrepareMainScreenForMenu2D()", sourceCode, StringComparison.Ordinal);
        Assert.Contains("void NintendoDsBootHost::PrepareMainScreenFor3D()", sourceCode, StringComparison.Ordinal);
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
    /// Verifies the Nintendo DS boot host can redirect menu profiling output to the native bottom-screen console without using engine-rendered diagnostics.
    /// </summary>
    [Fact]
    public void Source_whenProfilingDsMenu_rendersMenuOnTopAndWrites2dSnapshotToBottomConsole() {
        string repositoryRootPath = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "..", "..", "..", ".."));
        string sourcePath = Path.Combine(repositoryRootPath, "src", "platform", "ds", "NintendoDsBootHost.cpp");
        string sourceCode = File.ReadAllText(sourcePath);

        Assert.Contains("EngineRenderManager2D->SetBottomScreenPresentationEnabled(false);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("InitializeStatusConsole();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("NintendoDsRenderManager2DProfileSnapshot snapshot = EngineRenderManager2D->get_ProfileSnapshot();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("consoleSelect(&StatusConsole);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("consoleClear();", sourceCode, StringComparison.Ordinal);
        Assert.Contains("iprintf(\"2D total %.2f ms\\n\", snapshot.TotalFrameMilliseconds);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("iprintf(\"Text %.2f ms  %d\\n\", snapshot.TextMilliseconds, snapshot.TextPrimitiveCount);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("iprintf(\"Sprite %.2f ms  %d\\n\", snapshot.SpriteMilliseconds, snapshot.SpritePrimitiveCount);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("iprintf(\"Rect %.2f ms  %d\\n\", snapshot.RoundedRectMilliseconds, snapshot.RoundedRectPrimitiveCount);", sourceCode, StringComparison.Ordinal);
        Assert.Contains("EmitBottomConsoleProfileDiagnostic(frameIndex);", sourceCode, StringComparison.Ordinal);
    }
}
