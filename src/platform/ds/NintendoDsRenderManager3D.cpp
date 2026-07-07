#include "platform/ds/NintendoDsRenderManager3D.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
extern "C" {
#include <nds/arm9/background.h>
#include <nds/arm9/console.h>
#include <nds/interrupts.h>
#include <nds/system.h>
#include <nds/arm9/video.h>
#include <nds/arm9/videoGL.h>
#include <nds/arm9/trig_lut.h>
#include <nds/timers.h>
}

#include <algorithm>
#include <cmath>
#include <cstring>

#include "Asset.hpp"
#include "AssetSerializer.hpp"
#include "CameraClearSettings.hpp"
#include "Core.hpp"
#include "helcpp_config.hpp"
#include "LightComponent.hpp"
#include "AmbientLightComponent.hpp"
#include "DirectionalLightComponent.hpp"
#include "Entity.hpp"
#include "ICamera.hpp"
#include "IDrawable2D.hpp"
#include "IDrawable3D.hpp"
#include "IRenderQueue2D.hpp"
#include "IRenderQueue3D.hpp"
#if HE_CPP_FEATURE_SHADERS
#include "MaterialLayout.hpp"
#include "MaterialLayoutBinding.hpp"
#include "MaterialRenderState.hpp"
#include "StandardMaterialTextureBindingDefaults.hpp"
#endif
#include "LightDirectionUtility.hpp"
#include "ObjectManager.hpp"
#include "platform/ds/NintendoDsLightingMath.hpp"
#include "platform/ds/NintendoDsColorPacker.hpp"
#include "platform/ds/NintendoDsRenderManager2D.hpp"
#include "platform/ds/NintendoDsRenderQueueSnapshotVisitor.hpp"
#include "platform/ds/NintendoDsRuntimeMaterial.hpp"
#include "platform/ds/NintendoDsRuntimeModel.hpp"
#include "platform/ds/NintendoDsRuntimeTexture2D.hpp"
#include "platform/ds/NintendoDsAllocationDiagnostics.hpp"
#include "platform/ds/NintendoDsFramePacing.hpp"
#include "runtime/native_cast.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/io/file.hpp"

#ifndef HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
#define HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS 0
#endif

namespace helengine::ds {
    namespace {
        /// Stores the standard top-screen clear color for DS 3D output.
        constexpr uint16_t DefaultClearColor = 0x0000;

        /// Maximum untextured triangle count that should stay on the immediate submission path because display-list kick cost dominates tiny meshes.
        constexpr int32_t StaticDisplayListTriangleThreshold = 16;

        /// Forces a high-contrast hardware texture payload while isolating DS texture-state issues.
        constexpr bool ForceHardwareTextureDiagnosticPattern = false;

        /// Forces deterministic hardware texture coordinates while isolating authored model UV issues.
        constexpr bool ForceHardwareTextureDiagnosticCoordinates = false;

        /// Stores the shared engine perspective FOV used by the non-DS 3D backends, expressed in degrees for libnds.
        constexpr float DefaultPerspectiveFieldOfViewDegrees = 45.0f;

        /// Converts one CPU timing sample captured through libnds into milliseconds.
        double ConvertCpuTimingTicksToMilliseconds(uint32_t ticks) {
            return static_cast<double>(timerTicks2usec(ticks)) / 1000.0;
        }

        /// Formats one millisecond value with one decimal place for compact on-device debug rows.
        std::string FormatDebugMilliseconds(double milliseconds) {
            if (!std::isfinite(milliseconds) || milliseconds > 214748364.0 || milliseconds < -214748364.0) {
                return "--";
            }

            int32_t tenths = static_cast<int32_t>(std::round(milliseconds * 10.0));
            int32_t whole = tenths / 10;
            int32_t fractional = tenths % 10;
            if (fractional < 0) {
                fractional = -fractional;
            }

            return std::to_string(whole) + "." + std::to_string(fractional);
        }

        /// Formats one signed unit vector component with one decimal place for compact light-direction diagnostics.
        std::string FormatDebugSignedUnit(float value) {
            if (!std::isfinite(value)) {
                return "--";
            }

            float clampedValue = std::clamp(value, -1.0f, 1.0f);
            int32_t tenths = static_cast<int32_t>(std::round(clampedValue * 10.0f));
            int32_t whole = tenths / 10;
            int32_t fractional = tenths % 10;
            if (fractional < 0) {
                fractional = -fractional;
            }

            return std::to_string(whole) + "." + std::to_string(fractional);
        }

        /// Formats one cooked texture color format for compact on-device diagnostics.
        std::string FormatTextureColorFormat(TextureAssetColorFormat colorFormat) {
            if (colorFormat == TextureAssetColorFormat::Rgba32) {
                return "rgba32";
            } else if (colorFormat == TextureAssetColorFormat::Rgba4444) {
                return "rgba4444";
            } else if (colorFormat == TextureAssetColorFormat::Indexed4) {
                return "idx4";
            } else if (colorFormat == TextureAssetColorFormat::Indexed8) {
                return "idx8";
            }

            return "other";
        }

        /// Builds a high-contrast direct-color texture payload that should be visibly textured if DS hardware texturing is configured correctly.
        std::vector<uint16_t> BuildHardwareTextureDiagnosticPixels(int32_t textureWidth, int32_t textureHeight) {
            std::vector<uint16_t> hardwarePixels(static_cast<std::size_t>(textureWidth * textureHeight), 0);
            for (int32_t y = 0; y < textureHeight; y++) {
                for (int32_t x = 0; x < textureWidth; x++) {
                    bool alternate = (((x / 8) + (y / 8)) & 1) != 0;
                    hardwarePixels[static_cast<std::size_t>((y * textureWidth) + x)] = alternate
                        ? static_cast<uint16_t>(BIT(15) | RGB15(31, 0, 31))
                        : static_cast<uint16_t>(BIT(15) | RGB15(0, 31, 0));
                }
            }

            return hardwarePixels;
        }

        /// Keeps DS 2D camera traversal enabled so authored menu and UI presentation remains active during normal runtime draws.
        constexpr bool Skip2DCameraTraversalForDiagnostics = false;

        /// Enables first-frame draw-stage marker checkpoints while isolating the DS draw hang.
        constexpr bool EnableFirstFrameDrawStageMarkers = true;

        /// Stores whether the first-frame draw-stage marker sequence has already completed.
        bool FirstFrameDrawStageMarkersCompleted = false;

#if HELENGINE_DS_ENABLE_RUNTIME_DIAGNOSTICS
        /// Host trace path used to capture the top-screen camera/queue state before DS 2D traversal.
        constexpr const char* TopScreenCameraTracePath = "C:/tmp/helengine-ds-top-screen-camera-trace.log";

        /// Indicates whether the current DS run has already cleared the top-screen camera trace file.
        bool TopScreenCameraTraceReset = false;

        /// Appends one line to the host-side top-screen camera trace file without affecting gameplay behavior on failure.
        /// <param name="line">Trace payload to append.</param>
        void AppendTopScreenCameraTraceLine(const std::string& line) {
            try {
                if (!TopScreenCameraTraceReset) {
                    ::File::Delete(TopScreenCameraTracePath);
                    TopScreenCameraTraceReset = true;
                }

                ::FileStream stream(TopScreenCameraTracePath, ::FileMode::Append);
                stream.Write(reinterpret_cast<const uint8_t*>(line.data()), 0, line.size());
                uint8_t newline = static_cast<uint8_t>('\n');
                stream.Write(&newline, 0, 1);
                stream.Flush();
                stream.Close();
            } catch (...) {
            }
        }
#else
        /// Suppresses top-screen camera tracing when release-oriented DS builds disable native runtime diagnostics.
        /// <param name="line">Trace payload to ignore.</param>
        void AppendTopScreenCameraTraceLine(const std::string& line) {
            (void)line;
        }
#endif

        /// Paints one small marker into the top-screen bootstrap bitmap so DS draw-stage progress stays visible before hardware 3D takes ownership.
        /// <param name="color">Visible marker color.</param>
        void PaintFirstFrameDrawStageMarker(uint16_t color) {
            uint16_t* frameBuffer = reinterpret_cast<uint16_t*>(BG_BMP_RAM(0));
            if (frameBuffer == nullptr) {
                return;
            }

            constexpr int32_t MarkerSize = 12;
            for (int32_t row = 0; row < MarkerSize; row++) {
                for (int32_t column = 0; column < MarkerSize; column++) {
                    frameBuffer[(row * 256) + column] = color;
                }
            }
        }

        /// Presents one visible first-frame draw-stage checkpoint long enough to identify the last completed stage on hardware.
        /// <param name="color">Marker color for the current stage.</param>
        void PresentFirstFrameDrawStageMarker(uint16_t color) {
            if (!EnableFirstFrameDrawStageMarkers || FirstFrameDrawStageMarkersCompleted) {
                return;
            }

            PaintFirstFrameDrawStageMarker(color);
            for (int32_t frameIndex = 0; frameIndex < 20; frameIndex++) {
                swiWaitForVBlank();
            }
        }
    }

    /// Creates one DS 3D renderer with uninitialized hardware state.
    NintendoDsRenderManager3D::NintendoDsRenderManager3D()
        : HardwareInitialized(false)
        , RenderQueueSnapshotVisitor(new NintendoDsRenderQueueSnapshotVisitor())
        , LastBuildStage("NotStarted")
        , LastBuildAssetId()
        , FrameLightDirection(0.0f, -1.0f, 0.0f)
        , FrameDirectionalRadiance(0.0f, 0.0f, 0.0f)
        , FrameAmbientRadiance(0.0f, 0.0f, 0.0f)
        , LastHardware3DScreenTarget(NintendoDsScreenTarget::None)
        , LastConfiguredHardware3DScreenTarget(NintendoDsScreenTarget::None)
        , LastConfiguredBottomScreenPresentationEnabled(true)
        , LastCamera3DQueueCount(0)
        , LastSubmittedDrawableCount(0)
        , Last3DDisplayListCallCount(0)
        , Last3DQuadDisplayListCallCount(0)
        , Last3DDisplayListSubmittedWordCount(0)
        , Last3DTexturedDisplayListBuildCount(0)
        , Last3DTexturedDisplayListReuseCount(0)
        , LastTopScreen2DQueueCount(0)
        , LastBottomScreen2DQueueCount(0)
        , Last2DTraversalNetByteDelta(0)
        , Last3DSubmissionNetByteDelta(0)
        , LastReleaseMaterialNetByteDelta(0)
        , LastReleaseModelNetByteDelta(0)
        , Last2DTraversalMilliseconds(0.0)
        , Last3DSetupMilliseconds(0.0)
        , Last3DQueueSnapshotMilliseconds(0.0)
        , Last3DGeometryEmitMilliseconds(0.0)
        , Last3DTransformMilliseconds(0.0)
        , Last3DMaterialMilliseconds(0.0)
        , Last3DDisplayListMilliseconds(0.0)
        , Last3DTexturedDisplayListEnsureMilliseconds(0.0)
        , Last3DDisplayListPreWaitMilliseconds(0.0)
        , Last3DDisplayListKickMilliseconds(0.0)
        , Last3DDisplayListPostWaitMilliseconds(0.0)
        , Last3DFallbackGeometryMilliseconds(0.0)
        , Last3DTextureConfigureMilliseconds(0.0)
        , Last3DTextureBindMilliseconds(0.0)
        , Last3DFlushMilliseconds(0.0)
        , LastPresentMilliseconds(0.0)
        , LastHardwareTextureMaterialBound(false)
        , LastHardwareTextureUploadAttempted(false)
        , LastHardwareTextureUploaded(false)
        , LastHardwareTextureId(-1)
        , LastHardwareTextureWidth(0)
        , LastHardwareTextureHeight(0)
        , LastHardwareTextureColorLength(0)
        , LastHardwareTexturePaletteColorLength(0)
        , LastHardwareTextureFormat("none")
        , LastHardwareTextureLightingEnabled(false)
        , LastHardwareTexturedTriangleCount(0)
        , LastHardwareTexturedMaxDiffuse(0.0f)
        , CachedHardwareTextureEnabledValid(false)
        , CachedHardwareTextureEnabled(false)
        , CachedHardwareTextureIdValid(false)
        , CachedHardwareTextureId(-1)
        , CachedHardwarePolyFormatValid(false)
        , CachedHardwarePolyFormat(0)
        , CachedHardwareAmbientMaterialValid(false)
        , CachedHardwareAmbientMaterial(0)
        , CachedHardwareDiffuseMaterialValid(false)
        , CachedHardwareDiffuseMaterial(0)
        , CachedHardwareVertexColorValid(false)
        , CachedHardwareVertexColor(0) {
    }

    /// Resolves which Nintendo DS screen one runtime camera targets from its viewport origin.
    /// <param name="camera">Runtime camera whose viewport should be resolved.</param>
    /// <returns>Top or bottom screen target resolved from the camera viewport.</returns>
    NintendoDsScreenTarget NintendoDsRenderManager3D::ResolveCameraScreenTarget(ICamera* camera) const {
        if (camera == nullptr) {
            throw new ArgumentNullException("camera");
        }

        float4 viewport = camera->get_Viewport();
        float resolvedViewportY = viewport.Y;
        if (viewport.Z <= 1.0f && viewport.W <= 1.0f) {
            resolvedViewportY *= 192.0f;
        }

        if (resolvedViewportY >= 192.0f) {
            return NintendoDsScreenTarget::Bottom;
        }

        return NintendoDsScreenTarget::Top;
    }

    /// Accumulates whether one camera contributes 3D queue content to the top or bottom Nintendo DS screen.
    /// <param name="camera">Runtime camera to inspect.</param>
    /// <param name="topScreenHas3D">Receives whether the top screen has any 3D content.</param>
    /// <param name="bottomScreenHas3D">Receives whether the bottom screen has any 3D content.</param>
    void NintendoDsRenderManager3D::AccumulateCameraScreenQueues(ICamera* camera, bool& topScreenHas3D, bool& bottomScreenHas3D) const {
        if (camera == nullptr) {
            return;
        }

        IRenderQueue3D* renderQueue3D = camera->get_RenderQueue3D();
        if (renderQueue3D == nullptr || renderQueue3D->get_Count() <= 0) {
            return;
        }

        NintendoDsScreenTarget screenTarget = ResolveCameraScreenTarget(camera);
        if (screenTarget == NintendoDsScreenTarget::Bottom) {
            bottomScreenHas3D = true;
            return;
        }

        topScreenHas3D = true;
    }

    /// Resolves which Nintendo DS screen should own the hardware 3D pass for the current frame.
    /// <param name="cameras">Active runtime camera list for the current frame.</param>
    /// <param name="renderManager2D">Nintendo DS 2D renderer receiving camera queue traversal for the same frame.</param>
    /// <returns>Hardware 3D target screen chosen for the current frame.</returns>
    NintendoDsScreenTarget NintendoDsRenderManager3D::ResolveHardware3DScreenTarget(List<ICamera*>* cameras, NintendoDsRenderManager2D* renderManager2D) {
        if (cameras == nullptr) {
            throw new ArgumentNullException("cameras");
        } else if (renderManager2D == nullptr) {
            throw new ArgumentNullException("renderManager2D");
        }
        bool topScreenHas3D = false;
        bool bottomScreenHas3D = false;
        for (int32_t cameraIndex = 0; cameraIndex < cameras->Count(); cameraIndex++) {
            ICamera* camera = (*cameras)[cameraIndex];
            if (camera == nullptr) {
                continue;
            }

            IRenderQueue2D* renderQueue2D = camera->get_RenderQueue2D();
            IRenderQueue3D* renderQueue3D = camera->get_RenderQueue3D();
            bool cameraHas3DQueue = renderQueue3D != nullptr && renderQueue3D->get_Count() > 0;
            if (renderQueue2D != nullptr && renderQueue2D->get_Count() > 0) {
                NintendoDsScreenTarget queueScreenTarget = ResolveCameraScreenTarget(camera);
                if (queueScreenTarget == NintendoDsScreenTarget::Bottom) {
                    LastBottomScreen2DQueueCount = renderQueue2D->get_Count();
                } else {
                    LastTopScreen2DQueueCount = renderQueue2D->get_Count();
                }
            }

            AccumulateCameraScreenQueues(camera, topScreenHas3D, bottomScreenHas3D);
        }

        if (topScreenHas3D) {
            return NintendoDsScreenTarget::Top;
        }

        if (bottomScreenHas3D) {
            return NintendoDsScreenTarget::Bottom;
        }

        return NintendoDsScreenTarget::None;
    }

