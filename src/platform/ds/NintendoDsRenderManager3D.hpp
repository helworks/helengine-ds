#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "Asset.hpp"
#include "ModelAsset.hpp"
#include "PlatformMaterialAsset.hpp"
#include "RenderManager3D.hpp"
#include "RenderTarget.hpp"
#include "RuntimeMaterial.hpp"
#include "RuntimeModel.hpp"
#include "platform/ds/NintendoDsScreenTarget.hpp"
#include <string>
#include <vector>
#include "float2.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "runtime/array.hpp"
extern "C" {
#include <nds/arm9/console.h>
#include <nds/arm9/videoGL.h>
}

class ICamera;
class Core;
class IDrawable3D;
class ObjectManager;

namespace helengine::ds {
    class NintendoDsRenderManager2D;
    struct NintendoDsRenderManager2DProfileSnapshot;
    class NintendoDsRuntimeMaterial;
    class NintendoDsRuntimeModel;
    class NintendoDsRuntimeTexture2D;
    class NintendoDsRenderQueueSnapshotVisitor;

    /// Provides the first visible 3D runtime surface for Nintendo DS generated core.
    class NintendoDsRenderManager3D : public RenderManager3D {
    public:
        /// <summary>
        /// Creates one DS 3D renderer with uninitialized hardware state.
        /// </summary>
        NintendoDsRenderManager3D();

        /// <summary>
        /// Builds one DS runtime material from one raw packaged material asset path.
        /// </summary>
        /// <param name="assetContentManager">Content manager that can load companion packaged assets.</param>
        /// <param name="contentRootPath">Absolute packaged content root that owns the serialized material asset.</param>
        /// <param name="materialAssetPath">Absolute material asset path requested by the runtime loader.</param>
        /// <returns>DS runtime material carrying the authored metadata required for the first renderer slice.</returns>
        RuntimeMaterial* BuildMaterialFromRawAsset(ContentManager* assetContentManager, std::string contentRootPath, std::string materialAssetPath) override;

        /// <summary>
        /// Builds one DS runtime material from a cooked platform-owned material payload.
        /// </summary>
        /// <param name="materialAsset">Cooked platform-owned material payload.</param>
        /// <returns>DS runtime material carrying the cooked metadata required for the first renderer slice.</returns>
        RuntimeMaterial* BuildMaterialFromCooked(PlatformMaterialAsset* materialAsset) override;

        /// <summary>
        /// Builds one DS runtime material from one cooked platform-owned material payload serialized on disk.
        /// </summary>
        /// <param name="cookedAssetPath">Absolute NitroFS or host path to the serialized cooked material asset.</param>
        /// <returns>DS runtime material carrying the cooked metadata required for the first renderer slice.</returns>
        RuntimeMaterial* BuildMaterialFromCooked(std::string cookedAssetPath) override;

        /// <summary>
        /// Builds one DS runtime model from the authored model asset.
        /// </summary>
        /// <param name="data">Authored model asset.</param>
        /// <returns>DS runtime model carrying the authored metadata required for the first renderer slice.</returns>
        RuntimeModel* BuildModelFromRaw(ModelAsset* data) override;

        /// <summary>
        /// Builds one DS runtime model from one cooked model payload serialized on disk.
        /// </summary>
        /// <param name="cookedAssetPath">Absolute NitroFS or host path to the serialized cooked model asset.</param>
        /// <returns>DS runtime model carrying the adopted cooked geometry payload.</returns>
        RuntimeModel* BuildModelFromCooked(std::string cookedAssetPath) override;

        /// <summary>
        /// Rejects one off-screen render-target request because the DS backend exposes only real hardware paths.
        /// </summary>
        /// <param name="width">Requested render-target width.</param>
        /// <param name="height">Requested render-target height.</param>
        /// <returns>Unsupported on Nintendo DS.</returns>
        RenderTarget* CreateRenderTarget(int32_t width, int32_t height) override;

        /// <summary>
        /// Releases one DS runtime material and any DS-owned heap state attached to it after a scene unload.
        /// </summary>
        /// <param name="material">Runtime material to release.</param>
        void ReleaseMaterial(RuntimeMaterial* material) override;

