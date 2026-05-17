#include "platform/ds/NintendoDsRenderManager3D.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
extern "C" {
#include <nds/system.h>
#include <nds/arm9/video.h>
#include <nds/arm9/videoGL.h>
#include <nds/arm9/trig_lut.h>
}

#include <algorithm>
#include <cstring>

#include "CameraClearSettings.hpp"
#include "Core.hpp"
#include "MaterialConstantBufferAsset.hpp"
#include "LightComponent.hpp"
#include "AmbientLightComponent.hpp"
#include "DirectionalLightComponent.hpp"
#include "Entity.hpp"
#include "ICamera.hpp"
#include "IDrawable3D.hpp"
#include "IRenderQueue2D.hpp"
#include "IRenderQueue3D.hpp"
#include "LightDirectionUtility.hpp"
#include "ObjectManager.hpp"
#include "platform/ds/NintendoDsLightingMath.hpp"
#include "platform/ds/NintendoDsColorPacker.hpp"
#include "platform/ds/NintendoDsRenderManager2D.hpp"
#include "platform/ds/NintendoDsRenderQueueSnapshotVisitor.hpp"
#include "platform/ds/NintendoDsRuntimeMaterial.hpp"
#include "platform/ds/NintendoDsRuntimeModel.hpp"
#include "runtime/native_exceptions.hpp"

namespace helengine::ds {
    namespace {
        /// Stores the standard top-screen clear color for DS 3D output.
        constexpr uint16_t DefaultClearColor = 0x0000;
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
        , LastCamera3DQueueCount(0)
        , LastSubmittedDrawableCount(0)
        , LastTopScreen2DQueueCount(0)
        , LastBottomScreen2DQueueCount(0) {
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
            if (renderQueue2D != nullptr && renderQueue2D->get_Count() > 0) {
                NintendoDsScreenTarget queueScreenTarget = ResolveCameraScreenTarget(camera);
                if (queueScreenTarget == NintendoDsScreenTarget::Bottom) {
                    LastBottomScreen2DQueueCount = renderQueue2D->get_Count();
                } else {
                    LastTopScreen2DQueueCount = renderQueue2D->get_Count();
                }
                renderManager2D->DrawCamera(camera);
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

    /// Configures which Nintendo DS physical screen currently owns the hardware 3D main-engine presentation.
    /// <param name="targetScreen">Screen that should own the hardware 3D pass for the current frame.</param>
    /// <param name="renderManager2D">Nintendo DS 2D renderer that may reserve the bottom screen for native-console diagnostics.</param>
    void NintendoDsRenderManager3D::ConfigureHardware3DTarget(NintendoDsScreenTarget targetScreen, NintendoDsRenderManager2D* renderManager2D) {
        if (targetScreen == NintendoDsScreenTarget::Bottom) {
            lcdMainOnBottom();
        } else {
            lcdMainOnTop();
        }

        videoSetMode(MODE_0_3D | DISPLAY_BG3_ACTIVE);
        if (renderManager2D == nullptr || renderManager2D->get_BottomScreenPresentationEnabled()) {
            videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);
        }
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
        runtimeModel->Indices16 = data->Indices16;
        runtimeModel->Indices32 = data->Indices32;
        runtimeModel->Uses32BitIndices = data->Indices32 != nullptr && data->Indices32->Length > 0;
        LastBuildStage = "BuildModelFromRawComplete";
        return runtimeModel;
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

        ObjectManager* objectManager = core->get_ObjectManager();
        if (objectManager == nullptr) {
            throw new InvalidOperationException("Object manager was not initialized.");
        }

        List<ICamera*>* cameras = objectManager->get_Cameras();
        if (cameras == nullptr || cameras->Count() <= 0) {
            return;
        }

        NintendoDsRenderManager2D* renderManager2D = dynamic_cast<NintendoDsRenderManager2D*>(core->get_RenderManager2D());
        if (renderManager2D == nullptr) {
            throw new InvalidOperationException("Core render manager 2D was not a Nintendo DS renderer.");
        }

        renderManager2D->BeginFrame();
        LastHardware3DScreenTarget = NintendoDsScreenTarget::None;
        LastCamera3DQueueCount = 0;
        LastSubmittedDrawableCount = 0;
        LastTopScreen2DQueueCount = 0;
        LastBottomScreen2DQueueCount = 0;
        NintendoDsScreenTarget hardware3DScreenTarget = ResolveHardware3DScreenTarget(cameras, renderManager2D);
        if (hardware3DScreenTarget == NintendoDsScreenTarget::None) {
            lcdMainOnTop();
            videoSetMode(MODE_5_2D | DISPLAY_BG3_ACTIVE);
            if (renderManager2D->get_BottomScreenPresentationEnabled()) {
                videoSetModeSub(MODE_5_2D | DISPLAY_BG3_ACTIVE);
            }
            renderManager2D->PresentFrame();
            return;
        }

        LastHardware3DScreenTarget = hardware3DScreenTarget;
        renderManager2D->SetHardware3DScreenTarget(hardware3DScreenTarget);
        ResolveFrameLighting(objectManager);
        EnsureHardwareInitialized();
        ConfigureHardware3DTarget(hardware3DScreenTarget, renderManager2D);
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
            LastSubmittedDrawableCount = DrawRenderQueue(camera);
            break;
        }

        renderManager2D->PresentFrame();
    }

