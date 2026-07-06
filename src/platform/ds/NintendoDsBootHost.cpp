#include "platform/ds/NintendoDsBootHost.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <exception>
#include <new>
#include <sstream>
#include <string>

extern "C" {
#include <nds/arm9/background.h>
#include <nds/arm9/console.h>
#if __has_include(<nds/debug.h>)
#include <nds/debug.h>
#define HELENGINE_NINTENDO_DS_HAS_NOCASH_TRACE 1
#else
#define HELENGINE_NINTENDO_DS_HAS_NOCASH_TRACE 0
#endif
#include <nds/arm9/video.h>
#include <nds/interrupts.h>
#include <nds/system.h>
}

#if __has_include("BepuPhysicsWorld3D.hpp") && __has_include("BepuPhysicsWorld3DDiagnostics.hpp") && __has_include("BepuRuntimeComponentRegistration.hpp")
#define HELENGINE_NINTENDO_DS_HAS_BEPU_GENERATED_RUNTIME 1
#else
#define HELENGINE_NINTENDO_DS_HAS_BEPU_GENERATED_RUNTIME 0
#endif

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "Component.hpp"
#include "Core.hpp"
#include "CoreInitializationOptions.hpp"
#include "Entity.hpp"
#include "InputGamepadButton.hpp"
#include "LoadedSceneRecord.hpp"
#include "ObjectManager.hpp"
#include "PlatformInfo.hpp"
#include "RuntimeExecutionPhaseProbe.hpp"
#include "SceneAsset.hpp"
#include "SceneLoadMode.hpp"
#include "SceneManager.hpp"
#include "platform/ds/NintendoDsAllocationDiagnostics.hpp"
#include "platform/ds/NintendoDsFramePacing.hpp"
#include "platform/ds/NintendoDsInputBackend.hpp"
#include "platform/ds/NintendoDsPackagedAssetLoader.hpp"
#include "platform/ds/NintendoDsRenderManager2D.hpp"
#include "platform/ds/NintendoDsRenderManager3D.hpp"
#include "platform/ds/NintendoDsRuntimeDiagnosticsProvider.hpp"
#if HELENGINE_NINTENDO_DS_HAS_BEPU_GENERATED_RUNTIME
#include "BepuPhysicsWorld3D.hpp"
#include "BepuPhysicsWorld3DDiagnostics.hpp"
#include "BepuRuntimeComponentRegistration.hpp"
#endif
#include "RuntimeSceneLoadService.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "RuntimeSceneCatalogEntry.hpp"
#include "StandardPlatformAction.hpp"
#include "StandardPlatformActionBinding.hpp"
#include "StandardPlatformInputConfiguration.hpp"
#include "runtime/runtime_startup_manifest.hpp"
#include "runtime/runtime_scene_catalog_manifest.hpp"
#include "runtime/runtime_standard_platform_input_manifest.hpp"
#include "runtime/native_exceptions.hpp"
#endif

namespace helengine::ds {
    namespace {
        /// Uses the DS vertical blank cadence as the authoritative runtime frame step.
        constexpr double NintendoDsFrameDeltaSeconds = 1.0 / 60.0;

        /// Keeps the bottom-screen status console alive during runtime diagnostics so draw/update progress remains visible on hardware.
        constexpr bool KeepStatusConsoleDuringRuntimeDiagnostics = false;

        /// Counts visible DS VBlanks so the runtime can derive real elapsed frame time instead of reporting a synthetic constant rate.
        volatile uint32_t VBlankCount = 0;

        /// Stores the host-visible boot trace file path used by emulator launches.
        constexpr const char* BootTraceLogPath = "C:/tmp/helengine-ds-logs/helengine-ds-boot.log";

        /// Stores whether the current process has already reset the host-visible boot trace file.
        bool BootTraceLogReset = false;

        /// Stores the shared boot-status prefix trimmed from live bottom-screen diagnostics to preserve horizontal space.
        constexpr const char* BootTracePrefix = "[helengine-ds] ";

        /// Increments the visible-screen VBlank counter each time the DS display presents one hardware frame.
        void HandleVBlankInterrupt() {
            VBlankCount++;
        }

        /// Appends one trace line to the host-visible boot log file when the emulator host filesystem is available.
        /// <param name="message">Trace message to append.</param>
        void AppendBootTraceFileLine(const char* message) {
            if (message == nullptr || message[0] == '\0') {
                return;
            }

            FILE* file = std::fopen(BootTraceLogPath, BootTraceLogReset ? "ab" : "wb");
            if (file == nullptr) {
                return;
            }

            BootTraceLogReset = true;
            std::fprintf(file, "%s\n", message);
            std::fflush(file);
            std::fclose(file);
        }

        /// Emits one runtime trace line to the DS emulator debug channel and host stdout.
        void EmitTrace(const char* message) {
            if (message == nullptr) {
                return;
            }

            AppendBootTraceFileLine(message);
#if HELENGINE_NINTENDO_DS_HAS_NOCASH_TRACE
            nocashMessage(message);
#endif
            std::fprintf(stderr, "%s\n", message);
            std::fflush(stderr);
            std::printf("%s\n", message);
            std::fflush(stdout);
        }

