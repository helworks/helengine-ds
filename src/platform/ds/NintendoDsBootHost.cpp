#include "platform/ds/NintendoDsBootHost.hpp"

#include <algorithm>
#include <cstdio>
#include <exception>
#include <string>

extern "C" {
#include <nds/arm9/background.h>
#include <nds/arm9/console.h>
#include <nds/arm9/video.h>
#include <nds/interrupts.h>
#include <nds/system.h>
}

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "Core.hpp"
#include "CoreInitializationOptions.hpp"
#include "PlatformInfo.hpp"
#include "SceneAsset.hpp"
#include "platform/ds/NintendoDsInputBackend.hpp"
#include "platform/ds/NintendoDsPackagedAssetLoader.hpp"
#include "platform/ds/NintendoDsRenderManager2D.hpp"
#include "platform/ds/NintendoDsRenderManager3D.hpp"
#include "runtime/native_exceptions.hpp"
#endif

namespace helengine::ds {
    namespace {
        /// Uses the DS vertical blank cadence as the authoritative runtime frame step.
        constexpr double NintendoDsFrameDeltaSeconds = 1.0 / 60.0;

        /// Emits one runtime trace line to the DS emulator debug channel and host stdout.
        void EmitTrace(const char* message) {
            if (message == nullptr) {
                return;
            }

            std::fprintf(stderr, "%s\n", message);
            std::fflush(stderr);
            std::printf("%s\n", message);
            std::fflush(stdout);
        }
    }

    /// Creates the Nintendo DS bootstrap host with no initialized background state.
    NintendoDsBootHost::NintendoDsBootHost()
        : MainBackgroundId(-1)
        , SubBackgroundId(-1)
        , MainFrameBuffer(nullptr)
        , SubFrameBuffer(nullptr)
        , StartupManifestStatus(NintendoDsStartupManifestReader::Status::FileMissing)
        , StatusConsoleInitialized(false)
#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
        , EngineCore(nullptr)
        , EngineOptions(nullptr)
        , EngineRenderManager3D(nullptr)
        , EngineRenderManager2D(nullptr)
        , EngineInputBackend(nullptr)
        , EnginePlatformInfo(nullptr)
#endif
    {
    }

    /// Initializes both DS displays and presents the verification colors until shutdown.
    int NintendoDsBootHost::Run() {
#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
        try {
#endif
            powerOn(POWER_ALL);
            consoleDebugInit(DebugDevice_NOCASH);
            EmitTrace("[helengine-ds] boot host run begin");

            if (!InitializeVideo()) {
                EmitTrace("[helengine-ds] video initialization failed");
                return 1;
            }

            PaintScreenColors(BootstrapTopScreenColor, BootstrapBottomScreenColor);
            TryApplyStartupManifestColors();
            EmitTrace("[helengine-ds] bootstrap frame presented");
#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
            RunCheckpointedStartup();
#else
            RunIsolatedFrameLoop();
#endif
            return 1;
#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
        } catch (const std::exception& exception) {
            std::fprintf(stderr, "[helengine-ds] fatal exception: %s\n", exception.what());
            std::fflush(stderr);
            std::printf("[helengine-ds] fatal exception: %s\n", exception.what());
            std::fflush(stdout);
            ShowFatalErrorAndHalt(exception.what());
            return 1;
#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
        } catch (const Exception* exception) {
            const char* message = exception != nullptr ? exception->what() : "Unknown managed runtime exception.";
            std::fprintf(stderr, "[helengine-ds] fatal runtime exception: %s\n", message);
            std::fflush(stderr);
            std::printf("[helengine-ds] fatal runtime exception: %s\n", message);
            std::fflush(stdout);
            ShowFatalErrorAndHalt(message);
            delete exception;
            return 1;
#endif
        } catch (...) {
            EmitTrace("[helengine-ds] fatal unknown exception");
            ShowFatalErrorAndHalt("Unknown fatal exception.");
            return 1;
        }
#endif
    }

    /// Initializes the DS video mode, VRAM routing, and bitmap backgrounds.
    bool NintendoDsBootHost::InitializeVideo() {
        videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
        videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);

        vramSetBankA(VRAM_A_MAIN_BG);
        vramSetBankC(VRAM_C_SUB_BG);