        /// <summary>
        /// Releases one DS runtime model and its adopted geometry buffers after a scene unload.
        /// </summary>
        /// <param name="model">Runtime model to release.</param>
        void ReleaseModel(RuntimeModel* model) override;

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

        /// <summary>
        /// Gets which Nintendo DS screen most recently owned the hardware 3D pass.
        /// </summary>
        /// <returns>Most recently selected hardware 3D screen target.</returns>
        NintendoDsScreenTarget get_LastHardware3DScreenTarget() const;

        /// <summary>
        /// Gets the most recent 3D queue size observed for the selected hardware 3D camera.
        /// </summary>
        /// <returns>Most recent 3D queue size for the active hardware 3D camera.</returns>
        int32_t get_LastCamera3DQueueCount() const;

        /// <summary>
        /// Gets the most recent number of 3D drawables submitted during one frame.
        /// </summary>
        /// <returns>Most recent 3D submitted-drawable count.</returns>
        int32_t get_LastSubmittedDrawableCount() const;

        /// <summary>
        /// Gets the most recent 2D queue size observed for the top-screen camera set during one frame.
        /// </summary>
        /// <returns>Most recent top-screen 2D queue size.</returns>
        int32_t get_LastTopScreen2DQueueCount() const;

        /// <summary>
        /// Gets the most recent 2D queue size observed for the bottom-screen camera set during one frame.
        /// </summary>
        /// <returns>Most recent bottom-screen 2D queue size.</returns>
        int32_t get_LastBottomScreen2DQueueCount() const;

        /// <summary>
        /// Gets the most recent net allocator delta observed during the 2D camera traversal portion of one draw call.
        /// </summary>
        /// <returns>Most recent 2D camera traversal allocator delta in bytes.</returns>
        int32_t get_Last2DTraversalNetByteDelta() const;

        /// <summary>
        /// Gets the most recent net allocator delta observed during the 3D submission portion of one draw call.
        /// </summary>
        /// <returns>Most recent 3D submission allocator delta in bytes.</returns>
        int32_t get_Last3DSubmissionNetByteDelta() const;

        /// <summary>
        /// Gets the most recent net allocator delta observed while releasing one scene-owned runtime material.
        /// </summary>
        /// <returns>Most recent runtime-material release allocator delta in bytes.</returns>
        int32_t get_LastReleaseMaterialNetByteDelta() const;

        /// <summary>
        /// Gets the most recent net allocator delta observed while releasing one scene-owned runtime model.
        /// </summary>
        /// <returns>Most recent runtime-model release allocator delta in bytes.</returns>
        int32_t get_LastReleaseModelNetByteDelta() const;

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
        /// Stores which Nintendo DS screen most recently owned the hardware 3D pass.
        /// </summary>
        NintendoDsScreenTarget LastHardware3DScreenTarget;

        /// <summary>
        /// Stores which Nintendo DS screen most recently had its hardware display mode configured for 3D.
        /// </summary>
        NintendoDsScreenTarget LastConfiguredHardware3DScreenTarget;

        /// <summary>
        /// Stores whether the last hardware 3D display-mode configuration kept the bottom bitmap screen enabled.
        /// </summary>
        bool LastConfiguredBottomScreenPresentationEnabled;

        /// <summary>
        /// Stores the most recent 3D queue size observed for the selected hardware 3D camera.
        /// </summary>
        int32_t LastCamera3DQueueCount;

        /// <summary>
        /// Stores the most recent number of 3D drawables submitted during one frame.
        /// </summary>
        int32_t LastSubmittedDrawableCount;

        /// <summary>
        /// Stores the most recent number of packed display-list calls submitted during one frame.
        /// </summary>
        int32_t Last3DDisplayListCallCount;

        /// <summary>
        /// Stores the most recent number of packed display-list calls using quad primitives during one frame.
        /// </summary>
        int32_t Last3DQuadDisplayListCallCount;

        /// <summary>
        /// Stores the most recent total display-list command words submitted during one frame.
        /// </summary>
        uint32_t Last3DDisplayListSubmittedWordCount;

        /// <summary>
        /// Stores the most recent top-screen 2D queue size observed during one frame.
        /// </summary>
        int32_t LastTopScreen2DQueueCount;