        /// Builds one runtime scene catalog from the generated native scene-manifest entries.
        ::RuntimeSceneCatalog* BuildRuntimeSceneCatalog() {
            std::size_t sceneCount = 0;
            const HERuntimeSceneCatalogEntry* sceneEntries = he_runtime_scene_catalog_entries(&sceneCount);
            if (sceneEntries == nullptr || sceneCount == 0) {
                return nullptr;
            }

            Array<::RuntimeSceneCatalogEntry*>* catalogEntries = new Array<::RuntimeSceneCatalogEntry*>(static_cast<int32_t>(sceneCount));
            for (std::size_t index = 0; index < sceneCount; index++) {
                const HERuntimeSceneCatalogEntry& sourceEntry = sceneEntries[index];
                (*catalogEntries)[static_cast<int32_t>(index)] = new ::RuntimeSceneCatalogEntry(sourceEntry.SceneId, sourceEntry.CookedRelativePath);
            }

            return new ::RuntimeSceneCatalog(catalogEntries);
        }

        /// Builds one runtime standard-platform-input configuration from the generated manifest entries.
        ::StandardPlatformInputConfiguration* BuildStandardPlatformInputConfiguration() {
            std::size_t entryCount = 0;
            const HERuntimeStandardPlatformActionEntry* actionEntries = he_runtime_standard_platform_action_entries(&entryCount);
            List<::StandardPlatformActionBinding*>* bindings = new List<::StandardPlatformActionBinding*>();
            for (std::size_t index = 0; index < entryCount; index++) {
                const HERuntimeStandardPlatformActionEntry& sourceEntry = actionEntries[index];
                ::InputControlId controlId(
                    static_cast<::InputDeviceKind>(sourceEntry.DeviceKind),
                    static_cast<::InputControlKind>(sourceEntry.ControlKind),
                    sourceEntry.DeviceIndex,
                    sourceEntry.ControlIndex);
                bindings->Add(new ::StandardPlatformActionBinding(
                    static_cast<::StandardPlatformAction>(sourceEntry.ActionId),
                    controlId));
            }

            return new ::StandardPlatformInputConfiguration(bindings);
        }

    }

    /// Returns the visible-screen VBlank count maintained by the IRQ handler.
    uint32_t GetNintendoDsVBlankCount() {
        return VBlankCount;
    }

    /// Creates the Nintendo DS bootstrap host with no initialized background state.
    NintendoDsBootHost::NintendoDsBootHost()
        : MainBackgroundId(-1)
        , SubBackgroundId(-1)
        , MainFrameBuffer(nullptr)
        , SubFrameBuffer(nullptr)
        , LastCheckpointTopScreenColor(BootstrapTopScreenColor)
        , LastCheckpointBottomScreenColor(BootstrapBottomScreenColor)
        , StartupManifestStatus(NintendoDsStartupManifestReader::Status::FileMissing)
        , StatusConsoleInitialized(false)
        , StatusConsoleLogRow(0)
        , RuntimeDiagnosticsConsoleActive(false)
        , StartupSceneManagerStageSnapshot()
        , StartupSceneManagerSceneIdSnapshot()
        , StartupSceneManagerLoadedCountSnapshot(-1)
        , StartupSceneManagerPendingCountSnapshot(-1)
        , StatusConsole()
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

            if (!InitializeVideo()) {
                RecordBootStatus("[helengine-ds] video initialization failed");
                return 1;
            }

