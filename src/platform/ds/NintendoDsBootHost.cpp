#include "platform/ds/NintendoDsBootHost.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cstring>
#include <exception>
#include <new>
#include <string>

extern "C" {
#include <nds/arm9/background.h>
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS || HELENGINE_DS_ENABLE_FATAL_ERROR_CONSOLE
#include <nds/arm9/console.h>
#endif
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

#ifndef HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
#define HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS 0
#endif

#ifndef HELENGINE_DS_ENABLE_FATAL_ERROR_CONSOLE
#define HELENGINE_DS_ENABLE_FATAL_ERROR_CONSOLE 0
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
#include "platform/ds/audio/NintendoDsAudioBackend.hpp"
#include "platform/ds/NintendoDsFramePacing.hpp"
#include "platform/ds/NintendoDsInputBackend.hpp"
#include "platform/ds/NintendoDsPackagedAssetLoader.hpp"
#include "platform/ds/NintendoDsContentStreamSource.hpp"
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
#include "system/io/file-stream.hpp"
#include "system/io/file.hpp"
#include "FileMode.hpp"
#endif

namespace helengine::ds {
    namespace {
        /// Uses the DS vertical blank cadence as the authoritative runtime frame step.
        constexpr double NintendoDsFrameDeltaSeconds = 1.0 / 60.0;

        /// Keeps the bottom-screen status console alive during runtime diagnostics so draw/update progress remains visible on hardware.
        constexpr bool KeepStatusConsoleDuringRuntimeDiagnostics = true;

        /// Counts visible DS VBlanks so the runtime can derive real elapsed frame time instead of reporting a synthetic constant rate.
        volatile uint32_t VBlankCount = 0;

        /// Stores the shared boot-status prefix trimmed from live bottom-screen diagnostics to preserve horizontal space.
        constexpr const char* BootTracePrefix = "[helengine-ds] ";

        /// Increments the visible-screen VBlank counter each time the DS display presents one hardware frame.
        void HandleVBlankInterrupt() {
            VBlankCount++;
        }

#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
        /// Stores the host-visible boot trace file path used by emulator launches.
        constexpr const char* BootTraceLogPath = "C:/tmp/helengine-ds-logs/helengine-ds-boot.log";

        /// Stores whether the current process has already reset the host-visible boot trace file.
        bool BootTraceLogReset = false;

        /// Appends one trace line to the host-visible boot log file when the emulator host filesystem is available.
        /// <param name="message">Trace message to append.</param>
        void AppendBootTraceFileLine(const char* message) {
            if (message == nullptr || message[0] == '\0') {
                return;
            }

            try {
                if (!BootTraceLogReset) {
                    ::File::Delete(BootTraceLogPath);
                    BootTraceLogReset = true;
                }

                ::FileStream stream(BootTraceLogPath, ::FileMode::Append);
                stream.Write(reinterpret_cast<const uint8_t*>(message), 0, std::strlen(message));
                uint8_t newline = static_cast<uint8_t>('\n');
                stream.Write(&newline, 0, 1);
                stream.Flush();
                stream.Close();
            } catch (...) {
            }
        }

        /// Emits one runtime trace line to the DS emulator debug channel and host stdout.
        /// <param name="message">Trace message to emit.</param>
        void EmitTrace(const char* message) {
            if (message == nullptr) {
                return;
            }

#if HELENGINE_NINTENDO_DS_HAS_NOCASH_TRACE
            nocashMessage(message);
#endif
            std::fprintf(stderr, "%s\n", message);
            std::fflush(stderr);
            std::printf("%s\n", message);
            std::fflush(stdout);
        }
#else
        /// Suppresses host trace emission when release-oriented builds disable native DS runtime diagnostics.
        /// <param name="message">Trace message to ignore.</param>
        void EmitTrace(const char* message) {
            (void)message;
        }
#endif

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
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS || HELENGINE_DS_ENABLE_FATAL_ERROR_CONSOLE
        , StatusConsole()
#endif
#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
        , EngineCore(nullptr)
        , EngineOptions(nullptr)
        , EngineRenderManager3D(nullptr)
        , EngineRenderManager2D(nullptr)
        , EngineInputBackend(nullptr)
        , EngineAudioBackend(nullptr)
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
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
            consoleDebugInit(DebugDevice_NOCASH);
#endif

            if (!InitializeVideo()) {
                RecordBootStatus("[helengine-ds] video initialization failed");
                return 1;
            }

#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
            InitializeStatusConsole();
#endif
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
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
            std::fprintf(stderr, "[helengine-ds] fatal exception: %s\n", exception.what());
            std::fflush(stderr);
            std::printf("[helengine-ds] fatal exception: %s\n", exception.what());
            std::fflush(stdout);
#endif
            ShowFatalErrorAndHalt(exception.what());
            return 1;
#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
        } catch (const Exception* exception) {
            const char* message = exception != nullptr ? exception->what() : "Unknown managed runtime exception.";
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
            std::fprintf(stderr, "[helengine-ds] fatal runtime exception: %s\n", message);
            std::fflush(stderr);
            std::printf("[helengine-ds] fatal runtime exception: %s\n", message);
            std::fflush(stdout);
#endif
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
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS || HELENGINE_DS_ENABLE_FATAL_ERROR_CONSOLE
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
#endif
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
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
        AppendBootTraceFileLine(message);
        EmitTrace(message);
        WriteBootStatusToConsole(message);
#endif
    }

    /// Writes one live startup status line to the bottom-screen diagnostics console while boot is still in progress.
    /// <param name="message">Diagnostic message to present.</param>
    void NintendoDsBootHost::WriteBootStatusToConsole(const char* message) {
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
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
#else
        (void)message;
#endif
    }

    /// Dumps the buffered startup log to the bottom-screen diagnostics console.
    void NintendoDsBootHost::DumpBootLogToConsole() {
#if HELENGINE_DS_ENABLE_FATAL_ERROR_CONSOLE
        if (BootLog.empty()) {
            iprintf("(no boot log)\n");
            return;
        }

        iprintf("%s\n", BootLog.c_str());
#endif
    }

    /// Clears the bottom screen back to a blank hardware text background before the runtime main loop begins.
    void NintendoDsBootHost::PrepareBottomScreenForRuntimePresentation() {
        videoSetModeSub(MODE_0_2D);
        vramSetBankC(VRAM_C_SUB_BG);
        SubBackgroundId = -1;
        SubFrameBuffer = nullptr;
        StatusConsoleInitialized = false;
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS || HELENGINE_DS_ENABLE_FATAL_ERROR_CONSOLE
        std::memset(&StatusConsole, 0, sizeof(StatusConsole));
#endif
    }

    /// Paints one visible checkpoint pair so bootstrap progress remains observable even when text diagnostics are hidden.
    /// <param name="topScreenColor">Top-screen checkpoint color.</param>
    /// <param name="bottomScreenColor">Bottom-screen checkpoint color.</param>
    void NintendoDsBootHost::PaintCheckpoint(u16 topScreenColor, u16 bottomScreenColor) {
        PaintScreenColors(topScreenColor, bottomScreenColor);
    }

    /// Runs a bottom-screen console smoke test without initializing the generated engine runtime.
    void NintendoDsBootHost::RunStatusConsoleSmokeTest() {
#if !HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE || HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
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
#else
        RecordBootStatus("[helengine-ds] status console smoke test unavailable");
        while (true) {
            swiWaitForVBlank();
        }
#endif
    }

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
    /// Runs the generated-core startup checkpoints through startup-scene materialization.
    void NintendoDsBootHost::RunCheckpointedStartup() {
        RecordBootStatus("[helengine-ds] generated core startup begin");
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
        InitializeStatusConsole();
        consoleSelect(&StatusConsole);
        consoleClear();
        PrintStatusLine(0, "helengine-ds");
        PrintStatusLine(1, "checkpoint startup");
        PrintStatusLine(3, "Stage: core init");
#endif
        PaintCheckpoint(RGB15(0, 31, 0) | BIT(15), RGB15(0, 31, 0) | BIT(15));
        InitializeCore();
        RecordBootStatus("[helengine-ds] generated core initialized");
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
        PrintStatusLine(3, "Stage: scene load");
#endif
        PaintCheckpoint(RGB15(31, 31, 0) | BIT(15), RGB15(31, 31, 0) | BIT(15));
        LoadStartupScene();
        RecordBootStatus("[helengine-ds] startup scene load finished");
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
        PrintStatusLine(3, "Stage: main loop");
        PrintStatusLine(4, "Scene load: complete");
#endif
        PaintCheckpoint(RGB15(0, 31, 31) | BIT(15), RGB15(0, 31, 31) | BIT(15));
        RecordBootStatus("[helengine-ds] entering main loop");
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
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
#else
        PrepareBottomScreenForRuntimePresentation();
#endif
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
        EngineOptions->set_ContentStreamSource(new NintendoDsContentStreamSource("nitro:"));
        EngineOptions->set_UpdateOrderLayers(4);
        EngineOptions->set_RenderOrderLayers3D(4);
        EngineOptions->set_UpdateListInitialCapacity(64);
        EngineOptions->set_RenderList2DInitialCapacity(64);
        EngineOptions->set_RenderList3DInitialCapacity(64);
        EngineOptions->set_PhysicsFixedStepSeconds(1.0 / 12.0);
        EngineOptions->set_PhysicsMaxStepsPerUpdate(1);
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
        EngineOptions->set_RuntimeDiagnosticsProvider(new NintendoDsRuntimeDiagnosticsProvider(&StatusConsole));
#endif
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
        EngineAudioBackend = new NintendoDsAudioBackend();
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
        EngineCore->SetAudioBackend(EngineAudioBackend);
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
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
            ::RuntimeSceneLoadService* sceneLoadService = EngineCore->get_SceneLoadService();
            if (sceneLoadService != nullptr) {
                std::array<char, 160> diagnosticLine{};
                std::snprintf(
                    diagnosticLine.data(),
                    diagnosticLine.size(),
                    "SceneLoad fail stage=%s root=%ld depth=%ld component=%s",
                    sceneLoadService->get_LastTraceStage().c_str(),
                    static_cast<long>(sceneLoadService->get_LastTraceRootEntityIndex()),
                    static_cast<long>(sceneLoadService->get_LastTraceEntityDepth()),
                    sceneLoadService->get_LastTraceComponentTypeId().c_str());
                RecordBootStatus(diagnosticLine.data());

                std::array<char, 160> textDiagnosticLine{};
                std::snprintf(
                    textDiagnosticLine.data(),
                    textDiagnosticLine.size(),
                    "TextLoad stage=%s font=%s fontStage=%s",
                    sceneLoadService->get_LastTextLoadStage().c_str(),
                    sceneLoadService->get_LastTextFontRelativePath().c_str(),
                    sceneLoadService->get_LastFontDeserializeStage().c_str());
                RecordBootStatus(textDiagnosticLine.data());

                std::array<char, 160> textureDiagnosticLine{};
                std::snprintf(
                    textureDiagnosticLine.data(),
                    textureDiagnosticLine.size(),
                    "TextureLoad stage=%s texture=%s",
                    sceneLoadService->get_LastTextureLoadStage().c_str(),
                    sceneLoadService->get_LastTextureRelativePath().c_str());
                RecordBootStatus(textureDiagnosticLine.data());
            }

            if (EngineRenderManager3D != nullptr) {
                std::array<char, 160> renderDiagnosticLine{};
                std::snprintf(
                    renderDiagnosticLine.data(),
                    renderDiagnosticLine.size(),
                    "Render3D fail stage=%s asset=%s",
                    EngineRenderManager3D->get_LastBuildStage().c_str(),
                    EngineRenderManager3D->get_LastBuildAssetId().c_str());
                RecordBootStatus(renderDiagnosticLine.data());
            }

            if (EngineRenderManager2D != nullptr) {
                std::array<char, 160> render2dDiagnosticLine{};
                std::snprintf(
                    render2dDiagnosticLine.data(),
                    render2dDiagnosticLine.size(),
                    "Render2D fail stage=%s asset=%s size=%ldx%ld colors=%ld",
                    EngineRenderManager2D->get_LastTextureBuildStage().c_str(),
                    EngineRenderManager2D->get_LastTextureAssetId().c_str(),
                    static_cast<long>(EngineRenderManager2D->get_LastTextureWidth()),
                    static_cast<long>(EngineRenderManager2D->get_LastTextureHeight()),
                    static_cast<long>(EngineRenderManager2D->get_LastTextureColorLength()));
                RecordBootStatus(render2dDiagnosticLine.data());
            }

            std::array<char, 160> compactDiagnosticLine{};
            std::snprintf(
                compactDiagnosticLine.data(),
                compactDiagnosticLine.size(),
                "Diag SL=%s TL=%s TX=%s FD=%s R2=%s",
                sceneLoadService != nullptr ? sceneLoadService->get_LastTraceStage().c_str() : "n/a",
                sceneLoadService != nullptr ? sceneLoadService->get_LastTextLoadStage().c_str() : "n/a",
                sceneLoadService != nullptr ? sceneLoadService->get_LastTextureLoadStage().c_str() : "n/a",
                sceneLoadService != nullptr ? sceneLoadService->get_LastFontDeserializeStage().c_str() : "n/a",
                EngineRenderManager2D != nullptr ? EngineRenderManager2D->get_LastTextureBuildStage().c_str() : "n/a");
            RecordBootStatus(compactDiagnosticLine.data());
#endif
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
            throw ArgumentException();
        } else if (EngineOptions == nullptr || EngineOptions->get_SceneCatalog() == nullptr) {
            throw InvalidOperationException();
        }

        Array<::RuntimeSceneCatalogEntry*>* sceneCatalogEntries = EngineOptions->get_SceneCatalog()->get_Entries();
        if (sceneCatalogEntries == nullptr) {
            throw InvalidOperationException();
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

        throw InvalidOperationException();
    }

    /// Writes one padded diagnostics row to the bottom-screen console so shorter messages do not leave stale text behind.
    void NintendoDsBootHost::PrintStatusLine(int row, const char* text) {
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
        consoleSelect(&StatusConsole);
        iprintf("\x1b[%d;0H%-32.32s", row, text != nullptr ? text : "");
#else
        (void)row;
        (void)text;
#endif
    }

    /// Updates one tiny runtime heartbeat on the bottom screen so long-running scenes still show visible liveness without restoring the verbose diagnostic log.
    void NintendoDsBootHost::UpdateRuntimeHeartbeat(int32_t frameIndex) {
        if (EngineRenderManager2D != nullptr) {
            EngineRenderManager2D->SetRuntimeHeartbeatFrame(frameIndex);
        }
    }

    /// Records one runtime failure snapshot before an update or draw exception escapes to the top-level fatal handler.
    void NintendoDsBootHost::RecordRuntimeFailureDiagnostics(const char* phase, int32_t frameIndex, const char* exceptionKind, const char* message) {
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
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
        if (EngineRenderManager2D != nullptr) {
            std::array<char, 96> textureStageLine{};
            std::snprintf(
                textureStageLine.data(),
                textureStageLine.size(),
                "Tex %s",
                EngineRenderManager2D->get_LastTextureBuildStage().c_str());
            PrintStatusLine(13, textureStageLine.data());

            std::array<char, 96> textureMetaLine{};
            std::snprintf(
                textureMetaLine.data(),
                textureMetaLine.size(),
                "Tx %ldx%ld C%ld",
                static_cast<long>(EngineRenderManager2D->get_LastTextureWidth()),
                static_cast<long>(EngineRenderManager2D->get_LastTextureHeight()),
                static_cast<long>(EngineRenderManager2D->get_LastTextureColorLength()));
            PrintStatusLine(14, textureMetaLine.data());

            const std::string textureAssetId = EngineRenderManager2D->get_LastTextureAssetId();
            PrintStatusLine(15, textureAssetId.empty() ? "TexAsset n/a" : textureAssetId.c_str());
            if (!textureAssetId.empty()) {
                PrintStatusLine(16, textureAssetId.size() > 31 ? textureAssetId.c_str() + 31 : "");
                PrintStatusLine(17, textureAssetId.size() > 62 ? textureAssetId.c_str() + 62 : "");
            } else {
                PrintStatusLine(16, "");
                PrintStatusLine(17, "");
            }
        }

        if (EngineRenderManager3D != nullptr) {
            PrintStatusLine(18, EngineRenderManager3D->GetHardwareTextureDiagnosticsText().c_str());
            PrintStatusLine(19, EngineRenderManager3D->GetHardwareTextureLightingDiagnosticsText().c_str());
        }

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
#else
        (void)phase;
        (void)frameIndex;
        (void)exceptionKind;
        (void)message;
#endif
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
#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
            if (KeepStatusConsoleDuringRuntimeDiagnostics) {
                std::array<char, 32> frameLine {};
                std::snprintf(frameLine.data(), frameLine.size(), "F: %ld", static_cast<long>(frameIndex));
                PrintStatusLine(3, frameLine.data());
                PrintStatusLine(4, "U: >");
                PrintStatusLine(5, "D: .");
            }
#endif

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

#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
            if (KeepStatusConsoleDuringRuntimeDiagnostics) {
                PrintStatusLine(4, "U: <");
                PrintStatusLine(5, "D: >");
            }
#endif

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

#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
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
                if (EngineRenderManager3D != nullptr) {
                    PrintStatusLine(11, EngineRenderManager3D->GetHardwareTextureDiagnosticsText().c_str());
                    PrintStatusLine(12, EngineRenderManager3D->GetHardwareTextureLightingDiagnosticsText().c_str());
                }
            }
#endif

            if (EngineAudioBackend != nullptr) {
                EngineAudioBackend->Update();
            }

            UpdateRuntimeHeartbeat(frameIndex);
            frameIndex++;
        }
    }
