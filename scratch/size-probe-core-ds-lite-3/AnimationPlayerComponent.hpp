#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

class UpdateComponent;
class IUpdateable;
class AnimationClipAsset;
class float3;
class float4;
class Entity;

#include "UpdateComponent.hpp"
#include "runtime/native_disposable.hpp"
#include "IUpdateable.hpp"
#include "float3.hpp"
#include "float4.hpp"

class AnimationPlayerComponent : public ::UpdateComponent
{
public:
    virtual ~AnimationPlayerComponent() = default;

    ::AnimationClipAsset* get_Clip();

    void set_Clip(::AnimationClipAsset* value);

    bool get_PlayAutomatically();

    void set_PlayAutomatically(bool value);

    bool get_ShouldLoop();

    void set_ShouldLoop(bool value);

    ::AnimationClipAsset* get_CurrentClip();

    float get_CurrentTime();

    bool get_IsPlaying();

    bool get_IsPaused();

    float get_FrameDeltaTime();

    void set_FrameDeltaTime(float value);

    void Advance(float deltaTime);

    AnimationPlayerComponent();

    void ComponentAdded(::Entity* entity);

    void ComponentInitialized(::Entity* entity);

    void Pause();

    void Play(::AnimationClipAsset* clip, bool shouldLoop);

    void RebaseCurrentPoseToLocalTransform();

    void Resume();

    void Seek(float time);

    void Stop();

    void Update();

    uint8_t get_UpdateOrder();

    void set_UpdateOrder(uint8_t value);

    ::Entity* get_Parent();

    void set_Parent(::Entity* value);

    bool get_IsEditorUpdateExecutionSuppressionMarker();

    ::Entity* get_ParentUnsafe();

    bool get_IsDisposed();
private:
    ::AnimationClipAsset* clip;

    ::AnimationClipAsset* currentClip;

    float currentTime;

    bool isPlaying;

    bool isPaused;

    bool loop;

    bool playAutomatically;

    bool shouldLoop;

    float frameDeltaTime;

    ::float3 baseLocalPosition;

    ::float3 baseLocalScale;

    ::float4 baseLocalOrientation;

    void ApplyCurrentPose();

    void CompletePlayback();

    float ResolvePlaybackTime(float time);

    void TryPlayConfiguredClip();

    void ValidateClip(::AnimationClipAsset* clip);
};
