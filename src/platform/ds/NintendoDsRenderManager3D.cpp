#include "platform/ds/NintendoDsRenderManager3D.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
extern "C" {
#include <nds/arm9/console.h>
#include <nds/system.h>
#include <nds/arm9/video.h>
#include <nds/arm9/videoGL.h>
#include <nds/arm9/trig_lut.h>
#include <nds/timers.h>
}

#include <algorithm>
#include <cmath>
#include <cstring>

#include "CameraClearSettings.hpp"
#include "Core.hpp"
#include "MaterialConstantBufferAsset.hpp"
#include "LightComponent.hpp"
#include "AmbientLightComponent.hpp"
#include "DirectionalLightComponent.hpp"
#include "Entity.hpp"
#include "ICamera.hpp"
#include "IDrawable2D.hpp"
#include "IDrawable3D.hpp"
#include "IRenderQueue2D.hpp"
#include "IRenderQueue3D.hpp"
#include "MaterialLayout.hpp"
#include "MaterialLayoutBinding.hpp"
#include "MaterialPropertyBlock.hpp"
#include "MaterialRenderState.hpp"
#include "LightDirectionUtility.hpp"
#include "ObjectManager.hpp"
#include "StandardMaterialTextureBindingDefaults.hpp"
#include "platform/ds/NintendoDsLightingMath.hpp"
#include "platform/ds/NintendoDsColorPacker.hpp"
#include "platform/ds/NintendoDsRenderManager2D.hpp"
#include "platform/ds/NintendoDsRenderQueueSnapshotVisitor.hpp"
#include "platform/ds/NintendoDsRuntimeMaterial.hpp"
#include "platform/ds/NintendoDsRuntimeModel.hpp"
#include "platform/ds/NintendoDsRuntimeTexture2D.hpp"
#include "platform/ds/NintendoDsAllocationDiagnostics.hpp"
#include "platform/ds/NintendoDsFramePacing.hpp"
#include "runtime/native_exceptions.hpp"

namespace helengine::ds {
    namespace {
        /// Stores the standard top-screen clear color for DS 3D output.
        constexpr uint16_t DefaultClearColor = 0x0000;

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
        , LastTopScreen2DQueueCount(0)
        , LastBottomScreen2DQueueCount(0)
        , Last2DTraversalNetByteDelta(0)
        , Last3DSubmissionNetByteDelta(0)
        , LastPresentNetByteDelta(0)
        , LastReleaseMaterialNetByteDelta(0)
        , LastReleaseModelNetByteDelta(0)
        , Last2DTraversalMilliseconds(0.0)
        , Last3DSetupMilliseconds(0.0)
        , Last3DQueueSnapshotMilliseconds(0.0)
        , Last3DGeometryEmitMilliseconds(0.0)
        , Last3DTransformMilliseconds(0.0)
        , Last3DMaterialMilliseconds(0.0)
        , Last3DDisplayListMilliseconds(0.0)
        , Last3DDisplayListPreWaitMilliseconds(0.0)
        , Last3DDisplayListKickMilliseconds(0.0)
        , Last3DDisplayListPostWaitMilliseconds(0.0)
        , Last3DFallbackGeometryMilliseconds(0.0)
        , Last3DFlushMilliseconds(0.0)
        , LastPresentMilliseconds(0.0)
        , NativeDebugConsole()
        , NativeDebugOverlayInitialized(false)
        , NativeDebugOverlayLastSampleElapsedSeconds(0.0)
        , NativeDebugOverlayRenderFrameCount(0)
        , NativeDebugOverlayLastFps(0.0)
        , LastNativeDebugOverlayVBlankCount(0)
        , LastNativeDebugOverlayVBlankDelta(1)
        , NativeDebugOverlayMissedVBlankCount(0)
        , NativeDebugOverlayFramePacingInitialized(false)
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
        , LastHardwareTexturedMaxDiffuse(0.0f) {
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

        videoSetMode(MODE_0_3D | DISPLAY_BG0_ACTIVE);
        if (bottomScreenPresentationEnabled) {
            videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);
        }

        LastConfiguredHardware3DScreenTarget = targetScreen;
        LastConfiguredBottomScreenPresentationEnabled = bottomScreenPresentationEnabled;
    }