    /// Initializes Nintendo DS 3D video mode and hardware state before the first frame.
    void NintendoDsRenderManager3D::EnsureHardwareInitialized() {
        if (HardwareInitialized) {
            return;
        }

        vramSetBankB(VRAM_B_TEXTURE);
        glInit();
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

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(70.0f, 256.0f / 192.0f, 0.1f, 40.0f);

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

        RenderQueueSnapshotVisitor->Clear();
        renderQueue->VisitOrdered(RenderQueueSnapshotVisitor);
        List<IDrawable3D*>* drawables = RenderQueueSnapshotVisitor->get_Drawables();
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

        glFlush(0);
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
        } else if (runtimeModel->Positions == nullptr || runtimeModel->Positions->Length <= 0) {
            return;
        }

        Array<float3>* positions = runtimeModel->Positions;
        Entity* entity = drawable->get_Parent();
        float3 entityPosition = float3::get_Zero();
        float3 entityScale = float3::get_One();
        float4 entityOrientation = float4::get_Identity();
        if (entity != nullptr) {
            entityPosition = entity->get_Position();
            entityScale = entity->get_Scale();
            entityOrientation = entity->get_Orientation();
        }

        glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
        glBegin(GL_TRIANGLES);

        if (runtimeModel->Uses32BitIndices && runtimeModel->Indices32 != nullptr) {
            for (int32_t index = 0; index + 2 < runtimeModel->Indices32->Length; index += 3) {
                SubmitLitTriangle(
                    runtimeMaterial,
                    positions,
                    static_cast<int32_t>((*runtimeModel->Indices32)[index]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 1]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 2]),
                    entityPosition,
                    entityScale,
                    entityOrientation);
            }
        } else if (runtimeModel->Indices16 != nullptr) {
            for (int32_t index = 0; index + 2 < runtimeModel->Indices16->Length; index += 3) {
                SubmitLitTriangle(
                    runtimeMaterial,
                    positions,
                    static_cast<int32_t>((*runtimeModel->Indices16)[index]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 1]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 2]),
                    entityPosition,
                    entityScale,
                    entityOrientation);
            }
        } else {
            for (int32_t index = 0; index + 2 < positions->Length; index += 3) {
                SubmitLitTriangle(
                    runtimeMaterial,
                    positions,
                    index,
                    index + 1,
                    index + 2,
                    entityPosition,
                    entityScale,
                    entityOrientation);
            }
        }

        glEnd();
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

    /// Submits one lit triangle through the DS immediate-mode geometry path.
    void NintendoDsRenderManager3D::SubmitLitTriangle(
        NintendoDsRuntimeMaterial* runtimeMaterial,
        Array<float3>* positions,
        int32_t indexA,
        int32_t indexB,
        int32_t indexC,
        const float3& entityPosition,
        const float3& entityScale,
        const float4& entityOrientation) {
        if (runtimeMaterial == nullptr) {
            throw new ArgumentNullException("runtimeMaterial");
        } else if (positions == nullptr) {
            throw new ArgumentNullException("positions");
        } else if (indexA < 0 || indexB < 0 || indexC < 0) {
            return;
        } else if (indexA >= positions->Length || indexB >= positions->Length || indexC >= positions->Length) {
            return;
        }

        float3 vertexA = TransformVertex((*positions)[indexA], entityPosition, entityScale, entityOrientation);
        float3 vertexB = TransformVertex((*positions)[indexB], entityPosition, entityScale, entityOrientation);
        float3 vertexC = TransformVertex((*positions)[indexC], entityPosition, entityScale, entityOrientation);
        float3 faceNormal = NintendoDsLightingMath::ComputeTriangleNormal(vertexA, vertexB, vertexC);
        float diffuse = NintendoDsLightingMath::EvaluateDirectionalDiffuse(faceNormal, FrameLightDirection);
        float3 lighting = FrameAmbientRadiance + (FrameDirectionalRadiance * diffuse);
        lighting = NintendoDsLightingMath::ClampColor(lighting);

        float3 shapedLighting = NintendoDsLightingMath::ApplyDisplayContrastCurve(lighting);
        float3 finalColor = NintendoDsLightingMath::MultiplyColor(runtimeMaterial->BaseColor, shapedLighting);
        finalColor = NintendoDsLightingMath::ClampColor(finalColor);

        glColor(NintendoDsColorPacker::PackOpaqueColor(finalColor));
        glVertex3v16(floattov16(vertexA.X), floattov16(vertexA.Y), floattov16(vertexA.Z));
        glVertex3v16(floattov16(vertexB.X), floattov16(vertexB.Y), floattov16(vertexB.Z));
        glVertex3v16(floattov16(vertexC.X), floattov16(vertexC.Y), floattov16(vertexC.Z));
    }

    /// Applies one drawable entity transform to a model-space vertex.
    /// <param name="modelVertex">Model-space vertex to transform.</param>
    /// <returns>World-space vertex used by the first DS submission path.</returns>
    float3 NintendoDsRenderManager3D::TransformVertex(
        float3 modelVertex,
        const float3& entityPosition,
        const float3& entityScale,
        const float4& entityOrientation) {
        float3 scaledVertex = modelVertex * entityScale;
        float3 rotatedVertex = float4::RotateVector(scaledVertex, entityOrientation);
        return rotatedVertex + entityPosition;
    }
}
#endif
