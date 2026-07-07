#pragma once
#ifdef DrawText
#undef DrawText
#endif
#include <cstdint>

#include "SceneMemoryProbeActionKind.hpp"
#include "runtime/native_string.hpp"

class SceneMemoryProbeStep
{
public:
    virtual ~SceneMemoryProbeStep() = default;

    SceneMemoryProbeStep();

    ::SceneMemoryProbeActionKind ActionKind;

    ::SceneMemoryProbeActionKind get_ActionKind();
    void set_ActionKind(::SceneMemoryProbeActionKind value);

    std::string SceneId;

    const std::string& get_SceneId();
    void set_SceneId(std::string value);

    double DurationSeconds;

    double get_DurationSeconds();
    void set_DurationSeconds(double value);

    std::string Label;

    const std::string& get_Label();
    void set_Label(std::string value);
};