        MainBackgroundId = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
        SubBackgroundId = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);

        if (MainBackgroundId < 0) {
            return false;
        } else if (SubBackgroundId < 0) {
            return false;
        }

        MainFrameBuffer = static_cast<u16*>(bgGetGfxPtr(MainBackgroundId));
        SubFrameBuffer = static_cast<u16*>(bgGetGfxPtr(SubBackgroundId));

        if (MainFrameBuffer == nullptr) {
            return false;
        } else if (SubFrameBuffer == nullptr) {
            return false;
        }

        return true;
    }

    /// Paints the provided colors to the top and bottom display framebuffers.
    /// <param name="topScreenColor">Top-screen color to present.</param>
    /// <param name="bottomScreenColor">Bottom-screen color to present.</param>
    void NintendoDsBootHost::PaintScreenColors(u16 topScreenColor, u16 bottomScreenColor) {
        std::fill_n(MainFrameBuffer, FrameBufferPixelCount, topScreenColor);
        std::fill_n(SubFrameBuffer, FrameBufferPixelCount, bottomScreenColor);
        swiWaitForVBlank();
    }

    /// Attempts to load the packaged startup manifest and apply its colors when valid.
    void NintendoDsBootHost::TryApplyStartupManifestColors() {
        NintendoDsStartupManifestReader::Result result = StartupManifestReader.Read();
        StartupManifestStatus = result.ReadStatus;
        if (result.ReadStatus != NintendoDsStartupManifestReader::Status::Success) {
            return;
        }

        PaintScreenColors(result.TopScreenColor, result.BottomScreenColor);
    }

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
    /// Runs the generated-core startup checkpoints through startup-scene materialization.
    void NintendoDsBootHost::RunCheckpointedStartup() {
        EmitTrace("[helengine-ds] generated core startup begin");
        InitializeCore();
        EmitTrace("[helengine-ds] generated core initialized");
        LoadStartupScene();
        EmitTrace("[helengine-ds] startup scene load finished");
        PrepareMainScreenFor3D();
        RunMainLoop();
    }

    /// Initializes the generated-core runtime with minimal Nintendo DS platform backends.
    void NintendoDsBootHost::InitializeCore() {
        EmitTrace("[helengine-ds] core initialization begin");
        EngineCore = new Core();
        EngineOptions = EngineCore->get_InitializationOptions();
        EngineOptions->set_ContentRootPath("nitro:");
        EngineOptions->set_UpdateOrderLayers(4);
        EngineOptions->set_RenderOrderLayers3D(4);
        EngineOptions->set_UpdateListInitialCapacity(64);
        EngineOptions->set_RenderList2DInitialCapacity(64);
        EngineOptions->set_RenderList3DInitialCapacity(64);

        EngineRenderManager3D = new NintendoDsRenderManager3D();
        EngineRenderManager2D = new NintendoDsRenderManager2D();
        EngineInputBackend = new NintendoDsInputBackend();
        EnginePlatformInfo = new PlatformInfo("nintendo-ds", "1");

        EngineRenderManager3D->AddWindow(0, ScreenWidth, ScreenHeight);
        EngineCore->Initialize(
            EngineRenderManager3D,
            EngineRenderManager2D,
            EngineInputBackend,
            EnginePlatformInfo,
            EngineOptions);
        EmitTrace("[helengine-ds] core initialization complete");
    }

    /// Loads and materializes the packaged startup scene.
    void NintendoDsBootHost::LoadStartupScene() {
        EmitTrace("[helengine-ds] startup scene lookup begin");
        NintendoDsPackagedAssetLoader packagedAssetLoader("nitro:");
        if (!packagedAssetLoader.StartupSceneExists()) {
            EmitTrace("[helengine-ds] startup scene asset is missing; continuing without scene load");
            return;
        }

        EmitTrace("[helengine-ds] startup scene asset found");
        SceneAsset* startupScene = packagedAssetLoader.LoadStartupScene();
        EmitTrace("[helengine-ds] startup scene asset deserialized");
        EngineCore->get_SceneLoadService()->Load(startupScene);
        EmitTrace("[helengine-ds] startup scene runtime load complete");
    }

    /// Transfers the top screen into Nintendo DS 3D mode once bootstrap loading is complete.
    void NintendoDsBootHost::PrepareMainScreenFor3D() {
        videoSetMode(MODE_0_3D);
    }

    /// Runs the generated-core update and draw loop after startup succeeds.
    void NintendoDsBootHost::RunMainLoop() {
        while (true) {
            EngineCore->Update(NintendoDsFrameDeltaSeconds);
            EngineCore->Draw();
            swiWaitForVBlank();
        }
    }
#endif

    /// Shows one fatal error on-screen and halts the process for inspection.
    void NintendoDsBootHost::ShowFatalErrorAndHalt(const std::string& message) {
        if (!StatusConsoleInitialized) {
            videoSetModeSub(MODE_0_2D);
            consoleDemoInit();
            StatusConsoleInitialized = true;
        }
        consoleClear();
        iprintf("helengine-ds fatal error\n\n");
        iprintf("%s\n", message.c_str());
        iprintf("\nThe app is halted for diagnostics.\n");

        while (true) {
            swiWaitForVBlank();
        }
    }
}
