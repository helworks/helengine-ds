#pragma once

#include <nds/ndstypes.h>
#include <nds/arm9/console.h>

#include <string>

#include "platform/ds/NintendoDsStartupManifestReader.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
class Core;
class CoreInitializationOptions;
class PlatformInfo;
#endif

namespace helengine::ds {
    class NintendoDsInputBackend;
    class NintendoDsRenderManager2D;
    class NintendoDsRenderManager3D;
    class NintendoDsRuntimeDiagnosticsProvider;

    /// Owns the first Nintendo DS native video bootstrap and verification frame loop.
    class NintendoDsBootHost {
    public:
        /// Creates the Nintendo DS bootstrap host with no initialized background state.
        NintendoDsBootHost();

        /// Initializes both DS displays and presents the verification colors until shutdown.
        int Run();

    private:
        /// Stores the width of the 16-bit bitmap background used for each display.
        static constexpr int FrameBufferWidth = 256;

        /// Stores the height of the 16-bit bitmap background used for each display.
        static constexpr int FrameBufferHeight = 256;

        /// Stores the visible Nintendo DS screen width.
        static constexpr int ScreenWidth = 256;

        /// Stores the visible Nintendo DS screen height.
        static constexpr int ScreenHeight = 192;

        /// Stores the number of pixels written for each full-screen verification frame.
        static constexpr int FrameBufferPixelCount = FrameBufferWidth * FrameBufferHeight;

        /// Stores the bootstrap top-screen color with the bitmap visibility bit enabled.
        static constexpr u16 BootstrapTopScreenColor = 0x83E0;

        /// Stores the bootstrap bottom-screen color with the bitmap visibility bit enabled.
        static constexpr u16 BootstrapBottomScreenColor = 0xFFE0;

        /// Stores the checkpoint top-screen color used after manifest handling begins.
        static constexpr u16 CheckpointTopScreenColor = 0x83FF;

        /// Stores the checkpoint bottom-screen color used after manifest handling begins.
        static constexpr u16 CheckpointBottomScreenColor = 0xFC1F;

        /// Stores the main-engine background slot used for the top screen.
        int MainBackgroundId;

        /// Stores the sub-engine background slot used for the bottom screen.
        int SubBackgroundId;

        /// Stores the top-screen framebuffer pointer.
        u16* MainFrameBuffer;

        /// Stores the bottom-screen framebuffer pointer.
        u16* SubFrameBuffer;

        /// Stores the most recent top-screen checkpoint color so fatal handling can preserve the last visible stage.
        u16 LastCheckpointTopScreenColor;

        /// Stores the most recent bottom-screen checkpoint color so fatal handling can preserve the last visible stage when possible.
        u16 LastCheckpointBottomScreenColor;

        /// Stores the buffered startup log that is dumped to the bottom screen when fatal startup errors occur.
        std::string BootLog;

        /// Stores the startup-manifest reader used to load packaged NitroFS colors.
        NintendoDsStartupManifestReader StartupManifestReader;

        /// Stores the last startup-manifest read status observed by the host.
        NintendoDsStartupManifestReader::Status StartupManifestStatus;

        /// Stores whether the diagnostic status console has been initialized on the bottom screen.
        bool StatusConsoleInitialized;

        /// Stores the explicit sub-screen console used for runtime diagnostics.
        PrintConsole StatusConsole;

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
        /// Stores the generated runtime core instance.
        ::Core* EngineCore;

        /// Stores the generated runtime initialization options.
        ::CoreInitializationOptions* EngineOptions;

        /// Stores the minimal Nintendo DS 3D backend used to satisfy generated-core startup.
        NintendoDsRenderManager3D* EngineRenderManager3D;

        /// Stores the minimal Nintendo DS 2D backend used to satisfy generated-core startup.
        NintendoDsRenderManager2D* EngineRenderManager2D;

        /// Stores the Nintendo DS input backend used during runtime startup.
        NintendoDsInputBackend* EngineInputBackend;

        /// Stores the platform info instance injected into generated core.
        ::PlatformInfo* EnginePlatformInfo;

        /// Stores the Nintendo DS diagnostics provider injected into generated core.
        NintendoDsRuntimeDiagnosticsProvider* EngineRuntimeDiagnosticsProvider;

        /// Stores the last scene-manager trace stage emitted to the runtime diagnostics console.
        std::string LastEmittedSceneManagerStage;

        /// Stores the last scene-manager trace scene id emitted to the runtime diagnostics console.
        std::string LastEmittedSceneManagerSceneId;

        /// Stores the last loaded-scene count emitted to the runtime diagnostics console.
        int32_t LastEmittedSceneManagerLoadedCount;

        /// Stores the last pending-operation count emitted to the runtime diagnostics console.
        int32_t LastEmittedSceneManagerPendingCount;

        /// Stores the last non-empty runtime scene-load stage emitted to the diagnostics console.
        std::string LastEmittedSceneLoadStage;