    /// Draws all software 2D cameras after hardware-3D ownership has been resolved.
    void NintendoDsRenderManager3D::Draw2DCameraList(List<ICamera*>* cameras, NintendoDsRenderManager2D* renderManager2D) {
        if (cameras == nullptr) {
            throw new ArgumentNullException("cameras");
        } else if (renderManager2D == nullptr) {
            throw new ArgumentNullException("renderManager2D");
        }

        for (int32_t cameraIndex = 0; cameraIndex < cameras->Count(); cameraIndex++) {
            ICamera* camera = (*cameras)[cameraIndex];
            if (camera == nullptr) {
                continue;
            }

            renderManager2D->DrawCamera(camera);
        }
    }

    /// Configures which Nintendo DS physical screen currently owns the hardware 3D main-engine presentation.
    /// <param name="targetScreen">Screen that should own the hardware 3D pass for the current frame.</param>
    /// <param name="renderManager2D">Nintendo DS 2D renderer that may reserve the bottom screen for native-console diagnostics.</param>
    void NintendoDsRenderManager3D::ConfigureHardware3DTarget(NintendoDsScreenTarget targetScreen, NintendoDsRenderManager2D* renderManager2D) {
        bool bottomScreenPresentationEnabled = renderManager2D == nullptr || renderManager2D->get_BottomScreenPresentationEnabled();
        if (targetScreen == LastConfiguredHardware3DScreenTarget && bottomScreenPresentationEnabled == LastConfiguredBottomScreenPresentationEnabled) {
            return;
        }

        if (targetScreen == NintendoDsScreenTarget::Bottom) {
            lcdMainOnBottom();
        } else {
            lcdMainOnTop();
        }

        videoSetMode(MODE_0_3D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT);
        if (bottomScreenPresentationEnabled) {
            videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT);
        }

