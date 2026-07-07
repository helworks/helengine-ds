#ifdef DrawText
#undef DrawText
#endif
#include "AutomaticComponentAssetReferenceSupport.hpp"
#include "runtime/native_type.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_exceptions.hpp"
#include "AutomaticComponentAssetReferenceSupport.hpp"
#include "FontAsset.hpp"
#include "RuntimeTexture.hpp"
#include "RuntimeModel.hpp"
#include "RuntimeMaterial.hpp"
#include "AnimationClipAsset.hpp"
#include "SceneAssetReference.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "FontInfo.hpp"
#include "runtime/native_dictionary.hpp"
#include "FontChar.hpp"
#include "TextureAsset.hpp"
#include "float2.hpp"
#include "FontTightMetrics.hpp"
#include "runtime/array.hpp"
#include "RuntimeSubmesh.hpp"
#include "float3.hpp"
#include "MaterialRenderState.hpp"
#include "RuntimeMaterialLightingModel.hpp"
#include "runtime/native_list.hpp"
#include "PositionKeyframeTrackAsset.hpp"
#include "PositionOffsetKeyframeTrackAsset.hpp"
#include "ScaleKeyframeTrackAsset.hpp"
#include "RotationKeyframeTrackAsset.hpp"
#include "AnimationClipPlatformOverrideAsset.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "runtime/native_type.hpp"

std::string AutomaticComponentAssetReferenceSupport::BuildIndexedReferenceName(std::string memberName, int32_t index)
{
    if (String::IsNullOrWhiteSpace(memberName))
    {
throw ([&]() {
auto __ctor_arg_0000001D = "Member name must be provided.";
auto __ctor_arg_0000001E = "memberName";
return new ArgumentException(__ctor_arg_0000001D, __ctor_arg_0000001E);
})();
    }
    if (index < 0)
    {
throw ([&]() {
auto __ctor_arg_0000001F = "index";
auto __ctor_arg_00000020 = "Indexed asset-reference keys require a non-negative array index.";
return new ArgumentOutOfRangeException(__ctor_arg_0000001F, __ctor_arg_00000020);
})();
    }
return String::Concat(memberName, "[", std::to_string(index), "]");}

std::string AutomaticComponentAssetReferenceSupport::BuildReferenceName(std::string memberName)
{
    if (String::IsNullOrWhiteSpace(memberName))
    {
throw ([&]() {
auto __ctor_arg_00000021 = "Member name must be provided.";
auto __ctor_arg_00000022 = "memberName";
return new ArgumentException(__ctor_arg_00000021, __ctor_arg_00000022);
})();
    }
return memberName;}

bool AutomaticComponentAssetReferenceSupport::IsSupportedAssetReferenceArrayType(Type* valueType)
{
    if (valueType == nullptr || !valueType->get_IsArray() || valueType->GetArrayRank() != 1)
    {
return false;    }
Type *elementType = valueType->GetElementType();
return AutomaticComponentAssetReferenceSupport::IsSupportedAssetReferenceType(elementType);}

bool AutomaticComponentAssetReferenceSupport::IsSupportedAssetReferenceType(Type* valueType)
{
    if (valueType == nullptr)
    {
return false;    }
return valueType == he_cpp_type_of<FontAsset>("FontAsset") || valueType == he_cpp_type_of<RuntimeTexture>("RuntimeTexture") || valueType == he_cpp_type_of<RuntimeModel>("RuntimeModel") || valueType == he_cpp_type_of<RuntimeMaterial>("RuntimeMaterial") || valueType == he_cpp_type_of<AnimationClipAsset>("AnimationClipAsset");}

void* AutomaticComponentAssetReferenceSupport::ResolveRuntimeAssetReference(Type* valueType, ::SceneAssetReference* reference, ::RuntimeSceneAssetReferenceResolver* referenceResolver)
{
    if (valueType == nullptr)
    {
throw new ArgumentNullException("valueType");
    }
    if (!AutomaticComponentAssetReferenceSupport::IsSupportedAssetReferenceType(valueType))
    {
throw new InvalidOperationException(UnsupportedAssetReferenceTypeMessage);
    }
    if (reference == nullptr)
    {
return nullptr;    }
    if (referenceResolver == nullptr)
    {
throw new ArgumentNullException("referenceResolver");
    }
    if (valueType == he_cpp_type_of<FontAsset>("FontAsset"))
    {
return referenceResolver->ResolveFont(reference);    }
    if (valueType == he_cpp_type_of<RuntimeTexture>("RuntimeTexture"))
    {
return referenceResolver->ResolveTexture(reference);    }
    if (valueType == he_cpp_type_of<RuntimeModel>("RuntimeModel"))
    {
return referenceResolver->ResolveModel(reference);    }
    if (valueType == he_cpp_type_of<RuntimeMaterial>("RuntimeMaterial"))
    {
return referenceResolver->ResolveMaterial(reference);    }
    if (valueType == he_cpp_type_of<AnimationClipAsset>("AnimationClipAsset"))
    {
return referenceResolver->ResolveAnimationClip(reference);    }
throw new InvalidOperationException(UnsupportedAssetReferenceTypeMessage);
}