    /// Resolves one authored standard-material base color from cooked constant-buffer payloads.
    bool NintendoDsRenderManager3D::TryResolveStandardMaterialBaseColor(MaterialAsset* materialAsset, float3& resolvedColor) const {
        if (materialAsset == nullptr || materialAsset->ConstantBuffers == nullptr) {
            return false;
        }

        for (int32_t index = 0; index < materialAsset->ConstantBuffers->Length; index++) {
            MaterialConstantBufferAsset* constantBuffer = (*materialAsset->ConstantBuffers)[index];
            if (constantBuffer == nullptr || constantBuffer->get_Name() != StandardMaterialBaseColorBufferName) {
                continue;
            }

            float4 decodedColor;
            if (!TryDecodeFloat4ConstantBuffer(constantBuffer->get_Data(), decodedColor)) {
                return false;
            }

            resolvedColor = float3(
                std::clamp(decodedColor.X, 0.0f, 1.0f),
                std::clamp(decodedColor.Y, 0.0f, 1.0f),
                std::clamp(decodedColor.Z, 0.0f, 1.0f));
            return true;
        }

        return false;
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

    /// Builds one DS runtime material from authored material metadata.
    /// <param name="materialAsset">Authored material asset.</param>
    /// <param name="shaderAsset">Optional authored shader metadata carried by cross-platform runtime contracts.</param>
    /// <returns>DS runtime material carrying the authored metadata required for the first renderer slice.</returns>
    RuntimeMaterial* NintendoDsRenderManager3D::BuildMaterialFromRaw(MaterialAsset* materialAsset, ShaderAsset* shaderAsset) {
        if (materialAsset == nullptr) {
            throw new ArgumentNullException("materialAsset");
        }

        LastBuildStage = "BuildMaterialFromRawBegin";
        LastBuildAssetId = materialAsset->get_Id();
        NintendoDsRuntimeMaterial* runtimeMaterial = new NintendoDsRuntimeMaterial();
        runtimeMaterial->set_Id(materialAsset->get_Id());
        float3 resolvedBaseColor(1.0f, 1.0f, 1.0f);
        if (TryResolveStandardMaterialBaseColor(materialAsset, resolvedBaseColor)) {
            runtimeMaterial->BaseColor = resolvedBaseColor;
        }

        runtimeMaterial->PackedDiffuseColor = NintendoDsColorPacker::PackOpaqueWhite();
        runtimeMaterial->SupportsGeometrySubmission = true;
        if (materialAsset->RenderState != nullptr) {
            runtimeMaterial->SetRenderState(materialAsset->RenderState);
        }

        LastBuildStage = "BuildMaterialFromRawComplete";
        return runtimeMaterial;
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
        LastBuildStage = "BuildMaterialFromCookedComplete";
        return runtimeMaterial;
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
        NintendoDsRuntimeModel* runtimeModel = dynamic_cast<NintendoDsRuntimeModel*>(model);
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

        std::size_t allocatedAfterRelease = NintendoDsAllocationDiagnostics::GetTotalAllocatedSize();
        std::size_t freedAfterRelease = NintendoDsAllocationDiagnostics::GetTotalFreedSize();
        LastReleaseModelNetByteDelta = static_cast<int32_t>(
            (allocatedAfterRelease - allocatedBeforeRelease)
            - (freedAfterRelease - freedBeforeRelease));
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

        uint32_t* quadDisplayList = BuildHardwareLitQuadDisplayList(runtimeModel, displayListWordCount);
        if (quadDisplayList != nullptr) {
            runtimeModel->UsesHardwareLitQuadDisplayList = true;
            return quadDisplayList;
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

        displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_END, FIFO_NOP, FIFO_NOP, FIFO_NOP));
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

        displayListWords.push_back(FIFO_COMMAND_PACK(FIFO_END, FIFO_NOP, FIFO_NOP, FIFO_NOP));
        uint32_t* displayList = new uint32_t[displayListWords.size() + 1];
        displayList[0] = static_cast<uint32_t>(displayListWords.size());
        for (std::size_t wordIndex = 0; wordIndex < displayListWords.size(); wordIndex++) {
            displayList[wordIndex + 1] = displayListWords[wordIndex];
        }

        displayListWordCount = displayList[0];
        FlushHardwareLitDisplayList(displayList);
        return displayList;
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
    void NintendoDsRenderManager3D::SubmitStaticHardwareDisplayList(NintendoDsRuntimeModel* runtimeModel) {
        if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        } else if (runtimeModel->HardwareLitDisplayList == nullptr) {
            return;
        }

        uint32_t* displayList = runtimeModel->HardwareLitDisplayList;
        if (displayList[0] <= 0) {
            return;
        }

        uint32_t preWaitStartTimingTicks = cpuGetTiming();
        while ((DMA_CR(0) & DMA_BUSY) || (DMA_CR(1) & DMA_BUSY) || (DMA_CR(2) & DMA_BUSY) || (DMA_CR(3) & DMA_BUSY)) {
        }
        Last3DDisplayListPreWaitMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - preWaitStartTimingTicks);