        /// <summary>
        /// Stores the most recent bottom-screen 2D queue size observed during one frame.
        /// </summary>
        int32_t LastBottomScreen2DQueueCount;

        /// <summary>
        /// Stores the most recent net allocator delta observed during 2D camera traversal.
        /// </summary>
        int32_t Last2DTraversalNetByteDelta;

        /// <summary>
        /// Stores the most recent net allocator delta observed during 3D submission.
        /// </summary>
        int32_t Last3DSubmissionNetByteDelta;

        /// <summary>
        /// Stores the most recent net allocator delta observed while releasing one runtime material.
        /// </summary>
        int32_t LastReleaseMaterialNetByteDelta;

        /// <summary>
        /// Stores the most recent net allocator delta observed while releasing one runtime model.
        /// </summary>
        int32_t LastReleaseModelNetByteDelta;

        /// <summary>
        /// Stores the most recent 2D camera traversal duration observed during one draw call.
        /// </summary>
        double Last2DTraversalMilliseconds;

        /// <summary>
        /// Stores the most recent 3D target, lighting, camera, and hardware setup duration observed during one draw call.
        /// </summary>
        double Last3DSetupMilliseconds;

        /// <summary>
        /// Stores the most recent ordered 3D queue snapshot duration observed during one draw call.
        /// </summary>
        double Last3DQueueSnapshotMilliseconds;

        /// <summary>
        /// Stores the most recent 3D geometry emission duration observed during one draw call.
        /// </summary>
        double Last3DGeometryEmitMilliseconds;

        /// <summary>
        /// Stores the most recent accumulated drawable transform submission duration observed during one draw call.
        /// </summary>
        double Last3DTransformMilliseconds;

        /// <summary>
        /// Stores the most recent accumulated material and lighting state submission duration observed during one draw call.
        /// </summary>
        double Last3DMaterialMilliseconds;

        /// <summary>
        /// Stores the most recent accumulated packed display-list submission duration observed during one draw call.
        /// </summary>
        double Last3DDisplayListMilliseconds;

        /// <summary>
        /// Stores the most recent accumulated time spent waiting before packed display-list DMA can start.
        /// </summary>
        double Last3DDisplayListPreWaitMilliseconds;

        /// <summary>
        /// Stores the most recent accumulated time spent programming packed display-list DMA registers.
        /// </summary>
        double Last3DDisplayListKickMilliseconds;

        /// <summary>
        /// Stores the most recent accumulated time spent waiting for packed display-list DMA completion.
        /// </summary>
        double Last3DDisplayListPostWaitMilliseconds;

        /// <summary>
        /// Stores the most recent accumulated fallback vertex submission duration observed during one draw call.
        /// </summary>
        double Last3DFallbackGeometryMilliseconds;

        /// <summary>
        /// Stores the most recent DS geometry flush duration observed during one draw call.
        /// </summary>
        double Last3DFlushMilliseconds;

        /// <summary>
        /// Stores the most recent 2D bitmap presentation duration observed during one draw call.
        /// </summary>
        double LastPresentMilliseconds;

        /// <summary>
        /// Tracks whether the most recent 3D frame bound a material diffuse texture.
        /// </summary>
        bool LastHardwareTextureMaterialBound;

        /// <summary>
        /// Tracks whether the most recent 3D frame attempted a hardware texture upload.
        /// </summary>
        bool LastHardwareTextureUploadAttempted;

        /// <summary>
        /// Tracks whether the last bound hardware texture was marked uploaded after renderer processing.
        /// </summary>
        bool LastHardwareTextureUploaded;

        /// <summary>
        /// Stores the libnds texture id most recently bound by the 3D textured path.
        /// </summary>
        int32_t LastHardwareTextureId;

        /// <summary>
        /// Stores the width of the most recently bound 3D texture.
        /// </summary>
        int32_t LastHardwareTextureWidth;

        /// <summary>
        /// Stores the height of the most recently bound 3D texture.
        /// </summary>
        int32_t LastHardwareTextureHeight;

        /// <summary>
        /// Stores the cooked color payload byte length of the most recently bound 3D texture.
        /// </summary>
        int32_t LastHardwareTextureColorLength;

        /// <summary>
        /// Stores the cooked palette payload byte length of the most recently bound 3D texture.
        /// </summary>
        int32_t LastHardwareTexturePaletteColorLength;

