#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "MaterialAsset.hpp"
#include "ModelAsset.hpp"
#include "PlatformMaterialAsset.hpp"
#include "RenderManager3D.hpp"
#include "RenderTarget.hpp"
#include "RuntimeMaterial.hpp"
#include "RuntimeModel.hpp"
#include "ShaderAsset.hpp"
#include "platform/ds/NintendoDsScreenTarget.hpp"
#include <string>
#include "float3.hpp"
#include "float4.hpp"
#include "runtime/array.hpp"

class ICamera;
class IDrawable3D;
class ObjectManager;

namespace helengine::ds {
    class NintendoDsRenderManager2D;
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
        /// Builds one DS runtime material from authored material metadata.
        /// </summary>
        /// <param name="materialAsset">Authored material asset.</param>
        /// <param name="shaderAsset">Optional authored shader metadata carried by cross-platform runtime contracts.</param>
        /// <returns>DS runtime material carrying the authored metadata required for the first renderer slice.</returns>
        RuntimeMaterial* BuildMaterialFromRaw(MaterialAsset* materialAsset, ShaderAsset* shaderAsset) override;

        /// <summary>
        /// Builds one DS runtime material from a cooked platform-owned material payload.
        /// </summary>
        /// <param name="materialAsset">Cooked platform-owned material payload.</param>
        /// <returns>DS runtime material carrying the cooked metadata required for the first renderer slice.</returns>
        RuntimeMaterial* BuildMaterialFromCooked(PlatformMaterialAsset* materialAsset) override;

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

        /// <summary>
        /// Resets the last runtime asset-build diagnostic state before one traced scene-load attempt.
        /// </summary>
        void ResetLastBuildDiagnostics();

        /// <summary>
        /// Gets the last DS runtime asset-build stage reached by model or material construction.
        /// </summary>
        /// <returns>Last recorded asset-build stage.</returns>
        std::string get_LastBuildStage() const;

        /// <summary>
        /// Gets the last authored asset id seen by the DS runtime asset-build path.
        /// </summary>
        /// <returns>Last recorded authored asset id.</returns>
        std::string get_LastBuildAssetId() const;

    private:
        /// Stores the standard material constant-buffer name used for authored base color.
        static constexpr const char* StandardMaterialBaseColorBufferName = "BaseColorBuffer";

        /// <summary>
        /// Stores whether the Nintendo DS 3D hardware state has already been initialized.
        /// </summary>
        bool HardwareInitialized;

        /// <summary>
        /// Stores the reusable ordered render-queue snapshot visitor used for the current frame.
        /// </summary>
        NintendoDsRenderQueueSnapshotVisitor* RenderQueueSnapshotVisitor;

        /// <summary>
        /// Stores the last runtime asset-build stage reached during traced scene materialization.
        /// </summary>
        std::string LastBuildStage;

        /// <summary>
        /// Stores the last authored asset id observed during traced scene materialization.
        /// </summary>
        std::string LastBuildAssetId;

        /// <summary>
        /// Stores the resolved frame directional-light direction used for triangle lighting during the active draw call.
        /// </summary>
        float3 FrameLightDirection;

        /// <summary>
        /// Stores the resolved frame directional-light radiance used for triangle lighting during the active draw call.
        /// </summary>
        float3 FrameDirectionalRadiance;

        /// <summary>
        /// Stores the resolved frame ambient radiance used for triangle lighting during the active draw call.
        /// </summary>
        float3 FrameAmbientRadiance;

        /// <summary>
        /// Resolves one authored standard-material base color from cooked constant-buffer payloads.
        /// </summary>
        /// <param name="materialAsset">Authored material asset carrying cooked constant buffers.</param>
        /// <param name="resolvedColor">Resolved normalized RGB base color.</param>
        /// <returns>True when one valid base-color buffer was decoded.</returns>
        bool TryResolveStandardMaterialBaseColor(MaterialAsset* materialAsset, float3& resolvedColor) const;

        /// <summary>
        /// Decodes one float4 constant-buffer payload from little-endian bytes.
        /// </summary>
        /// <param name="data">Constant-buffer payload to decode.</param>
        /// <param name="decodedColor">Decoded float4 value.</param>
        /// <returns>True when the payload length was sufficient to decode four floats.</returns>
        static bool TryDecodeFloat4ConstantBuffer(Array<uint8_t>* data, float4& decodedColor);

        /// <summary>
        /// Resolves which Nintendo DS screen should own the hardware 3D pass for the current frame.
        /// </summary>
        /// <param name="cameras">Active runtime camera list for the current frame.</param>
        /// <param name="renderManager2D">Nintendo DS 2D renderer receiving camera queue traversal for the same frame.</param>
        /// <returns>Hardware 3D target screen chosen for the current frame.</returns>
        NintendoDsScreenTarget ResolveHardware3DScreenTarget(List<ICamera*>* cameras, NintendoDsRenderManager2D* renderManager2D);

        /// <summary>
        /// Accumulates whether one camera contributes 3D queue content to the top or bottom Nintendo DS screen.
        /// </summary>
        /// <param name="camera">Runtime camera to inspect.</param>
        /// <param name="topScreenHas3D">Receives whether the top screen has any 3D content.</param>
        /// <param name="bottomScreenHas3D">Receives whether the bottom screen has any 3D content.</param>
        void AccumulateCameraScreenQueues(ICamera* camera, bool& topScreenHas3D, bool& bottomScreenHas3D) const;

        /// <summary>
        /// Configures which Nintendo DS physical screen currently owns the hardware 3D main-engine presentation.
        /// </summary>
        /// <param name="targetScreen">Screen that should own the hardware 3D pass for the current frame.</param>
        void ConfigureHardware3DTarget(NintendoDsScreenTarget targetScreen);

        /// <summary>
        /// Resolves which Nintendo DS physical screen one runtime camera targets.
        /// </summary>
        /// <param name="camera">Runtime camera whose viewport should be resolved.</param>
        /// <returns>Top or bottom screen target resolved from the camera viewport.</returns>
        NintendoDsScreenTarget ResolveCameraScreenTarget(ICamera* camera) const;

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
        /// Resolves frame lighting once from runtime-managed light collections before drawable submission begins.
        /// </summary>
        /// <param name="objectManager">Runtime object manager that owns registered light collections.</param>
        void ResolveFrameLighting(ObjectManager* objectManager);

        /// <summary>
        /// Submits one lit triangle through the DS immediate-mode geometry path.
        /// </summary>
        void SubmitLitTriangle(
            NintendoDsRuntimeMaterial* runtimeMaterial,
            Array<float3>* positions,
            int32_t indexA,
            int32_t indexB,
            int32_t indexC,
            const float3& entityPosition,
            const float3& entityScale,
            const float4& entityOrientation);

        /// <summary>
        /// Applies one drawable entity transform to a model-space vertex.
        /// </summary>
        /// <param name="drawable">Drawable providing the entity transform.</param>
        /// <param name="modelVertex">Model-space vertex to transform.</param>
        /// <returns>World-space vertex used by the first DS submission path.</returns>
        float3 TransformVertex(
            float3 modelVertex,
            const float3& entityPosition,
            const float3& entityScale,
            const float4& entityOrientation);
    };
}
#endif