        LastConfiguredHardware3DScreenTarget = targetScreen;
        LastConfiguredBottomScreenPresentationEnabled = bottomScreenPresentationEnabled;
    }

    /// Decodes one float4 constant-buffer payload from little-endian bytes.
    bool NintendoDsRenderManager3D::TryDecodeFloat4ConstantBuffer(Array<uint8_t>* data, float4& decodedColor) {
        if (data == nullptr || data->Length < static_cast<int32_t>(sizeof(float) * 4) || data->Data == nullptr) {
            return false;
        }

        float channels[4];
        std::memcpy(channels, data->Data, sizeof(channels));
        decodedColor = float4(channels[0], channels[1], channels[2], channels[3]);
        return true;
    }

    /// Builds one DS runtime material from one raw packaged material asset path.
    /// <param name="assetContentManager">Content manager that can load companion packaged assets.</param>
    /// <param name="materialAssetPath">Absolute material asset path requested by the runtime loader.</param>
    /// <returns>DS runtime material carrying the authored metadata required for the first renderer slice.</returns>
    RuntimeMaterial* NintendoDsRenderManager3D::BuildMaterialFromRawAsset(ContentManager* assetContentManager, std::string materialAssetPath) {
        if (assetContentManager == nullptr) {
            throw new ArgumentNullException("assetContentManager");
        }
        if (String::IsNullOrWhiteSpace(materialAssetPath)) {
            throw new ArgumentException("Material asset path must be provided.", "materialAssetPath");
        }

        LastBuildStage = "BuildMaterialFromRawAssetUnsupported";
        LastBuildAssetId = materialAssetPath;
        throw new NotSupportedException("Nintendo DS requires cooked platform-owned material payloads.");
    }

    /// Builds one DS runtime material from a cooked platform-owned payload.
    /// <param name="materialAsset">Cooked platform-owned material payload.</param>
    /// <returns>DS runtime material carrying the cooked metadata required for the first renderer slice.</returns>
    RuntimeMaterial* NintendoDsRenderManager3D::BuildMaterialFromCooked(PlatformMaterialAsset* materialAsset) {
        if (materialAsset == nullptr) {
            throw new ArgumentNullException("materialAsset");
        }

        LastBuildStage = "BuildMaterialFromCookedBegin";
        LastBuildAssetId = materialAsset->get_Id();
        NintendoDsRuntimeMaterial* runtimeMaterial = new NintendoDsRuntimeMaterial();
        runtimeMaterial->set_Id(materialAsset->get_Id());
        runtimeMaterial->BaseColor = float3(
            static_cast<float>(materialAsset->BaseColorR) / 255.0f,
            static_cast<float>(materialAsset->BaseColorG) / 255.0f,
            static_cast<float>(materialAsset->BaseColorB) / 255.0f);
        runtimeMaterial->PackedDiffuseColor = NintendoDsColorPacker::PackOpaqueColor(runtimeMaterial->BaseColor);
        runtimeMaterial->SupportsGeometrySubmission = true;
        runtimeMaterial->LightingEnabled = materialAsset->Lit;
#if HE_CPP_FEATURE_SHADERS
        Array<MaterialLayoutBinding*>* textureBindings = new Array<MaterialLayoutBinding*>(1);
        (*textureBindings)[0] = new ::MaterialLayoutBinding(StandardMaterialTextureBindingDefaults::DiffuseTextureBindingName, ShaderResourceType::Texture2D, 0, 0, 0);
        ::MaterialLayout* dsMaterialLayout = new ::MaterialLayout(
            String::Empty,
            String::Empty,
            String::Empty,
            String::Empty,
            new ::MaterialRenderState(),
            textureBindings,
            Array<MaterialLayoutBinding*>::Empty(),
            Array<MaterialLayoutBinding*>::Empty());
        runtimeMaterial->SetLayout(dsMaterialLayout);
#endif
        LastBuildStage = "BuildMaterialFromCookedComplete";
        return runtimeMaterial;
    }

    /// Builds one DS runtime material from one cooked platform-owned payload serialized on disk.
    /// <param name="cookedAssetPath">Absolute NitroFS or host path to the serialized cooked material asset.</param>
    /// <returns>DS runtime material carrying the cooked metadata required for the first renderer slice.</returns>
    RuntimeMaterial* NintendoDsRenderManager3D::BuildMaterialFromCooked(std::string cookedAssetPath, IContentStreamSource* contentStreamSource) {
        LastBuildStage = "BuildMaterialFromCookedBegin";
        LastBuildAssetId = cookedAssetPath;
        if (cookedAssetPath.empty()) {
            throw new ArgumentException("Cooked material asset path must be provided.", "cookedAssetPath");
        }

        ::FileStream* stream = nullptr;
        ::Asset* asset = nullptr;
        try {
            stream = ::File::OpenRead(cookedAssetPath);
            LastBuildStage = "BuildMaterialFromCookedOpened";
            asset = ::AssetSerializer::Deserialize(stream);
            LastBuildStage = "BuildMaterialFromCookedDeserialized";
            delete stream;
            stream = nullptr;

            ::PlatformMaterialAsset* materialAsset = he_cpp_try_cast<PlatformMaterialAsset>(asset);
            if (materialAsset == nullptr) {
                throw new InvalidOperationException("Nintendo DS cooked material payloads must deserialize as PlatformMaterialAsset.");
            }

            LastBuildStage = "BuildMaterialFromCookedTyped";
            RuntimeMaterial* runtimeMaterial = BuildMaterialFromCooked(materialAsset);
            delete materialAsset;
            LastBuildStage = "BuildMaterialFromCookedComplete";
            return runtimeMaterial;
        } catch (...) {
            if (stream != nullptr) {
                delete stream;
            }
            if (asset != nullptr) {
                delete asset;
            }

            throw;
        }
    }

    /// Builds one DS runtime model from one cooked model payload serialized on disk.
    /// <param name="cookedAssetPath">Absolute NitroFS or host path to the serialized cooked model asset.</param>
    /// <returns>DS runtime model carrying the adopted cooked geometry payload.</returns>
    RuntimeModel* NintendoDsRenderManager3D::BuildModelFromCooked(std::string cookedAssetPath, IContentStreamSource* contentStreamSource) {
        LastBuildStage = "BuildModelFromCookedBegin";
        LastBuildAssetId = cookedAssetPath;
        if (cookedAssetPath.empty()) {
            throw new ArgumentException("Cooked model asset path must be provided.", "cookedAssetPath");
        }

        ::FileStream* stream = nullptr;
        ::Asset* asset = nullptr;
        try {
            stream = ::File::OpenRead(cookedAssetPath);
            LastBuildStage = "BuildModelFromCookedOpened";
            asset = ::AssetSerializer::Deserialize(stream);
            LastBuildStage = "BuildModelFromCookedDeserialized";
            delete stream;
            stream = nullptr;

            ::ModelAsset* modelAsset = he_cpp_try_cast<ModelAsset>(asset);
            if (modelAsset == nullptr) {
                throw new InvalidOperationException("Nintendo DS cooked model payloads must deserialize as ModelAsset.");
            }

            LastBuildStage = "BuildModelFromCookedTyped";
            RuntimeModel* runtimeModel = BuildModelFromRaw(modelAsset);
            delete modelAsset;
            LastBuildStage = "BuildModelFromCookedComplete";
            return runtimeModel;
        } catch (...) {
            if (stream != nullptr) {
                delete stream;
            }
            if (asset != nullptr) {
                delete asset;
            }

            throw;
        }
    }

    /// Builds one DS runtime model from the authored model asset.
    /// <param name="data">Authored model asset.</param>
    /// <returns>DS runtime model carrying the authored metadata required for the first renderer slice.</returns>
    RuntimeModel* NintendoDsRenderManager3D::BuildModelFromRaw(ModelAsset* data) {
        if (data == nullptr) {
            throw new ArgumentNullException("data");
        }

        LastBuildStage = "BuildModelFromRawBegin";
        LastBuildAssetId = data->get_Id();
        NintendoDsRuntimeModel* runtimeModel = new NintendoDsRuntimeModel();
        runtimeModel->set_Id(data->get_Id());
        runtimeModel->Positions = data->Positions;
        runtimeModel->TexCoords = data->TexCoords;
        runtimeModel->Indices16 = data->Indices16;
        runtimeModel->Indices32 = data->Indices32;
        runtimeModel->Uses32BitIndices = runtimeModel->Indices32 != nullptr && runtimeModel->Indices32->Length > 0;
        runtimeModel->HardwareLitDisplayList = BuildHardwareLitDisplayList(runtimeModel, runtimeModel->HardwareLitDisplayListWordCount);
        runtimeModel->HardwareTexturedDisplayList = nullptr;
        runtimeModel->HardwareTexturedDisplayListWordCount = 0;
        runtimeModel->HardwareTexturedDisplayListTextureWidth = 0;
        runtimeModel->HardwareTexturedDisplayListTextureHeight = 0;
        runtimeModel->UsesHardwareTexturedQuadDisplayList = false;
        data->Positions = Array<float3>::Empty();
        data->TexCoords = Array<float2>::Empty();
        data->Indices16 = Array<uint16_t>::Empty();
        data->Indices32 = Array<uint32_t>::Empty();
        LastBuildStage = "BuildModelFromRawComplete";
        return runtimeModel;
    }

    /// Releases DS renderer-owned material state while leaving the runtime material object owned by SceneManager.
    /// <param name="material">Runtime material to release.</param>
    void NintendoDsRenderManager3D::ReleaseMaterial(RuntimeMaterial* material) {
        if (material == nullptr) {
            throw new ArgumentNullException("material");
        }

        std::size_t allocatedBeforeRelease = NintendoDsAllocationDiagnostics::GetTotalAllocatedSize();
        std::size_t freedBeforeRelease = NintendoDsAllocationDiagnostics::GetTotalFreedSize();
        std::size_t allocatedAfterRelease = NintendoDsAllocationDiagnostics::GetTotalAllocatedSize();
        std::size_t freedAfterRelease = NintendoDsAllocationDiagnostics::GetTotalFreedSize();
        LastReleaseMaterialNetByteDelta = static_cast<int32_t>(
            (allocatedAfterRelease - allocatedBeforeRelease)
            - (freedAfterRelease - freedBeforeRelease));
    }

    /// Releases one DS runtime model's adopted geometry buffers while leaving the runtime model object owned by SceneManager.
    /// <param name="model">Runtime model to release.</param>
    void NintendoDsRenderManager3D::ReleaseModel(RuntimeModel* model) {
        if (model == nullptr) {
            throw new ArgumentNullException("model");
        }

        std::size_t allocatedBeforeRelease = NintendoDsAllocationDiagnostics::GetTotalAllocatedSize();
        std::size_t freedBeforeRelease = NintendoDsAllocationDiagnostics::GetTotalFreedSize();
        NintendoDsRuntimeModel* runtimeModel = ResolveRuntimeModel(model);
        if (runtimeModel != nullptr && runtimeModel->Positions != nullptr && runtimeModel->Positions != Array<float3>::Empty()) {
            delete runtimeModel->Positions;
            runtimeModel->Positions = Array<float3>::Empty();
        }

        if (runtimeModel != nullptr && runtimeModel->TexCoords != nullptr && runtimeModel->TexCoords != Array<float2>::Empty()) {
            delete runtimeModel->TexCoords;
            runtimeModel->TexCoords = Array<float2>::Empty();
        }

        if (runtimeModel != nullptr && runtimeModel->Indices16 != nullptr && runtimeModel->Indices16 != Array<uint16_t>::Empty()) {
            delete runtimeModel->Indices16;
            runtimeModel->Indices16 = Array<uint16_t>::Empty();
        }

        if (runtimeModel != nullptr && runtimeModel->Indices32 != nullptr && runtimeModel->Indices32 != Array<uint32_t>::Empty()) {
            delete runtimeModel->Indices32;
            runtimeModel->Indices32 = Array<uint32_t>::Empty();
        }

        if (runtimeModel != nullptr && runtimeModel->HardwareLitDisplayList != nullptr) {
            delete[] runtimeModel->HardwareLitDisplayList;
            runtimeModel->HardwareLitDisplayList = nullptr;
            runtimeModel->HardwareLitDisplayListWordCount = 0;
        }

        if (runtimeModel != nullptr && runtimeModel->HardwareTexturedDisplayList != nullptr) {
            delete[] runtimeModel->HardwareTexturedDisplayList;
            runtimeModel->HardwareTexturedDisplayList = nullptr;
            runtimeModel->HardwareTexturedDisplayListWordCount = 0;
            runtimeModel->HardwareTexturedDisplayListTextureWidth = 0;
            runtimeModel->HardwareTexturedDisplayListTextureHeight = 0;
            runtimeModel->UsesHardwareTexturedQuadDisplayList = false;
        }

        std::size_t allocatedAfterRelease = NintendoDsAllocationDiagnostics::GetTotalAllocatedSize();
        std::size_t freedAfterRelease = NintendoDsAllocationDiagnostics::GetTotalFreedSize();
        LastReleaseModelNetByteDelta = static_cast<int32_t>(
            (allocatedAfterRelease - allocatedBeforeRelease)
            - (freedAfterRelease - freedBeforeRelease));
    }

    /// Resolves one runtime-model pointer back to the DS-owned runtime-model specialization.
    /// <param name="runtimeModel">Runtime model pointer supplied through the shared renderer contract.</param>
    /// <returns>DS-owned runtime model pointer, or null when the caller passed no model.</returns>
    NintendoDsRuntimeModel* NintendoDsRenderManager3D::ResolveRuntimeModel(RuntimeModel* runtimeModel) {
        if (runtimeModel == nullptr) {
            return nullptr;
        }

        return static_cast<NintendoDsRuntimeModel*>(runtimeModel);
    }

    /// Resolves one runtime-material pointer back to the DS-owned runtime-material specialization.
    /// <param name="runtimeMaterial">Runtime material pointer supplied through the shared renderer contract.</param>
    /// <returns>DS-owned runtime material pointer, or null when the caller passed no material.</returns>
    NintendoDsRuntimeMaterial* NintendoDsRenderManager3D::ResolveRuntimeMaterial(RuntimeMaterial* runtimeMaterial) {
        if (runtimeMaterial == nullptr) {
            return nullptr;
        }

        return static_cast<NintendoDsRuntimeMaterial*>(runtimeMaterial);
    }

    /// Resolves one runtime-texture pointer back to the DS-owned runtime-texture specialization.
    /// <param name="runtimeTexture">Runtime texture pointer supplied through the shared renderer contract.</param>
    /// <returns>DS-owned runtime texture pointer, or null when the caller passed no texture.</returns>
    NintendoDsRuntimeTexture2D* NintendoDsRenderManager3D::ResolveRuntimeTexture(RuntimeTexture* runtimeTexture) {
        if (runtimeTexture == nullptr) {
            return nullptr;
        }

        return static_cast<NintendoDsRuntimeTexture2D*>(runtimeTexture);
    }

    /// Resolves one shared 2D render-manager pointer back to the DS-owned 2D renderer specialization.
    /// <param name="renderManager2D">Shared 2D renderer pointer supplied by the generated core.</param>
    /// <returns>DS-owned 2D renderer pointer.</returns>
    NintendoDsRenderManager2D* NintendoDsRenderManager3D::ResolveNintendoDsRenderManager2D(RenderManager2D* renderManager2D) {
        if (renderManager2D == nullptr) {
            return nullptr;
        }

        return static_cast<NintendoDsRenderManager2D*>(renderManager2D);
    }

    /// Increments the current-frame instance count tracked for one DS runtime model.
    /// <param name="runtimeModelInstanceCounts">Mutable current-frame instance-count table.</param>
    /// <param name="runtimeModel">Runtime model whose instance count should be incremented.</param>
    void NintendoDsRenderManager3D::IncrementRuntimeModelInstanceCount(
        std::vector<NintendoDsRuntimeModelInstanceCountEntry>& runtimeModelInstanceCounts,
        NintendoDsRuntimeModel* runtimeModel) const {
        if (runtimeModel == nullptr) {
            return;
        }

        for (std::size_t entryIndex = 0; entryIndex < runtimeModelInstanceCounts.size(); entryIndex++) {
            NintendoDsRuntimeModelInstanceCountEntry& entry = runtimeModelInstanceCounts[entryIndex];
            if (entry.RuntimeModel != runtimeModel) {
                continue;
            }

            entry.InstanceCount++;
            return;
        }

        runtimeModelInstanceCounts.push_back({ runtimeModel, 1 });
    }

    /// Resolves the number of drawables that reference one DS runtime model in the current frame.
    /// <param name="runtimeModelInstanceCounts">Current-frame instance-count table.</param>
    /// <param name="runtimeModel">Runtime model whose current-frame instance count should be returned.</param>
    /// <returns>Current-frame drawable count for the runtime model, or one when no explicit count was recorded.</returns>
    int32_t NintendoDsRenderManager3D::ResolveRuntimeModelInstanceCount(
        const std::vector<NintendoDsRuntimeModelInstanceCountEntry>& runtimeModelInstanceCounts,
        NintendoDsRuntimeModel* runtimeModel) const {
        if (runtimeModel == nullptr) {
            return 1;
        }

        for (std::size_t entryIndex = 0; entryIndex < runtimeModelInstanceCounts.size(); entryIndex++) {
            const NintendoDsRuntimeModelInstanceCountEntry& entry = runtimeModelInstanceCounts[entryIndex];
            if (entry.RuntimeModel == runtimeModel) {
                return entry.InstanceCount > 0 ? entry.InstanceCount : 1;
            }
        }

        return 1;
    }

    /// Builds one packed Nintendo DS FIFO command stream for fixed-function lit static geometry.
    uint32_t* NintendoDsRenderManager3D::BuildHardwareLitDisplayList(NintendoDsRuntimeModel* runtimeModel, uint32_t& displayListWordCount) {
        if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        }

        displayListWordCount = 0;
        runtimeModel->UsesHardwareLitQuadDisplayList = false;
        Array<float3>* positions = runtimeModel->Positions;
        if (positions == nullptr || positions->Length <= 0) {
            return nullptr;
        }

        std::vector<uint32_t> displayListWords;
        displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_BEGIN, FIFO_NOP, FIFO_NOP, FIFO_NOP));
        displayListWords.push_back(GL_TRIANGLES);
        bool useVertex10 = CanUseHardwareLitVertex10DisplayList(positions);

        if (runtimeModel->Uses32BitIndices && runtimeModel->Indices32 != nullptr) {
            for (int32_t index = 0; index + 2 < runtimeModel->Indices32->Length; index += 3) {
                AppendHardwareLitDisplayListTriangle(
                    displayListWords,
                    positions,
                    static_cast<int32_t>((*runtimeModel->Indices32)[index]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 1]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 2]),
                    useVertex10);
            }
        } else if (runtimeModel->Indices16 != nullptr) {
            for (int32_t index = 0; index + 2 < runtimeModel->Indices16->Length; index += 3) {
                AppendHardwareLitDisplayListTriangle(
                    displayListWords,
                    positions,
                    static_cast<int32_t>((*runtimeModel->Indices16)[index]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 1]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 2]),
                    useVertex10);
            }
        } else {
            for (int32_t index = 0; index + 2 < positions->Length; index += 3) {
                AppendHardwareLitDisplayListTriangle(displayListWords, positions, index, index + 1, index + 2, useVertex10);
            }
        }

        if (displayListWords.size() <= 3) {
            return nullptr;
        }

        uint32_t* displayList = new uint32_t[displayListWords.size() + 1];
        displayList[0] = static_cast<uint32_t>(displayListWords.size());
        for (std::size_t wordIndex = 0; wordIndex < displayListWords.size(); wordIndex++) {
            displayList[wordIndex + 1] = displayListWords[wordIndex];
        }

        displayListWordCount = displayList[0];
        FlushHardwareLitDisplayList(displayList);
        return displayList;
    }

    /// Builds one quad-only packed command stream when the indexed triangle list is fully reducible to DS quads.
    uint32_t* NintendoDsRenderManager3D::BuildHardwareLitQuadDisplayList(NintendoDsRuntimeModel* runtimeModel, uint32_t& displayListWordCount) {
        if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        }

        displayListWordCount = 0;
        Array<float3>* positions = runtimeModel->Positions;
        if (positions == nullptr || positions->Length <= 0) {
            return nullptr;
        }

        std::vector<uint32_t> displayListWords;
        displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_BEGIN, FIFO_NOP, FIFO_NOP, FIFO_NOP));
        displayListWords.push_back(GL_QUADS);
        bool useVertex10 = CanUseHardwareLitVertex10DisplayList(positions);

        bool appendedAnyQuad = false;
        if (runtimeModel->Uses32BitIndices && runtimeModel->Indices32 != nullptr && runtimeModel->Indices32->Length % 6 == 0) {
            for (int32_t index = 0; index + 5 < runtimeModel->Indices32->Length; index += 6) {
                bool appendedQuad = TryAppendHardwareLitDisplayListQuad(
                    displayListWords,
                    positions,
                    static_cast<int32_t>((*runtimeModel->Indices32)[index]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 1]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 2]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 3]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 4]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 5]),
                    useVertex10);
                if (!appendedQuad) {
                    return nullptr;
                }

                appendedAnyQuad = true;
            }
        } else if (runtimeModel->Indices16 != nullptr && runtimeModel->Indices16->Length % 6 == 0) {
            for (int32_t index = 0; index + 5 < runtimeModel->Indices16->Length; index += 6) {
                bool appendedQuad = TryAppendHardwareLitDisplayListQuad(
                    displayListWords,
                    positions,
                    static_cast<int32_t>((*runtimeModel->Indices16)[index]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 1]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 2]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 3]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 4]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 5]),
                    useVertex10);
                if (!appendedQuad) {
                    return nullptr;
                }

                appendedAnyQuad = true;
            }
        } else {
            return nullptr;
        }

        if (!appendedAnyQuad) {
            return nullptr;
        }

        uint32_t* displayList = new uint32_t[displayListWords.size() + 1];
        displayList[0] = static_cast<uint32_t>(displayListWords.size());
        for (std::size_t wordIndex = 0; wordIndex < displayListWords.size(); wordIndex++) {
            displayList[wordIndex + 1] = displayListWords[wordIndex];
        }

        displayListWordCount = displayList[0];
        FlushHardwareLitDisplayList(displayList);
        return displayList;
    }

    /// Builds one packed Nintendo DS FIFO command stream for fixed-function textured static geometry using one concrete hardware texture size.
    uint32_t* NintendoDsRenderManager3D::BuildHardwareTexturedDisplayList(
        NintendoDsRuntimeModel* runtimeModel,
        NintendoDsRuntimeTexture2D* runtimeTexture,
        uint32_t& displayListWordCount) {
        if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        } else if (runtimeTexture == nullptr) {
            throw new ArgumentNullException("runtimeTexture");
        }

        displayListWordCount = 0;
        runtimeModel->UsesHardwareTexturedQuadDisplayList = false;
        Array<float3>* positions = runtimeModel->Positions;
        Array<float2>* texCoords = runtimeModel->TexCoords;
        if (positions == nullptr || positions->Length <= 0) {
            return nullptr;
        } else if (texCoords == nullptr || texCoords->Length < positions->Length) {
            return nullptr;
        }

        uint32_t* texturedQuadDisplayList = BuildHardwareTexturedQuadDisplayList(runtimeModel, runtimeTexture, displayListWordCount);
        if (texturedQuadDisplayList != nullptr) {
            runtimeModel->UsesHardwareTexturedQuadDisplayList = true;
            return texturedQuadDisplayList;
        }

        std::vector<uint32_t> displayListWords;
        displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_BEGIN, FIFO_NOP, FIFO_NOP, FIFO_NOP));
        displayListWords.push_back(GL_TRIANGLES);
        bool useVertex10 = CanUseHardwareLitVertex10DisplayList(positions);

        if (runtimeModel->Uses32BitIndices && runtimeModel->Indices32 != nullptr) {
            for (int32_t index = 0; index + 2 < runtimeModel->Indices32->Length; index += 3) {
                AppendHardwareTexturedDisplayListTriangle(
                    displayListWords,
                    positions,
                    texCoords,
                    runtimeTexture,
                    static_cast<int32_t>((*runtimeModel->Indices32)[index]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 1]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 2]),
                    useVertex10);
            }
        } else if (runtimeModel->Indices16 != nullptr) {
            for (int32_t index = 0; index + 2 < runtimeModel->Indices16->Length; index += 3) {
                AppendHardwareTexturedDisplayListTriangle(
                    displayListWords,
                    positions,
                    texCoords,
                    runtimeTexture,
                    static_cast<int32_t>((*runtimeModel->Indices16)[index]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 1]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 2]),
                    useVertex10);
            }
        } else {
            for (int32_t index = 0; index + 2 < positions->Length; index += 3) {
                AppendHardwareTexturedDisplayListTriangle(displayListWords, positions, texCoords, runtimeTexture, index, index + 1, index + 2, useVertex10);
            }
        }

        if (displayListWords.size() <= 3) {
            return nullptr;
        }

        uint32_t* displayList = new uint32_t[displayListWords.size() + 1];
        displayList[0] = static_cast<uint32_t>(displayListWords.size());
        for (std::size_t wordIndex = 0; wordIndex < displayListWords.size(); wordIndex++) {
            displayList[wordIndex + 1] = displayListWords[wordIndex];
        }

        displayListWordCount = displayList[0];
        FlushHardwareLitDisplayList(displayList);
        return displayList;
    }

    /// Builds one quad-only packed textured command stream when the indexed triangle list is fully reducible to DS quads.
    uint32_t* NintendoDsRenderManager3D::BuildHardwareTexturedQuadDisplayList(
        NintendoDsRuntimeModel* runtimeModel,
        NintendoDsRuntimeTexture2D* runtimeTexture,
        uint32_t& displayListWordCount) {
        if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        } else if (runtimeTexture == nullptr) {
            throw new ArgumentNullException("runtimeTexture");
        }

        displayListWordCount = 0;
        Array<float3>* positions = runtimeModel->Positions;
        Array<float2>* texCoords = runtimeModel->TexCoords;
        if (positions == nullptr || positions->Length <= 0) {
            return nullptr;
        } else if (texCoords == nullptr || texCoords->Length < positions->Length) {
            return nullptr;
        }

        std::vector<uint32_t> displayListWords;
        displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_BEGIN, FIFO_NOP, FIFO_NOP, FIFO_NOP));
        displayListWords.push_back(GL_QUADS);
        bool useVertex10 = CanUseHardwareLitVertex10DisplayList(positions);

        bool appendedAnyQuad = false;
        if (runtimeModel->Uses32BitIndices && runtimeModel->Indices32 != nullptr && runtimeModel->Indices32->Length % 6 == 0) {
            for (int32_t index = 0; index + 5 < runtimeModel->Indices32->Length; index += 6) {
                bool appendedQuad = TryAppendHardwareTexturedDisplayListQuad(
                    displayListWords,
                    positions,
                    texCoords,
                    runtimeTexture,
                    static_cast<int32_t>((*runtimeModel->Indices32)[index]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 1]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 2]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 3]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 4]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 5]),
                    useVertex10);
                if (!appendedQuad) {
                    return nullptr;
                }

                appendedAnyQuad = true;
            }
        } else if (runtimeModel->Indices16 != nullptr && runtimeModel->Indices16->Length % 6 == 0) {
            for (int32_t index = 0; index + 5 < runtimeModel->Indices16->Length; index += 6) {
                bool appendedQuad = TryAppendHardwareTexturedDisplayListQuad(
                    displayListWords,
                    positions,
                    texCoords,
                    runtimeTexture,
                    static_cast<int32_t>((*runtimeModel->Indices16)[index]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 1]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 2]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 3]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 4]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 5]),
                    useVertex10);
                if (!appendedQuad) {
                    return nullptr;
                }

                appendedAnyQuad = true;
            }
        } else {
            return nullptr;
        }

        if (!appendedAnyQuad) {
            return nullptr;
        }

        uint32_t* displayList = new uint32_t[displayListWords.size() + 1];
        displayList[0] = static_cast<uint32_t>(displayListWords.size());
        for (std::size_t wordIndex = 0; wordIndex < displayListWords.size(); wordIndex++) {
            displayList[wordIndex + 1] = displayListWords[wordIndex];
        }

        displayListWordCount = displayList[0];
        FlushHardwareLitDisplayList(displayList);
        return displayList;
    }

    /// Ensures one runtime model owns a textured display list matching the currently bound hardware texture dimensions.
    void NintendoDsRenderManager3D::EnsureHardwareTexturedDisplayList(
        NintendoDsRuntimeModel* runtimeModel,
        NintendoDsRuntimeTexture2D* runtimeTexture) {
        if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        } else if (runtimeTexture == nullptr) {
            throw new ArgumentNullException("runtimeTexture");
        }

        uint32_t ensureStartTimingTicks = cpuGetTiming();
        if (runtimeModel->HardwareTexturedDisplayList != nullptr
            && runtimeModel->HardwareTexturedDisplayListTextureWidth == runtimeTexture->get_Width()
            && runtimeModel->HardwareTexturedDisplayListTextureHeight == runtimeTexture->get_Height()) {
            Last3DTexturedDisplayListReuseCount++;
            Last3DTexturedDisplayListEnsureMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - ensureStartTimingTicks);
            return;
        }

        if (runtimeModel->HardwareTexturedDisplayList != nullptr) {
            delete[] runtimeModel->HardwareTexturedDisplayList;
            runtimeModel->HardwareTexturedDisplayList = nullptr;
            runtimeModel->HardwareTexturedDisplayListWordCount = 0;
            runtimeModel->HardwareTexturedDisplayListTextureWidth = 0;
            runtimeModel->HardwareTexturedDisplayListTextureHeight = 0;
            runtimeModel->UsesHardwareTexturedQuadDisplayList = false;
        }

        runtimeModel->HardwareTexturedDisplayList = BuildHardwareTexturedDisplayList(runtimeModel, runtimeTexture, runtimeModel->HardwareTexturedDisplayListWordCount);
        if (runtimeModel->HardwareTexturedDisplayList == nullptr) {
            Last3DTexturedDisplayListEnsureMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - ensureStartTimingTicks);
            return;
        }

        Last3DTexturedDisplayListBuildCount++;
        runtimeModel->HardwareTexturedDisplayListTextureWidth = runtimeTexture->get_Width();
        runtimeModel->HardwareTexturedDisplayListTextureHeight = runtimeTexture->get_Height();
        Last3DTexturedDisplayListEnsureMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - ensureStartTimingTicks);
    }

    /// Flushes one immutable packed display-list payload after construction so frame submission can DMA without cache maintenance.
    void NintendoDsRenderManager3D::FlushHardwareLitDisplayList(uint32_t* displayList) const {
        if (displayList == nullptr) {
            throw new ArgumentNullException("displayList");
        } else if (displayList[0] <= 0) {
            return;
        }

        DC_FlushRange(displayList + 1, displayList[0] * sizeof(uint32_t));
    }

    /// Submits one immutable packed display list through synchronous DMA without per-frame data-cache flushing.
    void NintendoDsRenderManager3D::SubmitStaticHardwareDisplayList(NintendoDsRuntimeModel* runtimeModel, bool useHardwareTexture) {
        if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        }

        uint32_t* displayList = runtimeModel->HardwareLitDisplayList;
        if (useHardwareTexture && runtimeModel->HardwareTexturedDisplayList != nullptr) {
            displayList = runtimeModel->HardwareTexturedDisplayList;
        }
        if (displayList == nullptr) {
            return;
        }

        if (displayList[0] <= 0) {
            return;
        }

        Last3DDisplayListCallCount++;
        Last3DDisplayListSubmittedWordCount += static_cast<int32_t>(displayList[0]);
        if ((!useHardwareTexture && runtimeModel->UsesHardwareLitQuadDisplayList)
            || (useHardwareTexture && runtimeModel->UsesHardwareTexturedQuadDisplayList)) {
            Last3DQuadDisplayListCallCount++;
        }

        uint32_t preWaitStartTimingTicks = cpuGetTiming();
        while ((DMA_CR(0) & DMA_BUSY) || (DMA_CR(1) & DMA_BUSY) || (DMA_CR(2) & DMA_BUSY) || (DMA_CR(3) & DMA_BUSY)) {
        }
        Last3DDisplayListPreWaitMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - preWaitStartTimingTicks);

        uint32_t dmaKickStartTimingTicks = cpuGetTiming();
        glCallList(reinterpret_cast<u32*>(displayList));
        Last3DDisplayListKickMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - dmaKickStartTimingTicks);

        uint32_t postWaitStartTimingTicks = cpuGetTiming();
        while (DMA_CR(0) & DMA_BUSY) {
        }
        Last3DDisplayListPostWaitMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - postWaitStartTimingTicks);
    }

    /// Resolves whether one runtime model should use the prebuilt static display-list path or direct immediate submission for the current draw.
    /// <param name="runtimeModel">Runtime model being submitted.</param>
    /// <param name="useHardwareTexture">Whether the current draw already requires the textured immediate path.</param>
    /// <param name="runtimeModelInstanceCount">Number of times the same runtime model appears in the current queue snapshot.</param>
    /// <returns>True when the model should use the static display-list path for the current draw.</returns>
    bool NintendoDsRenderManager3D::ShouldUseStaticHardwareDisplayList(NintendoDsRuntimeModel* runtimeModel, bool useHardwareTexture, int32_t runtimeModelInstanceCount) const {
        if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        } else if (runtimeModelInstanceCount <= 0) {
            throw new ArgumentOutOfRangeException("runtimeModelInstanceCount");
        } else if (useHardwareTexture && runtimeModel->HardwareTexturedDisplayList == nullptr) {
            return false;
        } else if (!useHardwareTexture && runtimeModel->HardwareLitDisplayList == nullptr) {
            return false;
        } else if (runtimeModel->Positions == nullptr || runtimeModel->Positions->Length <= 0) {
            return true;
        } else if (runtimeModelInstanceCount > 1) {
            return true;
        }

        return ResolveTrianglePrimitiveCount(runtimeModel) > StaticDisplayListTriangleThreshold;
    }

    /// Counts how many triangles one runtime model would submit through the immediate geometry path.
    /// <param name="runtimeModel">Runtime model whose triangle count should be resolved.</param>
    /// <returns>Number of triangles represented by the current index or position data.</returns>
    int32_t NintendoDsRenderManager3D::ResolveTrianglePrimitiveCount(NintendoDsRuntimeModel* runtimeModel) const {
        if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        } else if (runtimeModel->Uses32BitIndices && runtimeModel->Indices32 != nullptr) {
            return runtimeModel->Indices32->Length / 3;
        } else if (runtimeModel->Indices16 != nullptr) {
            return runtimeModel->Indices16->Length / 3;
        } else if (runtimeModel->Positions != nullptr) {
            return runtimeModel->Positions->Length / 3;
        }

        return 0;
    }

    /// Attempts to append one quad represented by two indexed triangles that share the same diagonal.
    bool NintendoDsRenderManager3D::TryAppendHardwareLitDisplayListQuad(
        std::vector<uint32_t>& displayListWords,
        Array<float3>* positions,
        int32_t indexA,
        int32_t indexC,
        int32_t indexB,
        int32_t secondIndexC,
        int32_t secondIndexA,
        int32_t indexD,
        bool useVertex10) {
        if (positions == nullptr) {
            throw new ArgumentNullException("positions");
        } else if (indexA != secondIndexA || indexC != secondIndexC) {
            return false;
        } else if (indexA < 0 || indexB < 0 || indexC < 0 || indexD < 0) {
            return false;
        } else if (indexA >= positions->Length || indexB >= positions->Length || indexC >= positions->Length || indexD >= positions->Length) {
            return false;
        }

        float3 triangleNormalA = NintendoDsLightingMath::ComputeTriangleNormal((*positions)[indexA], (*positions)[indexC], (*positions)[indexB]);
        float3 triangleNormalB = NintendoDsLightingMath::ComputeTriangleNormal((*positions)[indexC], (*positions)[indexA], (*positions)[indexD]);
        double normalAlignment =
            (static_cast<double>(triangleNormalA.X) * static_cast<double>(triangleNormalB.X))
            + (static_cast<double>(triangleNormalA.Y) * static_cast<double>(triangleNormalB.Y))
            + (static_cast<double>(triangleNormalA.Z) * static_cast<double>(triangleNormalB.Z));
        if (normalAlignment < 0.999) {
            return false;
        }

        AppendHardwareLitDisplayListQuad(displayListWords, positions, indexA, indexD, indexC, indexB, useVertex10);
        return true;
    }

    /// Attempts to append one textured quad represented by two indexed triangles that share the same diagonal.
    bool NintendoDsRenderManager3D::TryAppendHardwareTexturedDisplayListQuad(
        std::vector<uint32_t>& displayListWords,
        Array<float3>* positions,
        Array<float2>* texCoords,
        NintendoDsRuntimeTexture2D* runtimeTexture,
        int32_t indexA,
        int32_t indexC,
        int32_t indexB,
        int32_t secondIndexC,
        int32_t secondIndexA,
        int32_t indexD,
        bool useVertex10) {
        if (positions == nullptr) {
            throw new ArgumentNullException("positions");
        } else if (texCoords == nullptr) {
            throw new ArgumentNullException("texCoords");
        } else if (runtimeTexture == nullptr) {
            throw new ArgumentNullException("runtimeTexture");
        } else if (indexA != secondIndexA || indexC != secondIndexC) {
            return false;
        } else if (indexA < 0 || indexB < 0 || indexC < 0 || indexD < 0) {
            return false;
        } else if (indexA >= positions->Length || indexB >= positions->Length || indexC >= positions->Length || indexD >= positions->Length) {
            return false;
        } else if (indexA >= texCoords->Length || indexB >= texCoords->Length || indexC >= texCoords->Length || indexD >= texCoords->Length) {
            return false;
        }

        float3 triangleNormalA = NintendoDsLightingMath::ComputeTriangleNormal((*positions)[indexA], (*positions)[indexC], (*positions)[indexB]);
        float3 triangleNormalB = NintendoDsLightingMath::ComputeTriangleNormal((*positions)[indexC], (*positions)[indexA], (*positions)[indexD]);
        double normalAlignment =
            (static_cast<double>(triangleNormalA.X) * static_cast<double>(triangleNormalB.X))
            + (static_cast<double>(triangleNormalA.Y) * static_cast<double>(triangleNormalB.Y))
            + (static_cast<double>(triangleNormalA.Z) * static_cast<double>(triangleNormalB.Z));
        if (normalAlignment < 0.999) {
            return false;
        }

        AppendHardwareTexturedDisplayListQuad(displayListWords, positions, texCoords, runtimeTexture, indexA, indexD, indexC, indexB, useVertex10);
        return true;
    }

    /// Appends one normal and four vertices to a packed Nintendo DS quad command stream.
    void NintendoDsRenderManager3D::AppendHardwareLitDisplayListQuad(
        std::vector<uint32_t>& displayListWords,
        Array<float3>* positions,
        int32_t indexA,
        int32_t indexD,
        int32_t indexC,
        int32_t indexB,
        bool useVertex10) {
        if (positions == nullptr) {
            throw new ArgumentNullException("positions");
        }

        float3 vertexA = (*positions)[indexA];
        float3 vertexD = (*positions)[indexD];
        float3 vertexC = (*positions)[indexC];
        float3 vertexB = (*positions)[indexB];
        float3 modelFaceNormal = NintendoDsLightingMath::ComputeTriangleNormal(vertexA, vertexD, vertexC);
        if (useVertex10) {
            displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_NORMAL, FIFO_VERTEX10, FIFO_VERTEX10, FIFO_VERTEX10));
            displayListWords.push_back(NORMAL_PACK(
                PackHardwareNormalComponent(modelFaceNormal.X),
                PackHardwareNormalComponent(modelFaceNormal.Y),
                PackHardwareNormalComponent(modelFaceNormal.Z)));
            AppendHardwareLitDisplayListVertex10(displayListWords, vertexA);
            AppendHardwareLitDisplayListVertex10(displayListWords, vertexD);
            AppendHardwareLitDisplayListVertex10(displayListWords, vertexC);
            displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_VERTEX10, FIFO_NOP, FIFO_NOP, FIFO_NOP));
            AppendHardwareLitDisplayListVertex10(displayListWords, vertexB);
            return;
        }

        displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_NORMAL, FIFO_VERTEX16, FIFO_VERTEX16, FIFO_VERTEX16));
        displayListWords.push_back(NORMAL_PACK(
            PackHardwareNormalComponent(modelFaceNormal.X),
            PackHardwareNormalComponent(modelFaceNormal.Y),
            PackHardwareNormalComponent(modelFaceNormal.Z)));
        AppendHardwareLitDisplayListVertex(displayListWords, vertexA);
        AppendHardwareLitDisplayListVertex(displayListWords, vertexD);
        AppendHardwareLitDisplayListVertex(displayListWords, vertexC);
        displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_VERTEX16, FIFO_NOP, FIFO_NOP, FIFO_NOP));
        AppendHardwareLitDisplayListVertex(displayListWords, vertexB);
    }

    /// Appends one normal, four texcoords, and four vertices to a packed Nintendo DS textured quad command stream.
    void NintendoDsRenderManager3D::AppendHardwareTexturedDisplayListQuad(
        std::vector<uint32_t>& displayListWords,
        Array<float3>* positions,
        Array<float2>* texCoords,
        NintendoDsRuntimeTexture2D* runtimeTexture,
        int32_t indexA,
        int32_t indexD,
        int32_t indexC,
        int32_t indexB,
        bool useVertex10) {
        if (positions == nullptr) {
            throw new ArgumentNullException("positions");
        } else if (texCoords == nullptr) {
            throw new ArgumentNullException("texCoords");
        } else if (runtimeTexture == nullptr) {
            throw new ArgumentNullException("runtimeTexture");
        }

        float3 vertexA = (*positions)[indexA];
        float3 vertexD = (*positions)[indexD];
        float3 vertexC = (*positions)[indexC];
        float3 vertexB = (*positions)[indexB];
        float3 modelFaceNormal = NintendoDsLightingMath::ComputeTriangleNormal(vertexA, vertexD, vertexC);
        if (useVertex10) {
            displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_NORMAL, FIFO_TEX_COORD, FIFO_VERTEX10, FIFO_TEX_COORD));
            displayListWords.push_back(NORMAL_PACK(
                PackHardwareNormalComponent(modelFaceNormal.X),
                PackHardwareNormalComponent(modelFaceNormal.Y),
                PackHardwareNormalComponent(modelFaceNormal.Z)));
            AppendHardwareTexturedDisplayListTexCoord(displayListWords, (*texCoords)[indexA], runtimeTexture);
            AppendHardwareLitDisplayListVertex10(displayListWords, vertexA);
            AppendHardwareTexturedDisplayListTexCoord(displayListWords, (*texCoords)[indexD], runtimeTexture);
            displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_VERTEX10, FIFO_TEX_COORD, FIFO_VERTEX10, FIFO_TEX_COORD));
            AppendHardwareLitDisplayListVertex10(displayListWords, vertexD);
            AppendHardwareTexturedDisplayListTexCoord(displayListWords, (*texCoords)[indexC], runtimeTexture);
            AppendHardwareLitDisplayListVertex10(displayListWords, vertexC);
            AppendHardwareTexturedDisplayListTexCoord(displayListWords, (*texCoords)[indexB], runtimeTexture);
            displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_VERTEX10, FIFO_NOP, FIFO_NOP, FIFO_NOP));
            AppendHardwareLitDisplayListVertex10(displayListWords, vertexB);
            return;
        }

        displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_NORMAL, FIFO_TEX_COORD, FIFO_VERTEX16, FIFO_TEX_COORD));
        displayListWords.push_back(NORMAL_PACK(
            PackHardwareNormalComponent(modelFaceNormal.X),
            PackHardwareNormalComponent(modelFaceNormal.Y),
            PackHardwareNormalComponent(modelFaceNormal.Z)));
        AppendHardwareTexturedDisplayListTexCoord(displayListWords, (*texCoords)[indexA], runtimeTexture);
        AppendHardwareLitDisplayListVertex(displayListWords, vertexA);
        AppendHardwareTexturedDisplayListTexCoord(displayListWords, (*texCoords)[indexD], runtimeTexture);
        displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_VERTEX16, FIFO_TEX_COORD, FIFO_VERTEX16, FIFO_TEX_COORD));
        AppendHardwareLitDisplayListVertex(displayListWords, vertexD);
        AppendHardwareTexturedDisplayListTexCoord(displayListWords, (*texCoords)[indexC], runtimeTexture);
        AppendHardwareLitDisplayListVertex(displayListWords, vertexC);
        AppendHardwareTexturedDisplayListTexCoord(displayListWords, (*texCoords)[indexB], runtimeTexture);
        displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_VERTEX16, FIFO_NOP, FIFO_NOP, FIFO_NOP));
        AppendHardwareLitDisplayListVertex(displayListWords, vertexB);
    }

    /// Appends one triangle's normal and vertices to a packed Nintendo DS FIFO command stream.
    void NintendoDsRenderManager3D::AppendHardwareLitDisplayListTriangle(
        std::vector<uint32_t>& displayListWords,
        Array<float3>* positions,
        int32_t indexA,
        int32_t indexB,
        int32_t indexC,
        bool useVertex10) {
        if (positions == nullptr) {
            throw new ArgumentNullException("positions");
        } else if (indexA < 0 || indexB < 0 || indexC < 0) {
            return;
        } else if (indexA >= positions->Length || indexB >= positions->Length || indexC >= positions->Length) {
            return;
        }

        float3 vertexA = (*positions)[indexA];
        float3 vertexB = (*positions)[indexB];
        float3 vertexC = (*positions)[indexC];
        float3 modelFaceNormal = NintendoDsLightingMath::ComputeTriangleNormal(vertexA, vertexB, vertexC);
        if (useVertex10) {
            displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_NORMAL, FIFO_VERTEX10, FIFO_VERTEX10, FIFO_VERTEX10));
            displayListWords.push_back(NORMAL_PACK(
                PackHardwareNormalComponent(modelFaceNormal.X),
                PackHardwareNormalComponent(modelFaceNormal.Y),
                PackHardwareNormalComponent(modelFaceNormal.Z)));
            AppendHardwareLitDisplayListVertex10(displayListWords, vertexA);
            AppendHardwareLitDisplayListVertex10(displayListWords, vertexB);
            AppendHardwareLitDisplayListVertex10(displayListWords, vertexC);
            return;
        }

        displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_NORMAL, FIFO_VERTEX16, FIFO_VERTEX16, FIFO_VERTEX16));
        displayListWords.push_back(NORMAL_PACK(
            PackHardwareNormalComponent(modelFaceNormal.X),
            PackHardwareNormalComponent(modelFaceNormal.Y),
            PackHardwareNormalComponent(modelFaceNormal.Z)));
        AppendHardwareLitDisplayListVertex(displayListWords, vertexA);
        AppendHardwareLitDisplayListVertex(displayListWords, vertexB);
        AppendHardwareLitDisplayListVertex(displayListWords, vertexC);
    }

    /// Appends one model-space vertex to a packed Nintendo DS FIFO command stream.
    void NintendoDsRenderManager3D::AppendHardwareLitDisplayListVertex(std::vector<uint32_t>& displayListWords, const float3& position) {
        displayListWords.push_back(VERTEX_PACK(floattov16(position.X), floattov16(position.Y)));
        displayListWords.push_back(static_cast<uint16_t>(floattov16(position.Z)));
    }

    /// Appends one model-space vertex to a packed Nintendo DS FIFO command stream using the compact VTX10 format.
    void NintendoDsRenderManager3D::AppendHardwareLitDisplayListVertex10(std::vector<uint32_t>& displayListWords, const float3& position) {
        displayListWords.push_back(
            (PackHardwareVertex10Component(position.X) & 0x3FF)
            | ((PackHardwareVertex10Component(position.Y) & 0x3FF) << 10)
            | ((PackHardwareVertex10Component(position.Z) & 0x3FF) << 20));
    }

    /// Appends one triangle's texcoords, normal, and vertices to a packed Nintendo DS FIFO command stream.
    void NintendoDsRenderManager3D::AppendHardwareTexturedDisplayListTriangle(
        std::vector<uint32_t>& displayListWords,
        Array<float3>* positions,
        Array<float2>* texCoords,
        NintendoDsRuntimeTexture2D* runtimeTexture,
        int32_t indexA,
        int32_t indexB,
        int32_t indexC,
        bool useVertex10) {
        if (positions == nullptr) {
            throw new ArgumentNullException("positions");
        } else if (texCoords == nullptr) {
            throw new ArgumentNullException("texCoords");
        } else if (runtimeTexture == nullptr) {
            throw new ArgumentNullException("runtimeTexture");
        } else if (indexA < 0 || indexB < 0 || indexC < 0) {
            return;
        } else if (indexA >= positions->Length || indexB >= positions->Length || indexC >= positions->Length) {
            return;
        } else if (indexA >= texCoords->Length || indexB >= texCoords->Length || indexC >= texCoords->Length) {
            return;
        }

        float3 vertexA = (*positions)[indexA];
        float3 vertexB = (*positions)[indexB];
        float3 vertexC = (*positions)[indexC];
        float3 modelFaceNormal = NintendoDsLightingMath::ComputeTriangleNormal(vertexA, vertexB, vertexC);
        if (useVertex10) {
            displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_NORMAL, FIFO_TEX_COORD, FIFO_VERTEX10, FIFO_TEX_COORD));
            displayListWords.push_back(NORMAL_PACK(
                PackHardwareNormalComponent(modelFaceNormal.X),
                PackHardwareNormalComponent(modelFaceNormal.Y),
                PackHardwareNormalComponent(modelFaceNormal.Z)));
            AppendHardwareTexturedDisplayListTexCoord(displayListWords, (*texCoords)[indexA], runtimeTexture);
            AppendHardwareLitDisplayListVertex10(displayListWords, vertexA);
            AppendHardwareTexturedDisplayListTexCoord(displayListWords, (*texCoords)[indexB], runtimeTexture);
            displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_VERTEX10, FIFO_TEX_COORD, FIFO_VERTEX10, FIFO_NOP));
            AppendHardwareLitDisplayListVertex10(displayListWords, vertexB);
            AppendHardwareTexturedDisplayListTexCoord(displayListWords, (*texCoords)[indexC], runtimeTexture);
            AppendHardwareLitDisplayListVertex10(displayListWords, vertexC);
            return;
        }

        displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_NORMAL, FIFO_TEX_COORD, FIFO_VERTEX16, FIFO_TEX_COORD));
        displayListWords.push_back(NORMAL_PACK(
            PackHardwareNormalComponent(modelFaceNormal.X),
            PackHardwareNormalComponent(modelFaceNormal.Y),
            PackHardwareNormalComponent(modelFaceNormal.Z)));
        AppendHardwareTexturedDisplayListTexCoord(displayListWords, (*texCoords)[indexA], runtimeTexture);
        AppendHardwareLitDisplayListVertex(displayListWords, vertexA);
        AppendHardwareTexturedDisplayListTexCoord(displayListWords, (*texCoords)[indexB], runtimeTexture);
        displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_VERTEX16, FIFO_TEX_COORD, FIFO_VERTEX16, FIFO_NOP));
        AppendHardwareLitDisplayListVertex(displayListWords, vertexB);
        AppendHardwareTexturedDisplayListTexCoord(displayListWords, (*texCoords)[indexC], runtimeTexture);
        AppendHardwareLitDisplayListVertex(displayListWords, vertexC);
    }

    /// Appends one packed DS texcoord word derived from a normalized model UV and one concrete runtime texture size.
    void NintendoDsRenderManager3D::AppendHardwareTexturedDisplayListTexCoord(
        std::vector<uint32_t>& displayListWords,
        const float2& texCoord,
        NintendoDsRuntimeTexture2D* runtimeTexture) const {
        if (runtimeTexture == nullptr) {
            throw new ArgumentNullException("runtimeTexture");
        }

        uint16_t packedX = static_cast<uint16_t>(floattot16(texCoord.X * static_cast<float>(runtimeTexture->get_Width())));
        uint16_t packedY = static_cast<uint16_t>(floattot16(texCoord.Y * static_cast<float>(runtimeTexture->get_Height())));
        displayListWords.push_back(static_cast<uint32_t>(packedX) | (static_cast<uint32_t>(packedY) << 16));
    }

    /// Resolves whether all model-space positions fit the compact signed 10-bit DS vertex command range.
    bool NintendoDsRenderManager3D::CanUseHardwareLitVertex10DisplayList(Array<float3>* positions) const {
        if (positions == nullptr) {
            throw new ArgumentNullException("positions");
        }

        for (int32_t index = 0; index < positions->Length; index++) {
            float3 position = (*positions)[index];
            if (position.X < -8.0f || position.X > 7.984375f) {
                return false;
            } else if (position.Y < -8.0f || position.Y > 7.984375f) {
                return false;
            } else if (position.Z < -8.0f || position.Z > 7.984375f) {
                return false;
            }
        }

        return true;
    }

    /// Resets the last runtime asset-build diagnostic state before one traced scene-load attempt.
    void NintendoDsRenderManager3D::ResetLastBuildDiagnostics() {
        LastBuildStage = "NotStarted";
        LastBuildAssetId.clear();
    }

    /// Gets the last DS runtime asset-build stage reached by model or material construction.
    /// <returns>Last recorded asset-build stage.</returns>
    std::string NintendoDsRenderManager3D::get_LastBuildStage() const {
        return LastBuildStage;
    }

    /// Gets the last authored asset id seen by the DS runtime asset-build path.
    /// <returns>Last recorded authored asset id.</returns>
    std::string NintendoDsRenderManager3D::get_LastBuildAssetId() const {
        return LastBuildAssetId;
    }

    /// Gets which Nintendo DS screen most recently owned the hardware 3D pass.
    /// <returns>Most recently selected hardware 3D screen target.</returns>
    NintendoDsScreenTarget NintendoDsRenderManager3D::get_LastHardware3DScreenTarget() const {
        return LastHardware3DScreenTarget;
    }

    /// Gets the most recent 3D queue size observed for the selected hardware 3D camera.
    /// <returns>Most recent 3D queue size for the active hardware 3D camera.</returns>
    int32_t NintendoDsRenderManager3D::get_LastCamera3DQueueCount() const {
        return LastCamera3DQueueCount;
    }

    /// Gets the most recent number of 3D drawables submitted during one frame.
    /// <returns>Most recent 3D submitted-drawable count.</returns>
    int32_t NintendoDsRenderManager3D::get_LastSubmittedDrawableCount() const {
        return LastSubmittedDrawableCount;
    }

    /// Gets the most recent top-screen 2D queue size observed during one frame.
    /// <returns>Most recent top-screen 2D queue size.</returns>
    int32_t NintendoDsRenderManager3D::get_LastTopScreen2DQueueCount() const {
        return LastTopScreen2DQueueCount;
    }

    /// Gets the most recent bottom-screen 2D queue size observed during one frame.
    /// <returns>Most recent bottom-screen 2D queue size.</returns>
    int32_t NintendoDsRenderManager3D::get_LastBottomScreen2DQueueCount() const {
        return LastBottomScreen2DQueueCount;
    }

    /// Gets the most recent net allocator delta observed during the 2D camera traversal portion of one draw call.
    int32_t NintendoDsRenderManager3D::get_Last2DTraversalNetByteDelta() const {
        return Last2DTraversalNetByteDelta;
    }

    /// Gets the most recent net allocator delta observed during the 3D submission portion of one draw call.
    int32_t NintendoDsRenderManager3D::get_Last3DSubmissionNetByteDelta() const {
        return Last3DSubmissionNetByteDelta;
    }

    /// Gets the most recent net allocator delta observed while releasing one scene-owned runtime material.
    /// <returns>Most recent runtime-material release allocator delta in bytes.</returns>
    int32_t NintendoDsRenderManager3D::get_LastReleaseMaterialNetByteDelta() const {
        return LastReleaseMaterialNetByteDelta;
    }

    /// Gets the most recent net allocator delta observed while releasing one scene-owned runtime model.
    /// <returns>Most recent runtime-model release allocator delta in bytes.</returns>
    int32_t NintendoDsRenderManager3D::get_LastReleaseModelNetByteDelta() const {
        return LastReleaseModelNetByteDelta;
    }

    /// Rejects one off-screen render-target request because the DS backend exposes only real hardware paths.
    /// <param name="width">Requested render-target width.</param>
    /// <param name="height">Requested render-target height.</param>
    /// <returns>Unsupported on Nintendo DS.</returns>
    RenderTarget* NintendoDsRenderManager3D::CreateRenderTarget(int32_t width, int32_t height) {
        throw new InvalidOperationException("Nintendo DS render targets are unsupported because this backend only exposes real hardware render paths.");
    }

    /// Draws the current generated-core 3D frame through the Nintendo DS renderer path.
    void NintendoDsRenderManager3D::Draw() {
        Core* core = Core::get_Instance();
        if (core == nullptr) {
            throw new InvalidOperationException("Core::Instance was not initialized.");
        }

        PresentFirstFrameDrawStageMarker(RGB15(31, 0, 0) | BIT(15));
        cpuStartTiming(0);
        ObjectManager* objectManager = core->get_ObjectManager();
        if (objectManager == nullptr) {
            throw new InvalidOperationException("Object manager was not initialized.");
        }

        NintendoDsRenderManager2D* renderManager2D = ResolveNintendoDsRenderManager2D(core->get_RenderManager2D());
        if (renderManager2D == nullptr) {
            throw new InvalidOperationException("Core render manager 2D was not a Nintendo DS renderer.");
        }

        PresentFirstFrameDrawStageMarker(RGB15(31, 16, 0) | BIT(15));
        PresentFirstFrameDrawStageMarker(RGB15(31, 20, 0) | BIT(15));
        List<ICamera*>* cameras = objectManager->get_Cameras();
        PresentFirstFrameDrawStageMarker(RGB15(31, 24, 0) | BIT(15));
        int32_t cameraCount = cameras != nullptr ? cameras->Count() : 0;
        PresentFirstFrameDrawStageMarker(RGB15(31, 28, 0) | BIT(15));
        if (cameras == nullptr || cameraCount <= 0) {
            PublishPerformanceOverlayMetrics(core, renderManager2D, true);
            FirstFrameDrawStageMarkersCompleted = true;
            return;
        }

        PresentFirstFrameDrawStageMarker(RGB15(31, 31, 0) | BIT(15));
        PresentFirstFrameDrawStageMarker(RGB15(31, 0, 0) | BIT(15));
        renderManager2D->BeginFrame();
        PresentFirstFrameDrawStageMarker(RGB15(0, 0, 31) | BIT(15));
        LastHardware3DScreenTarget = NintendoDsScreenTarget::None;
        LastCamera3DQueueCount = 0;
        LastSubmittedDrawableCount = 0;
        Last3DDisplayListCallCount = 0;
        Last3DQuadDisplayListCallCount = 0;
        Last3DDisplayListSubmittedWordCount = 0;
        Last3DTexturedDisplayListBuildCount = 0;
        Last3DTexturedDisplayListReuseCount = 0;
        LastTopScreen2DQueueCount = 0;
        LastBottomScreen2DQueueCount = 0;
        Last2DTraversalNetByteDelta = 0;
        Last3DSubmissionNetByteDelta = 0;
        Last2DTraversalMilliseconds = 0.0;
        Last3DSetupMilliseconds = 0.0;
        Last3DQueueSnapshotMilliseconds = 0.0;
        Last3DGeometryEmitMilliseconds = 0.0;
        Last3DTransformMilliseconds = 0.0;
        Last3DMaterialMilliseconds = 0.0;
        Last3DDisplayListMilliseconds = 0.0;
        Last3DTexturedDisplayListEnsureMilliseconds = 0.0;
        Last3DDisplayListPreWaitMilliseconds = 0.0;
        Last3DDisplayListKickMilliseconds = 0.0;
        Last3DDisplayListPostWaitMilliseconds = 0.0;
        Last3DFallbackGeometryMilliseconds = 0.0;
        Last3DTextureConfigureMilliseconds = 0.0;
        Last3DTextureBindMilliseconds = 0.0;
        Last3DFlushMilliseconds = 0.0;
        LastPresentMilliseconds = 0.0;
        LastHardwareTextureMaterialBound = false;
        LastHardwareTextureUploadAttempted = false;
        LastHardwareTextureUploaded = false;
        LastHardwareTextureId = -1;
        LastHardwareTextureWidth = 0;
        LastHardwareTextureHeight = 0;
        LastHardwareTextureColorLength = 0;
        LastHardwareTexturePaletteColorLength = 0;
        LastHardwareTextureFormat = "none";
        LastHardwareTextureLightingEnabled = false;
        LastHardwareTexturedTriangleCount = 0;
        LastHardwareTexturedMaxDiffuse = 0.0f;
        InvalidateHardwareStateCache();
        std::size_t initialAllocatedByteTotal = NintendoDsAllocationDiagnostics::GetTotalAllocatedSize();
        std::size_t initialFreedByteTotal = NintendoDsAllocationDiagnostics::GetTotalFreedSize();
        uint32_t traversalStartTimingTicks = cpuGetTiming();
        NintendoDsScreenTarget hardware3DScreenTarget = ResolveHardware3DScreenTarget(cameras, renderManager2D);
        renderManager2D->SetFrameQueueCounts(LastTopScreen2DQueueCount, LastBottomScreen2DQueueCount);
        AppendTopScreenCameraTraceLine("[helengine-ds] camera-count=" + std::to_string(cameraCount));
        for (int32_t cameraIndex = 0; cameraIndex < cameraCount; cameraIndex++) {
            ICamera* camera = (*cameras)[cameraIndex];
            if (camera == nullptr) {
                AppendTopScreenCameraTraceLine("[helengine-ds] camera[" + std::to_string(cameraIndex) + "]=null");
                continue;
            }

            IRenderQueue2D* renderQueue2D = camera->get_RenderQueue2D();
            NintendoDsScreenTarget queueScreenTarget = ResolveCameraScreenTarget(camera);
            std::string line = "[helengine-ds] camera[" + std::to_string(cameraIndex) + "] screen=";
            line += queueScreenTarget == NintendoDsScreenTarget::Bottom ? "bottom" : "top";
            line += " queue2d=";
            line += renderQueue2D == nullptr ? "null" : std::to_string(renderQueue2D->get_Count());
            AppendTopScreenCameraTraceLine(line);
        }
        if (!Skip2DCameraTraversalForDiagnostics) {
            Draw2DCameraList(cameras, renderManager2D);
        }
        PresentFirstFrameDrawStageMarker(RGB15(0, 31, 0) | BIT(15));
        Last2DTraversalMilliseconds = ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - traversalStartTimingTicks);
        std::size_t after2DTraversalAllocatedByteTotal = NintendoDsAllocationDiagnostics::GetTotalAllocatedSize();
        std::size_t after2DTraversalFreedByteTotal = NintendoDsAllocationDiagnostics::GetTotalFreedSize();
        Last2DTraversalNetByteDelta = static_cast<int32_t>(
            (after2DTraversalAllocatedByteTotal - initialAllocatedByteTotal)
            - (after2DTraversalFreedByteTotal - initialFreedByteTotal));
        if (hardware3DScreenTarget == NintendoDsScreenTarget::None) {
            lcdMainOnTop();
            vramSetBankA(VRAM_A_MAIN_BG);
            videoSetMode(MODE_0_2D | DISPLAY_BG0_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_LAYOUT | DISPLAY_SPR_EXT_PALETTE);
            LastConfiguredHardware3DScreenTarget = NintendoDsScreenTarget::None;
            renderManager2D->PresentBottomScreenFrame();
            LastPresentMilliseconds = 0.0;
            PublishPerformanceOverlayMetrics(core, renderManager2D, true);
            FirstFrameDrawStageMarkersCompleted = true;
            return;
        }

        PresentFirstFrameDrawStageMarker(RGB15(0, 31, 31) | BIT(15));
        uint32_t setupStartTimingTicks = cpuGetTiming();
        LastHardware3DScreenTarget = hardware3DScreenTarget;
        renderManager2D->SetHardware3DScreenTarget(hardware3DScreenTarget);
        ResolveFrameLighting(objectManager);
        EnsureHardwareInitialized();
        ConfigureHardware3DTarget(hardware3DScreenTarget, renderManager2D);
        PresentFirstFrameDrawStageMarker(RGB15(0, 0, 31) | BIT(15));
        for (int32_t cameraIndex = 0; cameraIndex < cameras->Count(); cameraIndex++) {
            ICamera* camera = (*cameras)[cameraIndex];
            if (camera == nullptr || ResolveCameraScreenTarget(camera) != hardware3DScreenTarget) {
                continue;
            }

            IRenderQueue3D* renderQueue3D = camera->get_RenderQueue3D();
            if (renderQueue3D == nullptr || renderQueue3D->get_Count() <= 0) {
                continue;
            }

            LastCamera3DQueueCount = renderQueue3D->get_Count();
            ClearFromCamera(camera);
            ConfigureCamera(camera);
            ConfigureFrameHardwareLight();
            Last3DSetupMilliseconds = ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - setupStartTimingTicks);
            LastSubmittedDrawableCount = DrawRenderQueue(camera);
            PresentFirstFrameDrawStageMarker(RGB15(31, 0, 31) | BIT(15));
            break;
        }
        renderManager2D->PresentBottomScreenFrame();
        LastPresentMilliseconds = 0.0;
        PublishPerformanceOverlayMetrics(core, renderManager2D, true);
        FirstFrameDrawStageMarkersCompleted = true;
        return;
    }

    /// Initializes Nintendo DS 3D video mode and hardware state before the first frame.
    void NintendoDsRenderManager3D::EnsureHardwareInitialized() {
        if (HardwareInitialized) {
            return;
        }

        glInit();
        vramSetBankA(VRAM_A_TEXTURE);
        vramSetBankB(VRAM_B_TEXTURE);
        glViewport(0, 0, 255, 191);
        glClearPolyID(63);
        HardwareInitialized = true;
    }

    /// Clears the top-screen 3D frame from one runtime camera clear configuration.
    /// <param name="camera">Runtime camera providing the clear configuration.</param>
    void NintendoDsRenderManager3D::ClearFromCamera(ICamera* camera) {
        if (camera == nullptr) {
            throw new ArgumentNullException("camera");
        }

        CameraClearSettings clearSettings = camera->get_ClearSettings();
        uint16_t clearColor = DefaultClearColor;
        if (clearSettings.get_ClearColorEnabled()) {
            clearColor = NintendoDsColorPacker::PackOpaqueColor(clearSettings.get_ClearColor());
        }

        glClearColor(
            clearColor & 31,
            (clearColor >> 5) & 31,
            (clearColor >> 10) & 31,
            31);
        glClearDepth(0x7FFF);
    }

    /// Configures the DS projection and view state for one runtime camera.
    /// <param name="camera">Runtime camera providing the active view.</param>
    void NintendoDsRenderManager3D::ConfigureCamera(ICamera* camera) {
        if (camera == nullptr) {
            throw new ArgumentNullException("camera");
        }

        Entity* cameraEntity = camera->get_Parent();
        float3 cameraPosition = float3::get_Zero();
        float4 cameraOrientation = float4::get_Identity();
        if (cameraEntity != nullptr) {
            cameraPosition = cameraEntity->get_Position();
            cameraOrientation = cameraEntity->get_Orientation();
        }

        float3 forward = float4::RotateVector(float3(0.0f, 0.0f, -1.0f), cameraOrientation);
        float3 up = float4::RotateVector(float3(0.0f, 1.0f, 0.0f), cameraOrientation);
        float3 lookAt = cameraPosition + forward;
        float nearPlaneDistance = camera->get_NearPlaneDistance();
        float farPlaneDistance = camera->get_FarPlaneDistance();

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(DefaultPerspectiveFieldOfViewDegrees, 256.0f / 192.0f, nearPlaneDistance, farPlaneDistance);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(
            cameraPosition.X,
            cameraPosition.Y,
            cameraPosition.Z,
            lookAt.X,
            lookAt.Y,
            lookAt.Z,
            up.X,
            up.Y,
            up.Z);
    }

    /// Draws the first-camera ordered 3D render queue through the DS geometry path.
    /// <param name="camera">Runtime camera owning the ordered 3D render queue.</param>
    int32_t NintendoDsRenderManager3D::DrawRenderQueue(ICamera* camera) {
        if (camera == nullptr) {
            throw new ArgumentNullException("camera");
        }

        IRenderQueue3D* renderQueue = camera->get_RenderQueue3D();
        if (renderQueue == nullptr) {
            return 0;
        }

        uint32_t queueSnapshotStartTimingTicks = cpuGetTiming();
        RenderQueueSnapshotVisitor->Clear();
        renderQueue->VisitOrdered(RenderQueueSnapshotVisitor);
        List<IDrawable3D*>* drawables = RenderQueueSnapshotVisitor->get_Drawables();
        Last3DQueueSnapshotMilliseconds = ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - queueSnapshotStartTimingTicks);
        std::vector<NintendoDsRuntimeModelInstanceCountEntry> runtimeModelInstanceCounts;
        for (int32_t drawableIndex = 0; drawableIndex < drawables->Count(); drawableIndex++) {
            IDrawable3D* drawable = (*drawables)[drawableIndex];
            if (drawable == nullptr) {
                continue;
            }

            NintendoDsRuntimeModel* runtimeModel = ResolveRuntimeModel(drawable->get_Model());
            if (runtimeModel == nullptr) {
                continue;
            }

            IncrementRuntimeModelInstanceCount(runtimeModelInstanceCounts, runtimeModel);
        }

        uint32_t geometryEmitStartTimingTicks = cpuGetTiming();
        int32_t submittedDrawables = 0;
        for (int32_t drawableIndex = 0; drawableIndex < drawables->Count(); drawableIndex++) {
            IDrawable3D* drawable = (*drawables)[drawableIndex];
            if (drawable == nullptr) {
                continue;
            }

            NintendoDsRuntimeModel* runtimeModel = ResolveRuntimeModel(drawable->get_Model());
            Array<RuntimeMaterial*>* runtimeMaterials = drawable->get_Materials();
            RuntimeMaterial* firstRuntimeMaterial = runtimeMaterials != nullptr && runtimeMaterials->get_Length() > 0
                ? (*runtimeMaterials)[0]
                : nullptr;
            NintendoDsRuntimeMaterial* runtimeMaterial = ResolveRuntimeMaterial(firstRuntimeMaterial);
            if (runtimeModel == nullptr || runtimeMaterial == nullptr) {
                continue;
            }

            if (!runtimeMaterial->SupportsGeometrySubmission) {
                continue;
            }

            int32_t runtimeModelInstanceCount = ResolveRuntimeModelInstanceCount(runtimeModelInstanceCounts, runtimeModel);

            SubmitOpaqueDrawable(drawable, runtimeModel, runtimeMaterial, runtimeModelInstanceCount);
            submittedDrawables++;
        }
        Last3DGeometryEmitMilliseconds = ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - geometryEmitStartTimingTicks);

        uint32_t flushStartTimingTicks = cpuGetTiming();
        glFlush(0);
        Last3DFlushMilliseconds = ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - flushStartTimingTicks);
        return submittedDrawables;
    }

    /// Submits one supported opaque drawable through the DS triangle path.
    /// <param name="drawable">Runtime drawable to submit.</param>
    /// <param name="runtimeModel">DS runtime model carrying vertex data.</param>
    /// <param name="runtimeMaterial">DS runtime material carrying the flat color.</param>
    /// <param name="runtimeModelInstanceCount">Number of times the same runtime model appears in the current queue snapshot.</param>
    void NintendoDsRenderManager3D::SubmitOpaqueDrawable(
        IDrawable3D* drawable,
        NintendoDsRuntimeModel* runtimeModel,
        NintendoDsRuntimeMaterial* runtimeMaterial,
        int32_t runtimeModelInstanceCount) {
        if (drawable == nullptr) {
            throw new ArgumentNullException("drawable");
        } else if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        } else if (runtimeMaterial == nullptr) {
            throw new ArgumentNullException("runtimeMaterial");
        } else if (runtimeModelInstanceCount <= 0) {
            throw new ArgumentOutOfRangeException("runtimeModelInstanceCount");
        } else if (runtimeModel->HardwareLitDisplayList == nullptr && (runtimeModel->Positions == nullptr || runtimeModel->Positions->Length <= 0)) {
            return;
        }

        uint32_t transformStartTimingTicks = cpuGetTiming();
        Entity* entity = drawable->get_Parent();
        float3 entityPosition = float3::get_Zero();
        float3 entityScale = float3::get_One();
        float4 entityOrientation = float4::get_Identity();
        if (entity != nullptr) {
            entityPosition = entity->get_Position();
            entityScale = entity->get_Scale();
            entityOrientation = entity->get_Orientation();
        }

        glPushMatrix();
        ApplyDrawableTransformToHardwareMatrix(entityPosition, entityScale, entityOrientation);
        Last3DTransformMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - transformStartTimingTicks);

        uint32_t materialStartTimingTicks = cpuGetTiming();
        ConfigureHardwareMaterial(runtimeMaterial);
        Array<float3>* positions = runtimeModel->Positions;
        Array<float2>* texCoords = runtimeModel->TexCoords;
        NintendoDsRuntimeTexture2D* hardwareTexture = nullptr;
        bool useHardwareTexture = positions != nullptr
            && texCoords != nullptr
            && texCoords->Length >= positions->Length
            && TryConfigureHardwareTexture(runtimeMaterial, hardwareTexture);
        if (useHardwareTexture) {
            EnsureHardwareTexturedDisplayList(runtimeModel, hardwareTexture);
        }

        Last3DMaterialMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - materialStartTimingTicks);

        if (ShouldUseStaticHardwareDisplayList(runtimeModel, useHardwareTexture, runtimeModelInstanceCount)) {
            uint32_t displayListStartTimingTicks = cpuGetTiming();
            SubmitStaticHardwareDisplayList(runtimeModel, useHardwareTexture);
            Last3DDisplayListMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - displayListStartTimingTicks);
            glPopMatrix(1);
            return;
        }

        uint32_t fallbackGeometryStartTimingTicks = cpuGetTiming();
        glBegin(GL_TRIANGLES);

        if (runtimeModel->Uses32BitIndices && runtimeModel->Indices32 != nullptr) {
            for (int32_t index = 0; index + 2 < runtimeModel->Indices32->Length; index += 3) {
                int32_t indexA = static_cast<int32_t>((*runtimeModel->Indices32)[index]);
                int32_t indexB = static_cast<int32_t>((*runtimeModel->Indices32)[index + 1]);
                int32_t indexC = static_cast<int32_t>((*runtimeModel->Indices32)[index + 2]);
                if (useHardwareTexture) {
                    SubmitHardwareTexturedTriangle(positions, texCoords, hardwareTexture, runtimeMaterial->LightingEnabled, indexA, indexB, indexC);
                } else {
                    SubmitHardwareLitTriangle(positions, indexA, indexB, indexC);
                }
            }
        } else if (runtimeModel->Indices16 != nullptr) {
            for (int32_t index = 0; index + 2 < runtimeModel->Indices16->Length; index += 3) {
                int32_t indexA = static_cast<int32_t>((*runtimeModel->Indices16)[index]);
                int32_t indexB = static_cast<int32_t>((*runtimeModel->Indices16)[index + 1]);
                int32_t indexC = static_cast<int32_t>((*runtimeModel->Indices16)[index + 2]);
                if (useHardwareTexture) {
                    SubmitHardwareTexturedTriangle(positions, texCoords, hardwareTexture, runtimeMaterial->LightingEnabled, indexA, indexB, indexC);
                } else {
                    SubmitHardwareLitTriangle(positions, indexA, indexB, indexC);
                }
            }
        } else {
            for (int32_t index = 0; index + 2 < positions->Length; index += 3) {
                if (useHardwareTexture) {
                    SubmitHardwareTexturedTriangle(positions, texCoords, hardwareTexture, runtimeMaterial->LightingEnabled, index, index + 1, index + 2);
                } else {
                    SubmitHardwareLitTriangle(positions, index, index + 1, index + 2);
                }
            }
        }

        glEnd();
        Last3DFallbackGeometryMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - fallbackGeometryStartTimingTicks);
        glPopMatrix(1);
    }

    /// Applies one drawable entity transform to the Nintendo DS model-view matrix before model-space vertices are submitted.
    void NintendoDsRenderManager3D::ApplyDrawableTransformToHardwareMatrix(
        const float3& entityPosition,
        const float3& entityScale,
        const float4& entityOrientation) {
        m4x3 transformMatrix;
        BuildDrawableTransformMatrix(transformMatrix, entityPosition, entityScale, entityOrientation);
        glMultMatrix4x3(&transformMatrix);
    }

    /// Builds one fixed-point Nintendo DS 4x3 affine transform matrix from runtime entity transform components.
    void NintendoDsRenderManager3D::BuildDrawableTransformMatrix(
        m4x3& transformMatrix,
        const float3& entityPosition,
        const float3& entityScale,
        const float4& entityOrientation) const {
        float x = entityOrientation.X;
        float y = entityOrientation.Y;
        float z = entityOrientation.Z;
        float w = entityOrientation.W;
        float doubledX = x + x;
        float doubledY = y + y;
        float doubledZ = z + z;
        float xx = x * doubledX;
        float xy = x * doubledY;
        float xz = x * doubledZ;
        float yy = y * doubledY;
        float yz = y * doubledZ;
        float zz = z * doubledZ;
        float wx = w * doubledX;
        float wy = w * doubledY;
        float wz = w * doubledZ;

        transformMatrix.m[0] = floattof32((1.0f - yy - zz) * entityScale.X);
        transformMatrix.m[1] = floattof32((xy + wz) * entityScale.X);
        transformMatrix.m[2] = floattof32((xz - wy) * entityScale.X);
        transformMatrix.m[3] = floattof32((xy - wz) * entityScale.Y);
        transformMatrix.m[4] = floattof32((1.0f - xx - zz) * entityScale.Y);
        transformMatrix.m[5] = floattof32((yz + wx) * entityScale.Y);
        transformMatrix.m[6] = floattof32((xz + wy) * entityScale.Z);
        transformMatrix.m[7] = floattof32((yz - wx) * entityScale.Z);
        transformMatrix.m[8] = floattof32((1.0f - xx - yy) * entityScale.Z);
        transformMatrix.m[9] = floattof32(entityPosition.X);
        transformMatrix.m[10] = floattof32(entityPosition.Y);
        transformMatrix.m[11] = floattof32(entityPosition.Z);
    }

    /// Configures Nintendo DS fixed-function frame light state from the active camera/view matrix.
    void NintendoDsRenderManager3D::ConfigureFrameHardwareLight() {
        uint16_t packedDirectionalLight = NintendoDsColorPacker::PackRegisterColor(NintendoDsLightingMath::ClampColor(FrameDirectionalRadiance));

        glLight(0,
            packedDirectionalLight,
            PackHardwareNormalComponent(FrameLightDirection.X),
            PackHardwareNormalComponent(FrameLightDirection.Y),
            PackHardwareNormalComponent(FrameLightDirection.Z));
        glMaterialf(GL_SPECULAR, 0);
        glMaterialf(GL_EMISSION, 0);
        ApplyHardwarePolyFormat(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0);
    }

    /// Invalidates cached DS hardware state so the next draw re-emits any required material and texture registers.
    void NintendoDsRenderManager3D::InvalidateHardwareStateCache() {
        CachedHardwareTextureEnabledValid = false;
        CachedHardwareTextureEnabled = false;
        CachedHardwareTextureIdValid = false;
        CachedHardwareTextureId = -1;
        CachedHardwarePolyFormatValid = false;
        CachedHardwarePolyFormat = 0;
        CachedHardwareAmbientMaterialValid = false;
        CachedHardwareAmbientMaterial = 0;
        CachedHardwareDiffuseMaterialValid = false;
        CachedHardwareDiffuseMaterial = 0;
        CachedHardwareVertexColorValid = false;
        CachedHardwareVertexColor = 0;
    }

    /// Applies one DS polygon-format register value only when it differs from the cached hardware state.
    void NintendoDsRenderManager3D::ApplyHardwarePolyFormat(uint32_t polyFormat) {
        if (!CachedHardwarePolyFormatValid || CachedHardwarePolyFormat != polyFormat) {
            glPolyFmt(polyFormat);
            CachedHardwarePolyFormat = polyFormat;
            CachedHardwarePolyFormatValid = true;
        }
    }

    /// Applies one DS ambient-material register value only when it differs from the cached hardware state.
    void NintendoDsRenderManager3D::ApplyHardwareAmbientMaterial(uint16_t packedAmbientMaterial) {
        if (!CachedHardwareAmbientMaterialValid || CachedHardwareAmbientMaterial != packedAmbientMaterial) {
            glMaterialf(GL_AMBIENT, packedAmbientMaterial);
            CachedHardwareAmbientMaterial = packedAmbientMaterial;
            CachedHardwareAmbientMaterialValid = true;
        }
    }

    /// Applies one DS diffuse-material register value only when it differs from the cached hardware state.
    void NintendoDsRenderManager3D::ApplyHardwareDiffuseMaterial(uint16_t packedDiffuseMaterial) {
        if (!CachedHardwareDiffuseMaterialValid || CachedHardwareDiffuseMaterial != packedDiffuseMaterial) {
            glMaterialf(GL_DIFFUSE, packedDiffuseMaterial);
            CachedHardwareDiffuseMaterial = packedDiffuseMaterial;
            CachedHardwareDiffuseMaterialValid = true;
        }
    }

    /// Applies one DS unlit vertex-color register value only when it differs from the cached hardware state.
    void NintendoDsRenderManager3D::ApplyHardwareVertexColor(uint16_t packedVertexColor) {
        if (!CachedHardwareVertexColorValid || CachedHardwareVertexColor != packedVertexColor) {
            glColor(packedVertexColor);
            CachedHardwareVertexColor = packedVertexColor;
            CachedHardwareVertexColorValid = true;
        }
    }

    /// Applies one DS texture-enable state only when it differs from the cached hardware state.
    void NintendoDsRenderManager3D::ApplyHardwareTextureEnabledState(bool enabled) {
        if (!CachedHardwareTextureEnabledValid || CachedHardwareTextureEnabled != enabled) {
            if (enabled) {
                glEnable(GL_TEXTURE_2D);
            } else {
                glDisable(GL_TEXTURE_2D);
            }

            CachedHardwareTextureEnabled = enabled;
            CachedHardwareTextureEnabledValid = true;
        }
    }

    /// Binds one DS texture id only when it differs from the cached hardware state.
    void NintendoDsRenderManager3D::ApplyHardwareTextureBinding(int32_t hardwareTextureId) {
        if (hardwareTextureId < 0) {
            throw new ArgumentOutOfRangeException("hardwareTextureId");
        }

        if (!CachedHardwareTextureIdValid || CachedHardwareTextureId != hardwareTextureId) {
            uint32_t bindStartTimingTicks = cpuGetTiming();
            glBindTexture(0, hardwareTextureId);
            CachedHardwareTextureId = hardwareTextureId;
            CachedHardwareTextureIdValid = true;
            Last3DTextureBindMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - bindStartTimingTicks);
        }
    }

    /// Configures Nintendo DS fixed-function material state for one lit drawable.
    void NintendoDsRenderManager3D::ConfigureHardwareMaterial(NintendoDsRuntimeMaterial* runtimeMaterial) {
        if (runtimeMaterial == nullptr) {
            throw new ArgumentNullException("runtimeMaterial");
        }

        float3 ambientMaterial = NintendoDsLightingMath::MultiplyColor(runtimeMaterial->BaseColor, NintendoDsLightingMath::ClampColor(FrameAmbientRadiance));
        uint16_t packedAmbientMaterial = NintendoDsColorPacker::PackRegisterColor(ambientMaterial);
        uint16_t packedDiffuseMaterial = NintendoDsColorPacker::PackRegisterColor(runtimeMaterial->BaseColor);

        if (runtimeMaterial->LightingEnabled) {
            ApplyHardwarePolyFormat(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0);
        } else {
            ApplyHardwarePolyFormat(POLY_ALPHA(31) | POLY_CULL_BACK);
            ApplyHardwareVertexColor(RGB15(31, 31, 31) | BIT(15));
        }

        ApplyHardwareAmbientMaterial(packedAmbientMaterial);
        ApplyHardwareDiffuseMaterial(packedDiffuseMaterial);
    }

    /// Configures a DS hardware texture for one material when a runtime texture is bound.
    bool NintendoDsRenderManager3D::TryConfigureHardwareTexture(NintendoDsRuntimeMaterial* runtimeMaterial, NintendoDsRuntimeTexture2D*& runtimeTexture) {
        if (runtimeMaterial == nullptr) {
            throw new ArgumentNullException("runtimeMaterial");
        }

        uint32_t configureStartTimingTicks = cpuGetTiming();
        runtimeTexture = ResolveRuntimeTexture(runtimeMaterial->ResolvePrimaryTexture());
        if (runtimeTexture == nullptr) {
            ApplyHardwareTextureEnabledState(false);
            Last3DTextureConfigureMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - configureStartTimingTicks);
            return false;
        }

        RecordHardwareTextureDiagnostics(runtimeTexture, false);
        EnsureHardwareTextureUploaded(runtimeTexture);
        ApplyHardwareTextureEnabledState(true);
        Last3DTextureConfigureMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - configureStartTimingTicks);
        ApplyHardwareTextureBinding(runtimeTexture->HardwareTextureId);
        return true;
    }

    /// Uploads one runtime texture into DS texture VRAM if it has not already been uploaded.
    void NintendoDsRenderManager3D::EnsureHardwareTextureUploaded(NintendoDsRuntimeTexture2D* runtimeTexture) {
        if (runtimeTexture == nullptr) {
            throw new ArgumentNullException("runtimeTexture");
        } else if (runtimeTexture->HardwareTextureUploaded) {
            RecordHardwareTextureDiagnostics(runtimeTexture, false);
            return;
        } else if (runtimeTexture->Colors == nullptr || runtimeTexture->Colors->Data == nullptr) {
            throw new InvalidOperationException("Nintendo DS hardware texture upload requires a runtime texture color payload.");
        }

        int32_t textureWidth = runtimeTexture->get_Width();
        int32_t textureHeight = runtimeTexture->get_Height();
        std::vector<uint16_t> hardwarePixels = BuildHardwareTexturePixels(runtimeTexture);
        DC_FlushRange(hardwarePixels.data(), hardwarePixels.size() * sizeof(uint16_t));
        glGenTextures(1, &runtimeTexture->HardwareTextureId);
        glBindTexture(0, runtimeTexture->HardwareTextureId);
        int32_t uploadResult = glTexImage2D(
            0,
            0,
            GL_RGB,
            ResolveHardwareTextureSize(textureWidth),
            ResolveHardwareTextureSize(textureHeight),
            0,
            TEXGEN_OFF,
            reinterpret_cast<const uint8_t*>(hardwarePixels.data()));
        if (uploadResult == 0) {
            RecordHardwareTextureDiagnostics(runtimeTexture, true);
            throw new InvalidOperationException("Nintendo DS hardware texture upload failed.");
        }

        runtimeTexture->HardwareTextureUploaded = true;
        RecordHardwareTextureDiagnostics(runtimeTexture, true);
    }

    /// Builds one temporary DS direct-color texture payload from the cooked runtime texture.
    std::vector<uint16_t> NintendoDsRenderManager3D::BuildHardwareTexturePixels(NintendoDsRuntimeTexture2D* runtimeTexture) const {
        if (runtimeTexture == nullptr) {
            throw new ArgumentNullException("runtimeTexture");
        }

        int32_t textureWidth = runtimeTexture->get_Width();
        int32_t textureHeight = runtimeTexture->get_Height();
        if (textureWidth <= 0 || textureHeight <= 0) {
            throw new InvalidOperationException("Nintendo DS hardware texture upload requires positive texture dimensions.");
        }

        if (ForceHardwareTextureDiagnosticPattern) {
            return BuildHardwareTextureDiagnosticPixels(textureWidth, textureHeight);
        }

        std::vector<uint16_t> hardwarePixels(static_cast<std::size_t>(textureWidth * textureHeight), 0);
        if (runtimeTexture->ColorFormat == TextureAssetColorFormat::Rgba32) {
            for (int32_t pixelIndex = 0; pixelIndex < textureWidth * textureHeight; pixelIndex++) {
                int32_t sourceIndex = pixelIndex * 4;
                hardwarePixels[static_cast<std::size_t>(pixelIndex)] = PackHardwareTexturePixel(
                    runtimeTexture->Colors->Data[sourceIndex],
                    runtimeTexture->Colors->Data[sourceIndex + 1],
                    runtimeTexture->Colors->Data[sourceIndex + 2],
                    runtimeTexture->Colors->Data[sourceIndex + 3]);
            }

            return hardwarePixels;
        }

        if (runtimeTexture->ColorFormat == TextureAssetColorFormat::Rgba4444) {
            for (int32_t pixelIndex = 0; pixelIndex < textureWidth * textureHeight; pixelIndex++) {
                int32_t sourceIndex = pixelIndex * 2;
                uint16_t packedColor = static_cast<uint16_t>(runtimeTexture->Colors->Data[sourceIndex] | (runtimeTexture->Colors->Data[sourceIndex + 1] << 8));
                uint8_t red = static_cast<uint8_t>(((packedColor >> 0) & 15) * 17);
                uint8_t green = static_cast<uint8_t>(((packedColor >> 4) & 15) * 17);
                uint8_t blue = static_cast<uint8_t>(((packedColor >> 8) & 15) * 17);
                uint8_t alpha = static_cast<uint8_t>(((packedColor >> 12) & 15) * 17);
                hardwarePixels[static_cast<std::size_t>(pixelIndex)] = PackHardwareTexturePixel(red, green, blue, alpha);
            }

            return hardwarePixels;
        }

        if (runtimeTexture->ColorFormat == TextureAssetColorFormat::Indexed4 || runtimeTexture->ColorFormat == TextureAssetColorFormat::Indexed8) {
            if (runtimeTexture->PaletteColors == nullptr || runtimeTexture->PaletteColors->Data == nullptr) {
                throw new InvalidOperationException("Nintendo DS indexed hardware texture upload requires a runtime texture palette payload.");
            }

            for (int32_t pixelIndex = 0; pixelIndex < textureWidth * textureHeight; pixelIndex++) {
                uint8_t paletteIndex;
                if (runtimeTexture->ColorFormat == TextureAssetColorFormat::Indexed4) {
                    uint8_t packedIndices = runtimeTexture->Colors->Data[pixelIndex / 2];
                    paletteIndex = (pixelIndex & 1) == 0
                        ? static_cast<uint8_t>(packedIndices & 15)
                        : static_cast<uint8_t>((packedIndices >> 4) & 15);
                } else {
                    paletteIndex = runtimeTexture->Colors->Data[pixelIndex];
                }

                int32_t paletteOffset = static_cast<int32_t>(paletteIndex) * 4;
                if (paletteOffset < 0 || paletteOffset + 3 >= runtimeTexture->PaletteColors->Length) {
                    throw new InvalidOperationException("Nintendo DS indexed hardware texture upload read beyond the cooked palette payload.");
                }

                hardwarePixels[static_cast<std::size_t>(pixelIndex)] = PackHardwareTexturePixel(
                    runtimeTexture->PaletteColors->Data[paletteOffset],
                    runtimeTexture->PaletteColors->Data[paletteOffset + 1],
                    runtimeTexture->PaletteColors->Data[paletteOffset + 2],
                    runtimeTexture->PaletteColors->Data[paletteOffset + 3]);
            }

            return hardwarePixels;
        }

        throw new InvalidOperationException("Nintendo DS hardware texture upload encountered an unsupported runtime texture format.");
    }

    /// Converts one RGBA texel into the DS direct-color texture representation.
    uint16_t NintendoDsRenderManager3D::PackHardwareTexturePixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) const {
        if (alpha < 128) {
            return 0;
        }

        return static_cast<uint16_t>(
            BIT(15)
            | ((red >> 3) & 31)
            | (((green >> 3) & 31) << 5)
            | (((blue >> 3) & 31) << 10));
    }

    /// Resolves the libnds texture-size enum for one supported power-of-two dimension.
    int32_t NintendoDsRenderManager3D::ResolveHardwareTextureSize(int32_t size) const {
        if (size == 8) {
            return TEXTURE_SIZE_8;
        } else if (size == 16) {
            return TEXTURE_SIZE_16;
        } else if (size == 32) {
            return TEXTURE_SIZE_32;
        } else if (size == 64) {
            return TEXTURE_SIZE_64;
        } else if (size == 128) {
            return TEXTURE_SIZE_128;
        } else if (size == 256) {
            return TEXTURE_SIZE_256;
        } else if (size == 512) {
            return TEXTURE_SIZE_512;
        } else if (size == 1024) {
            return TEXTURE_SIZE_1024;
        }

        throw new InvalidOperationException("Nintendo DS hardware textures require power-of-two dimensions from 8 to 1024 pixels.");
    }

    /// Packs one normalized model-space normal component into the signed 10-bit DS normal range.
    int32_t NintendoDsRenderManager3D::PackHardwareNormalComponent(float value) const {
        float clampedValue = std::clamp(value, -1.0f, 1.0f);
        return static_cast<int32_t>(clampedValue * 511.0f);
    }

    /// Packs one model-space coordinate into the signed 10-bit DS VTX10 range.
    int32_t NintendoDsRenderManager3D::PackHardwareVertex10Component(float value) const {
        float clampedValue = std::clamp(value, -8.0f, 7.984375f);
        return static_cast<int32_t>(std::round(clampedValue * 64.0f));
    }

    /// Resolves frame lighting once from runtime-managed light collections before drawable submission begins.
    void NintendoDsRenderManager3D::ResolveFrameLighting(ObjectManager* objectManager) {
        FrameLightDirection = float3(0.0f, -1.0f, 0.0f);
        FrameDirectionalRadiance = float3(0.0f, 0.0f, 0.0f);
        FrameAmbientRadiance = float3(0.0f, 0.0f, 0.0f);

        if (objectManager == nullptr) {
            return;
        }

        List<AmbientLightComponent*>* ambientLights = objectManager->get_AmbientLights();
        if (ambientLights != nullptr) {
            for (int32_t lightIndex = 0; lightIndex < ambientLights->Count(); lightIndex++) {
                AmbientLightComponent* ambientLight = (*ambientLights)[lightIndex];
                if (ambientLight == nullptr) {
                    continue;
                }

                float4 color = ambientLight->get_Color();
                FrameAmbientRadiance = FrameAmbientRadiance + float3(
                    color.X * ambientLight->get_Intensity(),
                    color.Y * ambientLight->get_Intensity(),
                    color.Z * ambientLight->get_Intensity());
            }
        }

        List<DirectionalLightComponent*>* directionalLights = objectManager->get_DirectionalLights();
        if (directionalLights == nullptr || directionalLights->Count() <= 0) {
            return;
        }

        DirectionalLightComponent* directionalLight = (*directionalLights)[0];
        if (directionalLight == nullptr) {
            return;
        }

        float4 color = directionalLight->get_Color();
        FrameLightDirection = LightDirectionUtility::GetLightDirection(directionalLight);
        FrameDirectionalRadiance = float3(
            color.X * directionalLight->get_Intensity(),
            color.Y * directionalLight->get_Intensity(),
            color.Z * directionalLight->get_Intensity());
    }

    /// Publishes the latest Nintendo DS renderer timing buckets through the generated-core performance overlay contract.
    void NintendoDsRenderManager3D::PublishPerformanceOverlayMetrics(Core* core, NintendoDsRenderManager2D* renderManager2D, bool usesMetrics) {
        if (core == nullptr) {
            throw new ArgumentNullException("core");
        } else if (renderManager2D == nullptr) {
            throw new ArgumentNullException("renderManager2D");
        }

        NintendoDsRenderManager2DProfileSnapshot profileSnapshot = renderManager2D->get_ProfileSnapshot();

        core->SetPerformanceOverlayMetrics(
            usesMetrics,
            usesMetrics ? profileSnapshot.TextMilliseconds : 0.0,
            usesMetrics ? profileSnapshot.SpriteMilliseconds : 0.0,
            usesMetrics ? static_cast<double>(profileSnapshot.UnsupportedRoundedRectPrimitiveCount) : 0.0,
            usesMetrics ? Last3DGeometryEmitMilliseconds : 0.0,
            usesMetrics ? Last3DFlushMilliseconds : 0.0,
            usesMetrics ? LastPresentMilliseconds : 0.0,
            usesMetrics ? profileSnapshot.UnsupportedTextPrimitiveCount : 0,
            usesMetrics ? profileSnapshot.UnsupportedSpritePrimitiveCount : 0);
        core->SetPerformanceOverlayTextRows(
            usesMetrics,
            std::string(),
            std::string(),
            usesMetrics ? FormatPerformanceOverlayDetailText() : std::string(),
            usesMetrics ? FormatPerformanceOverlayAdditionalText(profileSnapshot) : std::string());
    }

    /// Formats the compact DS trace row that summarizes queue counts and top-level timing buckets.
    std::string NintendoDsRenderManager3D::FormatPerformanceOverlayDetailText() const {
        return std::string("Q3D ")
            + std::to_string(LastCamera3DQueueCount)
            + " Sub " + std::to_string(LastSubmittedDrawableCount)
            + " 2D " + FormatDebugMilliseconds(Last2DTraversalMilliseconds);
    }

    /// Formats the DS multi-line trace block that expands deeper geometry-submission timings.
    std::string NintendoDsRenderManager3D::FormatPerformanceOverlayAdditionalText(const NintendoDsRenderManager2DProfileSnapshot& profileSnapshot) const {
        double cameraOverheadMilliseconds = std::max(
            0.0,
            profileSnapshot.CameraMilliseconds
                - profileSnapshot.TextMilliseconds
                - profileSnapshot.SpriteMilliseconds
                - profileSnapshot.RoundedRectMilliseconds
                - profileSnapshot.ClearMilliseconds);
        return std::string("Txt ")
            + FormatDebugMilliseconds(profileSnapshot.TextMilliseconds)
            + " Spr " + FormatDebugMilliseconds(profileSnapshot.SpriteMilliseconds)
            + " Cl " + FormatDebugMilliseconds(profileSnapshot.ClearMilliseconds)
            + "\nCam " + FormatDebugMilliseconds(profileSnapshot.CameraMilliseconds)
            + " Ovh " + FormatDebugMilliseconds(cameraOverheadMilliseconds)
            + " TxCfg " + FormatDebugMilliseconds(Last3DTextureConfigureMilliseconds)
            + " TxEns " + FormatDebugMilliseconds(Last3DTexturedDisplayListEnsureMilliseconds)
            + " B" + std::to_string(Last3DTexturedDisplayListBuildCount)
            + " R" + std::to_string(Last3DTexturedDisplayListReuseCount)
            + "\nSet "
            + FormatDebugMilliseconds(Last3DSetupMilliseconds)
            + " Geo " + FormatDebugMilliseconds(Last3DGeometryEmitMilliseconds)
            + " Fl " + FormatDebugMilliseconds(Last3DFlushMilliseconds)
            + "\nDL " + FormatDebugMilliseconds(Last3DDisplayListMilliseconds)
            + " Pre " + FormatDebugMilliseconds(Last3DDisplayListPreWaitMilliseconds)
            + " K " + FormatDebugMilliseconds(Last3DDisplayListKickMilliseconds)
            + " TxB " + FormatDebugMilliseconds(Last3DTextureBindMilliseconds)
            + " P " + FormatDebugMilliseconds(Last3DDisplayListPostWaitMilliseconds);
    }

    /// Captures compact diagnostics for the most recent runtime texture considered by the 3D hardware path.
    void NintendoDsRenderManager3D::RecordHardwareTextureDiagnostics(NintendoDsRuntimeTexture2D* runtimeTexture, bool uploadAttempted) {
        LastHardwareTextureMaterialBound = runtimeTexture != nullptr;
        LastHardwareTextureUploadAttempted = uploadAttempted;
        if (runtimeTexture == nullptr) {
            LastHardwareTextureUploaded = false;
            LastHardwareTextureId = -1;
            LastHardwareTextureWidth = 0;
            LastHardwareTextureHeight = 0;
            LastHardwareTextureColorLength = 0;
            LastHardwareTexturePaletteColorLength = 0;
            LastHardwareTextureFormat = "none";
            return;
        }

        LastHardwareTextureUploaded = runtimeTexture->HardwareTextureUploaded;
        LastHardwareTextureId = runtimeTexture->HardwareTextureId;
        LastHardwareTextureWidth = runtimeTexture->get_Width();
        LastHardwareTextureHeight = runtimeTexture->get_Height();
        LastHardwareTextureColorLength = runtimeTexture->Colors == nullptr ? 0 : runtimeTexture->Colors->Length;
        LastHardwareTexturePaletteColorLength = runtimeTexture->PaletteColors == nullptr ? 0 : runtimeTexture->PaletteColors->Length;
        LastHardwareTextureFormat = FormatTextureColorFormat(runtimeTexture->ColorFormat);
    }

    /// Formats the latest hardware texture diagnostics for the native overlay.
    std::string NintendoDsRenderManager3D::FormatHardwareTextureDiagnostics() const {
        return std::string("D3T Tex")
            + (LastHardwareTextureMaterialBound ? "1" : "0")
            + " A" + (LastHardwareTextureUploadAttempted ? "1" : "0")
            + " U" + (LastHardwareTextureUploaded ? "1" : "0")
            + " I" + std::to_string(LastHardwareTextureId)
            + " " + std::to_string(LastHardwareTextureWidth)
            + "x" + std::to_string(LastHardwareTextureHeight)
            + " C" + std::to_string(LastHardwareTextureColorLength)
            + " F" + LastHardwareTextureFormat;
    }

    /// Formats the latest textured-lighting diagnostics for the native overlay.
    std::string NintendoDsRenderManager3D::FormatHardwareTextureLightingDiagnostics() const {
        return std::string("D3U Lit")
            + (LastHardwareTextureLightingEnabled ? "1" : "0")
            + " Tri" + std::to_string(LastHardwareTexturedTriangleCount)
            + " Max" + FormatDebugSignedUnit(LastHardwareTexturedMaxDiffuse);
    }

    /// Submits one quad normal and vertices through the DS fixed-function lighting path.
    void NintendoDsRenderManager3D::SubmitHardwareLitQuad(
        Array<float3>* positions,
        int32_t indexA,
        int32_t indexD,
        int32_t indexC,
        int32_t indexB) {
        if (positions == nullptr) {
            throw new ArgumentNullException("positions");
        } else if (indexA < 0 || indexB < 0 || indexC < 0 || indexD < 0) {
            return;
        } else if (indexA >= positions->Length || indexB >= positions->Length || indexC >= positions->Length || indexD >= positions->Length) {
            return;
        }

        float3 vertexA = (*positions)[indexA];
        float3 vertexD = (*positions)[indexD];
        float3 vertexC = (*positions)[indexC];
        float3 vertexB = (*positions)[indexB];
        float3 modelFaceNormal = NintendoDsLightingMath::ComputeTriangleNormal(vertexA, vertexD, vertexC);
        SubmitHardwareNormal(modelFaceNormal);
        glVertex3v16(floattov16(vertexA.X), floattov16(vertexA.Y), floattov16(vertexA.Z));
        glVertex3v16(floattov16(vertexD.X), floattov16(vertexD.Y), floattov16(vertexD.Z));
        glVertex3v16(floattov16(vertexC.X), floattov16(vertexC.Y), floattov16(vertexC.Z));
        glVertex3v16(floattov16(vertexB.X), floattov16(vertexB.Y), floattov16(vertexB.Z));
    }

    /// Submits one triangle normal and vertices through the DS fixed-function lighting path.
    void NintendoDsRenderManager3D::SubmitHardwareLitTriangle(
        Array<float3>* positions,
        int32_t indexA,
        int32_t indexB,
        int32_t indexC) {
        if (positions == nullptr) {
            throw new ArgumentNullException("positions");
        } else if (indexA < 0 || indexB < 0 || indexC < 0) {
            return;
        } else if (indexA >= positions->Length || indexB >= positions->Length || indexC >= positions->Length) {
            return;
        }

        float3 vertexA = (*positions)[indexA];
        float3 vertexB = (*positions)[indexB];
        float3 vertexC = (*positions)[indexC];
        float3 modelFaceNormal = NintendoDsLightingMath::ComputeTriangleNormal(vertexA, vertexB, vertexC);
        SubmitHardwareNormal(modelFaceNormal);
        glVertex3v16(floattov16(vertexA.X), floattov16(vertexA.Y), floattov16(vertexA.Z));
        glVertex3v16(floattov16(vertexB.X), floattov16(vertexB.Y), floattov16(vertexB.Z));
        glVertex3v16(floattov16(vertexC.X), floattov16(vertexC.Y), floattov16(vertexC.Z));
    }

    /// Submits one triangle normal, texture coordinates, and vertices through the DS fixed-function texturing path.
    void NintendoDsRenderManager3D::SubmitHardwareTexturedTriangle(
        Array<float3>* positions,
        Array<float2>* texCoords,
        NintendoDsRuntimeTexture2D* runtimeTexture,
        bool lightingEnabled,
        int32_t indexA,
        int32_t indexB,
        int32_t indexC) {
        if (positions == nullptr) {
            throw new ArgumentNullException("positions");
        } else if (texCoords == nullptr) {
            throw new ArgumentNullException("texCoords");
        } else if (runtimeTexture == nullptr) {
            throw new ArgumentNullException("runtimeTexture");
        } else if (indexA < 0 || indexB < 0 || indexC < 0) {
            return;
        } else if (indexA >= positions->Length || indexB >= positions->Length || indexC >= positions->Length) {
            return;
        } else if (indexA >= texCoords->Length || indexB >= texCoords->Length || indexC >= texCoords->Length) {
            return;
        }

        float3 vertexA = (*positions)[indexA];
        float3 vertexB = (*positions)[indexB];
        float3 vertexC = (*positions)[indexC];
        float3 modelFaceNormal = NintendoDsLightingMath::ComputeTriangleNormal(vertexA, vertexB, vertexC);
        LastHardwareTextureLightingEnabled = lightingEnabled;
        LastHardwareTexturedTriangleCount++;
        float expectedDiffuse = NintendoDsLightingMath::EvaluateDirectionalDiffuse(modelFaceNormal, FrameLightDirection);
        if (expectedDiffuse > LastHardwareTexturedMaxDiffuse) {
            LastHardwareTexturedMaxDiffuse = expectedDiffuse;
        }

        if (ForceHardwareTextureDiagnosticCoordinates) {
            glColor3b(255, 255, 255);
            if (lightingEnabled) {
                SubmitHardwareNormal(modelFaceNormal);
            }

            glTexCoord2t16(0, 0);
            glVertex3v16(floattov16(vertexA.X), floattov16(vertexA.Y), floattov16(vertexA.Z));
            glTexCoord2t16(inttot16(runtimeTexture->get_Width()), 0);
            glVertex3v16(floattov16(vertexB.X), floattov16(vertexB.Y), floattov16(vertexB.Z));
            glTexCoord2t16(0, inttot16(runtimeTexture->get_Height()));
            glVertex3v16(floattov16(vertexC.X), floattov16(vertexC.Y), floattov16(vertexC.Z));
            return;
        }

        if (lightingEnabled) {
            SubmitHardwareNormal(modelFaceNormal);
        }

        SubmitHardwareTexturedVertex(positions, texCoords, runtimeTexture, lightingEnabled, indexA);
        SubmitHardwareTexturedVertex(positions, texCoords, runtimeTexture, lightingEnabled, indexB);
        SubmitHardwareTexturedVertex(positions, texCoords, runtimeTexture, lightingEnabled, indexC);
    }

    /// Submits one authored or synthesized normal through the DS fixed-function lighting path.
    void NintendoDsRenderManager3D::SubmitHardwareNormal(const float3& normal) const {
        float3 normalizedNormal = float3::Normalize(normal);
        glNormal(NORMAL_PACK(
            PackHardwareNormalComponent(normalizedNormal.X),
            PackHardwareNormalComponent(normalizedNormal.Y),
            PackHardwareNormalComponent(normalizedNormal.Z)));
    }

    /// Submits one textured vertex with a normalized model UV converted to DS texture coordinates.
    void NintendoDsRenderManager3D::SubmitHardwareTexturedVertex(
        Array<float3>* positions,
            Array<float2>* texCoords,
            NintendoDsRuntimeTexture2D* runtimeTexture,
            bool lightingEnabled,
            int32_t index) {
        if (positions == nullptr) {
            throw new ArgumentNullException("positions");
        } else if (texCoords == nullptr) {
            throw new ArgumentNullException("texCoords");
        } else if (runtimeTexture == nullptr) {
            throw new ArgumentNullException("runtimeTexture");
        } else if (index < 0 || index >= positions->Length || index >= texCoords->Length) {
            return;
        }

        float2 texCoord = (*texCoords)[index];
        if (!lightingEnabled) {
            glColor3b(255, 255, 255);
        }

        glTexCoord2t16(
            floattot16(texCoord.X * static_cast<float>(runtimeTexture->get_Width())),
            floattot16(texCoord.Y * static_cast<float>(runtimeTexture->get_Height())));
        glVertex3v16(floattov16((*positions)[index].X), floattov16((*positions)[index].Y), floattov16((*positions)[index].Z));
    }
}
#endif