        /// <summary>
        /// Stores a compact label for the cooked color format of the most recently bound 3D texture.
        /// </summary>
        std::string LastHardwareTextureFormat;

        /// <summary>
        /// Tracks whether the most recently submitted textured material requested fixed-function lighting.
        /// </summary>
        bool LastHardwareTextureLightingEnabled;

        /// <summary>
        /// Counts textured triangles submitted during the most recent 3D frame.
        /// </summary>
        int32_t LastHardwareTexturedTriangleCount;

        /// <summary>
        /// Stores the strongest CPU-expected diffuse term among textured triangles in the most recent 3D frame.
        /// </summary>
        float LastHardwareTexturedMaxDiffuse;

        /// <summary>
        /// Decodes one float4 constant-buffer payload from little-endian bytes.
        /// </summary>
        /// <param name="data">Constant-buffer payload to decode.</param>
        /// <param name="decodedColor">Decoded float4 value.</param>
        /// <returns>True when the payload length was sufficient to decode four floats.</returns>
        static bool TryDecodeFloat4ConstantBuffer(Array<uint8_t>* data, float4& decodedColor);

        /// <summary>
        /// Builds one packed Nintendo DS FIFO command stream for fixed-function lit static geometry.
        /// </summary>
        /// <param name="runtimeModel">Runtime model carrying adopted position and index buffers.</param>
        /// <param name="displayListWordCount">Receives the number of words after the display-list length word.</param>
        /// <returns>Owned packed display-list words, or null when no valid triangles exist.</returns>
        uint32_t* BuildHardwareLitDisplayList(NintendoDsRuntimeModel* runtimeModel, uint32_t& displayListWordCount);

        /// <summary>
        /// Builds one quad-only packed command stream when the indexed triangle list is fully reducible to DS quads.
        /// </summary>
        /// <param name="runtimeModel">Runtime model carrying adopted position and index buffers.</param>
        /// <param name="displayListWordCount">Receives the number of words after the display-list length word.</param>
        /// <returns>Owned packed quad display-list words, or null when the model cannot be represented only as quads.</returns>
        uint32_t* BuildHardwareLitQuadDisplayList(NintendoDsRuntimeModel* runtimeModel, uint32_t& displayListWordCount);

        /// <summary>
        /// Flushes one immutable packed display-list payload after construction so frame submission can DMA without cache maintenance.
        /// </summary>
        /// <param name="displayList">Display-list buffer whose first word stores payload word count.</param>
        void FlushHardwareLitDisplayList(uint32_t* displayList) const;

        /// <summary>
        /// Submits one immutable packed display list through synchronous DMA without per-frame data-cache flushing.
        /// </summary>
        /// <param name="runtimeModel">Runtime model carrying the pre-flushed display-list payload.</param>
        void SubmitStaticHardwareDisplayList(NintendoDsRuntimeModel* runtimeModel);

        /// <summary>
        /// Resolves whether one runtime model should use the prebuilt static display-list path or direct immediate submission for the current draw.
        /// </summary>
        /// <param name="runtimeModel">Runtime model being submitted.</param>
        /// <param name="useHardwareTexture">Whether the current draw already requires the textured immediate path.</param>
        /// <returns>True when the model should use the static display-list path for the current draw.</returns>
        bool ShouldUseStaticHardwareDisplayList(NintendoDsRuntimeModel* runtimeModel, bool useHardwareTexture) const;

        /// <summary>
        /// Counts how many triangles one runtime model would submit through the immediate geometry path.
        /// </summary>
        /// <param name="runtimeModel">Runtime model whose triangle count should be resolved.</param>
        /// <returns>Number of triangles represented by the current index or position data.</returns>
        int32_t ResolveTrianglePrimitiveCount(NintendoDsRuntimeModel* runtimeModel) const;

