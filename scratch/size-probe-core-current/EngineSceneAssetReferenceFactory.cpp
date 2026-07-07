#ifdef DrawText
#undef DrawText
#endif
#include "EngineSceneAssetReferenceFactory.hpp"
#include "SceneAssetReference.hpp"
#include "ModelUtils.hpp"
#include "EngineSceneAssetReferenceFactory.hpp"
#include "SceneAssetReferenceSourceKind.hpp"
#include "runtime/native_string.hpp"
#include "ModelAsset.hpp"
#include "float3.hpp"

::SceneAssetReference* EngineSceneAssetReferenceFactory::CreateCubeModel()
{
return EngineSceneAssetReferenceFactory::CreateGenerated(CubeRelativePath, ModelUtils::GeneratedCubeModelId);}

::SceneAssetReference* EngineSceneAssetReferenceFactory::CreatePlaneModel()
{
return EngineSceneAssetReferenceFactory::CreateGenerated(PlaneRelativePath, ModelUtils::GeneratedPlaneModelId);}

::SceneAssetReference* EngineSceneAssetReferenceFactory::CreateSphereModel()
{
return EngineSceneAssetReferenceFactory::CreateGenerated(SphereRelativePath, ModelUtils::GeneratedSphereModelId);}

::SceneAssetReference* EngineSceneAssetReferenceFactory::CreateStandardMaterial()
{
return EngineSceneAssetReferenceFactory::CreateGenerated(StandardMaterialRelativePath, StandardMaterialAssetId);}

::SceneAssetReference* EngineSceneAssetReferenceFactory::CreateGenerated(std::string relativePath, std::string assetId)
{
return new ::SceneAssetReference(static_cast<SceneAssetReferenceSourceKind>(SceneAssetReferenceSourceKind::Generated), relativePath, ProviderIdValue, assetId);}

