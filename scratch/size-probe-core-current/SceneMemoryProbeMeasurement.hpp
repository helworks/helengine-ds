#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "runtime/native_string.hpp"
#include "SceneMemoryProbeActionKind.hpp"

class SceneMemoryProbeMeasurement
{
public:
    virtual ~SceneMemoryProbeMeasurement() = default;

    SceneMemoryProbeMeasurement();

    std::string ProbeName;

    const std::string& get_ProbeName();
    void set_ProbeName(std::string value);

    int32_t CycleIndex;

    int32_t get_CycleIndex();
    void set_CycleIndex(int32_t value);

    int32_t StepIndex;

    int32_t get_StepIndex();
    void set_StepIndex(int32_t value);

    std::string Label;

    const std::string& get_Label();
    void set_Label(std::string value);

    ::SceneMemoryProbeActionKind ActionKind;

    ::SceneMemoryProbeActionKind get_ActionKind();
    void set_ActionKind(::SceneMemoryProbeActionKind value);

    uint64_t ResidentBytes;

    uint64_t get_ResidentBytes();
    void set_ResidentBytes(uint64_t value);

    uint64_t CommittedBytes;

    uint64_t get_CommittedBytes();
    void set_CommittedBytes(uint64_t value);

    std::string LoadedSceneIds;

    const std::string& get_LoadedSceneIds();
    void set_LoadedSceneIds(std::string value);

    int32_t Drawables2DCount;

    int32_t get_Drawables2DCount();
    void set_Drawables2DCount(int32_t value);

    int32_t Drawables3DCount;

    int32_t get_Drawables3DCount();
    void set_Drawables3DCount(int32_t value);

    int32_t DrawCallCount;

    int32_t get_DrawCallCount();
    void set_DrawCallCount(int32_t value);

    int32_t ActiveOwnedTextureCount;

    int32_t get_ActiveOwnedTextureCount();
    void set_ActiveOwnedTextureCount(int32_t value);

    int32_t ActiveOwnedFontCount;

    int32_t get_ActiveOwnedFontCount();
    void set_ActiveOwnedFontCount(int32_t value);

    int32_t ActiveOwnedModelCount;

    int32_t get_ActiveOwnedModelCount();
    void set_ActiveOwnedModelCount(int32_t value);

    int32_t ActiveOwnedMaterialCount;

    int32_t get_ActiveOwnedMaterialCount();
    void set_ActiveOwnedMaterialCount(int32_t value);
};