            InitializeStatusConsole();
            RecordBootStatus("[helengine-ds] boot host run begin");
            RecordBootStatus("[helengine-ds] video initialization complete");
            PaintScreenColors(BootstrapTopScreenColor, BootstrapBottomScreenColor);
            TryApplyStartupManifestColors();
            RecordBootStatus("[helengine-ds] bootstrap frame presented");
#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
            RunCheckpointedStartup();
#else
            RunStatusConsoleSmokeTest();
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
        irqSet(IRQ_VBLANK, HandleVBlankInterrupt);
        irqEnable(IRQ_VBLANK);

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
        LastCheckpointTopScreenColor = topScreenColor;
        LastCheckpointBottomScreenColor = bottomScreenColor;
        std::fill_n(MainFrameBuffer, FrameBufferPixelCount, topScreenColor);
        if (!StatusConsoleInitialized && SubFrameBuffer != nullptr) {
            std::fill_n(SubFrameBuffer, FrameBufferPixelCount, bottomScreenColor);
        }
        swiWaitForVBlank();
    }

    /// Paints one small top-screen diagnostic marker without overwriting the whole rendered scene.
    /// <param name="topScreenColor">Marker color to present.</param>
    void NintendoDsBootHost::PaintTopScreenMarker(u16 topScreenColor) {
        if (MainFrameBuffer == nullptr) {
            return;
        }

        LastCheckpointTopScreenColor = topScreenColor;
        constexpr int32_t MarkerSize = 12;
        for (int32_t row = 0; row < MarkerSize; row++) {
            for (int32_t column = 0; column < MarkerSize; column++) {
                MainFrameBuffer[(row * FrameBufferWidth) + column] = topScreenColor;
            }
        }
    }

    /// Attempts to load the packaged startup manifest and apply its colors when valid.
    void NintendoDsBootHost::TryApplyStartupManifestColors() {
        NintendoDsStartupManifestReader::Result result = StartupManifestReader.Read();
        StartupManifestStatus = result.ReadStatus;
        if (result.ReadStatus != NintendoDsStartupManifestReader::Status::Success) {
            RecordBootStatus("[helengine-ds] startup manifest unavailable");
            return;
        }

        RecordBootStatus("[helengine-ds] startup manifest applied");
        PaintScreenColors(result.TopScreenColor, result.BottomScreenColor);
    }

    /// Initializes the bottom-screen console used for live runtime diagnostics.
    void NintendoDsBootHost::InitializeStatusConsole() {
        if (StatusConsoleInitialized) {
            return;
        }

        videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);
        vramSetBankC(VRAM_C_SUB_BG);
        consoleInit(&StatusConsole, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
        consoleSelect(&StatusConsole);
        consoleClear();
        StatusConsoleInitialized = true;
        StatusConsoleLogRow = 0;
    }

    /// Appends one startup log line to the buffered fatal-diagnostics output.
    /// <param name="message">Diagnostic message to append.</param>
    void NintendoDsBootHost::AppendBootLog(const char* message) {
        if (message == nullptr || message[0] == '\0') {
            return;
        }

        if (!BootLog.empty()) {
            BootLog += "\n";
        }

        BootLog += message;
    }

    /// Emits one startup log line to both host traces and the buffered fatal-diagnostics log.
    /// <param name="message">Diagnostic message to record.</param>
    void NintendoDsBootHost::RecordBootStatus(const char* message) {
        AppendBootLog(message);
        EmitTrace(message);
        WriteBootStatusToConsole(message);
    }

    /// Writes one live startup status line to the bottom-screen diagnostics console while boot is still in progress.
    /// <param name="message">Diagnostic message to present.</param>
    void NintendoDsBootHost::WriteBootStatusToConsole(const char* message) {
        if (!StatusConsoleInitialized || RuntimeDiagnosticsConsoleActive || message == nullptr || message[0] == '\0') {
            return;
        }

        if (StatusConsoleLogRow >= 22) {
            consoleSelect(&StatusConsole);
            consoleClear();
            StatusConsoleLogRow = 0;
        }

        const char* visibleMessage = message;
        std::size_t prefixLength = std::strlen(BootTracePrefix);
        if (std::strncmp(message, BootTracePrefix, prefixLength) == 0) {
            visibleMessage = message + prefixLength;
        }

        consoleSelect(&StatusConsole);
        iprintf("%.31s\n", visibleMessage);
        StatusConsoleLogRow++;
    }

    /// Dumps the buffered startup log to the bottom-screen diagnostics console.
    void NintendoDsBootHost::DumpBootLogToConsole() {
        if (BootLog.empty()) {
            iprintf("(no boot log)\n");
            return;
        }

        iprintf("%s\n", BootLog.c_str());
    }

    /// Clears the bottom screen back to a blank hardware text background before the runtime main loop begins.
    void NintendoDsBootHost::PrepareBottomScreenForRuntimePresentation() {
        videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);
        vramSetBankC(VRAM_C_SUB_BG);
        PaintBottomScreenBg0ProofTile();
        SubBackgroundId = -1;
        SubFrameBuffer = nullptr;
        StatusConsoleInitialized = false;
        std::memset(&StatusConsole, 0, sizeof(StatusConsole));
        for (int32_t frameIndex = 0; frameIndex < 90; frameIndex++) {
            swiWaitForVBlank();
        }
    }

    /// Paints one centered red 8x8 BG0 tile on the bottom screen before runtime takes ownership.
    void NintendoDsBootHost::PaintBottomScreenBg0ProofTile() {
        constexpr int32_t DiagnosticTileIndex = 255;
        constexpr int32_t DiagnosticRow = 12;
        constexpr int32_t DiagnosticColumn = 16;
        int backgroundId = bgInitSub(0, BgType_Text4bpp, BgSize_T_256x256, 31, 0);
        if (backgroundId < 0) {
            return;
        }

        uint8_t* backgroundGraphics = reinterpret_cast<uint8_t*>(bgGetGfxPtr(backgroundId));
        uint16_t* mapEntries = static_cast<uint16_t*>(bgGetMapPtr(backgroundId));
        if (backgroundGraphics == nullptr || mapEntries == nullptr) {
            return;
        }

        std::memset(mapEntries, 0, 32 * 32 * sizeof(uint16_t));
        BG_PALETTE_SUB[0] = RGB15(0, 0, 0);
        BG_PALETTE_SUB[1] = RGB15(31, 0, 0);
        uint8_t* tilePixels = backgroundGraphics + (static_cast<std::size_t>(DiagnosticTileIndex) * 32);
        std::memset(tilePixels, 0x11, 32);
        int32_t mapIndex = (DiagnosticRow * 32) + DiagnosticColumn;
        mapEntries[mapIndex] = DiagnosticTileIndex;
    }

    /// Paints one visible checkpoint pair so bootstrap progress remains observable even when text diagnostics are hidden.
    /// <param name="topScreenColor">Top-screen checkpoint color.</param>
    /// <param name="bottomScreenColor">Bottom-screen checkpoint color.</param>
    void NintendoDsBootHost::PaintCheckpoint(u16 topScreenColor, u16 bottomScreenColor) {
        PaintScreenColors(topScreenColor, bottomScreenColor);
    }

    /// Runs a bottom-screen console smoke test without initializing the generated engine runtime.
    void NintendoDsBootHost::RunStatusConsoleSmokeTest() {
        RecordBootStatus("[helengine-ds] status console smoke test begin");
        InitializeStatusConsole();
        consoleSelect(&StatusConsole);
        consoleClear();
        iprintf("helengine-ds\n");
        iprintf("status console smoke test\n\n");
        iprintf("If you can read this,\n");
        iprintf("the bottom screen console works.\n\n");
        iprintf("Manifest status: %d\n", static_cast<int>(StartupManifestStatus));
        iprintf("Core init: starting\n");
#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
        RecordBootStatus("[helengine-ds] core init smoke test begin");
        InitializeCore();
        RecordBootStatus("[helengine-ds] core init smoke test complete");
        iprintf("Core init: complete\n");
        RecordBootStatus("[helengine-ds] startup scene existence check begin");
        NintendoDsPackagedAssetLoader packagedAssetLoader("nitro:");
        bool startupSceneExists = packagedAssetLoader.StartupSceneExists();
        if (startupSceneExists) {
            RecordBootStatus("[helengine-ds] startup scene exists");
            iprintf("Startup scene: exists\n");
            try {
                if (EngineRenderManager3D != nullptr) {
                    EngineRenderManager3D->ResetLastBuildDiagnostics();
                }

                LoadStartupScene();
                RecordBootStatus("[helengine-ds] startup scene runtime load complete");
                iprintf("Startup scene: loaded\n");
            } catch (...) {
                throw;
            }
        } else {
            RecordBootStatus("[helengine-ds] startup scene missing");
            iprintf("Startup scene: missing\n");
        }
#else
        iprintf("Core init: unavailable\n");
#endif
        iprintf("Waiting in VBlank loop.\n");
        RecordBootStatus("[helengine-ds] status console smoke test ready");

        while (true) {
            swiWaitForVBlank();
        }
    }

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
    /// Runs the generated-core startup checkpoints through startup-scene materialization.
    void NintendoDsBootHost::RunCheckpointedStartup() {
        RecordBootStatus("[helengine-ds] generated core startup begin");
        InitializeStatusConsole();
        consoleSelect(&StatusConsole);
        consoleClear();
        PrintStatusLine(0, "helengine-ds");
        PrintStatusLine(1, "checkpoint startup");
        PrintStatusLine(3, "Stage: core init");
        PaintCheckpoint(RGB15(0, 31, 0) | BIT(15), RGB15(0, 31, 0) | BIT(15));
        InitializeCore();
        RecordBootStatus("[helengine-ds] generated core initialized");
        PrintStatusLine(3, "Stage: scene load");
        PaintCheckpoint(RGB15(31, 31, 0) | BIT(15), RGB15(31, 31, 0) | BIT(15));
        LoadStartupScene();
        RecordBootStatus("[helengine-ds] startup scene load finished");
        PrintStatusLine(3, "Stage: main loop");
        PrintStatusLine(4, "Scene load: complete");
        PaintCheckpoint(RGB15(0, 31, 31) | BIT(15), RGB15(0, 31, 31) | BIT(15));
        RecordBootStatus("[helengine-ds] entering main loop");
        if (KeepStatusConsoleDuringRuntimeDiagnostics) {
            if (EngineRenderManager2D != nullptr) {
                EngineRenderManager2D->SetBottomScreenPresentationEnabled(false);
            }

            consoleSelect(&StatusConsole);
            consoleClear();
            RuntimeDiagnosticsConsoleActive = true;
            PrintStatusLine(0, "helengine-ds runtime");
            std::array<char, 32> startupStageLine {};
            std::snprintf(
                startupStageLine.data(),
                startupStageLine.size(),
                "SS:%ld/%ld %.18s",
                static_cast<long>(StartupSceneManagerLoadedCountSnapshot),
                static_cast<long>(StartupSceneManagerPendingCountSnapshot),
                StartupSceneManagerStageSnapshot.c_str());
            PrintStatusLine(1, startupStageLine.data());
            std::array<char, 32> startupIdLine {};
            std::string startupSceneSnapshotTail = StartupSceneManagerSceneIdSnapshot;
            if (startupSceneSnapshotTail.size() > 28) {
                startupSceneSnapshotTail = startupSceneSnapshotTail.substr(startupSceneSnapshotTail.size() - 28);
            }
            std::snprintf(
                startupIdLine.data(),
                startupIdLine.size(),
                "ST:%.28s",
                startupSceneSnapshotTail.c_str());
            PrintStatusLine(2, startupIdLine.data());
            PrintStatusLine(3, "F: 0");
            PrintStatusLine(4, "U: .");
            PrintStatusLine(5, "D: .");
            PrintStatusLine(6, "3: .");
            RecordBootStatus("[helengine-ds] runtime diagnostics console retained");
        } else {
            PrepareBottomScreenForRuntimePresentation();
        }
        RunMainLoop();
    }

    /// Initializes the generated-core runtime with minimal Nintendo DS platform backends.
    void NintendoDsBootHost::InitializeCore() {
        RecordBootStatus("[helengine-ds] core initialization begin");
        PrintStatusLine(4, "Core: new Core");
        EngineCore = new Core();
        RecordBootStatus("[helengine-ds] core initialization core allocated");
        PrintStatusLine(4, "Core: options");
        EngineOptions = EngineCore->get_InitializationOptions();
        EngineOptions->set_ContentRootPath("nitro:");
        EngineOptions->set_UpdateOrderLayers(4);
        EngineOptions->set_RenderOrderLayers3D(4);
        EngineOptions->set_UpdateListInitialCapacity(64);
        EngineOptions->set_RenderList2DInitialCapacity(64);
        EngineOptions->set_RenderList3DInitialCapacity(64);
        EngineOptions->set_PhysicsFixedStepSeconds(1.0 / 12.0);
        EngineOptions->set_PhysicsMaxStepsPerUpdate(1);
        EngineOptions->set_RuntimeDiagnosticsProvider(new NintendoDsRuntimeDiagnosticsProvider(&StatusConsole));
        PrintStatusLine(4, "Core: scene catalog");
        EngineOptions->set_SceneCatalog(BuildRuntimeSceneCatalog());
        RecordBootStatus("[helengine-ds] core initialization scene catalog set");
        PrintStatusLine(4, "Core: input config");
        EngineOptions->set_StandardPlatformInputConfiguration(BuildStandardPlatformInputConfiguration());
        RecordBootStatus("[helengine-ds] core initialization input config set");

        PrintStatusLine(4, "Core: backends");
        EngineRenderManager3D = new NintendoDsRenderManager3D();
        EngineRenderManager2D = new NintendoDsRenderManager2D();
        EngineInputBackend = new NintendoDsInputBackend();
        EnginePlatformInfo = new PlatformInfo("DS", "2.0");
        RecordBootStatus("[helengine-ds] core initialization backends allocated");

        PrintStatusLine(4, "Core: add window");
        EngineRenderManager3D->AddWindow(0, ScreenWidth, ScreenHeight);
        RecordBootStatus("[helengine-ds] core initialization window added");
        PrintStatusLine(4, "Core: initialize");
        EngineCore->Initialize(
            EngineRenderManager3D,
            EngineRenderManager2D,
            EngineInputBackend,
            EnginePlatformInfo,
            EngineOptions);
        EngineCore->SetPlatformOwnedPerformanceOverlayPresentation(true);
        RecordBootStatus("[helengine-ds] core initialization engine initialized");
#if HELENGINE_NINTENDO_DS_HAS_BEPU_GENERATED_RUNTIME
        BepuPhysicsWorld3DDiagnostics::SetDiagnosticsAllowed(false);
        PrintStatusLine(4, "Core: phys reg");
        BepuRuntimeComponentRegistration::Register(EngineCore);
        RecordBootStatus("[helengine-ds] core initialization physics runtime registered");
#else
        RecordBootStatus("[helengine-ds] core initialization generated BEPU runtime not present; physics runtime hookup skipped");
#endif
        PrintStatusLine(4, "Core: complete");
        RecordBootStatus("[helengine-ds] core initialization complete");
    }

    /// Loads and materializes the packaged startup scene.
    void NintendoDsBootHost::LoadStartupScene() {
        RecordBootStatus("[helengine-ds] startup scene lookup begin");
        NintendoDsPackagedAssetLoader packagedAssetLoader("nitro:");
        StartupSceneManagerStageSnapshot = "startup-begin";
        StartupSceneManagerSceneIdSnapshot = std::string();
        StartupSceneManagerLoadedCountSnapshot = -1;
        StartupSceneManagerPendingCountSnapshot = -1;
        PaintCheckpoint(RGB15(31, 0, 0) | BIT(15), RGB15(31, 0, 0) | BIT(15));
        const char* startupSceneRelativePath = he_get_runtime_startup_scene_relative_path();
        if (startupSceneRelativePath == nullptr || startupSceneRelativePath[0] == '\0') {
            StartupSceneManagerStageSnapshot = "manifest-missing";
            RecordBootStatus("[helengine-ds] startup scene manifest entry is missing; continuing without scene load");
            return;
        }

        if (!packagedAssetLoader.AssetExists(startupSceneRelativePath)) {
            StartupSceneManagerStageSnapshot = "asset-missing";
            StartupSceneManagerSceneIdSnapshot = startupSceneRelativePath;
            RecordBootStatus("[helengine-ds] startup scene asset is missing; continuing without scene load");
            return;
        }

        StartupSceneManagerStageSnapshot = "asset-found";
        StartupSceneManagerSceneIdSnapshot = startupSceneRelativePath;
        RecordBootStatus("[helengine-ds] startup scene asset found");
        PaintCheckpoint(RGB15(31, 31, 0) | BIT(15), RGB15(31, 31, 0) | BIT(15));
        std::string startupSceneId = ResolveStartupSceneId(startupSceneRelativePath);
        StartupSceneManagerStageSnapshot = "catalog-resolved";
        StartupSceneManagerSceneIdSnapshot = startupSceneId;
        RecordBootStatus("[helengine-ds] startup scene catalog entry resolved");
        PaintCheckpoint(RGB15(0, 31, 0) | BIT(15), RGB15(0, 31, 0) | BIT(15));
        try {
            EngineCore->get_SceneManager()->LoadScene(startupSceneId, SceneLoadMode::Single);
        } catch (...) {
            RecordBootStatus("[helengine-ds] startup scene runtime load failed");
            ::RuntimeSceneLoadService* sceneLoadService = EngineCore->get_SceneLoadService();
            if (sceneLoadService != nullptr) {
                std::ostringstream diagnosticBuilder;
                diagnosticBuilder
                    << "SceneLoad fail stage="
                    << sceneLoadService->get_LastTraceStage()
                    << " root="
                    << sceneLoadService->get_LastTraceRootEntityIndex()
                    << " depth="
                    << sceneLoadService->get_LastTraceEntityDepth()
                    << " component="
                    << sceneLoadService->get_LastTraceComponentTypeId();
                RecordBootStatus(diagnosticBuilder.str().c_str());

                std::ostringstream textDiagnosticBuilder;
                textDiagnosticBuilder
                    << "TextLoad stage="
                    << sceneLoadService->get_LastTextLoadStage()
                    << " font="
                    << sceneLoadService->get_LastTextFontRelativePath()
                    << " fontStage="
                    << sceneLoadService->get_LastFontDeserializeStage();
                RecordBootStatus(textDiagnosticBuilder.str().c_str());

                std::ostringstream textureDiagnosticBuilder;
                textureDiagnosticBuilder
                    << "TextureLoad stage="
                    << sceneLoadService->get_LastTextureLoadStage()
                    << " texture="
                    << sceneLoadService->get_LastTextureRelativePath();
                RecordBootStatus(textureDiagnosticBuilder.str().c_str());
            }

            if (EngineRenderManager3D != nullptr) {
                std::ostringstream renderDiagnosticBuilder;
                renderDiagnosticBuilder
                    << "Render3D fail stage="
                    << EngineRenderManager3D->get_LastBuildStage()
                    << " asset="
                    << EngineRenderManager3D->get_LastBuildAssetId();
                RecordBootStatus(renderDiagnosticBuilder.str().c_str());
            }

            if (EngineRenderManager2D != nullptr) {
                std::ostringstream render2dDiagnosticBuilder;
                render2dDiagnosticBuilder
                    << "Render2D fail stage="
                    << EngineRenderManager2D->get_LastTextureBuildStage()
                    << " asset="
                    << EngineRenderManager2D->get_LastTextureAssetId()
                    << " size="
                    << EngineRenderManager2D->get_LastTextureWidth()
                    << "x"
                    << EngineRenderManager2D->get_LastTextureHeight()
                    << " colors="
                    << EngineRenderManager2D->get_LastTextureColorLength();
                RecordBootStatus(render2dDiagnosticBuilder.str().c_str());
            }

            std::ostringstream compactDiagnosticBuilder;
            compactDiagnosticBuilder
                << "Diag SL="
                << (sceneLoadService != nullptr ? sceneLoadService->get_LastTraceStage() : std::string("n/a"))
                << " TL="
                << (sceneLoadService != nullptr ? sceneLoadService->get_LastTextLoadStage() : std::string("n/a"))
                << " TX="
                << (sceneLoadService != nullptr ? sceneLoadService->get_LastTextureLoadStage() : std::string("n/a"))
                << " FD="
                << (sceneLoadService != nullptr ? sceneLoadService->get_LastFontDeserializeStage() : std::string("n/a"))
                << " R2="
                << (EngineRenderManager2D != nullptr ? EngineRenderManager2D->get_LastTextureBuildStage() : std::string("n/a"));
            RecordBootStatus(compactDiagnosticBuilder.str().c_str());
            throw;
        }

        ::SceneManager* sceneManager = EngineCore != nullptr ? EngineCore->get_SceneManager() : nullptr;
        if (sceneManager != nullptr) {
            StartupSceneManagerStageSnapshot = sceneManager->get_LastTraceStage();
            StartupSceneManagerSceneIdSnapshot = sceneManager->get_LastTraceSceneId();
            StartupSceneManagerLoadedCountSnapshot = sceneManager->get_LastTraceLoadedSceneCount();
            StartupSceneManagerPendingCountSnapshot = sceneManager->get_LastTracePendingOperationCount();
        } else {
            StartupSceneManagerStageSnapshot = "scene-mgr null";
            StartupSceneManagerSceneIdSnapshot = "scene-id null";
            StartupSceneManagerLoadedCountSnapshot = -1;
            StartupSceneManagerPendingCountSnapshot = -1;
        }

        RecordBootStatus("[helengine-ds] startup scene runtime load complete");
        PaintCheckpoint(RGB15(0, 31, 31) | BIT(15), RGB15(0, 31, 31) | BIT(15));
    }

    /// Resolves the runtime scene id that owns one cooked startup-scene asset path.
    /// <param name="cookedRelativePath">Cooked-relative startup-scene asset path from the runtime startup manifest.</param>
    /// <returns>Stable runtime scene id registered for that cooked scene path.</returns>
    std::string NintendoDsBootHost::ResolveStartupSceneId(const std::string& cookedRelativePath) const {
        if (cookedRelativePath.empty()) {
            throw std::invalid_argument("Nintendo DS startup scene path is required.");
        } else if (EngineOptions == nullptr || EngineOptions->get_SceneCatalog() == nullptr) {
            throw std::runtime_error("Nintendo DS startup scene resolution requires an initialized runtime scene catalog.");
        }

        Array<::RuntimeSceneCatalogEntry*>* sceneCatalogEntries = EngineOptions->get_SceneCatalog()->get_Entries();
        if (sceneCatalogEntries == nullptr) {
            throw std::runtime_error("Nintendo DS runtime scene catalog entries were unavailable during startup scene resolution.");
        }

        for (int32_t index = 0; index < sceneCatalogEntries->Length; index++) {
            ::RuntimeSceneCatalogEntry* sceneCatalogEntry = (*sceneCatalogEntries)[index];
            if (sceneCatalogEntry == nullptr) {
                continue;
            } else if (sceneCatalogEntry->get_CookedRelativePath() != cookedRelativePath) {
                continue;
            }

            return sceneCatalogEntry->get_SceneId();
        }

        throw std::runtime_error(std::string("Nintendo DS runtime scene catalog did not contain startup scene path '") + cookedRelativePath + std::string("'."));
    }

    /// Writes one padded diagnostics row to the bottom-screen console so shorter messages do not leave stale text behind.
    void NintendoDsBootHost::PrintStatusLine(int row, const char* text) {
        consoleSelect(&StatusConsole);
        iprintf("\x1b[%d;0H%-32.32s", row, text != nullptr ? text : "");
    }

    /// Updates one tiny runtime heartbeat on the bottom screen so long-running scenes still show visible liveness without restoring the verbose diagnostic log.
    void NintendoDsBootHost::UpdateRuntimeHeartbeat(int32_t frameIndex) {
        if (EngineRenderManager2D != nullptr) {
            EngineRenderManager2D->SetRuntimeHeartbeatFrame(frameIndex);
        }
    }

    /// Records one runtime failure snapshot before an update or draw exception escapes to the top-level fatal handler.
    void NintendoDsBootHost::RecordRuntimeFailureDiagnostics(const char* phase, int32_t frameIndex, const char* exceptionKind, const char* message) {
        InitializeStatusConsole();
        std::array<char, 96> failureLine{};
        std::snprintf(
            failureLine.data(),
            failureLine.size(),
            "FAIL %s f=%ld",
            phase != nullptr ? phase : "unknown",
            static_cast<long>(frameIndex));
        PrintStatusLine(7, failureLine.data());
        PrintStatusLine(8, exceptionKind != nullptr ? exceptionKind : "Exception unknown");
        const char* failureMessage = message != nullptr ? message : "No exception message.";
        PrintStatusLine(9, failureMessage);
        std::size_t failureMessageLength = std::strlen(failureMessage);
        PrintStatusLine(10, failureMessageLength > 31 ? failureMessage + 31 : "");
        PrintStatusLine(11, failureMessageLength > 62 ? failureMessage + 62 : "");
        std::array<char, 96> phaseProbeLine{};
        std::snprintf(
            phaseProbeLine.data(),
            phaseProbeLine.size(),
            "Probe phase=%ld",
            static_cast<long>(::RuntimeExecutionPhaseProbe::get_CurrentPhaseId()));
        PrintStatusLine(12, phaseProbeLine.data());

        std::array<char, 128> phaseTraceLine{};
        std::snprintf(
            phaseTraceLine.data(),
            phaseTraceLine.size(),
            "[helengine-ds] runtime failure phase=%s exception=%s frame=%ld",
            phase != nullptr ? phase : "unknown",
            exceptionKind != nullptr ? exceptionKind : "unknown",
            static_cast<long>(frameIndex));
        EmitTrace(phaseTraceLine.data());

        ::RuntimeSceneLoadService* sceneLoadService = EngineCore != nullptr ? EngineCore->get_SceneLoadService() : nullptr;
        if (sceneLoadService != nullptr) {
            std::array<char, 128> sceneLoadLine{};
            std::snprintf(
                sceneLoadLine.data(),
                sceneLoadLine.size(),
                "SceneLoad root=%ld depth=%ld",
                static_cast<long>(sceneLoadService->get_LastTraceRootEntityIndex()),
                static_cast<long>(sceneLoadService->get_LastTraceEntityDepth()));
            EmitTrace(sceneLoadLine.data());
        }
    }

    /// Runs the generated-core update and draw loop after startup succeeds.
    void NintendoDsBootHost::RunMainLoop() {
        int32_t frameIndex = 0;
        uint32_t previousVBlankCount = VBlankCount;
        while (true) {
            if (VBlankCount == previousVBlankCount) {
                swiWaitForVBlank();
            }

            uint32_t currentVBlankCount = VBlankCount;
            uint32_t elapsedVBlanks = currentVBlankCount > previousVBlankCount ? currentVBlankCount - previousVBlankCount : 1;
            previousVBlankCount = currentVBlankCount;
            double elapsedSeconds = static_cast<double>(elapsedVBlanks) * NintendoDsFrameDeltaSeconds;
            if (KeepStatusConsoleDuringRuntimeDiagnostics) {
                std::array<char, 32> frameLine {};
                std::snprintf(frameLine.data(), frameLine.size(), "F: %ld", static_cast<long>(frameIndex));
                PrintStatusLine(3, frameLine.data());
                PrintStatusLine(4, "U: >");
                PrintStatusLine(5, "D: .");
            }

            try {
                EngineCore->Update(elapsedSeconds);
            } catch (const std::exception& exception) {
                RecordRuntimeFailureDiagnostics("Update", frameIndex, "std::exception", exception.what());
                throw;
            } catch (const Exception* exception) {
                RecordRuntimeFailureDiagnostics("Update", frameIndex, "managed Exception*", exception != nullptr ? exception->what() : "Unknown managed runtime exception.");
                throw;
            } catch (...) {
                RecordRuntimeFailureDiagnostics("Update", frameIndex, "unknown exception", "Unknown exception.");
                throw;
            }

            if (KeepStatusConsoleDuringRuntimeDiagnostics) {
                PrintStatusLine(4, "U: <");
                PrintStatusLine(5, "D: >");
            }

            try {
                EngineCore->Draw();
            } catch (const std::exception& exception) {
                RecordRuntimeFailureDiagnostics("Draw", frameIndex, "std::exception", exception.what());
                throw;
            } catch (const Exception* exception) {
                RecordRuntimeFailureDiagnostics("Draw", frameIndex, "managed Exception*", exception != nullptr ? exception->what() : "Unknown managed runtime exception.");
                throw;
            } catch (...) {
                RecordRuntimeFailureDiagnostics("Draw", frameIndex, "unknown exception", "Unknown exception.");
                throw;
            }

            if (KeepStatusConsoleDuringRuntimeDiagnostics) {
                PrintStatusLine(5, "D: <");
                if (EngineRenderManager3D != nullptr) {
                    std::array<char, 32> drawStatsLine {};
                    std::snprintf(
                        drawStatsLine.data(),
                        drawStatsLine.size(),
                        "3:T%ld Q%ld S%ld",
                        static_cast<long>(static_cast<int32_t>(EngineRenderManager3D->get_LastHardware3DScreenTarget())),
                        static_cast<long>(EngineRenderManager3D->get_LastCamera3DQueueCount()),
                        static_cast<long>(EngineRenderManager3D->get_LastSubmittedDrawableCount()));
                    PrintStatusLine(6, drawStatsLine.data());
                }

                ::ObjectManager* objectManager = EngineCore != nullptr ? EngineCore->get_ObjectManager() : nullptr;
                int32_t cameraCount = 0;
                int32_t entityCount = 0;
                int32_t updateableCount = 0;
                if (objectManager != nullptr && objectManager->get_Cameras() != nullptr) {
                    cameraCount = objectManager->get_Cameras()->Count();
                }
                if (objectManager != nullptr && objectManager->get_Entities() != nullptr) {
                    entityCount = objectManager->get_Entities()->get_Count();
                }
                if (objectManager != nullptr && objectManager->get_Updateables() != nullptr) {
                    updateableCount = objectManager->get_Updateables()->get_Count();
                }

                ::SceneManager* sceneManager = EngineCore != nullptr ? EngineCore->get_SceneManager() : nullptr;
                int32_t loadedSceneCount = -1;
                int32_t pendingOperationCount = -1;
                const char* sceneManagerStage = "scene-mgr n/a";
                const char* sceneManagerSceneId = "scene-id n/a";
                if (sceneManager != nullptr) {
                    loadedSceneCount = sceneManager->get_LastTraceLoadedSceneCount();
                    pendingOperationCount = sceneManager->get_LastTracePendingOperationCount();
                    sceneManagerStage = sceneManager->get_LastTraceStage().c_str();
                    sceneManagerSceneId = sceneManager->get_LastTraceSceneId().c_str();
                }

                std::array<char, 32> runtimeStateLine {};
                std::snprintf(
                    runtimeStateLine.data(),
                    runtimeStateLine.size(),
                    "C:%ld E:%ld U:%ld",
                    static_cast<long>(cameraCount),
                    static_cast<long>(entityCount),
                    static_cast<long>(updateableCount));
                PrintStatusLine(7, runtimeStateLine.data());

                std::array<char, 32> sceneManagerStageLine {};
                std::snprintf(
                    sceneManagerStageLine.data(),
                    sceneManagerStageLine.size(),
                    "SM:%ld/%ld %.21s",
                    static_cast<long>(loadedSceneCount),
                    static_cast<long>(pendingOperationCount),
                    sceneManagerStage);
                PrintStatusLine(8, sceneManagerStageLine.data());

                std::array<char, 32> sceneManagerSceneLine {};
                std::string startupSceneSnapshotTail = StartupSceneManagerSceneIdSnapshot;
                if (startupSceneSnapshotTail.size() > 12) {
                    startupSceneSnapshotTail = startupSceneSnapshotTail.substr(startupSceneSnapshotTail.size() - 12);
                }
                std::snprintf(
                    sceneManagerSceneLine.data(),
                    sceneManagerSceneLine.size(),
                    "ID:%.12s ST:%.12s",
                    sceneManagerSceneId,
                    startupSceneSnapshotTail.c_str());
                PrintStatusLine(9, sceneManagerSceneLine.data());

                std::array<char, 32> startupSceneManagerStageLine {};
                std::snprintf(
                    startupSceneManagerStageLine.data(),
                    startupSceneManagerStageLine.size(),
                    "SS:%ld/%ld %.18s",
                    static_cast<long>(StartupSceneManagerLoadedCountSnapshot),
                    static_cast<long>(StartupSceneManagerPendingCountSnapshot),
                    StartupSceneManagerStageSnapshot.c_str());
                PrintStatusLine(10, startupSceneManagerStageLine.data());
            }

            UpdateRuntimeHeartbeat(frameIndex);
            frameIndex++;
        }
    }
