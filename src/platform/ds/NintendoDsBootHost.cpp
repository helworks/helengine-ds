#include "platform/ds/NintendoDsBootHost.hpp"

#include <algorithm>
#include <cstdio>
#include <exception>
#include <new>
#include <sstream>
#include <string>

extern "C" {
#include <nds/arm9/background.h>
#include <nds/arm9/console.h>
#include <nds/arm9/video.h>
#include <nds/interrupts.h>
#include <nds/system.h>
}

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "Component.hpp"
#include "Core.hpp"
#include "CoreInitializationOptions.hpp"
#include "Entity.hpp"
#include "FPSComponent.hpp"
#include "ObjectManager.hpp"
#include "PlatformInfo.hpp"
#include "SceneAsset.hpp"
#include "platform/ds/NintendoDsAllocationDiagnostics.hpp"
#include "platform/ds/NintendoDsInputBackend.hpp"
#include "platform/ds/NintendoDsPackagedAssetLoader.hpp"
#include "platform/ds/NintendoDsRenderManager2D.hpp"
#include "platform/ds/NintendoDsRenderManager3D.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "RuntimeSceneCatalogEntry.hpp"
#include "runtime/runtime_startup_manifest.hpp"
#include "runtime/runtime_scene_catalog_manifest.hpp"
#include "runtime/native_exceptions.hpp"
#endif

namespace helengine::ds {
    namespace {
        /// Uses the DS vertical blank cadence as the authoritative runtime frame step.
        constexpr double NintendoDsFrameDeltaSeconds = 1.0 / 60.0;

        /// Stores the number of frames between DS runtime timing diagnostics.
        constexpr int32_t DiagnosticSampleFrameInterval = 120;

        /// Counts visible DS VBlanks so the runtime can derive real elapsed frame time instead of reporting a synthetic constant rate.
        volatile uint32_t VBlankCount = 0;

        /// Increments the visible-screen VBlank counter each time the DS display presents one hardware frame.
        void HandleVBlankInterrupt() {
            VBlankCount++;
        }

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

        /// Searches one entity subtree for the first active FPS overlay component.
        FPSComponent* FindFirstFpsComponentInEntity(Entity* entity) {
            if (entity == nullptr) {
                return nullptr;
            }

            List<Component*>* components = entity->get_Components();
            if (components != nullptr) {
                for (int32_t componentIndex = 0; componentIndex < components->Count(); componentIndex++) {
                    Component* component = (*components)[componentIndex];
                    FPSComponent* fpsComponent = dynamic_cast<FPSComponent*>(component);
                    if (fpsComponent != nullptr) {
                        return fpsComponent;
                    }
                }
            }

            List<Entity*>* children = entity->get_Children();
            if (children == nullptr) {
                return nullptr;
            }

            for (int32_t childIndex = 0; childIndex < children->Count(); childIndex++) {
                FPSComponent* fpsComponent = FindFirstFpsComponentInEntity((*children)[childIndex]);
                if (fpsComponent != nullptr) {
                    return fpsComponent;
                }
            }

            return nullptr;
        }

        /// Searches the live object-manager entity list for the first active FPS overlay component.
        FPSComponent* FindFirstFpsComponent(ObjectManager* objectManager) {
            if (objectManager == nullptr) {
                return nullptr;
            }

            List<Entity*>* entities = objectManager->get_Entities();
            if (entities == nullptr) {
                return nullptr;
            }

            for (int32_t entityIndex = 0; entityIndex < entities->Count(); entityIndex++) {
                FPSComponent* fpsComponent = FindFirstFpsComponentInEntity((*entities)[entityIndex]);
                if (fpsComponent != nullptr) {
                    return fpsComponent;
                }
            }

            return nullptr;
        }