        uint32_t dmaKickStartTimingTicks = cpuGetTiming();
        DMA_SRC(0) = reinterpret_cast<uint32_t>(displayList + 1);
        DMA_DEST(0) = 0x4000400;
        DMA_CR(0) = DMA_FIFO | displayList[0];
        Last3DDisplayListKickMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - dmaKickStartTimingTicks);

        uint32_t postWaitStartTimingTicks = cpuGetTiming();
        while (DMA_CR(0) & DMA_BUSY) {
        }
        Last3DDisplayListPostWaitMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - postWaitStartTimingTicks);
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

    /// Gets the most recent net allocator delta observed during the final present portion of one draw call.
    int32_t NintendoDsRenderManager3D::get_LastPresentNetByteDelta() const {
        return LastPresentNetByteDelta;
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

    /// Resolves whether the current frame has CPU-composited 2D work that must be copied to visible bitmap VRAM.
    bool NintendoDsRenderManager3D::ShouldPresent2DFrame(NintendoDsScreenTarget hardware3DScreenTarget, NintendoDsRenderManager2D* renderManager2D) const {
        if (renderManager2D == nullptr) {
            throw new ArgumentNullException("renderManager2D");
        }

        if (hardware3DScreenTarget == NintendoDsScreenTarget::None) {
            return true;
        }

        return renderManager2D->get_FrameHasVisibleSoftware2DWork();
    }

    /// Builds one placeholder render target to satisfy runtime APIs that request off-screen buffers.
    /// <param name="width">Requested render-target width.</param>
    /// <param name="height">Requested render-target height.</param>
    /// <returns>Placeholder runtime render target.</returns>
    RenderTarget* NintendoDsRenderManager3D::CreateRenderTarget(int32_t width, int32_t height) {
        RenderTarget* renderTarget = new RenderTarget();
        renderTarget->set_Width(width);
        renderTarget->set_Height(height);
        return renderTarget;
    }

    /// Draws the current generated-core 3D frame through the Nintendo DS renderer path.
    void NintendoDsRenderManager3D::Draw() {
        Core* core = Core::get_Instance();
        if (core == nullptr) {
            throw new InvalidOperationException("Core::Instance was not initialized.");
        }

        cpuStartTiming(0);
        ObjectManager* objectManager = core->get_ObjectManager();
        if (objectManager == nullptr) {
            throw new InvalidOperationException("Object manager was not initialized.");
        }

        NintendoDsRenderManager2D* renderManager2D = dynamic_cast<NintendoDsRenderManager2D*>(core->get_RenderManager2D());
        if (renderManager2D == nullptr) {
            throw new InvalidOperationException("Core render manager 2D was not a Nintendo DS renderer.");
        }

        List<ICamera*>* cameras = objectManager->get_Cameras();
        if (cameras == nullptr || cameras->Count() <= 0) {
            renderManager2D->SetBottomScreenPresentationEnabled(true);
            NativeDebugOverlayInitialized = false;
            PublishPerformanceOverlayMetrics(core, renderManager2D, false);
            return;
        }

        renderManager2D->BeginFrame();
        LastHardware3DScreenTarget = NintendoDsScreenTarget::None;
        LastCamera3DQueueCount = 0;
        LastSubmittedDrawableCount = 0;
        Last3DDisplayListCallCount = 0;
        Last3DQuadDisplayListCallCount = 0;
        Last3DDisplayListSubmittedWordCount = 0;
        LastTopScreen2DQueueCount = 0;
        LastBottomScreen2DQueueCount = 0;
        Last2DTraversalNetByteDelta = 0;
        Last3DSubmissionNetByteDelta = 0;
        LastPresentNetByteDelta = 0;
        Last2DTraversalMilliseconds = 0.0;
        Last3DSetupMilliseconds = 0.0;
        Last3DQueueSnapshotMilliseconds = 0.0;
        Last3DGeometryEmitMilliseconds = 0.0;
        Last3DTransformMilliseconds = 0.0;
        Last3DMaterialMilliseconds = 0.0;
        Last3DDisplayListMilliseconds = 0.0;
        Last3DDisplayListPreWaitMilliseconds = 0.0;
        Last3DDisplayListKickMilliseconds = 0.0;
        Last3DDisplayListPostWaitMilliseconds = 0.0;
        Last3DFallbackGeometryMilliseconds = 0.0;
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
        std::size_t initialAllocatedByteTotal = NintendoDsAllocationDiagnostics::GetTotalAllocatedSize();
        std::size_t initialFreedByteTotal = NintendoDsAllocationDiagnostics::GetTotalFreedSize();
        uint32_t traversalStartTimingTicks = cpuGetTiming();
        NintendoDsScreenTarget hardware3DScreenTarget = ResolveHardware3DScreenTarget(cameras, renderManager2D);
        bool useNativeDebugOverlay = hardware3DScreenTarget != NintendoDsScreenTarget::None;
        renderManager2D->SetBottomScreenPresentationEnabled(!useNativeDebugOverlay);
        Last2DTraversalMilliseconds = ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - traversalStartTimingTicks);
        std::size_t after2DTraversalAllocatedByteTotal = NintendoDsAllocationDiagnostics::GetTotalAllocatedSize();
        std::size_t after2DTraversalFreedByteTotal = NintendoDsAllocationDiagnostics::GetTotalFreedSize();
        Last2DTraversalNetByteDelta = static_cast<int32_t>(
            (after2DTraversalAllocatedByteTotal - initialAllocatedByteTotal)
            - (after2DTraversalFreedByteTotal - initialFreedByteTotal));
        if (hardware3DScreenTarget == NintendoDsScreenTarget::None) {
            lcdMainOnTop();
            vramSetBankA(VRAM_A_MAIN_BG);
            videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
            LastConfiguredHardware3DScreenTarget = NintendoDsScreenTarget::None;
            NativeDebugOverlayInitialized = false;
            if (renderManager2D->get_BottomScreenPresentationEnabled()) {
                videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);
            }
            Draw2DCameraList(cameras, renderManager2D);
            std::size_t beforePresentAllocatedByteTotal = NintendoDsAllocationDiagnostics::GetTotalAllocatedSize();
            std::size_t beforePresentFreedByteTotal = NintendoDsAllocationDiagnostics::GetTotalFreedSize();
            uint32_t presentStartTimingTicks = cpuGetTiming();
            renderManager2D->PresentFrame();
            LastPresentMilliseconds = ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - presentStartTimingTicks);
            std::size_t afterPresentAllocatedByteTotal = NintendoDsAllocationDiagnostics::GetTotalAllocatedSize();
            std::size_t afterPresentFreedByteTotal = NintendoDsAllocationDiagnostics::GetTotalFreedSize();
            LastPresentNetByteDelta = static_cast<int32_t>(
                (afterPresentAllocatedByteTotal - beforePresentAllocatedByteTotal)
                - (afterPresentFreedByteTotal - beforePresentFreedByteTotal));
            PublishPerformanceOverlayMetrics(core, renderManager2D, false);
            return;
        }

        uint32_t setupStartTimingTicks = cpuGetTiming();
        LastHardware3DScreenTarget = hardware3DScreenTarget;
        renderManager2D->SetHardware3DScreenTarget(hardware3DScreenTarget);
        ResolveFrameLighting(objectManager);
        EnsureHardwareInitialized();
        ConfigureHardware3DTarget(hardware3DScreenTarget, renderManager2D);
        EnsureNativeDebugOverlayInitialized();
        std::size_t before3DSubmissionAllocatedByteTotal = NintendoDsAllocationDiagnostics::GetTotalAllocatedSize();
        std::size_t before3DSubmissionFreedByteTotal = NintendoDsAllocationDiagnostics::GetTotalFreedSize();
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
            break;
        }
        std::size_t after3DSubmissionAllocatedByteTotal = NintendoDsAllocationDiagnostics::GetTotalAllocatedSize();
        std::size_t after3DSubmissionFreedByteTotal = NintendoDsAllocationDiagnostics::GetTotalFreedSize();
        Last3DSubmissionNetByteDelta = static_cast<int32_t>(
            (after3DSubmissionAllocatedByteTotal - before3DSubmissionAllocatedByteTotal)
            - (after3DSubmissionFreedByteTotal - before3DSubmissionFreedByteTotal));

        bool shouldPresent2DFrame = ShouldPresent2DFrame(hardware3DScreenTarget, renderManager2D);
        if (shouldPresent2DFrame) {
            std::size_t beforePresentAllocatedByteTotal = NintendoDsAllocationDiagnostics::GetTotalAllocatedSize();
            std::size_t beforePresentFreedByteTotal = NintendoDsAllocationDiagnostics::GetTotalFreedSize();
            uint32_t presentStartTimingTicks = cpuGetTiming();
            renderManager2D->PresentFrame();
            LastPresentMilliseconds = ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - presentStartTimingTicks);
            std::size_t afterPresentAllocatedByteTotal = NintendoDsAllocationDiagnostics::GetTotalAllocatedSize();
            std::size_t afterPresentFreedByteTotal = NintendoDsAllocationDiagnostics::GetTotalFreedSize();
            LastPresentNetByteDelta = static_cast<int32_t>(
                (afterPresentAllocatedByteTotal - beforePresentAllocatedByteTotal)
                - (afterPresentFreedByteTotal - beforePresentFreedByteTotal));
        } else {
            LastPresentNetByteDelta = 0;
        }
        PublishPerformanceOverlayMetrics(core, renderManager2D, true);
        DrawNativeDebugOverlay(core, objectManager, renderManager2D, true);
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

        uint32_t geometryEmitStartTimingTicks = cpuGetTiming();
        int32_t submittedDrawables = 0;
        for (int32_t drawableIndex = 0; drawableIndex < drawables->Count(); drawableIndex++) {
            IDrawable3D* drawable = (*drawables)[drawableIndex];
            if (drawable == nullptr) {
                continue;
            }

            NintendoDsRuntimeModel* runtimeModel = dynamic_cast<NintendoDsRuntimeModel*>(drawable->get_Model());
            NintendoDsRuntimeMaterial* runtimeMaterial = dynamic_cast<NintendoDsRuntimeMaterial*>(drawable->get_Material());
            if (runtimeModel == nullptr || runtimeMaterial == nullptr) {
                continue;
            }

            if (!runtimeMaterial->SupportsGeometrySubmission) {
                continue;
            }

            SubmitOpaqueDrawable(drawable, runtimeModel, runtimeMaterial);
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
    void NintendoDsRenderManager3D::SubmitOpaqueDrawable(
        IDrawable3D* drawable,
        NintendoDsRuntimeModel* runtimeModel,
        NintendoDsRuntimeMaterial* runtimeMaterial) {
        if (drawable == nullptr) {
            throw new ArgumentNullException("drawable");
        } else if (runtimeModel == nullptr) {
            throw new ArgumentNullException("runtimeModel");
        } else if (runtimeMaterial == nullptr) {
            throw new ArgumentNullException("runtimeMaterial");
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
        Last3DMaterialMilliseconds += ConvertCpuTimingTicksToMilliseconds(cpuGetTiming() - materialStartTimingTicks);
        if (!useHardwareTexture && runtimeModel->HardwareLitDisplayList != nullptr) {
            uint32_t displayListStartTimingTicks = cpuGetTiming();
            Last3DDisplayListCallCount++;
            Last3DDisplayListSubmittedWordCount += runtimeModel->HardwareLitDisplayListWordCount;
            if (runtimeModel->UsesHardwareLitQuadDisplayList) {
                Last3DQuadDisplayListCallCount++;
            }

            SubmitStaticHardwareDisplayList(runtimeModel);
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
        glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0);
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
            glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK | POLY_FORMAT_LIGHT0);
        } else {
            glPolyFmt(POLY_ALPHA(31) | POLY_CULL_BACK);
            glColor(RGB15(31, 31, 31) | BIT(15));
        }

        glMaterialf(GL_AMBIENT, packedAmbientMaterial);
        glMaterialf(GL_DIFFUSE, packedDiffuseMaterial);
    }

    /// Configures a DS hardware texture for one material when a runtime texture is bound.
    bool NintendoDsRenderManager3D::TryConfigureHardwareTexture(NintendoDsRuntimeMaterial* runtimeMaterial, NintendoDsRuntimeTexture2D*& runtimeTexture) {
        if (runtimeMaterial == nullptr) {
            throw new ArgumentNullException("runtimeMaterial");
        }

        runtimeTexture = dynamic_cast<NintendoDsRuntimeTexture2D*>(runtimeMaterial->ResolveTexture());
        if (runtimeTexture == nullptr) {
            glDisable(GL_TEXTURE_2D);
            return false;
        }

        RecordHardwareTextureDiagnostics(runtimeTexture, false);
        EnsureHardwareTextureUploaded(runtimeTexture);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(0, runtimeTexture->HardwareTextureId);
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

        core->SetPerformanceOverlayMetrics(
            usesMetrics,
            usesMetrics ? Last2DTraversalMilliseconds : 0.0,
            usesMetrics ? Last3DSetupMilliseconds : 0.0,
            usesMetrics ? Last3DQueueSnapshotMilliseconds : 0.0,
            usesMetrics ? Last3DGeometryEmitMilliseconds : 0.0,
            usesMetrics ? Last3DFlushMilliseconds : 0.0,
            usesMetrics ? LastPresentMilliseconds : 0.0,
            usesMetrics ? LastSubmittedDrawableCount : 0,
            usesMetrics ? LastCamera3DQueueCount : 0);
    }

    /// Initializes the native DS text background used for diagnostics on hardware-3D scenes.
    void NintendoDsRenderManager3D::EnsureNativeDebugOverlayInitialized() {
        if (NativeDebugOverlayInitialized) {
            return;
        }

        videoSetModeSub(MODE_0_2D | DISPLAY_BG0_ACTIVE);
        vramSetBankC(VRAM_C_SUB_BG);
        consoleInit(&NativeDebugConsole, 0, BgType_Text4bpp, BgSize_T_256x256, 31, 0, false, true);
        consoleSelect(&NativeDebugConsole);
        consoleClear();
        NativeDebugOverlayInitialized = true;
    }

    /// Draws the current diagnostic rows through the native DS text background instead of software bitmap text.
    void NintendoDsRenderManager3D::DrawNativeDebugOverlay(Core* core, ObjectManager* objectManager, NintendoDsRenderManager2D* renderManager2D, bool usesMetrics) {
        if (core == nullptr) {
            throw new ArgumentNullException("core");
        } else if (objectManager == nullptr) {
            throw new ArgumentNullException("objectManager");
        } else if (renderManager2D == nullptr) {
            throw new ArgumentNullException("renderManager2D");
        }

        EnsureNativeDebugOverlayInitialized();
        SampleNativeDebugOverlayFramePacing();
        NintendoDsRenderManager2DProfileSnapshot profileSnapshot = renderManager2D->get_ProfileSnapshot();
        List<IDrawable2D*>* drawables2D = objectManager->get_Drawables2D();
        List<IDrawable3D*>* drawables3D = objectManager->get_Drawables3D();
        int32_t drawable2DCount = drawables2D == nullptr ? 0 : drawables2D->Count();
        int32_t drawable3DCount = drawables3D == nullptr ? 0 : drawables3D->Count();

        PrintNativeDebugOverlayLine(0, FormatNativeDebugOverlayRenderFps(core));
        PrintNativeDebugOverlayLine(1, "Memory Res: --");
        PrintNativeDebugOverlayLine(2, "Memory Com: --");
        PrintNativeDebugOverlayLine(3, std::string("Drawables 2D: ") + std::to_string(drawable2DCount));
        PrintNativeDebugOverlayLine(4, std::string("Drawables 3D: ") + std::to_string(drawable3DCount) + " DrawCalls: " + std::to_string(core->get_LastRenderManager3DDrawCallCount()));
        if (!usesMetrics) {
            PrintNativeDebugOverlayLine(5, "D3A --");
            PrintNativeDebugOverlayLine(6, "D3B --");
            PrintNativeDebugOverlayLine(7, "D2D --");
            PrintNativeDebugOverlayLine(8, "D3C --");
            PrintNativeDebugOverlayLine(9, "D3D --");
            PrintNativeDebugOverlayLine(10, "D3E --");
            PrintNativeDebugOverlayLine(11, "D3F --");
            PrintNativeDebugOverlayLine(12, "D3L --");
            PrintNativeDebugOverlayLine(13, "D3T --");
            PrintNativeDebugOverlayLine(14, "D3U --");
            return;
        }

        PrintNativeDebugOverlayLine(
            5,
            std::string("D3A 2D") + FormatDebugMilliseconds(Last2DTraversalMilliseconds)
                + " S" + FormatDebugMilliseconds(Last3DSetupMilliseconds)
                + " Q" + FormatDebugMilliseconds(Last3DQueueSnapshotMilliseconds));
        PrintNativeDebugOverlayLine(
            6,
            std::string("D3B G") + FormatDebugMilliseconds(Last3DGeometryEmitMilliseconds)
                + " F" + FormatDebugMilliseconds(Last3DFlushMilliseconds)
                + " P" + FormatDebugMilliseconds(LastPresentMilliseconds));
        PrintNativeDebugOverlayLine(
            7,
            std::string("D2D T") + FormatDebugMilliseconds(profileSnapshot.TextMilliseconds)
                + "/" + std::to_string(profileSnapshot.TextPrimitiveCount)
                + " C" + FormatDebugMilliseconds(profileSnapshot.ClearMilliseconds)
                + " S" + FormatDebugMilliseconds(profileSnapshot.SpriteMilliseconds)
                + " R" + FormatDebugMilliseconds(profileSnapshot.RoundedRectMilliseconds));
        PrintNativeDebugOverlayLine(
            8,
            std::string("D3C VB") + std::to_string(LastNativeDebugOverlayVBlankDelta)
                + " Miss" + std::to_string(NativeDebugOverlayMissedVBlankCount));
        PrintNativeDebugOverlayLine(
            9,
            std::string("D3D X") + FormatDebugMilliseconds(Last3DTransformMilliseconds)
                + " M" + FormatDebugMilliseconds(Last3DMaterialMilliseconds)
                + " L" + FormatDebugMilliseconds(Last3DDisplayListMilliseconds)
                + " V" + FormatDebugMilliseconds(Last3DFallbackGeometryMilliseconds));
        PrintNativeDebugOverlayLine(
            10,
            std::string("D3E W") + std::to_string(Last3DDisplayListSubmittedWordCount)
                + " C" + std::to_string(Last3DDisplayListCallCount)
                + " Q" + std::to_string(Last3DQuadDisplayListCallCount));
        PrintNativeDebugOverlayLine(
            11,
            std::string("D3F A") + FormatDebugMilliseconds(Last3DDisplayListPreWaitMilliseconds)
                + " K" + FormatDebugMilliseconds(Last3DDisplayListKickMilliseconds)
                + " B" + FormatDebugMilliseconds(Last3DDisplayListPostWaitMilliseconds));
        PrintNativeDebugOverlayLine(
            12,
            std::string("D3L W")
                + FormatDebugSignedUnit(FrameLightDirection.X)
                + "," + FormatDebugSignedUnit(FrameLightDirection.Y)
                + "," + FormatDebugSignedUnit(FrameLightDirection.Z)
                + " P" + FormatDebugSignedUnit(FrameLightDirection.X)
                + "," + FormatDebugSignedUnit(FrameLightDirection.Y)
                + "," + FormatDebugSignedUnit(FrameLightDirection.Z));
        PrintNativeDebugOverlayLine(13, FormatHardwareTextureDiagnostics());
        PrintNativeDebugOverlayLine(14, FormatHardwareTextureLightingDiagnostics());
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

    /// Writes one fixed-width row to the native diagnostics text background.
    void NintendoDsRenderManager3D::PrintNativeDebugOverlayLine(int32_t row, const std::string& text) {
        consoleSelect(&NativeDebugConsole);
        iprintf("\x1b[%d;0H%-32.32s", row, text.c_str());
    }

    /// Formats the native debug overlay render-FPS row from the latest sample window.
    std::string NintendoDsRenderManager3D::FormatNativeDebugOverlayRenderFps(Core* core) {
        if (core == nullptr) {
            throw new ArgumentNullException("core");
        }

        NativeDebugOverlayRenderFrameCount++;
        double totalElapsedSeconds = core->get_TotalElapsedSeconds();
        if (NativeDebugOverlayLastSampleElapsedSeconds <= 0.0) {
            NativeDebugOverlayLastSampleElapsedSeconds = totalElapsedSeconds;
        }

        double elapsedSeconds = totalElapsedSeconds - NativeDebugOverlayLastSampleElapsedSeconds;
        if (elapsedSeconds >= 0.5) {
            double safeElapsedSeconds = elapsedSeconds <= 0.0 ? 1.0 : elapsedSeconds;
            NativeDebugOverlayLastFps = static_cast<double>(NativeDebugOverlayRenderFrameCount) / safeElapsedSeconds;
            NativeDebugOverlayRenderFrameCount = 0;
            NativeDebugOverlayLastSampleElapsedSeconds = totalElapsedSeconds;
        }

        return std::string("Render FPS: ") + FormatDebugMilliseconds(NativeDebugOverlayLastFps)
            + " (" + FormatDebugMilliseconds(core->get_LastRenderManager3DDrawMilliseconds()) + " ms)";
    }

    /// Samples hardware VBlank pacing for native debug overlay diagnostics.
    void NintendoDsRenderManager3D::SampleNativeDebugOverlayFramePacing() {
        uint32_t currentVBlankCount = GetNintendoDsVBlankCount();
        if (!NativeDebugOverlayFramePacingInitialized) {
            LastNativeDebugOverlayVBlankCount = currentVBlankCount;
            LastNativeDebugOverlayVBlankDelta = 1;
            NativeDebugOverlayFramePacingInitialized = true;
            return;
        }

        uint32_t rawVBlankDelta = currentVBlankCount > LastNativeDebugOverlayVBlankCount
            ? currentVBlankCount - LastNativeDebugOverlayVBlankCount
            : 1;
        LastNativeDebugOverlayVBlankDelta = static_cast<int32_t>(rawVBlankDelta);
        if (LastNativeDebugOverlayVBlankDelta > 1) {
            NativeDebugOverlayMissedVBlankCount += LastNativeDebugOverlayVBlankDelta - 1;
        }

        LastNativeDebugOverlayVBlankCount = currentVBlankCount;
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
        glNormal(NORMAL_PACK(
            PackHardwareNormalComponent(modelFaceNormal.X),
            PackHardwareNormalComponent(modelFaceNormal.Y),
            PackHardwareNormalComponent(modelFaceNormal.Z)));
        glVertex3v16(floattov16((*positions)[indexA].X), floattov16((*positions)[indexA].Y), floattov16((*positions)[indexA].Z));
        glVertex3v16(floattov16((*positions)[indexB].X), floattov16((*positions)[indexB].Y), floattov16((*positions)[indexB].Z));
        glVertex3v16(floattov16((*positions)[indexC].X), floattov16((*positions)[indexC].Y), floattov16((*positions)[indexC].Z));
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
            glTexCoord2t16(0, 0);
            if (lightingEnabled) {
                glNormal(NORMAL_PACK(
                    PackHardwareNormalComponent(modelFaceNormal.X),
                    PackHardwareNormalComponent(modelFaceNormal.Y),
                    PackHardwareNormalComponent(modelFaceNormal.Z)));
            }

            glVertex3v16(floattov16(vertexA.X), floattov16(vertexA.Y), floattov16(vertexA.Z));
            glTexCoord2t16(inttot16(runtimeTexture->get_Width()), 0);
            if (lightingEnabled) {
                glNormal(NORMAL_PACK(
                    PackHardwareNormalComponent(modelFaceNormal.X),
                    PackHardwareNormalComponent(modelFaceNormal.Y),
                    PackHardwareNormalComponent(modelFaceNormal.Z)));
            }

            glVertex3v16(floattov16(vertexB.X), floattov16(vertexB.Y), floattov16(vertexB.Z));
            glTexCoord2t16(0, inttot16(runtimeTexture->get_Height()));
            if (lightingEnabled) {
                glNormal(NORMAL_PACK(
                    PackHardwareNormalComponent(modelFaceNormal.X),
                    PackHardwareNormalComponent(modelFaceNormal.Y),
                    PackHardwareNormalComponent(modelFaceNormal.Z)));
            }

            glVertex3v16(floattov16(vertexC.X), floattov16(vertexC.Y), floattov16(vertexC.Z));
            return;
        }

        SubmitHardwareTexturedVertex(positions, texCoords, runtimeTexture, lightingEnabled, modelFaceNormal, indexA);
        SubmitHardwareTexturedVertex(positions, texCoords, runtimeTexture, lightingEnabled, modelFaceNormal, indexB);
        SubmitHardwareTexturedVertex(positions, texCoords, runtimeTexture, lightingEnabled, modelFaceNormal, indexC);
    }

    /// Submits one textured vertex with a normalized model UV converted to DS texture coordinates.
    void NintendoDsRenderManager3D::SubmitHardwareTexturedVertex(
        Array<float3>* positions,
            Array<float2>* texCoords,
            NintendoDsRuntimeTexture2D* runtimeTexture,
            bool lightingEnabled,
            const float3& modelFaceNormal,
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
        if (lightingEnabled) {
            glNormal(NORMAL_PACK(
                PackHardwareNormalComponent(modelFaceNormal.X),
                PackHardwareNormalComponent(modelFaceNormal.Y),
                PackHardwareNormalComponent(modelFaceNormal.Z)));
        }

        glVertex3v16(floattov16((*positions)[index].X), floattov16((*positions)[index].Y), floattov16((*positions)[index].Z));
    }
}
#endif