        /// <summary>
        /// Attempts to append one quad represented by two indexed triangles that share the same diagonal.
        /// </summary>
        /// <param name="displayListWords">Mutable packed command stream body, excluding the length word.</param>
        /// <param name="positions">Model-space positions used by the triangle pair.</param>
        /// <param name="indexA">First index of the first triangle.</param>
        /// <param name="indexC">Second index of the first triangle and first index of the second triangle.</param>
        /// <param name="indexB">Third index of the first triangle.</param>
        /// <param name="secondIndexC">First index of the second triangle.</param>
        /// <param name="secondIndexA">Second index of the second triangle.</param>
        /// <param name="indexD">Third index of the second triangle.</param>
        /// <returns>True when one compatible quad was appended.</returns>
        bool TryAppendHardwareLitDisplayListQuad(
            std::vector<uint32_t>& displayListWords,
            Array<float3>* positions,
            int32_t indexA,
            int32_t indexC,
            int32_t indexB,
            int32_t secondIndexC,
            int32_t secondIndexA,
            int32_t indexD,
            bool useVertex10);

        /// <summary>
        /// Appends one normal and four vertices to a packed Nintendo DS quad command stream.
        /// </summary>
        /// <param name="displayListWords">Mutable packed command stream body, excluding the length word.</param>
        /// <param name="positions">Model-space positions used by the quad.</param>
        /// <param name="indexA">First quad position index.</param>
        /// <param name="indexD">Second quad position index.</param>
        /// <param name="indexC">Third quad position index.</param>
        /// <param name="indexB">Fourth quad position index.</param>
        /// <param name="useVertex10">Whether vertices should use the compact one-word DS VTX10 command.</param>
        void AppendHardwareLitDisplayListQuad(
            std::vector<uint32_t>& displayListWords,
            Array<float3>* positions,
            int32_t indexA,
            int32_t indexD,
            int32_t indexC,
            int32_t indexB,
            bool useVertex10);

        /// <summary>
        /// Appends one triangle's normal and vertices to a packed Nintendo DS FIFO command stream.
        /// </summary>
        /// <param name="displayListWords">Mutable packed command stream body, excluding the length word.</param>
        /// <param name="positions">Model-space positions used by the triangle.</param>
        /// <param name="indexA">First triangle position index.</param>
        /// <param name="indexB">Second triangle position index.</param>
        /// <param name="indexC">Third triangle position index.</param>
        /// <param name="useVertex10">Whether vertices should use the compact one-word DS VTX10 command.</param>
        void AppendHardwareLitDisplayListTriangle(
            std::vector<uint32_t>& displayListWords,
            Array<float3>* positions,
            int32_t indexA,
            int32_t indexB,
            int32_t indexC,
            bool useVertex10);

        /// <summary>
        /// Appends one model-space vertex to a packed Nintendo DS FIFO command stream.
        /// </summary>
        /// <param name="displayListWords">Mutable packed command stream body, excluding the length word.</param>
        /// <param name="position">Model-space position to encode as v16 values.</param>
        void AppendHardwareLitDisplayListVertex(std::vector<uint32_t>& displayListWords, const float3& position);

        /// <summary>
        /// Appends one model-space vertex to a packed Nintendo DS FIFO command stream using the compact VTX10 format.
        /// </summary>
        /// <param name="displayListWords">Mutable packed command stream body, excluding the length word.</param>
        /// <param name="position">Model-space position to encode as signed 10-bit vertex values.</param>
        void AppendHardwareLitDisplayListVertex10(std::vector<uint32_t>& displayListWords, const float3& position);

        /// <summary>
        /// Resolves whether all model-space positions fit the compact signed 10-bit DS vertex command range.
        /// </summary>
        /// <param name="positions">Model-space positions to inspect.</param>
        /// <returns>True when every vertex can be emitted through VTX10 without overflow.</returns>
        bool CanUseHardwareLitVertex10DisplayList(Array<float3>* positions) const;

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
        /// Draws all software 2D cameras after hardware-3D ownership has been resolved.
        /// </summary>
        /// <param name="cameras">Active runtime camera list for the current frame.</param>
        /// <param name="renderManager2D">Nintendo DS 2D renderer receiving camera queue traversal for the same frame.</param>
        void Draw2DCameraList(List<ICamera*>* cameras, NintendoDsRenderManager2D* renderManager2D);