        /// Emits one structured DS runtime timing diagnostic from the live FPS overlay state.
        void EmitRuntimeTimingDiagnostic(int32_t frameIndex, Core* core) {
            if (core == nullptr) {
                EmitTrace("[helengine-ds] timing frame core=null");
                return;
            }

            std::ostringstream diagnosticBuilder;
            diagnosticBuilder
                << "[helengine-ds] timing frame="
                << frameIndex
                << " total="
                << core->get_TotalElapsedSeconds()
                << " delta="
                << core->get_FrameDeltaSeconds()
                << " drawMs="
                << core->get_LastRenderManager3DDrawMilliseconds();

            ObjectManager* objectManager = core->get_ObjectManager();
            FPSComponent* fpsComponent = FindFirstFpsComponent(objectManager);
            if (fpsComponent == nullptr) {
                diagnosticBuilder << " fps=missing";
            } else {
                diagnosticBuilder
                    << " fpsRefresh="
                    << fpsComponent->get_RefreshIntervalSeconds()
                    << " fpsUpdateText="
                    << fpsComponent->get_UpdateFpsText()
                    << " fpsRenderText="
                    << fpsComponent->get_RenderFpsText();
            }

            EmitTrace(diagnosticBuilder.str().c_str());
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
            RecordBootStatus("[helengine-ds] boot host run begin");

            if (!InitializeVideo()) {
                RecordBootStatus("[helengine-ds] video initialization failed");
                return 1;
            }

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
    }

