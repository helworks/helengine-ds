#ifdef DrawText
#undef DrawText
#endif
#include "AnimationPlayerComponent.hpp"
#include "UpdateComponent.hpp"
#include "runtime/native_exceptions.hpp"
#include "float3.hpp"
#include "Entity.hpp"
#include "Core.hpp"
#include "AnimationClipAsset.hpp"
#include "float4.hpp"
#include "AnimationClipEvaluator.hpp"
#include "float2.hpp"
#include "runtime/native_string.hpp"
#include "float4x4.hpp"
#include "runtime/native_list.hpp"
#include "Component.hpp"
#include "CoreInitializationOptions.hpp"
#include "ContentManager.hpp"
#include "ObjectManager.hpp"
#include "IEntityFactory.hpp"
#include "RenderManager3D.hpp"
#include "FontAsset.hpp"
#include "int2.hpp"
#include "RenderManager2D.hpp"
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
#include "runtime/array.hpp"
#include "PositionKeyframeTrackAsset.hpp"
#include "PositionOffsetKeyframeTrackAsset.hpp"
#include "ScaleKeyframeTrackAsset.hpp"
#include "RotationKeyframeTrackAsset.hpp"
#include "AnimationClipPlatformOverrideAsset.hpp"
#include "PositionKeyframeAsset.hpp"
#include "RotationKeyframeAsset.hpp"
#include "AnimationPlayerComponent.hpp"
#include "system/math.hpp"
#include "runtime/array.hpp"
#include "runtime/native_exceptions.hpp"
#include "system/diagnostics/stopwatch.hpp"

::AnimationClipAsset* AnimationPlayerComponent::get_Clip()
{
return this->clip;}

void AnimationPlayerComponent::set_Clip(::AnimationClipAsset* value)
{
this->clip = value;
}

bool AnimationPlayerComponent::get_PlayAutomatically()
{
return this->playAutomatically;}

void AnimationPlayerComponent::set_PlayAutomatically(bool value)
{
this->playAutomatically = value;
}

bool AnimationPlayerComponent::get_ShouldLoop()
{
return this->shouldLoop;}

void AnimationPlayerComponent::set_ShouldLoop(bool value)
{
this->shouldLoop = value;
}

::AnimationClipAsset* AnimationPlayerComponent::get_CurrentClip()
{
return this->currentClip;}

float AnimationPlayerComponent::get_CurrentTime()
{
return this->currentTime;}

bool AnimationPlayerComponent::get_IsPlaying()
{
return this->isPlaying;}

bool AnimationPlayerComponent::get_IsPaused()
{
return this->isPaused;}

float AnimationPlayerComponent::get_FrameDeltaTime()
{
return this->frameDeltaTime;}

void AnimationPlayerComponent::set_FrameDeltaTime(float value)
{
this->frameDeltaTime = value;
}

void AnimationPlayerComponent::Advance(float deltaTime)
{
    if (!this->isPlaying || this->isPaused || this->currentClip == nullptr)
    {
return;    }
const float nextTime = this->currentTime + deltaTime;
    if (!this->loop && nextTime >= this->currentClip->Duration)
    {
this->currentTime = this->currentClip->Duration;
this->ApplyCurrentPose();
this->CompletePlayback();
return;    }
this->currentTime = this->ResolvePlaybackTime(nextTime);
this->ApplyCurrentPose();
}

AnimationPlayerComponent::AnimationPlayerComponent() : clip(), currentClip(), currentTime(), isPlaying(), isPaused(), loop(), playAutomatically(), shouldLoop(), frameDeltaTime(), baseLocalPosition(), baseLocalScale(), baseLocalOrientation()
{
this->frameDeltaTime = 1.0f / 60.0f;
}

void AnimationPlayerComponent::ComponentAdded(::Entity* entity)
{
UpdateComponent::ComponentAdded(entity);
this->TryPlayConfiguredClip();
}

void AnimationPlayerComponent::ComponentInitialized(::Entity* entity)
{
UpdateComponent::ComponentInitialized(entity);
this->TryPlayConfiguredClip();
}

void AnimationPlayerComponent::Pause()
{
    if (this->currentClip == nullptr)
    {
throw new InvalidOperationException("Cannot pause animation playback when no clip is active.");
    }
this->isPaused = true;
this->isPlaying = false;
}

void AnimationPlayerComponent::Play(::AnimationClipAsset* clip, bool shouldLoop)
{
    if (clip == nullptr)
    {
throw new ArgumentNullException("clip");
    }
else {
    if (this->Parent == nullptr)
    {
throw new InvalidOperationException("AnimationPlayerComponent must be added to an entity before playback can begin.");
    }
}
this->ValidateClip(clip);
this->currentClip = clip;
this->currentTime = 0.0f;
this->loop = shouldLoop;
this->isPlaying = true;
this->isPaused = false;
this->baseLocalPosition = this->Parent->get_LocalPosition();
this->baseLocalScale = this->Parent->get_LocalScale();
this->baseLocalOrientation = this->Parent->get_LocalOrientation();
this->ApplyCurrentPose();
    if (this->currentClip->Duration <= 0.0f)
    {
this->CompletePlayback();
    }
}

void AnimationPlayerComponent::RebaseCurrentPoseToLocalTransform()
{
    if (this->Parent == nullptr || this->currentClip == nullptr)
    {
return;    }
    if (this->currentClip->PositionTracks->get_Length() == 0)
    {
::float3 rebasedPosition = this->Parent->get_LocalPosition();
    if (this->currentClip->PositionOffsetTracks->get_Length() == 1)
    {
rebasedPosition = rebasedPosition - AnimationClipEvaluator::EvaluatePositionTrack((*this->currentClip->PositionOffsetTracks)[0], this->currentTime);
    }
this->baseLocalPosition = rebasedPosition;
    }
    if (this->currentClip->ScaleTracks->get_Length() == 0)
    {
this->baseLocalScale = this->Parent->get_LocalScale();
    }
    if (this->currentClip->RotationTracks->get_Length() == 0)
    {
this->baseLocalOrientation = this->Parent->get_LocalOrientation();
    }
}

