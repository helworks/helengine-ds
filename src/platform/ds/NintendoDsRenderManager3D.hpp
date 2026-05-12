#pragma once

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "MaterialAsset.hpp"
#include "ModelAsset.hpp"
#include "RenderManager3D.hpp"
#include "RenderTarget.hpp"
#include "RuntimeMaterial.hpp"
#include "RuntimeModel.hpp"
#include "ShaderAsset.hpp"

namespace helengine::ds {
    /// Provides the minimal 3D runtime surface required to initialize generated core on Nintendo DS.
    class NintendoDsRenderManager3D : public RenderManager3D {
    public:
        /// <summary>
        /// Builds one placeholder runtime material from the authored material asset.
        /// </summary>
        /// <param name="materialAsset">Authored material asset.</param>
        /// <param name="shaderAsset">Authored shader asset resolved for the material.</param>
        /// <returns>Placeholder runtime material carrying the authored metadata.</returns>
        RuntimeMaterial* BuildMaterialFromRaw(MaterialAsset* materialAsset, ShaderAsset* shaderAsset) override;

        /// <summary>
        /// Builds one placeholder runtime model from the authored model asset.
        /// </summary>
        /// <param name="data">Authored model asset.</param>
        /// <returns>Placeholder runtime model carrying the authored metadata.</returns>
        RuntimeModel* BuildModelFromRaw(ModelAsset* data) override;

        /// <summary>
        /// Builds one placeholder render target to satisfy runtime APIs that request off-screen buffers.
        /// </summary>
        /// <param name="width">Requested render-target width.</param>
        /// <param name="height">Requested render-target height.</param>
        /// <returns>Placeholder runtime render target.</returns>
        RenderTarget* CreateRenderTarget(int32_t width, int32_t height) override;
    };
}
#endif
