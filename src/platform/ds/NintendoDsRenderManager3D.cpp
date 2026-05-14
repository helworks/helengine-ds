#include "platform/ds/NintendoDsRenderManager3D.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
extern "C" {
#include <nds/arm9/video.h>
#include <nds/arm9/videoGL.h>
#include <nds/arm9/trig_lut.h>
}

#include <algorithm>

#include "Component.hpp"
#include "CameraClearSettings.hpp"
#include "Core.hpp"
#include "AmbientLightComponent.hpp"
#include "DirectionalLightComponent.hpp"
#include "Entity.hpp"
#include "ICamera.hpp"
#include "IDrawable3D.hpp"
#include "LightDirectionUtility.hpp"
#include "ObjectManager.hpp"
#include "platform/ds/NintendoDsLightingMath.hpp"
#include "platform/ds/NintendoDsColorPacker.hpp"
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
        , RenderQueueSnapshotVisitor(new NintendoDsRenderQueueSnapshotVisitor()) {
    }

    /// Builds one DS runtime material from the authored material asset.
    /// <param name="materialAsset">Authored material asset.</param>
    /// <param name="shaderAsset">Authored shader asset resolved for the material.</param>
    /// <returns>DS runtime material carrying the authored metadata required for the first renderer slice.</returns>
    RuntimeMaterial* NintendoDsRenderManager3D::BuildMaterialFromRaw(MaterialAsset* materialAsset, ShaderAsset* shaderAsset) {
        if (materialAsset == nullptr) {
            throw new ArgumentNullException("materialAsset");
        } else if (shaderAsset == nullptr) {
            throw new ArgumentNullException("shaderAsset");
        }

        NintendoDsRuntimeMaterial* runtimeMaterial = new NintendoDsRuntimeMaterial();
        runtimeMaterial->set_Id(materialAsset->get_Id());
        runtimeMaterial->PackedDiffuseColor = NintendoDsColorPacker::PackOpaqueWhite();
        runtimeMaterial->SupportsGeometrySubmission = true;
        if (materialAsset->RenderState != nullptr) {
            runtimeMaterial->SetRenderState(materialAsset->RenderState);
        }

        return runtimeMaterial;
    }

    /// Builds one DS runtime model from the authored model asset.
    /// <param name="data">Authored model asset.</param>
    /// <returns>DS runtime model carrying the authored metadata required for the first renderer slice.</returns>
    RuntimeModel* NintendoDsRenderManager3D::BuildModelFromRaw(ModelAsset* data) {
        if (data == nullptr) {
            throw new ArgumentNullException("data");
        }

        NintendoDsRuntimeModel* runtimeModel = new NintendoDsRuntimeModel();
        runtimeModel->set_Id(data->get_Id());
        runtimeModel->Positions = data->Positions;
        runtimeModel->Indices16 = data->Indices16;
        runtimeModel->Indices32 = data->Indices32;
        runtimeModel->Uses32BitIndices = data->Indices32 != nullptr && data->Indices32->Length > 0;
        return runtimeModel;
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

        ICamera* selectedCamera = (*cameras)[0];
        EnsureHardwareInitialized();
        ClearFromCamera(selectedCamera);
        ConfigureCamera(selectedCamera);
        DrawRenderQueue(selectedCamera);
    }

    /// Initializes Nintendo DS 3D video mode and hardware state before the first frame.
    void NintendoDsRenderManager3D::EnsureHardwareInitialized() {
        if (HardwareInitialized) {
            return;
        }

        vramSetBankA(VRAM_A_TEXTURE);
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
        float3 lightDirection(0.0f, -1.0f, 0.0f);
        float3 directionalRadiance(0.0f, 0.0f, 0.0f);
        float3 ambientRadiance(0.0f, 0.0f, 0.0f);
        ResolveSceneLighting(lightDirection, directionalRadiance, ambientRadiance);
        glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE);
        glBegin(GL_TRIANGLES);

        if (runtimeModel->Uses32BitIndices && runtimeModel->Indices32 != nullptr) {
            for (int32_t index = 0; index + 2 < runtimeModel->Indices32->Length; index += 3) {
                SubmitLitTriangle(
                    drawable,
                    positions,
                    static_cast<int32_t>((*runtimeModel->Indices32)[index]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 1]),
                    static_cast<int32_t>((*runtimeModel->Indices32)[index + 2]),
                    lightDirection,
                    directionalRadiance,
                    ambientRadiance);
            }
        } else if (runtimeModel->Indices16 != nullptr) {
            for (int32_t index = 0; index + 2 < runtimeModel->Indices16->Length; index += 3) {
                SubmitLitTriangle(
                    drawable,
                    positions,
                    static_cast<int32_t>((*runtimeModel->Indices16)[index]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 1]),
                    static_cast<int32_t>((*runtimeModel->Indices16)[index + 2]),
                    lightDirection,
                    directionalRadiance,
                    ambientRadiance);
            }
        } else {
            for (int32_t index = 0; index + 2 < positions->Length; index += 3) {
                SubmitLitTriangle(
                    drawable,
                    positions,
                    index,
                    index + 1,
                    index + 2,
                    lightDirection,
                    directionalRadiance,
                    ambientRadiance);
            }
        }

        glEnd();
    }

    /// Resolves the current scene lighting needed by the DS grayscale renderer.
    void NintendoDsRenderManager3D::ResolveSceneLighting(float3& lightDirection, float3& directionalRadiance, float3& ambientRadiance) const {
        Core* core = Core::get_Instance();
        if (core == nullptr) {
            return;
        }

        ObjectManager* objectManager = core->get_ObjectManager();
        if (objectManager == nullptr) {
            return;
        }

        List<Entity*>* entities = objectManager->get_Entities();
        if (entities == nullptr) {
            return;
        }

        bool directionalResolved = false;
        for (int32_t entityIndex = 0; entityIndex < entities->Count(); entityIndex++) {
            Entity* entity = (*entities)[entityIndex];
            if (entity == nullptr) {
                continue;
            }

            List<Component*>* components = entity->get_Components();
            if (components == nullptr) {
                continue;
            }

            for (int32_t componentIndex = 0; componentIndex < components->Count(); componentIndex++) {
                Component* component = (*components)[componentIndex];
                if (component == nullptr) {
                    continue;
                }

                LightComponent* lightComponent = dynamic_cast<LightComponent*>(component);
                if (lightComponent == nullptr) {
                    continue;
                }

                float4 color = lightComponent->get_Color();
                float3 radiance(
                    color.X * lightComponent->get_Intensity(),
                    color.Y * lightComponent->get_Intensity(),
                    color.Z * lightComponent->get_Intensity());

                if (dynamic_cast<AmbientLightComponent*>(lightComponent) != nullptr) {
                    ambientRadiance = ambientRadiance + radiance;
                    continue;
                }

                DirectionalLightComponent* directionalLight = dynamic_cast<DirectionalLightComponent*>(lightComponent);
                if (directionalLight == nullptr || directionalResolved) {
                    continue;
                }

                lightDirection = LightDirectionUtility::GetLightDirection(directionalLight);
                directionalRadiance = radiance;
                directionalResolved = true;
            }
        }
    }

    /// Submits one lit triangle through the DS immediate-mode geometry path.
    void NintendoDsRenderManager3D::SubmitLitTriangle(
        IDrawable3D* drawable,
        Array<float3>* positions,
        int32_t indexA,
        int32_t indexB,
        int32_t indexC,
        float3 lightDirection,
        float3 directionalRadiance,
        float3 ambientRadiance) {
        if (drawable == nullptr) {
            throw new ArgumentNullException("drawable");
        } else if (positions == nullptr) {
            throw new ArgumentNullException("positions");
        } else if (indexA < 0 || indexB < 0 || indexC < 0) {
            return;
        } else if (indexA >= positions->Length || indexB >= positions->Length || indexC >= positions->Length) {
            return;
        }

        float3 vertexA = TransformVertex(drawable, (*positions)[indexA]);
        float3 vertexB = TransformVertex(drawable, (*positions)[indexB]);
        float3 vertexC = TransformVertex(drawable, (*positions)[indexC]);
        float3 faceNormal = NintendoDsLightingMath::ComputeTriangleNormal(vertexA, vertexB, vertexC);
        float diffuse = NintendoDsLightingMath::EvaluateDirectionalDiffuse(faceNormal, lightDirection);
        float ambientLuminance = NintendoDsLightingMath::ComputeLuminance(ambientRadiance);
        float directionalLuminance = NintendoDsLightingMath::ComputeLuminance(directionalRadiance);
        float scaledLighting = ambientLuminance + (diffuse * directionalLuminance);
        scaledLighting = std::clamp(scaledLighting, 0.0f, 1.0f);

        float displayLighting = NintendoDsLightingMath::ApplyDisplayContrastCurve(scaledLighting);
        glColor(NintendoDsLightingMath::ScalePackedGreyscale(displayLighting));
        glVertex3v16(floattov16(vertexA.X), floattov16(vertexA.Y), floattov16(vertexA.Z));
        glVertex3v16(floattov16(vertexB.X), floattov16(vertexB.Y), floattov16(vertexB.Z));
        glVertex3v16(floattov16(vertexC.X), floattov16(vertexC.Y), floattov16(vertexC.Z));
    }

    /// Applies one drawable entity transform to a model-space vertex.
    /// <param name="drawable">Drawable providing the entity transform.</param>
    /// <param name="modelVertex">Model-space vertex to transform.</param>
    /// <returns>World-space vertex used by the first DS submission path.</returns>
    float3 NintendoDsRenderManager3D::TransformVertex(IDrawable3D* drawable, float3 modelVertex) {
        if (drawable == nullptr) {
            throw new ArgumentNullException("drawable");
        }

        Entity* entity = drawable->get_Parent();
        if (entity == nullptr) {
            return modelVertex;
        }

        float3 scaledVertex = modelVertex * entity->get_Scale();
        float3 rotatedVertex = float4::RotateVector(scaledVertex, entity->get_Orientation());
        return rotatedVertex + entity->get_Position();
    }
}
#endif