#endif

    /// Shows one fatal error on-screen and halts the process for inspection.
    void NintendoDsBootHost::ShowFatalErrorAndHalt(const std::string& message) {
        InitializeStatusConsole();
        if (MainFrameBuffer != nullptr) {
            std::fill_n(MainFrameBuffer, FrameBufferPixelCount, LastCheckpointTopScreenColor);
        }
        consoleSelect(&StatusConsole);
        consoleClear();
        iprintf("helengine-ds fatal\n\n");
        if (message == "std::bad_alloc") {
            for (std::size_t index = 0; index < 6; index++) {
                NintendoDsAllocationDiagnostics::LiveAllocationSizeSnapshot sizeSite = NintendoDsAllocationDiagnostics::GetTopLiveAllocationSizeSnapshot(index);
                if (sizeSite.Size == 0 || sizeSite.LiveCount == 0) {
                    continue;
                }

                std::array<char, 32> bucketLine{};
                std::snprintf(
                    bucketLine.data(),
                    bucketLine.size(),
                    "B%lu s%lu n%lu b%lu",
                    static_cast<unsigned long>(index),
                    static_cast<unsigned long>(sizeSite.Size),
                    static_cast<unsigned long>(sizeSite.LiveCount),
                    static_cast<unsigned long>(sizeSite.LiveBytes));
                PrintStatusLine(static_cast<int>(index + 1), bucketLine.data());
            }
            std::array<char, 32> failLine{};
            std::snprintf(
                failLine.data(),
                failLine.size(),
                "fail %lu req %lu",
                static_cast<unsigned long>(NintendoDsAllocationDiagnostics::GetLastFailedSize()),
                static_cast<unsigned long>(NintendoDsAllocationDiagnostics::GetAllocationRequestCount()));
            PrintStatusLine(8, failLine.data());
            std::array<char, 32> liveLine{};
            std::snprintf(
                liveLine.data(),
                liveLine.size(),
                "live %lu phase %ld",
                static_cast<unsigned long>(NintendoDsAllocationDiagnostics::GetCurrentAllocatedSize()),
                static_cast<long>(::RuntimeExecutionPhaseProbe::get_CurrentPhaseId()));
            PrintStatusLine(9, liveLine.data());
        } else {
            DumpBootLogToConsole();
        }
        iprintf("\n--- exception ---\n");
        iprintf("%s\n", message.c_str());
        EmitTrace(message.c_str());

        while (true) {
            swiWaitForVBlank();
        }
    }
}