void AnimationPlayerComponent::Resume()
{
    if (this->currentClip == nullptr)
    {
throw new InvalidOperationException("Cannot resume animation playback when no clip is active.");
    }
this->isPaused = false;
this->isPlaying = true;
}

void AnimationPlayerComponent::Seek(float time)
{
    if (this->currentClip == nullptr)
    {
throw new InvalidOperationException("Cannot seek animation playback when no clip is active.");
    }
this->currentTime = this->ResolvePlaybackTime(time);
this->ApplyCurrentPose();
}

void AnimationPlayerComponent::Stop()
{
    if (this->Parent != nullptr)
    {
this->Parent->set_LocalPosition(this->baseLocalPosition);
this->Parent->set_LocalScale(this->baseLocalScale);
this->Parent->set_LocalOrientation(this->baseLocalOrientation);
    }
this->currentClip = nullptr;
this->currentTime = 0.0f;
this->isPlaying = false;
this->isPaused = false;
this->loop = false;
}

void AnimationPlayerComponent::Update()
{
UpdateComponent::Update();
::Core *core = Core::Instance;
    if (core != nullptr)
    {
this->Advance(core->DeltaTime);
return;    }
this->Advance(this->frameDeltaTime);
}

uint8_t AnimationPlayerComponent::get_UpdateOrder()
{
return UpdateComponent::get_UpdateOrder();
}

void AnimationPlayerComponent::set_UpdateOrder(uint8_t value)
{
UpdateComponent::set_UpdateOrder(value);
}

::Entity* AnimationPlayerComponent::get_Parent()
{
return Component::get_Parent();
}

void AnimationPlayerComponent::set_Parent(::Entity* value)
{
Component::set_Parent(value);
}

bool AnimationPlayerComponent::get_IsEditorUpdateExecutionSuppressionMarker()
{
return Component::get_IsEditorUpdateExecutionSuppressionMarker();
}

::Entity* AnimationPlayerComponent::get_ParentUnsafe()
{
return Component::get_ParentUnsafe();
}

bool AnimationPlayerComponent::get_IsDisposed()
{
return Component::get_IsDisposed();
}

void AnimationPlayerComponent::ApplyCurrentPose()
{
    if (this->Parent == nullptr || this->currentClip == nullptr)
    {
return;    }
::float3 resolvedPosition = this->baseLocalPosition;
    if (this->currentClip->PositionTracks->get_Length() == 1)
    {
resolvedPosition = AnimationClipEvaluator::EvaluatePositionTrack((*this->currentClip->PositionTracks)[0], this->currentTime);
    }
    if (this->currentClip->PositionOffsetTracks->get_Length() == 1)
    {
resolvedPosition = resolvedPosition + AnimationClipEvaluator::EvaluatePositionTrack((*this->currentClip->PositionOffsetTracks)[0], this->currentTime);
    }
::float3 resolvedScale = this->baseLocalScale;
    if (this->currentClip->ScaleTracks->get_Length() == 1)
    {
resolvedScale = AnimationClipEvaluator::EvaluatePositionTrack((*this->currentClip->ScaleTracks)[0], this->currentTime);
    }
::float4 resolvedOrientation = this->baseLocalOrientation;
    if (this->currentClip->RotationTracks->get_Length() == 1)
    {
resolvedOrientation = AnimationClipEvaluator::EvaluateRotationTrack((*this->currentClip->RotationTracks)[0], this->currentTime);
    }
this->Parent->set_LocalPosition(resolvedPosition);
this->Parent->set_LocalScale(resolvedScale);
this->Parent->set_LocalOrientation(resolvedOrientation);
}

void AnimationPlayerComponent::CompletePlayback()
{
this->isPlaying = false;
this->isPaused = false;
}

float AnimationPlayerComponent::ResolvePlaybackTime(float time)
{
    if (this->currentClip == nullptr || this->currentClip->Duration <= 0.0f)
    {
return 0.0f;    }
else {
    if (this->loop)
    {
const double duration = this->currentClip->Duration;
double wrapped = std::fmod(time, duration);
    if (wrapped < 0.0)
    {
wrapped += duration;
    }
return static_cast<float>(wrapped);    }
else {
    if (time <= 0.0f)
    {
return 0.0f;    }
else {
    if (time >= this->currentClip->Duration)
    {
return this->currentClip->Duration;    }
}
}
}
return time;}

void AnimationPlayerComponent::TryPlayConfiguredClip()
{
    if (!this->playAutomatically)
    {
return;    }
else {
    if (this->clip == nullptr)
    {
throw new InvalidOperationException("AnimationPlayerComponent requires one authored Clip asset before automatic playback can begin.");
    }
}
this->Play(this->clip, this->shouldLoop);
}

void AnimationPlayerComponent::ValidateClip(::AnimationClipAsset* clip)
{
    if (clip->Duration < 0.0f)
    {
throw new InvalidOperationException("Animation clips cannot declare a negative duration.");
    }
else {
    if (clip->PositionTracks->get_Length() > 1 || clip->PositionOffsetTracks->get_Length() > 1 || clip->ScaleTracks->get_Length() > 1 || clip->RotationTracks->get_Length() > 1)
    {
throw new InvalidOperationException("Animation clips can currently bind only one track per transform channel.");
    }
}
}