        /// Stores the most recent runtime loop phase entered by the DS host so hard stalls can be identified without relying on exceptions.
        std::string LastObservedRuntimePhase;

        /// Stores the most recent DS host substage inside the main loop so stalls between update and draw can be localized.
        std::string LastBootHostStage;

        /// Stores the cumulative allocated-byte total observed at the previous runtime diagnostic refresh.
        std::size_t LastEmittedAllocatedByteTotal;

        /// Stores the cumulative freed-byte total observed at the previous runtime diagnostic refresh.
        std::size_t LastEmittedFreedByteTotal;

        /// Stores the allocator net-byte delta consumed by the previous diagnostics refresh.
        int32_t LastEmittedDiagnosticNetByteDelta;

#endif

        /// Initializes the DS video mode, VRAM routing, and bitmap backgrounds.
        bool InitializeVideo();

        /// Paints the provided colors to the top and bottom display framebuffers.
        /// <param name="topScreenColor">Top-screen color to present.</param>
        /// <param name="bottomScreenColor">Bottom-screen color to present.</param>
        void PaintScreenColors(u16 topScreenColor, u16 bottomScreenColor);

        /// Attempts to load the packaged startup manifest and apply its colors when valid.
        void TryApplyStartupManifestColors();

        /// Initializes the bottom-screen console used for live runtime diagnostics.
        void InitializeStatusConsole();

        /// Appends one startup log line to the buffered fatal-diagnostics output.
        /// <param name="message">Diagnostic message to append.</param>
        void AppendBootLog(const char* message);

        /// Emits one startup log line to both host traces and the buffered fatal-diagnostics log.
        /// <param name="message">Diagnostic message to record.</param>
        void RecordBootStatus(const char* message);

        /// Dumps the buffered startup log to the bottom-screen diagnostics console.
        void DumpBootLogToConsole();

        /// Paints one visible checkpoint pair so bootstrap progress remains observable even when text diagnostics are hidden.
        /// <param name="topScreenColor">Top-screen checkpoint color.</param>
        /// <param name="bottomScreenColor">Bottom-screen checkpoint color.</param>
        void PaintCheckpoint(u16 topScreenColor, u16 bottomScreenColor);

        /// Runs a bottom-screen console smoke test without initializing the generated engine runtime.
        void RunStatusConsoleSmokeTest();

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
        /// Runs the generated-core startup checkpoints through startup-scene materialization.
        void RunCheckpointedStartup();

        /// Initializes the generated-core runtime with minimal Nintendo DS platform backends.
        void InitializeCore();

        /// Loads and materializes the packaged startup scene.
        void LoadStartupScene();

        /// Resolves the runtime scene id that owns one cooked startup-scene asset path.
        /// <param name="cookedRelativePath">Cooked-relative startup-scene asset path from the runtime startup manifest.</param>
        /// <returns>Stable runtime scene id registered for that cooked scene path.</returns>
        std::string ResolveStartupSceneId(const std::string& cookedRelativePath) const;

        /// Emits one live allocation diagnostic snapshot to the bottom-screen console.
        /// <param name="frameIndex">Current runtime frame index.</param>
        /// <param name="accumulatedUpdateNetByteDelta">Net allocated bytes produced by update phases since the previous diagnostics refresh.</param>
        /// <param name="accumulatedDrawNetByteDelta">Net allocated bytes produced by draw phases since the previous diagnostics refresh.</param>
        void EmitSceneManagerDiagnostic(int32_t frameIndex, int32_t accumulatedUpdateNetByteDelta, int32_t accumulatedDrawNetByteDelta);

        /// Updates the always-visible version and allocation diagnostics.
        /// <param name="accumulatedUpdateNetByteDelta">Net allocated bytes produced by update phases since the previous diagnostics refresh.</param>
        /// <param name="accumulatedDrawNetByteDelta">Net allocated bytes produced by draw phases since the previous diagnostics refresh.</param>
        void UpdateLiveStageConsole(int32_t accumulatedUpdateNetByteDelta, int32_t accumulatedDrawNetByteDelta);

        /// Writes one padded diagnostics row to the bottom-screen console so shorter messages do not leave stale text behind.
        /// <param name="row">One-based console row to update.</param>
        /// <param name="text">Text that should replace the row contents.</param>
        void PrintStatusLine(int row, const char* text);

        /// Records one runtime failure snapshot before an update or draw exception escapes to the top-level fatal handler.
        /// <param name="phase">Runtime phase that failed.</param>
        /// <param name="frameIndex">Current runtime frame index.</param>
        /// <param name="exceptionKind">Exception category caught by the host boundary.</param>
        /// <param name="message">Exception message associated with the failure when available.</param>
        void RecordRuntimeFailureDiagnostics(const char* phase, int32_t frameIndex, const char* exceptionKind, const char* message);

        /// Runs the generated-core update and draw loop after startup succeeds.
        void RunMainLoop();
#endif

        /// Shows one fatal error on-screen and halts the process for inspection.
        void ShowFatalErrorAndHalt(const std::string& message);
    };
}