#endif

    /// Shows one fatal error on-screen and halts the process for inspection.
    void NintendoDsBootHost::ShowFatalErrorAndHalt(const std::string& message) {
#if HELENGINE_DS_ENABLE_FATAL_ERROR_CONSOLE
        InitializeStatusConsole();
        if (MainFrameBuffer != nullptr) {
            std::fill_n(MainFrameBuffer, FrameBufferPixelCount, LastCheckpointTopScreenColor);
        }
        consoleSelect(&StatusConsole);
        consoleClear();
        iprintf("helengine-ds fatal\n\n");
        if (message == "std::bad_alloc") {
            NintendoDsAllocationDiagnostics::HeapSnapshot heapSnapshot = NintendoDsAllocationDiagnostics::GetHeapSnapshot();
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
            std::array<char, 32> heapUsedLine{};
            std::snprintf(
                heapUsedLine.data(),
                heapUsedLine.size(),
                "heap u%lu f%lu",
                static_cast<unsigned long>(heapSnapshot.UsedBytes),
                static_cast<unsigned long>(heapSnapshot.FreeBytes));
            PrintStatusLine(10, heapUsedLine.data());
            std::array<char, 32> heapArenaLine{};
            std::snprintf(
                heapArenaLine.data(),
                heapArenaLine.size(),
                "heap a%lu x%lu",
                static_cast<unsigned long>(heapSnapshot.ArenaBytes),
                static_cast<unsigned long>(heapSnapshot.ExpandableBytes));
            PrintStatusLine(11, heapArenaLine.data());
            std::array<char, 32> heapAvailableLine{};
            std::snprintf(
                heapAvailableLine.data(),
                heapAvailableLine.size(),
                "heap t%lu av%lu",
                static_cast<unsigned long>(heapSnapshot.TotalCapacityBytes),
                static_cast<unsigned long>(heapSnapshot.TotalAvailableBytes));
            PrintStatusLine(12, heapAvailableLine.data());
        } else {
            DumpBootLogToConsole();
        }
        iprintf("\n--- exception ---\n");
        iprintf("%s\n", message.c_str());
        EmitTrace(message.c_str());
#else
        EmitTrace(message.c_str());
#endif

        while (true) {
            swiWaitForVBlank();
        }
    }
}
