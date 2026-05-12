#include "platform/ds/NintendoDsRenderManager3D.hpp"

#if HELENGINE_NINTENDO_DS_HAS_GENERATED_CORE
#include "runtime/native_exceptions.hpp"

namespace helengine::ds {
    /// Builds one placeholder runtime material from the authored material asset.
    /// <param name="materialAsset">Authored material asset.</param>
    /// <param name="shaderAsset">Authored shader asset resolved for the material.</param>
    /// <returns>Placeholder runtime material carrying the authored metadata.</returns>
    RuntimeMaterial* NintendoDsRenderManager3D::BuildMaterialFromRaw(MaterialAsset* materialAsset, ShaderAsset* shaderAsset) {
        if (materialAsset == nullptr) {
            throw new ArgumentNullException("materialAsset");
        } else if (shaderAsset == nullptr) {
            throw new ArgumentNullException("shaderAsset");
        }

        RuntimeMaterial* runtimeMaterial = new RuntimeMaterial();
        runtimeMaterial->set_Id(materialAsset->get_Id());
        if (materialAsset->RenderState != nullptr) {
            runtimeMaterial->SetRenderState(materialAsset->RenderState);
        }

        return runtimeMaterial;
    }

    /// Builds one placeholder runtime model from the authored model asset.
    /// <param name="data">Authored model asset.</param>
    /// <returns>Placeholder runtime model carrying the authored metadata.</returns>
    RuntimeModel* NintendoDsRenderManager3D::BuildModelFromRaw(ModelAsset* data) {
        if (data == nullptr) {
            throw new ArgumentNullException("data");
        }

        RuntimeModel* runtimeModel = new RuntimeModel();
        runtimeModel->set_Id(data->get_Id());
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
}
#endif