    /// Dumps the buffered startup log to the bottom-screen diagnostics console.
    void NintendoDsBootHost::DumpBootLogToConsole() {
        if (BootLog.empty()) {
            iprintf("(no boot log)\n");
            return;
        }

        iprintf("%s\n", BootLog.c_str());
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
            RecordBootStatus("[helengine-ds] startup scene deserialize begin");
            SceneAsset* startupScene = packagedAssetLoader.LoadStartupScene();
            if (startupScene != nullptr) {
                RecordBootStatus("[helengine-ds] startup scene deserialize complete");
                iprintf("Startup scene: deserialized\n");
                RecordBootStatus("[helengine-ds] startup scene runtime load begin");
                try {
                    if (EngineRenderManager3D != nullptr) {
                        EngineRenderManager3D->ResetLastBuildDiagnostics();
                    }

                    EngineCore->get_SceneLoadService()->Load(startupScene);
                } catch (...) {
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
                        << " FD="
                        << (sceneLoadService != nullptr ? sceneLoadService->get_LastFontDeserializeStage() : std::string("n/a"))
                        << " R2="
                        << (EngineRenderManager2D != nullptr ? EngineRenderManager2D->get_LastTextureBuildStage() : std::string("n/a"));
                    RecordBootStatus(compactDiagnosticBuilder.str().c_str());

                    throw;
                }
                RecordBootStatus("[helengine-ds] startup scene runtime load complete");
                iprintf("Startup scene: loaded\n");
            } else {
                RecordBootStatus("[helengine-ds] startup scene deserialize returned null");
                iprintf("Startup scene: null\n");
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
        PaintCheckpoint(RGB15(0, 31, 0) | BIT(15), RGB15(0, 31, 0) | BIT(15));
        InitializeCore();
        RecordBootStatus("[helengine-ds] generated core initialized");
        PaintCheckpoint(RGB15(31, 31, 0) | BIT(15), RGB15(31, 31, 0) | BIT(15));
        LoadStartupScene();
        RecordBootStatus("[helengine-ds] startup scene load finished");
        PaintCheckpoint(RGB15(0, 31, 31) | BIT(15), RGB15(0, 31, 31) | BIT(15));
        RecordBootStatus("[helengine-ds] entering main loop");
        RunMainLoop();
    }

    /// Initializes the generated-core runtime with minimal Nintendo DS platform backends.
    void NintendoDsBootHost::InitializeCore() {
        RecordBootStatus("[helengine-ds] core initialization begin");
        EngineCore = new Core();
        EngineOptions = EngineCore->get_InitializationOptions();
        EngineOptions->set_ContentRootPath("nitro:");
        EngineOptions->set_UpdateOrderLayers(4);
        EngineOptions->set_RenderOrderLayers3D(4);
        EngineOptions->set_UpdateListInitialCapacity(64);
        EngineOptions->set_RenderList2DInitialCapacity(64);
        EngineOptions->set_RenderList3DInitialCapacity(64);
        EngineOptions->set_SceneCatalog(BuildRuntimeSceneCatalog());

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
        RecordBootStatus("[helengine-ds] core initialization complete");
    }

    /// Loads and materializes the packaged startup scene.
    void NintendoDsBootHost::LoadStartupScene() {
        RecordBootStatus("[helengine-ds] startup scene lookup begin");
        NintendoDsPackagedAssetLoader packagedAssetLoader("nitro:");
        PaintCheckpoint(RGB15(31, 0, 0) | BIT(15), RGB15(31, 0, 0) | BIT(15));
        if (!packagedAssetLoader.StartupSceneExists()) {
            RecordBootStatus("[helengine-ds] startup scene asset is missing; continuing without scene load");
            return;
        }

        RecordBootStatus("[helengine-ds] startup scene asset found");
        PaintCheckpoint(RGB15(31, 31, 0) | BIT(15), RGB15(31, 31, 0) | BIT(15));
        SceneAsset* startupScene = packagedAssetLoader.LoadStartupScene();
        RecordBootStatus("[helengine-ds] startup scene asset deserialized");
        PaintCheckpoint(RGB15(0, 31, 0) | BIT(15), RGB15(0, 31, 0) | BIT(15));
        try {
            EngineCore->get_SceneLoadService()->Load(startupScene);
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
                << " FD="
                << (sceneLoadService != nullptr ? sceneLoadService->get_LastFontDeserializeStage() : std::string("n/a"))
                << " R2="
                << (EngineRenderManager2D != nullptr ? EngineRenderManager2D->get_LastTextureBuildStage() : std::string("n/a"));
            RecordBootStatus(compactDiagnosticBuilder.str().c_str());
            throw;
        }
        RecordBootStatus("[helengine-ds] startup scene runtime load complete");
        PaintCheckpoint(RGB15(0, 31, 31) | BIT(15), RGB15(0, 31, 31) | BIT(15));
    }

    /// Runs the generated-core update and draw loop after startup succeeds.
    void NintendoDsBootHost::RunMainLoop() {
        int32_t frameIndex = 0;
        uint32_t previousVBlankCount = VBlankCount;
        while (true) {
            swiWaitForVBlank();
            uint32_t currentVBlankCount = VBlankCount;
            uint32_t elapsedVBlanks = currentVBlankCount > previousVBlankCount ? currentVBlankCount - previousVBlankCount : 1;
            previousVBlankCount = currentVBlankCount;
            double elapsedSeconds = static_cast<double>(elapsedVBlanks) * NintendoDsFrameDeltaSeconds;
            EngineCore->Update(elapsedSeconds);
            EngineCore->Draw();
            frameIndex++;
            if ((frameIndex % DiagnosticSampleFrameInterval) == 0) {
                EmitRuntimeTimingDiagnostic(frameIndex, EngineCore);
            }
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
        DumpBootLogToConsole();
        if (message == "std::bad_alloc") {
            iprintf(
                "\nAlloc fail=%lu last=%lu req=%lu count=%lu\n",
                static_cast<unsigned long>(NintendoDsAllocationDiagnostics::GetLastFailedSize()),
                static_cast<unsigned long>(NintendoDsAllocationDiagnostics::GetLastSuccessfulSize()),
                static_cast<unsigned long>(NintendoDsAllocationDiagnostics::GetLastRequestedSize()),
                static_cast<unsigned long>(NintendoDsAllocationDiagnostics::GetAllocationRequestCount()));
        }
        iprintf("\n--- exception ---\n");
        iprintf("%s\n", message.c_str());
        EmitTrace(message.c_str());

        while (true) {
            swiWaitForVBlank();
        }
    }
}
