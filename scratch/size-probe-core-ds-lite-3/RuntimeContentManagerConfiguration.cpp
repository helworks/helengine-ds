#ifdef DrawText
#undef DrawText
#endif
#include "RuntimeContentManagerConfiguration.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "ContentManager.hpp"
#include "RuntimeContentProcessorIds.hpp"
#include "AssetContentProcessor_1.hpp"
#include "MaterialAsset.hpp"
#include "RuntimeContentManagerConfiguration.hpp"
#include "ModelAsset.hpp"
#include "TextureAsset.hpp"
#include "TextAsset.hpp"
#include "BinaryContentProcessor_1.hpp"
#include "SceneAsset.hpp"
#include "EditorAssetBinarySerializer.hpp"
#include "AnimationClipAsset.hpp"
#include "FontAsset.hpp"
#include "FontAssetBinarySerializer.hpp"
#include "IContentProcessor_1.hpp"
#include "runtime/array.hpp"
#include "runtime/native_dictionary.hpp"
#include "ContentProcessorRegistration.hpp"
#include "runtime/native_type.hpp"
#include "system/io/stream.hpp"
#include "MaterialRenderState.hpp"
#include "float3.hpp"
#include "float2.hpp"
#include "ModelSubmeshAsset.hpp"
#include "TextureAssetColorFormat.hpp"
#include "TextureAssetAlphaPrecision.hpp"
#include "system/func.hpp"
#include "SceneEntityAsset.hpp"
#include "SceneAssetReference.hpp"
#include "SceneSettingsAsset.hpp"
#include "EditorBinaryRecordKind.hpp"
#include "Asset.hpp"
#include "EngineBinaryHeader.hpp"
#include "EngineBinaryReader.hpp"
#include "AnimationClipPlatformOverrideAsset.hpp"
#include "AnimationInterpolationMode.hpp"
#include "EditorAssetBinaryValueKind.hpp"
#include "float4.hpp"
#include "SceneComponentAssetRecord.hpp"
#include "PlatformMaterialAsset.hpp"
#include "PlatformPositionKeyframeTrackAsset.hpp"
#include "PlatformRotationKeyframeTrackAsset.hpp"
#include "PositionKeyframeAsset.hpp"
#include "PositionKeyframeTrackAsset.hpp"
#include "PositionOffsetKeyframeTrackAsset.hpp"
#include "RotationKeyframeAsset.hpp"
#include "RotationKeyframeTrackAsset.hpp"
#include "ScaleKeyframeTrackAsset.hpp"
#include "SceneCanvasProfile.hpp"
#include "SceneEntityPlatformAddedComponentAsset.hpp"
#include "SceneEntityPlatformComponentOverrideAsset.hpp"
#include "SceneEntityPlatformExistenceOverrideAsset.hpp"
#include "SceneEntityPlatformTransformOverrideAsset.hpp"
#include "FontInfo.hpp"
#include "RuntimeTexture.hpp"
#include "FontChar.hpp"
#include "FontTightMetrics.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"
#include "system/io/stream.hpp"

void RuntimeContentManagerConfiguration::ConfigureSharedAssetContentManager(::ContentManager* contentManager)
{
    if (contentManager == nullptr)
    {
throw new ArgumentNullException("contentManager");
    }
RuntimeContentManagerConfiguration::RegisterProcessorIfMissing<MaterialAsset*>(contentManager, RuntimeContentProcessorIds::MaterialAsset, new ::AssetContentProcessor_1<::MaterialAsset*>(), new Array<std::string>({ MaterialAssetExtension }));
RuntimeContentManagerConfiguration::RegisterProcessorIfMissing<ModelAsset*>(contentManager, RuntimeContentProcessorIds::ModelAsset, new ::AssetContentProcessor_1<::ModelAsset*>(), nullptr);
RuntimeContentManagerConfiguration::RegisterProcessorIfMissing<TextureAsset*>(contentManager, RuntimeContentProcessorIds::TextureAsset, new ::AssetContentProcessor_1<::TextureAsset*>(), nullptr);
RuntimeContentManagerConfiguration::RegisterProcessorIfMissing<TextAsset*>(contentManager, RuntimeContentProcessorIds::TextAsset, new ::AssetContentProcessor_1<::TextAsset*>(), nullptr);
RuntimeContentManagerConfiguration::RegisterProcessorIfMissing<SceneAsset*>(contentManager, RuntimeContentProcessorIds::SceneAsset, new ::BinaryContentProcessor_1<::SceneAsset*>(new Func<Stream*, SceneAsset*>(static_cast<SceneAsset* (*)(Stream*)>(&EditorAssetBinarySerializer::DeserializeSceneAsset))), new Array<std::string>({ SceneAsset::FileExtension }));
RuntimeContentManagerConfiguration::RegisterProcessorIfMissing<AnimationClipAsset*>(contentManager, RuntimeContentProcessorIds::AnimationClipAsset, new ::AssetContentProcessor_1<::AnimationClipAsset*>(), nullptr);
RuntimeContentManagerConfiguration::RegisterProcessorIfMissing<FontAsset*>(contentManager, RuntimeContentProcessorIds::FontAsset, new ::BinaryContentProcessor_1<::FontAsset*>(new Func<Stream*, FontAsset*>(static_cast<FontAsset* (*)(Stream*)>(&FontAssetBinarySerializer::Deserialize))), new Array<std::string>({ FontAssetExtension }));
}

template <typename T>
void RuntimeContentManagerConfiguration::RegisterProcessorIfMissing(::ContentManager* contentManager, std::string processorId, ::IContentProcessor_1<T>* processor, Array<std::string>* extensions)
{
    if (contentManager == nullptr)
    {
throw new ArgumentNullException("contentManager");
    }
    if (String::IsNullOrWhiteSpace(processorId))
    {
throw ([&]() {
auto __ctor_arg_0000012F = "Processor id must be provided.";
auto __ctor_arg_00000130 = "processorId";
return new ArgumentException(__ctor_arg_0000012F, __ctor_arg_00000130);
})();
    }
    if (processor == nullptr)
    {
throw new ArgumentNullException("processor");
    }
    if (contentManager->IsProcessorRegistered(processorId))
    {
return;    }
contentManager->RegisterProcessor<T>(processorId, processor, extensions);
}