        /// <summary>
        /// Configures which Nintendo DS physical screen currently owns the hardware 3D main-engine presentation.
        /// </summary>
        /// <param name="targetScreen">Screen that should own the hardware 3D pass for the current frame.</param>
        /// <param name="renderManager2D">Nintendo DS 2D renderer that may reserve the bottom screen for native-console diagnostics.</param>
        void ConfigureHardware3DTarget(NintendoDsScreenTarget targetScreen, NintendoDsRenderManager2D* renderManager2D);

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
        /// Applies one drawable entity transform to the Nintendo DS model-view matrix before model-space vertices are submitted.
        /// </summary>
        /// <param name="entityPosition">World-space entity position.</param>
        /// <param name="entityScale">World-space entity scale.</param>
        /// <param name="entityOrientation">World-space entity orientation.</param>
        void ApplyDrawableTransformToHardwareMatrix(
            const float3& entityPosition,
            const float3& entityScale,
            const float4& entityOrientation);

        /// <summary>
        /// Builds one fixed-point Nintendo DS 4x3 affine transform matrix from runtime entity transform components.
        /// </summary>
        /// <param name="transformMatrix">Output matrix written in the order consumed by libnds matrix multiplication.</param>
        /// <param name="entityPosition">World-space entity position.</param>
        /// <param name="entityScale">World-space entity scale.</param>
        /// <param name="entityOrientation">World-space entity orientation.</param>
        void BuildDrawableTransformMatrix(
            m4x3& transformMatrix,
            const float3& entityPosition,
            const float3& entityScale,
            const float4& entityOrientation) const;

        /// <summary>
        /// Configures Nintendo DS fixed-function frame light state from the active camera/view matrix.
        /// </summary>
        void ConfigureFrameHardwareLight();

        /// <summary>
        /// Configures Nintendo DS fixed-function material state for one lit drawable.
        /// </summary>
        /// <param name="runtimeMaterial">DS runtime material carrying the authored base color.</param>
        void ConfigureHardwareMaterial(NintendoDsRuntimeMaterial* runtimeMaterial);

        /// <summary>
        /// Configures a DS hardware texture for one material when a runtime texture is bound.
        /// </summary>
        /// <param name="runtimeMaterial">DS runtime material that may resolve to a runtime texture.</param>
        /// <param name="runtimeTexture">Receives the DS runtime texture bound for hardware sampling.</param>
        /// <returns>True when a compatible runtime texture was bound for hardware sampling.</returns>
        bool TryConfigureHardwareTexture(NintendoDsRuntimeMaterial* runtimeMaterial, NintendoDsRuntimeTexture2D*& runtimeTexture);

        /// <summary>
        /// Uploads one runtime texture into DS texture VRAM if it has not already been uploaded.
        /// </summary>
        /// <param name="runtimeTexture">Runtime texture carrying the cooked source texel payload.</param>
        void EnsureHardwareTextureUploaded(NintendoDsRuntimeTexture2D* runtimeTexture);

        /// <summary>
        /// Builds one temporary DS direct-color texture payload from the cooked runtime texture.
        /// </summary>
        /// <param name="runtimeTexture">Runtime texture carrying the cooked source texel payload.</param>
        /// <returns>Direct-color DS texture pixels in row-major order.</returns>
        std::vector<uint16_t> BuildHardwareTexturePixels(NintendoDsRuntimeTexture2D* runtimeTexture) const;

        /// <summary>
        /// Converts one RGBA texel into the DS direct-color texture representation.
        /// </summary>
        /// <param name="red">Source red channel.</param>
        /// <param name="green">Source green channel.</param>
        /// <param name="blue">Source blue channel.</param>
        /// <param name="alpha">Source alpha channel.</param>
        /// <returns>Packed DS direct-color texture texel.</returns>
        uint16_t PackHardwareTexturePixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha) const;

        /// <summary>
        /// Resolves the libnds texture-size enum for one supported power-of-two dimension.
        /// </summary>
        /// <param name="size">Texture width or height in pixels.</param>
        /// <returns>Libnds texture-size enum value.</returns>
        int32_t ResolveHardwareTextureSize(int32_t size) const;

        /// <summary>
        /// Packs one normalized model-space normal component into the signed 10-bit DS normal range.
        /// </summary>
        /// <param name="value">Normalized normal component.</param>
        /// <returns>Signed 10-bit normal component encoded for NORMAL_PACK.</returns>
        int32_t PackHardwareNormalComponent(float value) const;

