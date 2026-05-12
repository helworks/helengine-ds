#pragma once

#include <nds/ndstypes.h>

#include <string>

#include "platform/ds/NintendoDsStartupManifestReader.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
class Core;
class CoreInitializationOptions;
#endif

namespace helengine::ds {
    class NintendoDsInputBackend;
    class NintendoDsRenderManager2D;
    class NintendoDsRenderManager3D;

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

        /// Stores the main-engine background slot used for the top screen.
        int MainBackgroundId;

        /// Stores the sub-engine background slot used for the bottom screen.
        int SubBackgroundId;

        /// Stores the top-screen framebuffer pointer.
        u16* MainFrameBuffer;

        /// Stores the bottom-screen framebuffer pointer.
        u16* SubFrameBuffer;

        /// Stores the startup-manifest reader used to load packaged NitroFS colors.
        NintendoDsStartupManifestReader StartupManifestReader;

        /// Stores the last startup-manifest read status observed by the host.
        NintendoDsStartupManifestReader::Status StartupManifestStatus;

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
#endif

        /// Initializes the DS video mode, VRAM routing, and bitmap backgrounds.
        bool InitializeVideo();

        /// Paints the provided colors to the top and bottom display framebuffers.
        /// <param name="topScreenColor">Top-screen color to present.</param>
        /// <param name="bottomScreenColor">Bottom-screen color to present.</param>
        void PaintScreenColors(u16 topScreenColor, u16 bottomScreenColor);

        /// Attempts to load the packaged startup manifest and apply its colors when valid.
        void TryApplyStartupManifestColors();

        /// Keeps the bootstrap frame alive when runtime startup is disabled or unavailable.
        void RunIsolatedFrameLoop();

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
        /// Runs the generated-core startup checkpoints through startup-scene materialization.
        void RunCheckpointedStartup();

        /// Initializes the generated-core runtime with minimal Nintendo DS platform backends.
        void InitializeCore();

        /// Loads and materializes the packaged startup scene.
        void LoadStartupScene();

        /// Runs the generated-core update and draw loop after startup succeeds.
        void RunMainLoop();
#endif

        /// Shows one fatal error on-screen and halts the process for inspection.
        void ShowFatalErrorAndHalt(const std::string& message);
    };
}
