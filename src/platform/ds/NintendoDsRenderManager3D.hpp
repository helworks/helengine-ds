#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "MaterialAsset.hpp"
#include "ModelAsset.hpp"
#include "RenderManager3D.hpp"
#include "RenderTarget.hpp"
#include "RuntimeMaterial.hpp"
#include "RuntimeModel.hpp"
#include "ShaderAsset.hpp"

class ICamera;
class IDrawable3D;

namespace helengine::ds {
    class NintendoDsRuntimeMaterial;
    class NintendoDsRuntimeModel;
    class NintendoDsRenderQueueSnapshotVisitor;

    /// Provides the first visible 3D runtime surface for Nintendo DS generated core.
    class NintendoDsRenderManager3D : public RenderManager3D {
    public:
        /// <summary>
        /// Creates one DS 3D renderer with uninitialized hardware state.
        /// </summary>
        NintendoDsRenderManager3D();

        /// <summary>
        /// Builds one DS runtime material from the authored material asset.
        /// </summary>
        /// <param name="materialAsset">Authored material asset.</param>
        /// <param name="shaderAsset">Authored shader asset resolved for the material.</param>
        /// <returns>DS runtime material carrying the authored metadata required for the first renderer slice.</returns>
        RuntimeMaterial* BuildMaterialFromRaw(MaterialAsset* materialAsset, ShaderAsset* shaderAsset) override;

        /// <summary>
        /// Builds one DS runtime model from the authored model asset.
        /// </summary>
        /// <param name="data">Authored model asset.</param>
        /// <returns>DS runtime model carrying the authored metadata required for the first renderer slice.</returns>
        RuntimeModel* BuildModelFromRaw(ModelAsset* data) override;

        /// <summary>
        /// Builds one placeholder render target to satisfy runtime APIs that request off-screen buffers.
        /// </summary>
        /// <param name="width">Requested render-target width.</param>
        /// <param name="height">Requested render-target height.</param>
        /// <returns>Placeholder runtime render target.</returns>
        RenderTarget* CreateRenderTarget(int32_t width, int32_t height) override;

        /// <summary>
        /// Draws the current generated-core 3D frame through the Nintendo DS renderer path.
        /// </summary>
        void Draw() override;

    private:
        /// <summary>
        /// Stores whether the Nintendo DS 3D hardware state has already been initialized.
        /// </summary>
        bool HardwareInitialized;

        /// <summary>
        /// Stores the reusable ordered render-queue snapshot visitor used for the current frame.
        /// </summary>
        NintendoDsRenderQueueSnapshotVisitor* RenderQueueSnapshotVisitor;

        /// <summary>
        /// Initializes Nintendo DS 3D video mode and hardware state before the first frame.
        /// </summary>
        void EnsureHardwareInitialized();

        /// <summary>
        /// Clears the top-screen 3D frame from one runtime camera clear configuration.
        /// </summary>
        /// <param name="camera">Runtime camera providing the clear configuration.</param>
        void ClearFromCamera(ICamera* camera);

        /// <summary>
        /// Configures the DS projection and view state for one runtime camera.
        /// </summary>
        /// <param name="camera">Runtime camera providing the active view.</param>
        void ConfigureCamera(ICamera* camera);

        /// <summary>
        /// Draws the first-camera ordered 3D render queue through the DS geometry path.
        /// </summary>
        /// <param name="camera">Runtime camera owning the ordered 3D render queue.</param>
        int32_t DrawRenderQueue(ICamera* camera);

        /// <summary>
        /// Submits one supported opaque drawable through the DS triangle path.
        /// </summary>
        /// <param name="drawable">Runtime drawable to submit.</param>
        /// <param name="runtimeModel">DS runtime model carrying vertex data.</param>
        /// <param name="runtimeMaterial">DS runtime material carrying the flat color.</param>
        void SubmitOpaqueDrawable(IDrawable3D* drawable, NintendoDsRuntimeModel* runtimeModel, NintendoDsRuntimeMaterial* runtimeMaterial);

        /// <summary>
        /// Applies one drawable entity transform to a model-space vertex.
        /// </summary>
        /// <param name="drawable">Drawable providing the entity transform.</param>
        /// <param name="modelVertex">Model-space vertex to transform.</param>
        /// <returns>World-space vertex used by the first DS submission path.</returns>
        float3 TransformVertex(IDrawable3D* drawable, float3 modelVertex);
    };
}
#endif