        /// <summary>
        /// Packs one model-space coordinate into the signed 10-bit DS VTX10 range.
        /// </summary>
        /// <param name="value">Model-space coordinate to encode with six fractional bits.</param>
        /// <returns>Signed 10-bit vertex coordinate encoded for the FIFO_VERTEX10 command.</returns>
        int32_t PackHardwareVertex10Component(float value) const;

        /// <summary>
        /// Resolves frame lighting once from runtime-managed light collections before drawable submission begins.
        /// </summary>
        /// <param name="objectManager">Runtime object manager that owns registered light collections.</param>
        void ResolveFrameLighting(ObjectManager* objectManager);

        /// <summary>
        /// Publishes the latest Nintendo DS renderer timing buckets through the generated-core performance overlay contract.
        /// </summary>
        /// <param name="core">Active generated-core runtime instance.</param>
        /// <param name="renderManager2D">Nintendo DS 2D renderer that owns the current frame's software-raster profile.</param>
        /// <param name="usesMetrics">True when the current frame produced hardware 3D profiling metrics.</param>
        void PublishPerformanceOverlayMetrics(Core* core, NintendoDsRenderManager2D* renderManager2D, bool usesMetrics);

        /// <summary>
        /// Formats the compact DS trace row that summarizes queue counts and top-level timing buckets.
        /// </summary>
        /// <returns>Compact detail row shown beneath the FPS rows.</returns>
        std::string FormatPerformanceOverlayDetailText() const;

        /// <summary>
        /// Formats the DS multi-line trace block that expands deeper geometry-submission timings.
        /// </summary>
        /// <param name="profileSnapshot">Current-frame 2D profile snapshot used to attribute mixed 2D and 3D costs.</param>
        /// <returns>Multi-line text block rendered beneath the detail row.</returns>
        std::string FormatPerformanceOverlayAdditionalText(const NintendoDsRenderManager2DProfileSnapshot& profileSnapshot) const;

        /// <summary>
        /// Captures compact diagnostics for the most recent runtime texture considered by the 3D hardware path.
        /// </summary>
        /// <param name="runtimeTexture">Runtime texture that was considered for hardware sampling.</param>
        /// <param name="uploadAttempted">True when this sample followed a hardware upload attempt.</param>
        void RecordHardwareTextureDiagnostics(NintendoDsRuntimeTexture2D* runtimeTexture, bool uploadAttempted);

        /// <summary>
        /// Formats the latest hardware texture diagnostics for the native overlay.
        /// </summary>
        /// <returns>Compact native overlay row describing the latest texture state.</returns>
        std::string FormatHardwareTextureDiagnostics() const;

        /// <summary>
        /// Formats the latest textured-lighting diagnostics for the native overlay.
        /// </summary>
        /// <returns>Compact native overlay row describing textured material lighting state.</returns>
        std::string FormatHardwareTextureLightingDiagnostics() const;

        /// <summary>
        /// Submits one quad normal and vertices through the DS fixed-function lighting path.
        /// </summary>
        void SubmitHardwareLitQuad(
            Array<float3>* positions,
            int32_t indexA,
            int32_t indexD,
            int32_t indexC,
            int32_t indexB);

        /// <summary>
        /// Submits one triangle normal and vertices through the DS fixed-function lighting path.
        /// </summary>
        void SubmitHardwareLitTriangle(
            Array<float3>* positions,
            int32_t indexA,
            int32_t indexB,
            int32_t indexC);

        /// <summary>
        /// Submits one triangle normal, texture coordinates, and vertices through the DS fixed-function texturing path.
        /// </summary>
        void SubmitHardwareTexturedTriangle(
            Array<float3>* positions,
            Array<float2>* texCoords,
            NintendoDsRuntimeTexture2D* runtimeTexture,
            bool lightingEnabled,
            int32_t indexA,
            int32_t indexB,
            int32_t indexC);

        /// <summary>
        /// Submits one textured vertex with a normalized model UV converted to DS texture coordinates.
        /// </summary>
        void SubmitHardwareTexturedVertex(
            Array<float3>* positions,
            Array<float2>* texCoords,
            NintendoDsRuntimeTexture2D* runtimeTexture,
            bool lightingEnabled,
            const float3& modelFaceNormal,
            int32_t index);
    };
}
#endif
