#ifdef DrawText
#undef DrawText
#endif
#include "SceneAssetReferenceFactory.hpp"
#include "SceneAssetReference.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_string.hpp"
#include "SceneAssetReferenceSourceKind.hpp"
#include "EngineBinaryReader.hpp"
#include "SceneAssetReferenceFactory.hpp"
#include "EngineBinaryEndianness.hpp"
#include "system/io/stream.hpp"
#include "runtime/array.hpp"
#include "system/func.hpp"
#include "float2.hpp"
#include "float3.hpp"
#include "float4.hpp"
#include "int2.hpp"
#include "int4.hpp"
#include "SceneEntityReference.hpp"
#include "runtime/native_span.hpp"
#include "EngineBinaryReadContext.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "runtime/native_span.hpp"
#include "runtime/native_string.hpp"
#include "system/io/stream.hpp"

::SceneAssetReference* SceneAssetReferenceFactory::CreateFileSystemAnimationClip(std::string relativePath)
{
return SceneAssetReferenceFactory::CreateFileSystem(relativePath);}

::SceneAssetReference* SceneAssetReferenceFactory::CreateFileSystemFont(std::string relativePath)
{
return SceneAssetReferenceFactory::CreateFileSystem(relativePath);}

::SceneAssetReference* SceneAssetReferenceFactory::CreateFileSystemMaterial(std::string relativePath)
{
return SceneAssetReferenceFactory::CreateFileSystem(relativePath);}

::SceneAssetReference* SceneAssetReferenceFactory::CreateFileSystemModel(std::string relativePath)
{
return SceneAssetReferenceFactory::CreateFileSystem(relativePath);}

::SceneAssetReference* SceneAssetReferenceFactory::CreateFileSystemTexture(std::string relativePath)
{
return SceneAssetReferenceFactory::CreateFileSystem(relativePath);}

::SceneAssetReference* SceneAssetReferenceFactory::ReadOptionalReference(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
    if (reader->ReadByte() == 0)
    {
return nullptr;    }
return SceneAssetReferenceFactory::ReadRequiredReference(reader);}

::SceneAssetReference* SceneAssetReferenceFactory::ReadRequiredReference(::EngineBinaryReader* reader)
{
    if (reader == nullptr)
    {
throw new ArgumentNullException("reader");
    }
EngineBinaryReadContext::set_CurrentReadStage("SceneAssetReference:SourceKind");
::SceneAssetReferenceSourceKind sourceKind = static_cast<SceneAssetReferenceSourceKind>(reader->ReadInt32());
EngineBinaryReadContext::set_CurrentReadStage("SceneAssetReference:RelativePath");
const std::string relativePath = reader->ReadString();
EngineBinaryReadContext::set_CurrentReadStage("SceneAssetReference:ProviderId");
const std::string providerId = reader->ReadString();
EngineBinaryReadContext::set_CurrentReadStage("SceneAssetReference:AssetId");
const std::string assetId = reader->ReadString();
EngineBinaryReadContext::set_LastCheckpoint(std::string("SceneAssetReferenceEnd:") + relativePath + std::string("@") + std::to_string(reader->GetStreamPosition()));
return SceneAssetReferenceFactory::Rehydrate(static_cast<SceneAssetReferenceSourceKind>(sourceKind), relativePath, providerId, assetId);}

::SceneAssetReference* SceneAssetReferenceFactory::Rehydrate(::SceneAssetReferenceSourceKind sourceKind, std::string relativePath, std::string providerId, std::string assetId)
{
    if (sourceKind == SceneAssetReferenceSourceKind::FileSystem)
    {
return SceneAssetReferenceFactory::CreateFileSystem(relativePath);    }
    if (String::IsNullOrWhiteSpace(relativePath))
    {
throw new InvalidOperationException("Generated scene asset references must include a relative path.");
    }
    if (String::IsNullOrWhiteSpace(providerId))
    {
throw new InvalidOperationException("Generated scene asset references must include a provider id.");
    }
    if (String::IsNullOrWhiteSpace(assetId))
    {
throw new InvalidOperationException("Generated scene asset references must include an asset id.");
    }
return new ::SceneAssetReference(static_cast<SceneAssetReferenceSourceKind>(SceneAssetReferenceSourceKind::Generated), relativePath, providerId, assetId);}

::SceneAssetReference* SceneAssetReferenceFactory::CreateFileSystem(std::string relativePath)
{
    if (String::IsNullOrWhiteSpace(relativePath))
    {
throw ([&]() {
auto __ctor_arg_00000152 = "File-backed asset references must include a relative path.";
auto __ctor_arg_00000153 = "relativePath";
return new ArgumentException(__ctor_arg_00000152, __ctor_arg_00000153);
})();
    }
return new ::SceneAssetReference(static_cast<SceneAssetReferenceSourceKind>(SceneAssetReferenceSourceKind::FileSystem), relativePath, String::Empty, String::Empty);}

