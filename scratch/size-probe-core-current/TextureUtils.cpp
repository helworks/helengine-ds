#ifdef DrawText
#undef DrawText
#endif
#include "TextureUtils.hpp"
#include "TextureAsset.hpp"
#include "RuntimeTexture.hpp"
#include "Core.hpp"
#include "RenderManager2D.hpp"
#include "TextureUtils.hpp"
#include "runtime/array.hpp"
#include "TextureAssetColorFormat.hpp"
#include "TextureAssetAlphaPrecision.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "ObjectManager.hpp"
#include "IEntityFactory.hpp"
#include "RenderManager3D.hpp"
#include "runtime/native_string.hpp"
#include "FontAsset.hpp"
#include "int2.hpp"
#include "InputSystem.hpp"
#include "StandardPlatformInput.hpp"
#include "PointerInteractionSystem.hpp"
#include "PlatformInfo.hpp"
#include "PhysicsFixedStepScheduler.hpp"
#include "IPhysicsRuntime.hpp"
#include "RuntimeSceneAssetReferenceResolver.hpp"
#include "RuntimeSceneLoadService.hpp"
#include "SceneManager.hpp"
#include "RuntimeDiagnosticsService.hpp"
#include "RuntimeComponentRegistry.hpp"
#include "ITextClipboardService.hpp"
#include "TextBoxShortcutRegistry.hpp"
#include "runtime/native_dictionary.hpp"
#include "system/diagnostics/stopwatch.hpp"
#include "IRuntimeUpdateStageDiagnosticsProvider.hpp"
#include "IInputBackend.hpp"
#include "IRuntimeComponentDeserializer.hpp"
#include "RuntimeSceneCatalog.hpp"
#include "IRoundedRectDrawable2D.hpp"
#include "ISpriteDrawable2D.hpp"
#include "ITextDrawable2D.hpp"
#include "runtime/array.hpp"
#include "system/diagnostics/stopwatch.hpp"

::RuntimeTexture* TextureUtils::get_PixelTexture()
{
    if (pixelTexture == nullptr)
    {
pixelTexture = TextureUtils::BuildSolidPixelTexture(static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(255));
    }
return pixelTexture;}

::RuntimeTexture* TextureUtils::get_BlackPixelTexture()
{
    if (blackPixelTexture == nullptr)
    {
blackPixelTexture = TextureUtils::BuildSolidPixelTexture(static_cast<uint8_t>(0), static_cast<uint8_t>(0), static_cast<uint8_t>(0), static_cast<uint8_t>(255));
    }
return blackPixelTexture;}

::RuntimeTexture* TextureUtils::pixelTexture;

::RuntimeTexture* TextureUtils::blackPixelTexture;

::RuntimeTexture* TextureUtils::BuildSolidPixelTexture(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
::TextureAsset *rawTex = new ::TextureAsset();
rawTex->Colors = new Array<uint8_t>({ red, green, blue, alpha });
rawTex->Width = 1;
rawTex->Height = 1;
rawTex->IsEngineOwned = true;
::RuntimeTexture *runtimeTexture = Core::Instance->RenderManager2D->BuildTextureFromRaw(rawTex);
runtimeTexture->set_IsEngineOwned(true);
return runtimeTexture;}

